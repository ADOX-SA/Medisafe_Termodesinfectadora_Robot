from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfbase import pdfmetrics
from reportlab.platypus import (
    BaseDocTemplate,
    Frame,
    KeepTogether,
    PageBreak,
    PageTemplate,
    Paragraph,
    Spacer,
    Table,
    TableStyle,
)


ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "output" / "pdf" / "Resumen_Ciclo_Prueba_Medisafe.pdf"
OUTPUT.parent.mkdir(parents=True, exist_ok=True)

FONT_REGULAR = Path(r"C:\Windows\Fonts\segoeui.ttf")
FONT_BOLD = Path(r"C:\Windows\Fonts\segoeuib.ttf")
pdfmetrics.registerFont(TTFont("Segoe", str(FONT_REGULAR)))
pdfmetrics.registerFont(TTFont("SegoeBold", str(FONT_BOLD)))

NAVY = colors.HexColor("#16324F")
BLUE = colors.HexColor("#2176AE")
LIGHT_BLUE = colors.HexColor("#EAF4FB")
TEAL = colors.HexColor("#2A9D8F")
LIGHT_TEAL = colors.HexColor("#E9F7F5")
ORANGE = colors.HexColor("#E76F51")
LIGHT_ORANGE = colors.HexColor("#FCEEEA")
GOLD = colors.HexColor("#E9C46A")
INK = colors.HexColor("#27313B")
MUTED = colors.HexColor("#65727E")
LINE = colors.HexColor("#CCD7E0")
PAPER = colors.HexColor("#F7F9FB")


styles = getSampleStyleSheet()
title_style = ParagraphStyle(
    "TitleMedisafe", parent=styles["Title"], fontName="SegoeBold", fontSize=24,
    leading=28, textColor=NAVY, alignment=TA_LEFT, spaceAfter=5 * mm,
)
subtitle_style = ParagraphStyle(
    "Subtitle", parent=styles["Normal"], fontName="Segoe", fontSize=10.5,
    leading=15, textColor=MUTED, spaceAfter=5 * mm,
)
h1 = ParagraphStyle(
    "H1", parent=styles["Heading1"], fontName="SegoeBold", fontSize=17,
    leading=21, textColor=NAVY, spaceBefore=2 * mm, spaceAfter=4 * mm,
)
h2 = ParagraphStyle(
    "H2", parent=styles["Heading2"], fontName="SegoeBold", fontSize=11.5,
    leading=14, textColor=BLUE, spaceAfter=2 * mm,
)
body = ParagraphStyle(
    "Body", parent=styles["BodyText"], fontName="Segoe", fontSize=9.2,
    leading=13, textColor=INK, spaceAfter=2 * mm,
)
small = ParagraphStyle(
    "Small", parent=body, fontSize=8.1, leading=11, textColor=MUTED,
)
card_title = ParagraphStyle(
    "CardTitle", parent=body, fontName="SegoeBold", fontSize=11.5,
    leading=14, textColor=NAVY, spaceAfter=1.5 * mm,
)
card_body = ParagraphStyle(
    "CardBody", parent=body, fontSize=8.7, leading=12,
)
table_head = ParagraphStyle(
    "TableHead", parent=body, fontName="SegoeBold", fontSize=7.5,
    leading=9, textColor=colors.white, alignment=TA_CENTER,
)
table_cell = ParagraphStyle(
    "TableCell", parent=body, fontSize=7.2, leading=9.2, spaceAfter=0,
)
table_cell_bold = ParagraphStyle(
    "TableCellBold", parent=table_cell, fontName="SegoeBold", textColor=NAVY,
)
note = ParagraphStyle(
    "Note", parent=body, fontSize=8.4, leading=12, textColor=INK,
)


def P(text, style=body):
    return Paragraph(text, style)


def header_footer(canvas, doc):
    canvas.saveState()
    width, height = A4
    canvas.setFillColor(NAVY)
    canvas.rect(0, height - 12 * mm, width, 12 * mm, fill=1, stroke=0)
    canvas.setFillColor(colors.white)
    canvas.setFont("SegoeBold", 8.5)
    canvas.drawString(16 * mm, height - 7.5 * mm, "MEDISAFE - CICLO DE LAVADO")
    canvas.setStrokeColor(LINE)
    canvas.line(16 * mm, 13 * mm, width - 16 * mm, 13 * mm)
    canvas.setFillColor(MUTED)
    canvas.setFont("Segoe", 7.5)
    canvas.drawRightString(width - 16 * mm, 8.5 * mm, f"Pagina {doc.page}")
    canvas.restoreState()


