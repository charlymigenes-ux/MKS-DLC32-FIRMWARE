# WebUI para MKS DLC32

Interfaz web propia que reemplaza a la WebUI original de ESP3D en el DLC32.
Se sirve desde SPIFFS del ESP32; el firmware la entrega comprimida.

## Archivos fuente

- `index.html` — estructura de las cuatro vistas: Control, Archivos SD,
  Parámetros e Idioma (idioma de la interfaz y del LCD).
- `css/style.css` — hoja de estilos única.
- `js/i18n.js` — diccionario español → inglés y su buscador (`i18nLookup`).
- `js/app.js` — toda la lógica (sin bundler, sin framework, JS plano).
- `check_i18n.js` — verificador de cobertura del diccionario (ver abajo).

Esta carpeta es la fuente única de la WebUI. Edita aquí, no en el HTML
compilado ni en copias fuera del repo.

## Cómo se compila y se flashea

    python3 webui/build.py

`build.py` junta los cuatro archivos en un solo HTML con el CSS y el JS
embebidos, lo comprime con gzip -9 y escribe:

- `webui/dist/index.html` — el HTML unificado, sin comprimir (gitignorado,
  sirve para inspeccionar o probar en el navegador).
- `Firmware/Grbl_Esp32/data/index.html.gz` — lo que realmente se flashea.

Luego, para subirlo al SPIFFS del ESP32:

    pio run -e mks_dlc32_8mb -t uploadfs

## Verificar que el .gz esté al día

    python3 webui/build.py --check

`data/index.html.gz` es un artefacto generado que vive fuera de esta carpeta,
así que es fácil commitear fuentes editadas junto a un `.gz` viejo y terminar
flasheando una interfaz que no corresponde al código. `--check` falla si los
dos no coinciden; sirve como paso previo a commitear o dentro de CI.

La compresión es reproducible byte a byte (gzip con `mtime=0`), por eso la
comparación directa del `.gz` es confiable.

## Idioma de la interfaz (ES / EN)

El texto de partida es **siempre el español**: el HTML y todo lo que escribe
`app.js` están en español, y `js/i18n.js` declara cómo se dicen en inglés.

- El selector vive en la vista **Idioma**, tarjeta *Idioma de esta WebUI*. La
  elección se guarda en `localStorage` (`dlc32.lang`) y solo afecta a ese
  navegador.
- La traducción se aplica sobre el DOM con un `MutationObserver`, así que
  cubre tanto lo estático como lo que el JS pinta después (tabla de
  parámetros, lista SD, avisos, pie), más los atributos `placeholder`, `title`,
  `alt` y `aria-label`. `confirm()`, `alert()` y `prompt()` se traducen en el
  momento de llamarlos.
- `i18nLookup()` busca en este orden: igualdad exacta, prefijo
  (`Última actualización: 12:00:01`), sufijo (`3 rechazado(s)`) y troceado por
  `|` (`SD 14 GB | Usado 165 MB | Libre 14 GB`). Prefijo y sufijo exigen
  frontera de palabra: `Estado` no traduce `Estado_final.gcode`.
- Si una cadena no está en el diccionario **se queda en español**: nada
  desaparece y la vuelta a ES es exacta (se guarda el original).

Para añadir un texto nuevo: escríbelo en español en `index.html`/`app.js` y
añade la pareja `"español": "english"` en `js/i18n.js`. Luego comprueba que no
quede nada sin cubrir:

    node webui/check_i18n.js

(Sale con código 1 si hay textos en español sin traducir o claves duplicadas.)
Los nombres oficiales de Grbl (`Stallguard X`, `$0`…`$3` en inglés, unidades
como `mm/min`) se muestran igual en los dos idiomas.

## Vista Idioma (dos selectores)

La vista **Idioma** tiene dos tarjetas independientes:

1. **Idioma de esta WebUI** — traduce la propia interfaz (sección anterior).
2. **Idioma de la pantalla (LCD)** — cambia el idioma del equipo, que es otro
   tema: el controlador guarda `$40` en NVS y de él dependen los textos que
   muestra la pantalla.

| `$40` | Idioma   | Fuentes de cadenas        |
|-------|----------|---------------------------|
| `0`   | 中文      | `language_en.h` (en la práctica) |
| `1`   | English  | `language_en.h`           |
| `2`   | Deutsch  | `language_en.h` (en la práctica) |
| `3`   | Español  | `language_es.h`           |

Detalles que conviene conocer:

