# Capturas de la WebUI

`mock.py` es una **placa falsa**: sirve `webui/dist/index.html`, un WebSocket de estado en vivo y respuestas a los
comandos que usa la página (`$$`, `[ESP800]`, lista de la SD, vista previa de un G-code). Con Firefox sin pantalla se
capturan las vistas de [`docs/webui/`](../../docs/webui). Los datos son ficticios.

```bash
python3 ../../webui/build.py          # genera webui/dist/index.html
python3 mock.py 8120 &                # placa falsa en 8120 (WebSocket en 8121, como en la placa real: puerto + 1)
./shoot.sh webui-control-oscuro 1280,900 control 1 es
```