def callout(text, background=LIGHT_BLUE, border=BLUE):
    t = Table([[P(text, note)]], colWidths=[174 * mm])
    t.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, -1), background),
        ("BOX", (0, 0), (-1, -1), 0.8, border),
        ("LEFTPADDING", (0, 0), (-1, -1), 4 * mm),
        ("RIGHTPADDING", (0, 0), (-1, -1), 4 * mm),
        ("TOPPADDING", (0, 0), (-1, -1), 3 * mm),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 3 * mm),
    ]))
    return t


def stage_card(title, rows, accent=BLUE):
    data = [[P(title, card_title), ""]]
    for label, value in rows:
        data.append([P(label, table_cell_bold), P(value, card_body)])
    t = Table(data, colWidths=[38 * mm, 136 * mm], hAlign="LEFT")
    style = [
        ("SPAN", (0, 0), (1, 0)),
        ("BACKGROUND", (0, 0), (1, 0), colors.HexColor("#EEF3F7")),
        ("LINEBEFORE", (0, 0), (0, -1), 3, accent),
        ("BOX", (0, 0), (-1, -1), 0.6, LINE),
        ("INNERGRID", (0, 1), (-1, -1), 0.3, colors.HexColor("#E6ECF1")),
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("LEFTPADDING", (0, 0), (-1, -1), 3 * mm),
        ("RIGHTPADDING", (0, 0), (-1, -1), 3 * mm),
        ("TOPPADDING", (0, 0), (-1, -1), 2.2 * mm),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 2.2 * mm),
    ]
    t.setStyle(TableStyle(style))
    return KeepTogether([t, Spacer(1, 3.5 * mm)])


def program_parameters_table(mode, program_names, values):
    mode_title = "Modo estandar" if mode == "standard" else "Modo robot"
    manual_title = (
        "Standard Program Parameters" if mode == "standard"
        else "Robotic Program Parameters"
    )
    program_header = [
        P("Parametros", table_head),
        P("Unidades", table_head),
    ] + [P(f"P{i}", table_head) for i in range(1, 7)]
    name_header = [
        P("Programa", table_cell_bold),
        "",
    ] + [P(name, table_cell) for name in program_names]
    data = [program_header, name_header]
    for parameter, units, row_values in values:
        data.append([
            P(parameter, table_cell_bold),
            P(units, table_cell),
            *[P(str(value), table_cell) for value in row_values],
        ])

    table = Table(
        data,
        colWidths=[42 * mm, 18 * mm] + [19 * mm] * 6,
        repeatRows=2,
    )
    table.setStyle(TableStyle([
        ("BACKGROUND", (0, 0), (-1, 0), NAVY),
        ("BACKGROUND", (0, 1), (-1, 1), LIGHT_BLUE),
        ("ROWBACKGROUNDS", (0, 2), (-1, -1), [colors.white, PAPER]),
        ("BOX", (0, 0), (-1, -1), 0.8, NAVY),
        ("INNERGRID", (0, 0), (-1, -1), 0.35, LINE),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("ALIGN", (1, 0), (-1, -1), "CENTER"),
        ("LEFTPADDING", (0, 0), (-1, -1), 1.5 * mm),
        ("RIGHTPADDING", (0, 0), (-1, -1), 1.5 * mm),
        ("TOPPADDING", (0, 0), (-1, -1), 2.2 * mm),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 2.2 * mm),
    ]))
    return [
        Spacer(1, 2 * mm),
        P(f"Programas Medisafe - {mode_title}", h1),
        P(
            f"Tabla completa del manual: <b>{manual_title}</b>. "
            "Los tiempos estan expresados en minutos.",
            small,
        ),
        table,
        Spacer(1, 2 * mm),
    ]


