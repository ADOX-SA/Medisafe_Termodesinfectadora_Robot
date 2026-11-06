# Medisafe Termodesinfectadora Robot — paquete de carga

Archivos para la actualización por la Node OTA:

- `node_ota.bin`: firmware de la Node ESP8266.
- `medisafe_c8t6.bin`: STM32F103C8T6, versión 1.32 OTA.
- `medisafe_cbt6.bin`: STM32F103CBT6, versión 1.32 OTA.
- `medisafe_nextion_v1.31.hmi`: proyecto fuente de la pantalla Nextion.

Para actualizar la pantalla por Wi-Fi se debe compilar el HMI con Nextion Editor y copiar el `.tft` resultante en esta misma carpeta. La Node transfiere el `.tft`, no el `.hmi`.

La conexión OTA es igual a la lavadora de lúmenes: Node a USART1 del STM32 (PA9/PA10, 115200), y la pantalla permanece en USART3.
