#include "nextion_ota.h"
#include "nextion.h"
#include "relay_module.h"
#include "user_gpio.h"
#include "watchdog.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define NODE_RX_FIFO_SIZE    2048u
#define FRAME_MAX_DATA       1024u
#define NODE_APP_BAUD        115200u
#define NEXTION_NORMAL_BAUD  9600u
#define NEXTION_UPLOAD_BAUD  115200u

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

static volatile uint8_t node_fifo[NODE_RX_FIFO_SIZE];
static volatile uint16_t node_write;
static volatile uint16_t node_read;
static uint8_t node_rx_byte;
static char command[48];
static uint8_t command_length;
static bool receiving_frames;
static uint8_t frame[6u + FRAME_MAX_DATA + 2u];
static uint16_t frame_length;
static uint16_t frame_expected;
static uint32_t expected_file_size;
static uint32_t received_file_size;
static uint16_t nextion_bytes_since_ack;
static volatile bool ping_reply_pending;
static uint8_t ping_match;
static bool skip_ping_line;
static bool ota_active;

static void ota_watchdog_refresh(void)
{
    watchdog_refresh();
}

static void ota_finish(void)
{
    receiving_frames = false;
    ota_active = false;
    (void)HAL_UART_AbortReceive(&huart3);
    huart3.Init.BaudRate = NEXTION_NORMAL_BAUD;
    (void)HAL_UART_Init(&huart3);
    Nextion_ReceiveData();
}

static void ota_safe_stop(void)
{
    /* Las salidas físicas locales y las 16 salidas Modbus quedan desactivadas
       antes de bloquear el ciclo normal durante la transferencia. */
    gpio_RelesOff();
    memset(rm_output1, 0, sizeof(rm_output1));
    RelayModule_WriteAllOutputs(1);
}

static uint16_t crc16_update(uint16_t crc, uint8_t data)
{
    crc ^= data;
    for (uint8_t bit = 0u; bit < 8u; ++bit)
        crc = (crc & 1u) ? (uint16_t)((crc >> 1u) ^ 0xA001u) : (uint16_t)(crc >> 1u);
    return crc;
}

static uint16_t crc16(const uint8_t *data, uint16_t length)
{
    uint16_t result = 0xFFFFu;
    while (length--) result = crc16_update(result, *data++);
    return result;
}

static void node_reply(uint8_t response, uint16_t sequence)
{
    const uint8_t data[3] = {response, (uint8_t)sequence, (uint8_t)(sequence >> 8u)};
    (void)HAL_UART_Transmit(&huart1, (uint8_t *)data, sizeof(data), 100u);
}

static bool node_ping_raw_reply(void)
{
    static const uint8_t pong[] = "NXPONG\n";
    const uint32_t until = HAL_GetTick() + 100u;
    (void)HAL_UART_AbortReceive(&huart1);
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    GPIOA->CRH = (GPIOA->CRH & ~((0xFu << 4u) | (0xFu << 8u))) |
                  (0xBu << 4u) | (0x4u << 8u);
    USART1->CR1 = 0u;
    USART1->CR2 = 0u;
    USART1->CR3 = 0u;
    USART1->BRR = HAL_RCC_GetPCLK2Freq() / NODE_APP_BAUD;
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
    for (uint32_t i = 0u; i < sizeof(pong); ++i) {
        while ((USART1->SR & USART_SR_TXE) == 0u)
            if ((int32_t)(HAL_GetTick() - until) >= 0) return false;
        USART1->DR = pong[i];
    }
    while ((USART1->SR & USART_SR_TC) == 0u)
        if ((int32_t)(HAL_GetTick() - until) >= 0) return false;
    if (HAL_UART_Init(&huart1) != HAL_OK) return false;
    return HAL_UART_Receive_IT(&huart1, &node_rx_byte, 1u) == HAL_OK;
}