standard_names = [
    "Lavado<br/>estandar", "Relavado", "Lavado<br/>intensivo",
    "Desinfeccion<br/>estandar", "Desinfeccion<br/>intensiva",
    "Desinfeccion<br/>profunda",
]
robot_names = [
    "Lavado<br/>estandar", "Relavado", "Lavado<br/>Da Vinci",
    "Desinfeccion<br/>estandar", "Desinfeccion<br/>intensiva",
    "Desinfeccion<br/>Da Vinci",
]
standard_values = [
    ("Tiempo de prelavado", "min", [3, 0, 3, 3, 3, 3]),
    ("Quimico de lavado 1", "ml", [360, 360, 360, 360, 360, 360]),
    ("Temperatura de lavado", "C", [40, 40, 40, 40, 40, 40]),
    ("Desgasificado", "min", [3, 3, 3, 3, 3, 3]),
    ("Tiempo de lavado", "min", [5, 10, 20, 5, 10, 20]),
    ("Temperatura de enjuague", "C", [60, 60, 60, 60, 60, 60]),
    ("Tiempo de enjuague", "min", [2, 2, 2, 2, 2, 2]),
    ("Temperatura de desinfeccion", "C", [0, 0, 0, 90, 90, 90]),
    ("Tiempo de desinfeccion", "min", [1, 0, 1, 1, 1, 1]),
    ("Tiempo total tipico del ciclo", "min", [40, "10*", 50, 55, 60, 70]),
]
robot_values = [
    ("Tiempo de prelavado", "min", [3, 0, 3, 3, 3, 3]),
    ("Quimico de lavado 1", "ml", [360, 360, 360, 360, 360, 360]),
    ("Temperatura de lavado", "C", [40, 40, 40, 40, 40, 40]),
    ("Desgasificado", "min", [3, 3, 3, 3, 3, 3]),
    ("Tiempo de lavado", "min", [5, 10, 20, 5, 10, 20]),
    ("Temperatura de enjuague", "C", [60, 60, 60, 60, 60, 60]),
    ("Tiempo de enjuague", "min", [2, 2, 2, 2, 2, 2]),
    ("Temperatura de desinfeccion", "C", [0, 0, 0, 91, 91, 91]),
    ("Tiempo de desinfeccion", "min", [1, 0, 1, 3, 3, 3]),
    ("Tiempo total tipico del ciclo", "min", [45, "10*", 60, 65, 70, 80]),
]


doc = BaseDocTemplate(
    str(OUTPUT), pagesize=A4,
    leftMargin=18 * mm, rightMargin=18 * mm,
    topMargin=19 * mm, bottomMargin=18 * mm,
    title="Resumen del ciclo de lavado Medisafe",
    author="Equipo de desarrollo Medisafe",
    subject="Secuencia, agua, niveles, temperaturas, tiempos y actuadores",
)
frame = Frame(doc.leftMargin, doc.bottomMargin, doc.width, doc.height, id="normal")
doc.addPageTemplates([PageTemplate(id="Medisafe", frames=frame, onPage=header_footer)])

story = []
story += program_parameters_table("standard", standard_names, standard_values)
story += program_parameters_table("robot", robot_names, robot_values)
story += [P("* Por cada relavado. Ver seccion 3-11-10 del manual.", small)]
story += [PageBreak()]
story += [Spacer(1, 5 * mm), P("Resumen del ciclo de lavado", title_style)]
story += [P(
    "Secuencia funcional implementada en la maquina de estados Medisafe. El documento describe "
    "el origen del agua, el nivel de llenado, la temperatura, el uso del calefactor, los tiempos, "
    "la dosificacion de detergente, el ultrasonido y el drenaje de cada instancia.", subtitle_style
)]

sequence = [
    "PREPARE<br/>TANK", "PRE<br/>WASH", "WASH", "RINSE", "FINAL<br/>RINSE",
    "DISINFECTION", "COOLING", "END",
]
seq_table = Table([[P(x, ParagraphStyle(
    f"seq{x}", parent=small, fontName="SegoeBold", fontSize=6.8, leading=8,
    alignment=TA_CENTER, textColor=colors.white)) for x in sequence]],
    colWidths=[21.75 * mm] * 8, rowHeights=[16 * mm])
seq_table.setStyle(TableStyle([
    ("BACKGROUND", (0, 0), (-1, -1), BLUE),
    ("BACKGROUND", (5, 0), (5, 0), ORANGE),
    ("BACKGROUND", (6, 0), (6, 0), TEAL),
    ("BACKGROUND", (7, 0), (7, 0), NAVY),
    ("BOX", (0, 0), (-1, -1), 0.6, colors.white),
    ("INNERGRID", (0, 0), (-1, -1), 1, colors.white),
    ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
]))
story += [seq_table, Spacer(1, 6 * mm)]