- El rango de `$40` es `0..3` (`SettingsDefinitions.cpp`, `language_select`).
- Chino y alemán no tienen fichero propio en el árbol, así que muestran los
  textos ingleses: sin eso, `mc_language` quedaría con punteros a `NULL` y las
  etiquetas saldrían con el "Text" por defecto de LVGL.
- `mc_language_init()` se llama al arrancar (`mks_grbl_parg_init`) y se vuelve
  a llamar desde el refresco periódico de `MKS_FREERTOS_TASK.cpp` cuando
  cambia `$40`, así que un `$40=3` escrito desde la WebUI se ve en la pantalla
  sin reiniciar. Los labels ya dibujados se actualizan al repintar la página.
- La fuente del LCD es `dlc32Font` (`src/lv_pic/dlc32Font.c`) con rango
  32..258, es decir ASCII + Latin-1: tildes, `ñ`, `¿` y `¡` se ven bien; los
  glifos chinos no (el botón 中文 puede salir con un recuadro).
- La página de Idiomas del LCD dibuja tres botones (中文 / English / Español,
  filas `y=110/170/230` de 130x50 en una pantalla de 480x320).

## Textos del LCD: todas las páginas

Antes solo la portada usaba `mc_language`: el resto de páginas pegaban los
textos ingleses directamente en el código (`"Yes"`, `"Cancel"`, `"Back"`,
`"Pause"`, `"Is Caving this File?"`…), así que con `$40=3` el menú que sale al
seleccionar un archivo seguía en inglés.

Ahora todas las pantallas cuelgan de `mc_language`:

- **botones y etiquetas**: `Back`, `Up`, `Next`, `Yes`, `Cancel`, `Confirm`,
  `Add`, `Reduce`, `Pause`, `Start`, `Stop`, `Adjustment`, `Knife`, `Cooling`,
  `Position`, `Sculpture`, `Spindle`, `Frame`, `Language`, `Scan`, `Reconnect`,
  `Password`, `Exit`, `Z Home`, `No SD Card`, `XY Clear`, `Z Clear` y las
  velocidades (`Low/Mid/High Speed`);
- **popups**: `Info` / `Warning` / `Error`, avisos de homing, sonda, desbloqueo,
  "SD ocupada", "archivo demasiado grande", "¿Tallar este archivo?",
  "¿Quieres parar la impresión?", "Archivo impreso", wifi y actualizaciones;
- **formatos con `%d`**: `Potencia:%d%%`, `Velocidad:%d%%`, `Avance:%d%%`,
  `Vel. del husillo:%d%%`, `Vel. rápida:%d%%`;
- la **página de pruebas** (`mks_test`).

Las cadenas viven en `language_es.h` / `language_en.h`, se cargan en los dos
bloques de `mc_language_init()`, y los helpers de popup ahora aceptan
`const char *` (`MKS_draw_lvgl.h`). No cambian por ser técnicos: `Wifi`,
`中文 / English / Deutsch / Español`, `X:0`, `0.1mm`, `S:0%`, `OK`, `ERR`.

## Estado del trabajo (vista Control)

La tarjeta **ESTADO DE MÁQUINA** tiene una línea de estado del trabajo con
texto, además del punto de color:

| Texto (ES) | Significado |
|------------|-------------|
| `Inactivo` | sin trabajo (`Idle`) |
| `Trabajo en curso` | `Run` |
| `Trabajo en pausa` | `Hold:*` |
| `Trabajo finalizado` | terminó solo |
| `Trabajo detenido` | lo paraste con **Detener** |
| `Alarma`, `Puerta abierta`, `Modo revisión`, `Reposo` | resto de estados de Grbl |

- Al lado aparece el **nombre del archivo** cuando el trabajo lo lanzó la WebUI
  (`$SD/Run`). Si se lanza desde el LCD, el firmware no comunica el nombre y
  solo se muestra el estado.
- Se actualiza con cada informe (`<Run|…>`) que llega por WebSocket. Si la línea
  de *Última actualización* se queda congelada, el controlador ha dejado de
  mandar datos y el panel no podrá reflejar nada.
- El botón principal sigue con `Iniciar trabajo` / `Pausar` / `Reanudar`.
- La máquina de estados está probada por `node webui/test_job_status.js`,
  que extrae `STATE_TEXT` + `updateJobStatus()` de `app.js` y la alimenta con
  informes `<Run|…>` / `<Hold:0|…>` / `<Idle|…>` / `<Alarm|…>` sobre un DOM de
  mentira: comprueba los cinco estados y el nombre del archivo sin arrancar
  ningún trabajo real.
