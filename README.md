# MKS-DLC32-FIRMWARE
[![License: GPL v3](https://img.shields.io/badge/license-GPLv3-39D62E)](./LICENSE)
[![Last commit](https://img.shields.io/github/last-commit/charlymigenes-ux/MKS-DLC32-FIRMWARE?color=39D62E)](https://github.com/charlymigenes-ux/MKS-DLC32-FIRMWARE/commits/main)
[![Status](https://img.shields.io/badge/status-active%20development-e6b422)](./CHANGELOG.md)

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

<table>
  <tr>
    <td><img width="630" alt="dashboard" src="https://github.com/user-attachments/assets/079cd1aa-b36f-44bd-aaca-7af3f551e80b" /></td>
    <td><img width="630" alt="3D printers panel" src="https://github.com/user-attachments/assets/6c8ed92c-8071-45b1-8b74-921328046201" /></td>
    <td><img width="630" alt="Laser / CNC panel" src="https://github.com/user-attachments/assets/430b80e6-f9e1-487e-b1c2-bf2f65b81954" /></td>
  </tr>
  <tr>
    <td><img width="630" alt="Materials (Spoolman) panel" src="https://github.com/user-attachments/assets/12671e69-666d-4778-a075-7f4c526fff14" /></td>
    <td><img width="630" alt="Quoting tool" src="https://github.com/user-attachments/assets/01f5a678-6027-4a15-88d9-d94ccb0d15ca" /></td>
    <td><img width="630" alt="Camera viewer" src="https://github.com/user-attachments/assets/03268387-f2d4-4191-a236-0e2e8513b52a" /></td>
</table>

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