story += [P("Vista rapida", h1)]
headers = ["Instancia", "Agua", "Nivel", "Temp.", "Calefactor", "Tiempo activo"]
rows = [
    ["PREPARE_TANK", "-", "EMPTY", "-", "No", "Drenaje + 30 s si hay agua"],
    ["PRE_WASH", "Fria", "MEDIUM", "35 C", "Si", "30 s despues de alcanzar 35 C"],
    ["WASH", "Fria + caliente", "HIGH", "40 C", "Si", "Degas 30 s + lavado 30 s"],
    ["RINSE", "Caliente", "MEDIUM", "60 C", "Si", "30 s despues de alcanzar 60 C"],
    ["FINAL_RINSE", "Caliente", "MEDIUM", "Entrada", "No", "30 s"],
    ["DISINFECTION", "Caliente", "MEDIUM", "90/91 C", "Si", "30 s a temperatura"],
    ["COOLING", "Fria", "MEDIUM", "Hasta <=60 C", "No", "Espera configurada no aplicada"],
    ["END", "-", "EMPTY", "-", "No", "Finaliza, imprime y avisa"],
]
table_data = [[P(x, table_head) for x in headers]] + [
    [P(cell, table_cell_bold if col == 0 else table_cell) for col, cell in enumerate(row)]
    for row in rows
]
summary = Table(table_data, colWidths=[31 * mm, 27 * mm, 20 * mm, 22 * mm, 22 * mm, 52 * mm], repeatRows=1)
summary.setStyle(TableStyle([
    ("BACKGROUND", (0, 0), (-1, 0), NAVY),
    ("ROWBACKGROUNDS", (0, 1), (-1, -1), [colors.white, PAPER]),
    ("BOX", (0, 0), (-1, -1), 0.6, LINE),
    ("INNERGRID", (0, 0), (-1, -1), 0.35, LINE),
    ("VALIGN", (0, 0), (-1, -1), "TOP"),
    ("LEFTPADDING", (0, 0), (-1, -1), 2 * mm),
    ("RIGHTPADDING", (0, 0), (-1, -1), 2 * mm),
    ("TOPPADDING", (0, 0), (-1, -1), 2.2 * mm),
    ("BOTTOMPADDING", (0, 0), (-1, -1), 2.2 * mm),
]))
story += [summary, Spacer(1, 5 * mm)]

story += [Spacer(1, 5 * mm), P("Detalle por instancia", h1)]
story += [stage_card("1. PREPARE_TANK - preparacion de cuba", [
    ("Objetivo", "Garantizar que el ciclo siempre comience con la cuba vacia."),
    ("Llenado", "No llena. Si detecta cualquier nivel distinto de EMPTY, abre drenaje y enciende la bomba de drenaje."),
    ("Finalizacion", "Al detectar EMPTY mantiene el drenaje 30 s adicionales y pasa a PRE_WASH."),
], NAVY)]
story += [stage_card("2. PRE_WASH - prelavado", [
    ("Agua y nivel", "Agua fria hasta MEDIUM. El nivel HIGH tambien se acepta como condicion de corte."),
    ("Temperatura", "Objetivo 35 C, calculado con el promedio de los dos sensores."),
    ("Calefactor", "Si. Despues del llenado enciende bomba trifasica y calefactor. Durante el calentamiento el solenoide de tapa queda apagado."),
    ("Proceso", "Al alcanzar 35 C apaga el calefactor, mantiene la bomba y alterna el solenoide de tapa: 10 s encendido / 5 s apagado."),
    ("Tiempo", "30 s de lavado; el conteo comienza recien al alcanzar la temperatura."),
    ("Quimico / US", "Sin detergente y sin ultrasonido."),
    ("Drenaje", "Drena hasta EMPTY y continua 30 s adicionales."),
], BLUE)]
story += [stage_card("3. WASH - llenado, dosificacion y calentamiento", [
    ("Agua y nivel", "Abre agua fria y caliente simultaneamente. Dosifica al detectar LOW y continua llenando hasta HIGH."),
    ("Detergente", "Enciende la bomba de detergente durante 10 s. La tabla nominal indica 360 ml; el firmware de prueba todavia controla por tiempo."),
    ("Temperatura", "Objetivo 40 C, usando el promedio de ambos sensores."),
    ("Calefactor", "Si. En HIGH enciende bomba trifasica y calefactor. Solenoide de tapa apagado durante el calentamiento. Al llegar a 40 C apaga el calefactor."),
], BLUE)]
story += [stage_card("4. WASH_DEGAS - desgasificado", [
    ("Nivel", "Mantiene el agua en HIGH; no agrega agua."),
    ("Bomba", "Bomba trifasica apagada durante esta fase."),
    ("Ultrasonido", "Pulsado 10 s encendido / 5 s apagado."),
    ("Tiempo", "30 s totales de desgasificado."),
], GOLD)]
story += [stage_card("5. WASH principal - lavado ultrasonico", [
    ("Temperatura", "Comienza despues de calentar a 40 C. El calefactor permanece apagado durante esta fase."),
    ("Ultrasonido", "10 s encendido con bomba trifasica apagada."),
    ("Recirculacion", "5 s con ultrasonido apagado y bomba trifasica encendida. El solenoide de tapa permanece apagado."),
    ("Tiempo", "30 s totales. Luego drena hasta EMPTY y continua 30 s adicionales."),
], GOLD)]

