#!/usr/bin/env python3
"""Placa falsa para capturar la WebUI: sirve webui/dist/index.html, un WebSocket de estado y respuestas
a los comandos que usa la pagina. Solo para sacar capturas (los datos son ficticios).
uso: python3 mock.py [puerto_http]   (el WebSocket va en puerto_http + 1, como en la placa)"""
import asyncio, json, math, sys, threading, time, urllib.parse, http.server, socketserver, pathlib
import websockets
from websockets.asyncio.server import serve

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8120
HTML = (pathlib.Path(__file__).resolve().parent / ".." / ".." / "webui" / "dist" / "index.html").read_text(encoding="utf-8")   # python3 webui/build.py lo genera
CLIENTS = set()
LOOP = None
MPOS = (125.4, 87.0, 0.0)
SETTINGS = {0: "10", 1: "25", 2: "0", 3: "2", 4: "0", 5: "1", 6: "0", 10: "1", 11: "0.010", 12: "0.002", 13: "0",
            20: "0", 21: "1", 22: "1", 23: "3", 24: "300.000", 25: "3000.000", 26: "250", 27: "2.000", 28: "1000.000",
            30: "1000.000", 31: "0.000", 32: "1", 33: "1000.000", 34: "0.000", 35: "0.000", 36: "1000.000",
            38: "1", 39: "1", 40: "3", 100: "80.000", 101: "80.000", 102: "80.000", 110: "10000.000", 111: "10000.000",
            112: "8000.000", 120: "500.000", 121: "500.000", 122: "500.000", 130: "300.000", 131: "300.000", 132: "80.000"}
FILES = [("virgenRosas75x130.gc", "8.28 MB"), ("guadaclasica75x130.gc", "5.18 MB"), ("cena2-75x130.gc", "3.60 MB"),
         ("trescruces75x130.gc", "2.58 MB"), ("Sagrado75x130INV.gc", "2.75 MB"), ("CristoCloseUp130x75.gc", "1.27 MB"),
         ("teponiña202x270.gc", "24.54 MB"), ("efelante120x120.gc", "247.46 KB"), ("buda120x120.gc", "196.38 KB"),
         ("CirsCruz_120x120.gc", "1.43 MB"), ("logo_taller.gc", "47.10 KB"), ("prueba_linea.gc", "1.00 KB")]
BANNER = ["Grbl 1.1h ['$' for help]", "[MSG:Using machine:MKS DLC32]", "[MSG:Start mDNS with hostname:http://mx-laser.local/]",
          "[MSG:Connected - IP is 192.168.0.61]", "ok"]

def esp800():
    return ("FW version:1.1 (20261001) # FW target:grbl-embedded  # FW HW:Direct SD  # primary sd:/sd # secondary sd:none # "
            "authentication:no # webcommunication: Sync: 81:192.168.0.61 # hostname:mx-laser # axis:3")

def gcode():
    # rosa polar de 7 petalos con pasos pequenos: una figura reconocible en la vista previa
    out = ["G21", "G90", "M4 S0", "G0 X60 Y40"]
    n = 3600
    for i in range(n + 1):
        t = i / n * 2 * math.pi
        r = math.cos(3.5 * t) * (0.55 + 0.45 * math.cos(t * 7))
        out.append("G1 X%.3f Y%.3f S%d F3000" % (60 + 55 * r * math.cos(t), 40 + 36 * r * math.sin(t), 250 + int(150 * abs(r))))
    out += ["M5", "G0 X0 Y0"]
    return "\n".join(out) + "\n"

def push(text):
    if LOOP is None or not CLIENTS:
        return
    data = (text if text.endswith("\n") else text + "\n").encode()
    asyncio.run_coroutine_threadsafe(_broadcast(data), LOOP)

async def _broadcast(data):
    websockets.broadcast(CLIENTS, data)

def answer(cmd):
    c = cmd.strip()
    if c == "$$":
        for k in sorted(SETTINGS):
            push("$%d=%s" % (k, SETTINGS[k]))
        push("ok")
    elif c.startswith("$") and c[1:].isdigit() and int(c[1:]) in SETTINGS:
        push("$%s=%s" % (c[1:], SETTINGS[int(c[1:])]))
        push("ok")
    elif c == "?":
        push(status())
    else:
        push("ok")

