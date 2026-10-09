# Simulador del LCD

Compila **el mismo código de las pantallas** (`Firmware/Grbl_Esp32/src/mks/MKS_draw_*.cpp`) con LVGL 6.1.1 en el
PC y lo ejecuta contra un «firmware falso» (`stubs.cpp`, `src/Grbl.h`: posiciones, tarjeta SD con archivos de
ejemplo, ajustes, Wifi). No hay que flashear nada para ver un cambio de pantalla.

```bash
cd tools/lcd-sim
make -j4 shots      # compila y regenera ../../docs/pantallas/{es,en,zh}/*.png (480x320)
./sim out/es es 01_principal 04_sd_pagina1   # solo algunas escenas, en español
```

Solo hace falta `gcc`/`g++`, `make` y `python3` (el PNG lo escribe `ppm2png.py` con la biblioteca estándar).

- La navegación es **con toques simulados** en las coordenadas reales (`tap(x, y)` en `main.cpp`), así que se
  prueban también los eventos de los botones. Los comandos que mandaría el LCD a Grbl salen por pantalla (`[cmd] ...`).
- Los datos (coordenadas, porcentaje de trabajo, archivos de la SD) son ficticios: ver `stubs.cpp`.
- Al añadir una pantalla nueva basta añadir una escena en `main.cpp`. Si el código usa una función de Grbl que
  no está simulada, el enlazado dirá cuál: se añade a `stubs.cpp`.
- Diferencias con la placa: la memoria de LVGL y los tiempos reales no se simulan, y el reloj de la pantalla principal
  muestra la hora del PC.
