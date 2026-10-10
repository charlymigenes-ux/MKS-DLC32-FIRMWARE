<div align="center">

# MKS DLC32 · Firmware abierto para láser y CNC

**Grbl_ESP32 con interfaz web propia y pantalla táctil rediseñada, en español, inglés y chino**

[![Licencia GPLv3](https://img.shields.io/badge/licencia-GPLv3-2de0a7?style=for-the-badge)](LICENSE)
[![ESP32](https://img.shields.io/badge/ESP32-Grbl__ESP32-1f2333?style=for-the-badge&logo=espressif&logoColor=2de0a7)](https://github.com/bdring/Grbl_Esp32)
[![LVGL](https://img.shields.io/badge/LVGL-6.1.1-3f4666?style=for-the-badge)](https://lvgl.io)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-listo-f5822a?style=for-the-badge&logo=platformio&logoColor=white)](https://platformio.org)
[![Idiomas](https://img.shields.io/badge/idiomas-ES%20%C2%B7%20EN%20%C2%B7%20%E4%B8%AD%E6%96%87-2de0a7?style=for-the-badge)](#-idiomas)

<br>

[![Probado en](https://img.shields.io/badge/probado%20en-MKS%20LTS%20V1.1%20%C2%B7%20TTS%2055%20PRO%205W-1f2333?style=flat-square&labelColor=2de0a7&color=1f2333)](#-hardware-probado)

<br>

<table>
<tr>
<td align="center"><img src="docs/pantallas/es/01_principal.png" width="400" alt="Pantalla principal del LCD"><br><sub><b>Pantalla táctil</b> (480×320)</sub></td>
<td align="center"><img src="docs/webui/webui-control-oscuro.png" width="560" alt="WebUI en modo oscuro"><br><sub><b>WebUI</b> en el navegador</sub></td>
</tr>
</table>

</div>

---

## Contenido

[Hardware probado](#-hardware-probado) · [Qué incluye](#-qué-incluye) · [Pantalla LCD](#-pantalla-táctil-lcd) · [WebUI](#-webui) · [Modo láser y seguridad](#-modo-láser-y-seguridad) · [Sonidos](#-sonidos) · [Idiomas](#-idiomas) · [Ajustes `$`](#-ajustes--clave) · [Compilar y flashear](#-compilar-y-flashear) · [Actualizar la WebUI](#-actualizar-la-webui-sin-cable) · [Imagen de inicio](#-imagen-de-inicio) · [Simulador](#-simulador-del-lcd-y-capturas) · [Estructura](#-estructura-del-repositorio) · [Problemas frecuentes](#-problemas-frecuentes) · [Qué cambió](#-qué-cambió-respecto-al-firmware-de-mks) · [Créditos](#-créditos-y-licencia)

---

## 🧪 Hardware probado

| Dato | Detalle |
|---|---|
| **Placa** | **MKS LTS V1.1** (así está serigrafiada) |
| **Máquina** | **TTS 55 PRO de 5 W** (láser de diodo) |
| **Chip** | ESP32-D0WD, flash de **8 MB** |

> [!IMPORTANT]
> **Este firmware se desarrolla y prueba solo en esa combinación.** En otras placas o máquinas puede
> funcionar, pero no está verificado: revisa los pasos por mm (`$100`–`$102`), la velocidad y el recorrido
> (`$110`–`$132`) y el modo láser (`$32`) **antes de mover nada**.

| Placa | Estado |
|---|---|
| MKS LTS V1.1 (8 MB) | ✅ Probada: entorno `mks_dlc32_8mb` |
| MKS DLC32 **V2.x** (4 u 8 MB) | ⚠️ **Sin probar.** Hay entornos experimentales (`mks_dlc32_v2_1` de 4 MB y `mks_dlc32_v2_1_8mb` de 8 MB) con el mapa de pines de la V2 (salida del láser GPIO32, sonda GPIO22), que coincide con el mapa publicado de MKS pero **no está verificado en una placa**. Esta placa no entra sola en modo de descarga desde el USB: hay que llevar IO0 (`SDA` del conector «I²C interface») a GND al arrancar |
| Otras (Sculpfun, AVR, STM32…) | ❌ No compatible: el firmware es específico de la MKS DLC32 |

## ✨ Qué incluye

<table>
<tr>
<td width="50%" valign="top">

### 🖐️ Pantalla táctil rediseñada
- Estilo oscuro con esmeralda, **480×320**, botones grandes
- **Español, inglés y chino** (con tildes y ñ)
- Pantalla principal con **reloj por NTP** y coordenadas de trabajo/origen
- **Lista de la SD paginada** (`1/N`), circular, que abre el archivo al tocarlo
- **Marco** del trabajo con aviso para archivos grandes
- **Trabajo en curso** con tres diales: velocidad, potencia y rápido
- Herramientas: Wifi, idioma, **Beeper**, placa y acerca de
- Salta sola a la pantalla de trabajo si lo lanzas desde la web

</td>
<td width="50%" valign="top">

### 🌐 WebUI propia (un solo archivo de 45 KB)
- Control manual, estado en vivo y **consola**
- **Archivos SD** con vista previa del recorrido
- **Parámetros** con catálogo, copia de seguridad, exportar/importar
- Ajuste en vivo de **velocidad y potencia** (10 %–200 %)
- **Tema claro/oscuro**, panel lateral contraíble, tamaño de texto
- **Español, inglés y chino**
- **Actualiza la propia WebUI** desde el navegador, sin cable

</td>
</tr>
<tr>
<td valign="top">

### 🔥 Láser de verdad
- `$32=1`: modo láser con **M4** funcional (`SpindleType::LASER`)
- Botón de láser con **potencias seguras 0 / 5 / 30 / 70 %**; nunca dispara al 100 % de golpe
- «Atrás» con el láser encendido **lo apaga**
- Rótulos de *Potencia y Velocidad* o *Husillo y Avance* según `$32`

</td>
<td valign="top">

### 🛠️ Correcciones de fondo
- **Sin tartamudeo ni reinicios:** la rampa del PWM ya no corre dentro de la interrupción
- Nombres de archivo **UTF-8** (`ñ`, acentos) en la SD
- Con un trabajo en marcha **no se desmonta la SD** al entrar en otra pantalla
- Sonidos de **fin de trabajo, error y alarma** (`$39`)

</td>
</tr>
</table>

---

## 🖐️ Pantalla táctil (LCD)

Todas las capturas las genera el [simulador](#-simulador-del-lcd-y-capturas) con **el mismo código que corre en la placa**
(datos de ejemplo). Están en [`docs/pantallas/`](docs/pantallas) en los tres idiomas.

<table>
<tr>
<td align="center"><img src="docs/pantallas/es/01_principal.png" width="300"><br><sub><b>Principal</b><br>hora, coordenadas y tres accesos</sub></td>
<td align="center"><img src="docs/pantallas/es/02_control.png" width="300"><br><sub><b>Control</b><br>jog, ceros, sonda y láser</sub></td>
<td align="center"><img src="docs/pantallas/es/03b_laser_30.png" width="300"><br><sub><b>Láser al 30 %</b><br>el popup se queda abierto</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/pantallas/es/04_sd_pagina1.png" width="300"><br><sub><b>Archivos SD</b><br>páginas circulares <code>1/3</code></sub></td>
<td align="center"><img src="docs/pantallas/es/07_archivo.png" width="300"><br><sub><b>Archivo elegido</b><br>posicionar, marco e iniciar</sub></td>
<td align="center"><img src="docs/pantallas/es/14_trabajo.png" width="300"><br><sub><b>Trabajo en curso</b><br>tres diales y progreso</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/pantallas/es/10_idioma.png" width="300"><br><sub><b>Idioma</b><br>chino · inglés · alemán* · español</sub></td>
<td align="center"><img src="docs/pantallas/es/11_beeper.png" width="300"><br><sub><b>Beeper</b><br>al tocar y alertas</sub></td>
<td align="center"><img src="docs/pantallas/es/12_placa.png" width="300"><br><sub><b>Placa</b><br>chip, flash y versión</sub></td>
</tr>
</table>

<sub>* El alemán muestra los textos en inglés: el firmware solo trae textos en inglés, español y chino.</sub>

<details>
<summary><b>Ver todas las pantallas en inglés y en chino</b></summary>

<br>

**English**

<table>
<tr>
<td><img src="docs/pantallas/en/01_principal.png" width="260"></td>
<td><img src="docs/pantallas/en/02_control.png" width="260"></td>
<td><img src="docs/pantallas/en/04_sd_pagina1.png" width="260"></td>
</tr>
<tr>
<td><img src="docs/pantallas/en/07_archivo.png" width="260"></td>
<td><img src="docs/pantallas/en/14_trabajo.png" width="260"></td>
<td><img src="docs/pantallas/en/11_beeper.png" width="260"></td>
</tr>
</table>

**中文**

<table>
<tr>
<td><img src="docs/pantallas/zh/01_principal.png" width="260"></td>
<td><img src="docs/pantallas/zh/02_control.png" width="260"></td>
<td><img src="docs/pantallas/zh/04_sd_pagina1.png" width="260"></td>
</tr>
<tr>
<td><img src="docs/pantallas/zh/07_archivo.png" width="260"></td>
<td><img src="docs/pantallas/zh/14_trabajo.png" width="260"></td>
<td><img src="docs/pantallas/zh/11_beeper.png" width="260"></td>
</tr>
</table>

**Resto de pantallas (español)**

<table>
<tr>
<td><img src="docs/pantallas/es/00_logo.png" width="260"><br><sub>Logo de inicio</sub></td>
<td><img src="docs/pantallas/es/02b_control_mas.png" width="260"><br><sub>Control, cabecera 2</sub></td>
<td><img src="docs/pantallas/es/03_laser_popup.png" width="260"><br><sub>Potencia del láser</sub></td>
</tr>
<tr>
<td><img src="docs/pantallas/es/05_sd_pagina2.png" width="260"><br><sub>SD, página 2</sub></td>
<td><img src="docs/pantallas/es/09_wifi.png" width="260"><br><sub>Wifi</sub></td>
<td><img src="docs/pantallas/es/13_acerca.png" width="260"><br><sub>Acerca de</sub></td>
</tr>
</table>

</details>

### Cómo se usa

1. **Elegir y lanzar un archivo:** *Tallado* → toca el archivo (se abre solo) → coloca el cabezal con la cruceta → **Marco** (opcional) → **Iniciar**.
2. **Encender el láser a mano** (para enfocar): en *Control* pulsa **Láser** y elige 5 / 30 / 70 %. El popup queda abierto para cambiar la potencia o apagar con **0 %**; **Atrás** lo apaga y cierra.
3. **Páginas de la SD:** **Subir** y **Siguiente** son circulares: desde la página 1, *Subir* salta a la última.
4. **Ceros y sonda:** *Cero XY*, *Cero Z* y *Cero XYZ* fijan el origen de trabajo en la posición actual (⚠️ no mueven nada). *Sonda Z* usa el palpador de la placa.

---

## 🌐 WebUI

La interfaz web va **dentro del firmware** (un solo `index.html.gz` de unos 45 KB en la memoria interna), sin
internet ni dependencias externas. Entra a `http://<ip-de-la-placa>/`.

<table>
<tr>
<td align="center" width="50%"><img src="docs/webui/webui-control-oscuro.png" width="100%"><br><sub><b>Control</b> · tema oscuro</sub></td>
<td align="center" width="50%"><img src="docs/webui/webui-control-claro.png" width="100%"><br><sub><b>Control</b> · tema claro</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/webui/webui-archivos-sd.png" width="100%"><br><sub><b>Archivos SD</b> con vista previa del recorrido</sub></td>
<td align="center"><img src="docs/webui/webui-parametros.png" width="100%"><br><sub><b>Parámetros</b> con catálogo y respaldo</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/webui/webui-idioma.png" width="100%"><br><sub><b>Idioma</b> de la web y de la pantalla</sub></td>
<td align="center"><img src="docs/webui/webui-control-zh.png" width="100%"><br><sub>La misma interfaz en <b>chino</b></sub></td>
</tr>
<tr>
<td align="center" colspan="2"><img src="docs/webui/webui-acerca.png" width="50%"><br><sub><b>Acerca de</b>: datos de la placa, actualizar la WebUI, instrucciones y créditos</sub></td>
</tr>
</table>

| Vista | Qué permite |
|---|---|
| **Control** | Estado en vivo (X/Y/Z, máquina y trabajo), jog con rueda y Z±, distancia y velocidad, **Origen**/**Posición**, modo **Láser/CNC** (`$32`), láser fijo con OFF/5/25/50/100 %, **consola** con filtro, modo verboso y tres macros configurables, **overrides** de velocidad y potencia de 10 % a 200 % |
| **Archivos SD** | Lista con búsqueda y orden, subida múltiple con progreso, **vista previa** (dimensiones, recorrido, puntos), **Iniciar / Pausar / Detener**, **Encuadrar**, descargar, renombrar y eliminar, actividad reciente |
| **Parámetros** | Catálogo de `$` por categorías con descripción y unidad, búsqueda, **Guardar con respaldo automático**, restaurar, exportar/importar, valores por defecto (`$RST=$`) y resumen de cambios pendientes |
| **Idioma** | Idioma de la interfaz (se guarda en el navegador) e idioma de la pantalla LCD (`$40`) |
| **Acerca de** | Versión y datos de la placa, **Actualizar WebUI**, instrucciones y créditos |

---

## 🔥 Modo láser y seguridad

- **`$32=1`** activa el modo láser de Grbl: la potencia sigue al movimiento (**M4**, potencia dinámica) y el láser **no** queda encendido en los G0. En modo CNC (`$32=0`) la salida se comporta como un husillo PWM normal.
- Con `$32=1`, un `M3 S…` suelto no enciende nada mientras el modo de movimiento sea G0. Por eso el botón de láser usa `G1 F1000 M3 S<n>` y la potencia se calcula sobre `$30`.
- El botón **Láser** de la pantalla **no enciende de golpe**: pide primero la potencia (**0 / 5 / 30 / 70 %**).
- **Salir de Control** con el láser encendido manda `M5`.
- ⚠️ **Usa gafas de protección** y no dejes el láser fijo sobre el material.

## 🔔 Sonidos

El zumbador de la placa es de encendido/apagado (sin tono), así que los sonidos son **patrones de pitidos**:

| Evento | Sonido |
|---|---|
| Trabajo terminado | pi-pi-pi—piiii (tres cortos y uno largo) |
| Error que aborta el trabajo | dos tonos largos |
| Alarma (límites, etc.) | tres tonos largos (el paro con Ctrl-X **no** suena) |

Se activan con **`$39`** (*Herramientas → Beeper → Alertas*). El pitido al tocar la pantalla es independiente: **`$38`**.

## 🌍 Idiomas

| | Pantalla LCD (`$40`) | WebUI |
|---|---|---|
| Español | ✅ `3` | ✅ |
| English | ✅ `1` | ✅ |
| 中文 | ✅ `0` (131 caracteres de la fuente) | ✅ |
| Deutsch | muestra inglés | — |

---

## 🔧 Ajustes `$` clave

Los ajustes viven en la memoria no volátil (NVS) y **sobreviven a un reflasheo**. Los valores de fábrica solo se aplican tras `$RST=*` o borrar la flash.

| Ajuste | Valor de fábrica | Qué es |
|---|---|---|
| `$28` | 1000 Hz | **Frecuencia PWM del láser** |
| `$30` / `$31` | 1000 / 0 | Potencia máxima y mínima (S1000 = 100 %) |
| `$32` | 1 | **Modo láser** |
| `$38` | 1 | Pitido al tocar la pantalla |
| `$39` | 1 | **Sonidos de alerta** (fin de trabajo, error, alarma) |
| `$40` | 1 | Idioma del LCD: 0 中文 · 1 English · 3 Español |
| `$100`–`$102` | 80 | Pasos por mm (X, Y, Z) |
| `$110`–`$112` | 10000 / 10000 / 8000 | Velocidad máxima (mm/min) |
| `$120`–`$122` | 500 | Aceleración (mm/s²) |
| `$130`–`$132` | 300 / 300 / 80 | Recorrido máximo (mm) |
| `$21`, `$22` | 1, 1 | Límites duros y homing activados |

> Los valores de fábrica están calibrados para una máquina de **300 × 300 mm con finales de carrera**. Ajústalos a la tuya.

---

## 🚀 Compilar y flashear

Necesitas [PlatformIO](https://platformio.org/). Desde `Firmware/`:

```bash
pio run -e mks_dlc32_8mb                                          # compilar
pio run -e mks_dlc32_8mb -t upload   --upload-port <puerto>       # firmware      (<puerto>: /dev/ttyUSB0, COM5…)
pio run -e mks_dlc32_8mb -t uploadfs --upload-port <puerto>       # WebUI (SPIFFS)
```

Resultado esperado: **Flash ≈ 47 %, RAM ≈ 25 %**.

| Entorno | Hardware |
|---|---|
| `mks_dlc32_8mb` | **MKS LTS V1.1, flash de 8 MB** (probado) |
| `mks_dlc32_v2_1` | MKS DLC32 **V2.x, flash de 4 MB** (**experimental, sin probar**) |
| `mks_dlc32_v2_1_8mb` | MKS DLC32 **V2.x, flash de 8 MB** (**experimental, sin probar**) |
| `mks_dlc32_v2_1_CoreXY` | Entorno original de MKS para CoreXY, sin probar |

> [!WARNING]
> **Flashea solo con la máquina parada.** Abrir el puerto serie reinicia el ESP32 y detiene el trabajo. Y que **ningún otro programa tenga el puerto abierto** (monitores serie, gestores de impresoras…): un flasheo a medias deja la placa sin firmware hasta repetirlo.

Después de flashear, la placa se conecta a tu Wifi desde *Herramientas → Wifi* y la WebUI queda en `http://<ip>/`.

## 📡 Actualizar la WebUI sin cable

*Acerca de → **Actualizar WebUI***: elige `index.html.gz` (o un `index.html`, que el navegador comprime) y espera a que la página se recargue. Se niega si hay un trabajo en marcha.

```bash
python3 webui/build.py            # genera Firmware/Grbl_Esp32/data/index.html.gz
python3 webui/build.py --check    # comprueba que el .gz esté al día con las fuentes
```

Si la interfaz dejara de abrir, entra en `http://<ip>/?index=yes`: la placa muestra una página de emergencia para volver a subir el archivo. Detalles en [`webui/README.md`](webui/README.md).

## 🖼️ Imagen de inicio

La imagen que se ve al encender (**PNG de 480×320**) va dentro del firmware. Se convierte con una herramienta sin dependencias:

```bash
python3 tools/png2lvgl.py logo.png      # reemplaza Firmware/Grbl_Esp32/src/lv_pic/mks_logo.c
```

Luego compila y flashea el firmware.

## 🧰 Simulador del LCD y capturas

[`tools/lcd-sim`](tools/lcd-sim) compila **el mismo código de las pantallas** en el PC (LVGL 6.1.1 + un «firmware falso») y vuelca PNG de cada pantalla. Sirve para **ver un cambio sin flashear**.

```bash
cd tools/lcd-sim && make -j4 shots      # regenera docs/pantallas/{es,en,zh}   (solo gcc, make y python3)
```

Las capturas de la WebUI las hace [`tools/webui-shots`](tools/webui-shots) con una placa falsa y Firefox sin pantalla.

---

## 🗂️ Estructura del repositorio

```text
.
├── Firmware/                    PlatformIO (Grbl_ESP32 + MKS)
│   ├── platformio.ini · ini/    entornos de compilación
│   └── Grbl_Esp32/
│       ├── src/                 firmware (Protocol, Stepper, Spindles, WebUI…)
│       │   ├── mks/             pantallas del LCD (LVGL), idiomas, tareas
│       │   ├── lv_pic/          iconos, fuentes e imagen de inicio
│       │   └── Machines/        mapa de pines y valores de fábrica (MKS DLC32)
│       ├── data/index.html.gz   WebUI compilada (lo que sirve la placa)
│       └── ld/                  tabla de particiones de 8 MB
├── webui/                       fuentes de la WebUI (HTML, CSS, JS, idiomas, pruebas)
├── tools/
│   ├── lcd-sim/                 simulador del LCD
│   ├── webui-shots/             placa falsa y capturas de la WebUI
│   └── png2lvgl.py              PNG → imagen de inicio
└── docs/                        capturas (pantallas del LCD y WebUI)
```

## 🩺 Problemas frecuentes

<details>
<summary><b>Un archivo con <code>ñ</code> o acentos en el nombre no arrancaba</b></summary>

Estaba resuelto en esta versión: los bytes UTF-8 de los nombres ya no se confunden con comandos de tiempo real de Grbl. Si ves `error:66` con un archivo así, comprueba que tienes el firmware actualizado.
</details>

<details>
<summary><b>Pulsé «Tallado» con un trabajo en marcha y se paró</b></summary>

En el firmware original, abrir la lista de la SD desmontaba la tarjeta y mataba el trabajo. Aquí, con un trabajo en curso, esos botones llevan a la pantalla de trabajo y no tocan la SD. La LCD también salta sola a esa pantalla si lanzas el archivo desde la WebUI.
</details>

<details>
<summary><b>El flasheo falla con <code>Invalid head of packet</code> o la placa no responde después</b></summary>

Otro programa tiene abierto el puerto serie. Ciérralo (o detén el servicio que lo use), comprueba que el puerto está libre y **repite el flasheo completo**: la placa no arrancará hasta que termine bien.
</details>

<details>
<summary><b>La hora de la pantalla principal sale <code>--:--</code></b></summary>

La placa no tiene reloj propio: la hora llega por NTP en cuanto el Wifi conecta (zona fija `CST6`). Sin salida a internet se queda en `--:--`.
</details>

<details>
<summary><b>El láser enciende al elegir un % pero «no hace nada» a mano</b></summary>

En modo láser (`$32=1`) un `M3 S…` suelto no enciende mientras el modo de movimiento sea G0. El botón de la pantalla ya manda `G1 F1000 M3 S…`; si lo haces desde la consola, usa lo mismo.
</details>

## 📜 Qué cambió respecto al firmware de MKS

- **Rampa del PWM fuera de la interrupción** (causaba tartamudeo, ruido y reinicios en grabados con segmentos cortos).
- **Laser real**: `SpindleType::LASER`, `$32=1` por defecto y M4 funcional; potencia calculada sobre `$30`.
- **LCD** rediseñada, con fuentes Latin-1 y chino; listas y diales nuevos; marco con aviso; reloj por NTP.
- **WebUI propia** en lugar de la de ESP3D: control, SD con vista previa, parámetros con respaldo, tres idiomas y actualización por web.
- **SD**: nombres UTF-8, páginas circulares, y la LCD ya no desmonta la tarjeta durante un trabajo.
- **Sonidos** de fin de trabajo, error y alarma (`$39`).
- **Valores de fábrica** calibrados (300×300 mm, homing, modo láser) y nombre de placa correcto (*MKS LTS V1.1*).
- Diagnóstico `[diag]` opcional tras `#define ENABLE_JOB_DIAG` (apagado).

## 🙏 Créditos y licencia

Este proyecto se apoya en el trabajo de otros:

- [**MKS DLC32** (Makerbase)](https://github.com/makerbase-mks/MKS-DLC32-FIRMWARE): el firmware original en el que se basa esta versión.
- [**Grbl_ESP32**](https://github.com/bdring/Grbl_Esp32): núcleo de control de movimiento para ESP32.
- [**Grbl**](https://github.com/gnea/grbl): intérprete de G-code y planificador originales.
- [**LVGL**](https://lvgl.io): biblioteca gráfica de la pantalla táctil.

Licencia **GNU General Public License v3** (ver [`LICENSE`](LICENSE)). Se distribuye **sin ninguna garantía**: un láser o una CNC mal configurados pueden dañar la máquina o causar lesiones. Úsalo bajo tu responsabilidad.