story += [PageBreak(), P("Detalle por instancia - continuacion", h1)]
story += [stage_card("6. RINSE - enjuague", [
    ("Agua y nivel", "Agua caliente hasta MEDIUM. El nivel HIGH tambien se acepta como condicion de corte."),
    ("Temperatura", "Objetivo 60 C, usando el promedio de los dos sensores."),
    ("Calefactor", "Si. Bomba trifasica y calefactor encendidos durante el calentamiento; solenoide de tapa apagado."),
    ("Proceso", "Al alcanzar 60 C apaga el calefactor, mantiene la bomba y alterna tapa 10 s encendida / 5 s apagada."),
    ("Tiempo", "30 s, contados despues de alcanzar 60 C."),
    ("Drenaje", "Drena hasta EMPTY y continua 30 s adicionales."),
], BLUE)]
story += [stage_card("7. FINAL_RINSE - enjuague final", [
    ("Agua y nivel", "Agua caliente hasta MEDIUM."),
    ("Temperatura", "No fija una temperatura objetivo: trabaja con la temperatura del agua de entrada."),
    ("Calefactor", "No."),
    ("Proceso", "Bomba trifasica y solenoide de tapa encendidos continuamente."),
    ("Tiempo", "30 s, contados despues de completar el llenado."),
    ("Drenaje", "Drena hasta EMPTY y continua 30 s adicionales."),
], BLUE)]
story += [stage_card("8. DISINFECTION - desinfeccion termica", [
    ("Agua y nivel", "Agua caliente hasta MEDIUM."),
    ("Temperatura", "90 C en modo estandar; 91 C en modo robot."),
    ("Calefactor", "Si. Calienta con bomba trifasica y solenoide de tapa encendidos."),
    ("Control", "Al llegar al objetivo inicia el tiempo. Reactiva el calefactor si baja 1 C y lo apaga al recuperar temperatura."),
    ("Tiempo", "30 s a temperatura en ambos modos dentro del ciclo corto de prueba."),
    ("Ultrasonido", "No se utiliza."),
    ("Drenaje", "Drena hasta EMPTY y continua 30 s adicionales."),
], ORANGE)]
story += [PageBreak(), stage_card("9. COOLING - enfriamiento", [
    ("Agua y nivel", "Agua fria hasta MEDIUM."),
    ("Calefactor / bomba", "Calefactor apagado; no recircula con bomba trifasica."),
    ("Criterio", "Drena y repite el llenado mientras el promedio de temperatura sea mayor a 60 C."),
    ("Drenaje intermedio", "Sin tiempo extra entre repeticiones."),
    ("Drenaje final", "Cuando alcanza <=60 C, drena hasta EMPTY y continua 30 s adicionales."),
    ("Observacion", "MEDSAF_TIME_COOLING vale 30 s, pero actualmente no se carga ni se consulta: el drenaje comienza inmediatamente despues del llenado."),
], TEAL)]
story += [stage_card("10. END - ciclo finalizado", [
    ("Acciones", "Apaga el sistema, imprime ticket OK, informa END a Nextion, muestra el boton de descarga y ejecuta el aviso sonoro."),
    ("Condicion esperada", "Cuba en EMPTY y puerta bloqueada hasta la secuencia de finalizacion correspondiente."),
], NAVY)]

doc.build(story)
print(OUTPUT)
