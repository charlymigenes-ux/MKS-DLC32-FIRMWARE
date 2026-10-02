# MKS-DLC32-FIRMWARE

Firmware Grbl_ESP32 para la controladora **MKS DLC32**, con interfaz web propia y pantalla táctil (LVGL) en
español, inglés y chino. Pensado para láser y CNC de uso general. Basado en el firmware original de
[Makerbase](https://github.com/makerbase-mks/MKS-DLC32-FIRMWARE).

## Hardware probado

| | |
|---|---|
| **Placa** | **MKS LTS V1.1** (así está serigrafiada) |
| **Máquina** | **TTS 55 PRO de 5 W** (láser de diodo) |

Todo lo de este repositorio está probado en esa combinación. En otras placas o máquinas puede funcionar,
pero no está verificado: revisa `$100`–`$132` (pasos, velocidades, recorrido) y `$32` antes de mover nada.

## Qué incluye

- **WebUI nueva** (`webui/`): control, archivos de la SD, parámetros, idioma (español, inglés y chino),
  panel lateral contraíble, actualización de la propia WebUI desde Acerca de.
- **Pantalla LCD rediseñada**: pantalla principal con reloj por NTP, lista de la SD paginada, control,
  trabajo en curso con ajuste de velocidad y potencia, herramientas con Wifi e idioma.
- **Modo láser (`$32=1`)** con rótulos de potencia y velocidad; en modo CNC (`$32=0`), husillo y avance.
- Corrección del tartamudeo y de los reinicios: la rampa del PWM ya no corre dentro de la interrupción.

## Compilar y flashear

Necesitas [PlatformIO](https://platformio.org/). Desde `Firmware/`:

```bash
pio run -e mks_dlc32_8mb                                          # compilar
pio run -e mks_dlc32_8mb -t upload --upload-port /dev/ttyUSB0     # flashear el firmware
pio run -e mks_dlc32_8mb -t uploadfs --upload-port /dev/ttyUSB0   # flashear la WebUI (SPIFFS)
```

Flashea **solo con la máquina parada**: abrir el puerto serie reinicia la placa y detiene el trabajo.

## WebUI

Las fuentes están en `webui/`; `python3 webui/build.py` las junta en `Firmware/Grbl_Esp32/data/index.html.gz`.
Detalles y pruebas en [`webui/README.md`](webui/README.md). Una vez en marcha, se puede actualizar desde
**Acerca de → Actualizar WebUI** sin cable.

## Imagen de inicio

Un PNG de 480×320 se convierte con `python3 tools/png2lvgl.py logo.png` (reemplaza `lv_pic/mks_logo.c`);
después hay que compilar y flashear el firmware.

## Licencia

GNU General Public License v3 (ver `LICENSE`). Se distribuye sin ninguna garantía.