static bool nextion_wait_ready(uint32_t timeout_ms)
{
    const uint32_t until = HAL_GetTick() + timeout_ms;
    uint8_t value;
    while ((int32_t)(HAL_GetTick() - until) < 0) {
        if (HAL_UART_Receive(&huart3, &value, 1u, 25u) == HAL_OK && value == 0x05u) return true;
        ota_watchdog_refresh();
    }
    return false;
}

static bool nextion_begin(uint32_t size)
{
    static const uint32_t normal_bauds[] = {9600u, 115200u, 57600u, 38400u, 19200u};
    char text[40];
    const uint8_t end[3] = {0xFFu, 0xFFu, 0xFFu};
    const int count = snprintf(text, sizeof(text), "whmi-wri %lu,%lu,0", (unsigned long)size,
                               (unsigned long)NEXTION_UPLOAD_BAUD);
    if (count <= 0 || count >= (int)sizeof(text)) return false;
    (void)HAL_UART_AbortReceive(&huart3);
    HAL_Delay(1200u);
    for (uint32_t i = 0u; i < (sizeof(normal_bauds) / sizeof(normal_bauds[0])); ++i) {
        huart3.Init.BaudRate = normal_bauds[i];
        if (HAL_UART_Init(&huart3) != HAL_OK) continue;
        static const uint8_t sync_cmd[] = {'0', '0', 0xFFu, 0xFFu, 0xFFu};
        (void)HAL_UART_Transmit(&huart3, (uint8_t *)sync_cmd, sizeof(sync_cmd), 1000u);
        HAL_Delay(100u);
        uint8_t discarded;
        while (HAL_UART_Receive(&huart3, &discarded, 1u, 10u) == HAL_OK) { }
        (void)HAL_UART_Transmit(&huart3, (uint8_t *)text, (uint16_t)count, 1000u);
        (void)HAL_UART_Transmit(&huart3, (uint8_t *)end, sizeof(end), 1000u);
        huart3.Init.BaudRate = NEXTION_UPLOAD_BAUD;
        if (HAL_UART_Init(&huart3) != HAL_OK) continue;
        HAL_Delay(100u);
        if (!nextion_wait_ready(2500u)) continue;
        HAL_Delay(1000u);
        nextion_bytes_since_ack = 0u;
        return true;
    }
    return false;
}

static bool nextion_write(const uint8_t *data, uint16_t length)
{
    while (length > 0u) {
        uint16_t segment = (uint16_t)(4096u - nextion_bytes_since_ack);
        if (segment > length) segment = length;
        if (HAL_UART_Transmit(&huart3, (uint8_t *)data, segment, 1000u) != HAL_OK) return false;
        ota_watchdog_refresh();
        data += segment;
        length = (uint16_t)(length - segment);
        nextion_bytes_since_ack = (uint16_t)(nextion_bytes_since_ack + segment);
        if (nextion_bytes_since_ack == 4096u) {
            if (!nextion_wait_ready(10000u)) return false;
            nextion_bytes_since_ack = 0u;
        }
    }
    return true;
}

static void start_command(void)
{
    if (strcmp(command, "NXPING") == 0) { ping_reply_pending = true; return; }
    unsigned long size = 0u;
    if (sscanf(command, "NXBEGIN %lu", &size) != 1 || size == 0u) {
        (void)HAL_UART_Transmit(&huart1, (uint8_t *)"NXERROR\n", 8u, 100u);
        return;
    }
    expected_file_size = (uint32_t)size;
    received_file_size = 0u;
    frame_length = 0u;
    frame_expected = 0u;
    nextion_bytes_since_ack = 0u;
    ota_safe_stop();
    ota_active = true;
    receiving_frames = nextion_begin(expected_file_size);
    if (receiving_frames)
        (void)HAL_UART_Transmit(&huart1, (uint8_t *)"NXREADY\n", 8u, 100u);
    else {
        (void)HAL_UART_Transmit(&huart1, (uint8_t *)"NXERRDISPLAY\n", 13u, 100u);
        ota_finish();
    }
}