def status():
    return "<Idle|MPos:%.3f,%.3f,%.3f|FS:0,0|WCO:100.000,80.000,0.000|Ov:100,100,100>" % MPOS

INJECT_HEAD = """<script>(function(){var q=new URLSearchParams(location.search);
 try{localStorage.setItem('dlc32-dark-mode',q.get('dark')==='1'?'1':'0');localStorage.setItem('dlc32.lang',q.get('lang')||'es');
 localStorage.setItem('dlc32-sidebar-collapsed','0');}catch(e){}})();</script><img src="/hold" style="position:absolute;width:1px;height:1px">"""
INJECT_TAIL = """<script>(function(){var q=new URLSearchParams(location.search);var v=q.get('shot')||'control';
 setTimeout(function(){var b=document.querySelector('.nav-item[data-view="'+v+'"]');if(b)b.click();
  if(v==='sdfile'){setTimeout(function(){var r=document.querySelector('#sd-table-body tr:nth-child(4)');if(r)r.click();
   setTimeout(function(){var p=document.getElementById('sd-preview-load-btn');if(p)p.click();},300);},500);}
 var ip=document.getElementById('pill-ip');if(ip)ip.textContent='IP 192.168.0.61';var a=document.getElementById('about-ip');if(a)a.textContent='192.168.0.61';
 },1800);setTimeout(function(){var ip=document.getElementById('pill-ip');if(ip)ip.textContent='IP 192.168.0.61';var a=document.getElementById('about-ip');if(a)a.textContent='192.168.0.61';},1800);})();</script>"""

class H(http.server.BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def _send(self, body, ctype="text/plain; charset=utf-8", code=200):
        b = body.encode() if isinstance(body, str) else body
        self.send_response(code); self.send_header("Content-Type", ctype); self.send_header("Content-Length", str(len(b)))
        self.end_headers(); self.wfile.write(b)
    def do_GET(self):
        u = urllib.parse.urlparse(self.path); q = urllib.parse.parse_qs(u.query)
        if u.path == "/":
            h = HTML.replace("<head>", "<head>" + INJECT_HEAD, 1).replace("</body>", INJECT_TAIL + "</body>", 1)
            self._send(h, "text/html; charset=utf-8")
        elif u.path == "/hold":
            time.sleep(3.2); self._send(b"", "image/gif", 204)
        elif u.path in ("/command", "/command_silent"):
            cmd = (q.get("commandText") or q.get("plain") or [""])[0]
            if cmd.startswith("[ESP800]"):
                self._send(esp800())
            elif cmd.startswith("[ESP"):
                self._send("ok")
            else:
                self._send("")
                threading.Timer(0.05, answer, [cmd]).start()
        elif u.path == "/upload":
            files = [{"name": "System Volume Information", "shortname": "System Volume Information", "size": "-1", "datetime": ""}]
            files += [{"name": n, "shortname": n, "size": s, "datetime": ""} for n, s in FILES]
            self._send(json.dumps({"files": files, "path": "/", "total": "14.41 GB", "used": "196.75 MB", "occupation": "1", "status": "Ok"}), "application/json")
        elif u.path.startswith("/SD/"):
            self._send(gcode())
        else:
            self._send("not found", code=404)

class TS(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True
    allow_reuse_address = True

async def ws_handler(ws):
    CLIENTS.add(ws)
    try:
        await ws.send(b"CURRENT_ID:0ACTIVE_ID:0\n")
        for line in BANNER:
            await ws.send((line + "\n").encode())
        async for _ in ws:
            pass
    finally:
        CLIENTS.discard(ws)

async def ticker():
    while True:
        await asyncio.sleep(0.8)
        websockets.broadcast(CLIENTS, (status() + "\n").encode())

async def amain():
    global LOOP
    LOOP = asyncio.get_running_loop()
    async with serve(ws_handler, "127.0.0.1", PORT + 1, subprotocols=["arduino"]):
        await ticker()

if __name__ == "__main__":
    threading.Thread(target=lambda: TS(("127.0.0.1", PORT), H).serve_forever(), daemon=True).start()
    print("mock en http://127.0.0.1:%d (ws %d)" % (PORT, PORT + 1), flush=True)
    asyncio.run(amain())