static void process_frame(void)
{
    const uint16_t sequence = (uint16_t)frame[2] | ((uint16_t)frame[3] << 8u);
    const uint16_t size = (uint16_t)frame[4] | ((uint16_t)frame[5] << 8u);
    const uint16_t received_crc = (uint16_t)frame[6u + size] | ((uint16_t)frame[7u + size] << 8u);
    if (size > FRAME_MAX_DATA || frame_length != (uint16_t)(8u + size) ||
        crc16(&frame[2], (uint16_t)(4u + size)) != received_crc) { node_reply(0x16u, sequence); return; }
    if (sequence == 0xFFFFu) {
        node_reply(received_file_size == expected_file_size ? 0xACu : 0x15u, sequence);
        ota_finish();
        return;
    }
    if ((uint32_t)size > expected_file_size - received_file_size || !nextion_write(&frame[6], size)) {
        node_reply(0x17u, sequence);
        ota_finish();
        return;
    }
    received_file_size += size;
    node_reply(0xACu, sequence);
}

void nextion_ota_init(void)
{
    node_write = node_read = 0u;
    command_length = 0u;
    receiving_frames = false;
    ping_reply_pending = false;
    ping_match = 0u;
    skip_ping_line = false;
    ota_active = false;
    frame_length = frame_expected = 0u;
    nextion_bytes_since_ack = 0u;
    (void)HAL_UART_Receive_IT(&huart1, &node_rx_byte, 1u);
}

void nextion_ota_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart != &huart1) return;
    const uint16_t next = (uint16_t)((node_write + 1u) % NODE_RX_FIFO_SIZE);
    if (next != node_read) { node_fifo[node_write] = node_rx_byte; node_write = next; }
    (void)HAL_UART_Receive_IT(&huart1, &node_rx_byte, 1u);
}

void nextion_ota_restart_rx(void)
{
    (void)HAL_UART_Receive_IT(&huart1, &node_rx_byte, 1u);
}

void nextion_ota_runcycle(void)
{
    if (ping_reply_pending) {
        ping_reply_pending = false;
        HAL_Delay(100u);
        (void)node_ping_raw_reply();
    }
    while (node_read != node_write) {
        const uint8_t value = node_fifo[node_read];
        node_read = (uint16_t)((node_read + 1u) % NODE_RX_FIFO_SIZE);
        if (!receiving_frames) {
            static const char ping_text[] = "NXPING";
            if (skip_ping_line) { if (value == '\n') skip_ping_line = false; continue; }
            if (value == (uint8_t)ping_text[ping_match]) {
                if (++ping_match == (sizeof(ping_text) - 1u)) {
                    ping_reply_pending = true; ping_match = 0u; command_length = 0u; skip_ping_line = true; continue;
                }
            } else ping_match = (value == 'N') ? 1u : 0u;
            if (value == 'N' && (command_length == 0u || command[0] != 'N')) command_length = 0u;
            if (value == '\n') { command[command_length] = '\0'; start_command(); command_length = 0u; }
            else if (command_length < sizeof(command) - 1u) command[command_length++] = (char)value;
            else command_length = 0u;
            continue;
        }
        if (frame_length == 0u && value != 0xA5u) continue;
        if (frame_length == 1u && value != 0x5Au) { frame_length = 0u; continue; }
        frame[frame_length++] = value;
        if (frame_length == 6u) {
            const uint16_t size = (uint16_t)frame[4] | ((uint16_t)frame[5] << 8u);
            frame_expected = size <= FRAME_MAX_DATA ? (uint16_t)(8u + size) : 0u;
            if (frame_expected == 0u) frame_length = 0u;
        }
        if (frame_expected != 0u && frame_length == frame_expected) { process_frame(); frame_length = frame_expected = 0u; }
    }
}

bool nextion_ota_is_active(void)
{
    return ota_active;
}
