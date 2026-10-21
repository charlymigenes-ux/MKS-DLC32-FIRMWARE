(function () {
  "use strict";

  var boardHost = location.hostname || "192.168.0.1";
  var WEBUI_VERSION = "1.0";
  var wsPort = (parseInt(location.port, 10) || 80) + 1;
  var ws = null;
  var wsBuffer = "";
  var reconnectTimer = null;

  var els = {};
  document.querySelectorAll("[id]").forEach(function (el) {
    els[el.id] = el;
  });

  // ---------- i18n: español (fuente) -> inglés ----------
  // El HTML y todos los textos que escribe el JS siguen estando en español;
  // esta capa los traduce al vuelo cuando el usuario elige inglés en la vista
  // Language. Ventajas: si una cadena no esta en el diccionario se queda en
  // español en vez de desaparecer, y no hay que tocar ni el HTML ni los cientos
  // de mensajes de app.js. La eleccion se guarda en localStorage.
  var UI_LANG = "es";
  try {
    var savedLang = localStorage.getItem("dlc32.lang");
    if (savedLang === "en" || savedLang === "es" || savedLang === "zh" || savedLang === "pt") UI_LANG = savedLang;
  } catch (e) {}

  var I18N_ATTRS = ["placeholder", "title", "alt", "aria-label"];

  // El diccionario y el buscador viven en js/i18n.js, que se inlina antes que
  // este fichero. Si faltara, la interfaz sigue funcionando: simplemente no
  // cambia de idioma.
  var i18nDict = typeof i18nLookup === "function" ? i18nLookup : function (t) { return t; };

  function tr(text) {
    if (UI_LANG === "es" || typeof text !== "string") return text;
    return i18nDict(text);
  }

  // Los nombres de archivo y carpeta (translate="no") son datos del usuario: no se traducen
  function i18nSkipped(node) {
    var el = node.nodeType === 1 ? node : node.parentNode;
    return !!(el && el.closest && el.closest('[translate="no"]'));
  }

  function i18nTextNode(node) {
    if (i18nSkipped(node)) return;
    var cur = node.nodeValue;
    if (typeof cur !== "string") return;

    if (UI_LANG === "es") {
      // Volver al español: el original se guardo al traducir.
      if (node._i18nEn !== undefined && cur === node._i18nEn) node.nodeValue = node._i18nEs;
      node._i18nEn = undefined;
      node._i18nEs = undefined;
      return;
    }
    if (node._i18nEn !== undefined) {
      if (cur === node._i18nEn) return;        // ya traducido
      if (cur === node._i18nEs) {              // el JS reescribio el mismo texto ES node.nodeValue = node._i18nEn;
        return;
      }
      node._i18nEn = undefined;                // texto nuevo: nueva fuente
      node._i18nEs = undefined;
    }

    var m = /^(\s*)([\s\S]*?)(\s*)$/.exec(cur);
    if (!m) return;
    var en = i18nDict(m[2]);
    if (en === m[2]) return;                   // sin traduccion: se queda en español
    var out = m[1] + en + m[3];
    node._i18nEs = cur;
    node._i18nEn = out;
    if (out !== cur) node.nodeValue = out;
  }

  function i18nAttr(el, name) {
    var cur = el.getAttribute(name);
    if (cur === null || cur === undefined) return;
    var st = el._i18nAttr || (el._i18nAttr = {});

    if (UI_LANG === "es") {
      if (st[name] && cur === st[name].en) el.setAttribute(name, st[name].es);
      delete st[name];
      return;
    }
    if (st[name]) {
      if (cur === st[name].en) return;
      if (cur === st[name].es) { el.setAttribute(name, st[name].en); return; }
      delete st[name];
    }
    var en = i18nDict(cur);
    if (en === cur) return;
    st[name] = { es: cur, en: en };
    el.setAttribute(name, en);
  }

  function i18nWalk(node) {
    if (!node) return;
    if (node.nodeType === 3) { i18nTextNode(node); return; }
    if (node.nodeType !== 1) return;
    if (node.tagName === "SCRIPT" || node.tagName === "STYLE" || node.getAttribute("translate") === "no") return;
    for (var i = 0; i < I18N_ATTRS.length; i++) i18nAttr(node, I18N_ATTRS[i]);
    var kids = node.childNodes;
    for (var j = 0; j < kids.length; j++) i18nWalk(kids[j]);
  }

  function i18nApplyAll() {
    i18nWalk(document.body);
  }

  function i18nStart() {
    i18nApplyAll();
    if (typeof MutationObserver === "undefined") return;
    // Cubre tambien lo que genera JS a posteriori: tabla de parametros, lista
    // SD, avisos, lineas de consola...
    var mo = new MutationObserver(function (records) {
      for (var i = 0; i < records.length; i++) {
        var r = records[i];
        if (r.type === "characterData") {
          i18nTextNode(r.target);
        } else if (r.type === "attributes") {
          i18nAttr(r.target, r.attributeName);
        } else {
          for (var j = 0; j < r.addedNodes.length; j++) i18nWalk(r.addedNodes[j]);
        }
      }
    });
    mo.observe(document.body, {
      childList: true,
      subtree: true,
      characterData: true,
      attributes: true,
      attributeFilter: I18N_ATTRS
    });
  }

  function i18nSetLang(lang) {
    if (lang !== "en" && lang !== "es" && lang !== "zh" && lang !== "pt") return;
    // Se vuelve primero al español (restaura los originales) y luego se aplica el idioma nuevo.
    if (UI_LANG !== "es") { UI_LANG = "es"; i18nApplyAll(); }
    if (typeof i18nUse === "function") i18nUse(lang);
    UI_LANG = lang;
    try { localStorage.setItem("dlc32.lang", lang); } catch (e) {}
    i18nApplyAll();
    document.querySelectorAll(".lang-option[data-uilang]").forEach(function (b) {
      b.classList.toggle("active", b.dataset.uilang === lang);
    });
    var badge = document.getElementById("uilang-current");
    if (badge) badge.textContent = lang === "en" ? "English" : (lang === "zh" ? "中文" : (lang === "pt" ? "Português" : "Español"));
  }

  // confirm()/alert() no pasan por el DOM: se traducen en el momento de llamar.
  if (typeof window.confirm === "function") {
    var nativeConfirm = window.confirm;
    window.confirm = function (msg) {
      return nativeConfirm(typeof msg === "string" ? tr(msg) : msg);
    };
  }
  if (typeof window.alert === "function") {
    var nativeAlert = window.alert;
    window.alert = function (msg) {
      nativeAlert(typeof msg === "string" ? tr(msg) : msg);
    };
  }
  if (typeof window.prompt === "function") {
    var nativePrompt = window.prompt;
    window.prompt = function (msg, def) {
      return nativePrompt(typeof msg === "string" ? tr(msg) : msg, def);
    };
  }

  // localStorage puede contener basura (una version vieja, un valor a mano):
  // sin try/catch el JSON.parse de una clave corrupta revienta el IIFE y la
  // interfaz se queda a medias (en concreto, settings e init no llegan a correr).
  function parseJsonSafe(raw, fallback) {
    try {
      var v = JSON.parse(raw);
      return v === null || v === undefined ? fallback : v;
    } catch (e) {
      return fallback;
    }
  }

  // ---------- Navegación ----------
  document.querySelectorAll(".nav-item[data-view]").forEach(function (btn) {
    btn.addEventListener("click", function () {
      document.querySelectorAll(".nav-item").forEach(function (b) { b.classList.remove("active"); });
      btn.classList.add("active");
      var view = btn.dataset.view;
      document.querySelectorAll(".view").forEach(function (v) { v.classList.remove("active"); });
      document.getElementById("view-" + view).classList.add("active");
      if (view === "sdfile") loadSdFiles();
      if (view === "settings") loadSettings(true);
      if (view === "language") loadLanguage();
      if (view === "about") renderAbout();
    });
  });

  // ---------- Modo de la maquina: laser ($32=1) o CNC ($32=0) ----------
  // Los elementos con data-laser / data-cnc cambian de rotulo segun el modo (texto en espanol;
  // la capa i18n lo traduce). Se lee $32 al conectar y siempre que llega una linea "$32=n".
  var machineLaser = true;  // hasta leer $32
  function applyMachineMode() {
    document.querySelectorAll("[data-laser]").forEach(function (el) {
      el.textContent = machineLaser ? el.dataset.laser : el.dataset.cnc;
    });
    document.querySelectorAll("#mode-switch [data-mode]").forEach(function (b) {
      b.classList.toggle("active", (b.dataset.mode === "1") === machineLaser);
    });
  }
  function setMachineMode(laser) {
    if (machineLaser === laser) return;
    machineLaser = laser;
    applyMachineMode();
  }
  document.querySelectorAll("#mode-switch [data-mode]").forEach(function (b) {
    b.addEventListener("click", function () {
      var laser = b.dataset.mode === "1";
      if (laser === machineLaser) return;
      if (isRunningState(machineState) || isPausedState(machineState)) {
        alert(tr("Detén el trabajo antes de cambiar el modo de la máquina."));
        return;
      }
      var msg = laser
        ? "¿Cambiar a modo LÁSER ($32=1)? Activa el M4 de potencia dinámica y rotula la interfaz como Potencia y Velocidad."
        : "¿Cambiar a modo CNC ($32=0)? Desactiva el modo láser del controlador y rotula la interfaz como Husillo y Avance.";
      if (!confirm(tr(msg))) return;
      sendCommand("$32=" + (laser ? "1" : "0"));
      setTimeout(function () { sendCommand("$32", true); }, 400);  // confirma el valor real
    });
  });

  // ---------- Velocidad y potencia del laser (overrides GRBL en tiempo real) ----------
  // Cada boton manda un caracter de tiempo real (0x90..0x9D) por el mismo canal que el resto
  // de comandos; el valor mostrado se corrige con el campo Ov: del informe de estado.
  var OVR_MIN = 10;
  var OVR_MAX = 200;
  var OVR = {
    feed:    { val: 100, cmds: { "-10": "\u0092", "-1": "\u0094", "+1": "\u0093", "+10": "\u0091", "100": "\u0090" } },
    spindle: { val: 100, cmds: { "-10": "\u009B", "-1": "\u009D", "+1": "\u009C", "+10": "\u009A", "100": "\u0099" } }
  };
  function ovrRender(k) {
    var v = OVR[k].val;
    els["ovr-" + k + "-val"].textContent = v;
    els["ovr-" + k + "-fill"].style.width = Math.max(0, Math.min(100, (v - OVR_MIN) * 100 / (OVR_MAX - OVR_MIN))) + "%";
  }
  function ovrSet(k, v) {
    if (isNaN(v)) return;
    v = Math.max(OVR_MIN, Math.min(OVR_MAX, v));
    if (OVR[k].val !== v) {
      OVR[k].val = v;
      ovrRender(k);
    }
  }
  document.querySelectorAll("[data-ovr-cmd]").forEach(function (btn) {
    btn.addEventListener("click", function () {
      var p = btn.dataset.ovrCmd.split(":");
      var k = p[0];
      var d = p[1];
      sendCommand(OVR[k].cmds[d], true);
      ovrSet(k, d === "100" ? 100 : OVR[k].val + parseInt(d, 10));  // optimista; la placa lo confirma
    });
  });
  ovrRender("feed");
  ovrRender("spindle");

  // ---------- Panel lateral contraíble ----------
  // El estado se recuerda en localStorage; sin preferencia guardada arranca
  // contraído en pantallas estrechas. Los textos van en español (fuente): la
  // capa i18n traduce tambien title y aria-label.
  var SIDEBAR_KEY = "dlc32-sidebar-collapsed";
  function sidebarInitiallyCollapsed() {
    try {
      var v = localStorage.getItem(SIDEBAR_KEY);
      if (v === "1" || v === "0") return v === "1";
    } catch (e) {}
    return window.innerWidth <= 1000;
  }
  function applySidebar(collapsed, save) {
    document.body.classList.toggle("sidebar-collapsed", collapsed);
    var t = els["sidebar-toggle"];
    var label = collapsed ? "Expandir panel" : "Contraer panel";
    t.setAttribute("aria-expanded", collapsed ? "false" : "true");
    t.setAttribute("title", label);
    t.setAttribute("aria-label", label);
    if (save) {
      try { localStorage.setItem(SIDEBAR_KEY, collapsed ? "1" : "0"); } catch (e) {}
    }
  }
  els["sidebar-toggle"].addEventListener("click", function () {
    applySidebar(!document.body.classList.contains("sidebar-collapsed"), true);
  });
  applySidebar(sidebarInitiallyCollapsed(), false);

  // ---------- Vista Acerca de ----------
  var boardInfoText = "";
  function aboutField(re) {
    var m = re.exec(boardInfoText);
    return m ? m[1].trim() : "--";
  }
  function renderAbout() {
    els["about-fw"].textContent = aboutField(/FW version:([^#]+)/);
    els["about-target"].textContent = aboutField(/FW target:([^#]+)/);
    els["about-hw"].textContent = aboutField(/FW HW:([^#]+)/);
    els["about-host"].textContent = aboutField(/hostname:([^\s#]+)/);
    els["about-ip"].textContent = boardHost;
    els["about-webui"].textContent = WEBUI_VERSION;
  }

  // ---------- WebSocket: consola/estado en vivo ----------
  function connectWs() {
    try {
      ws = new WebSocket("ws://" + boardHost + ":" + wsPort, ["arduino"]);
      ws.binaryType = "arraybuffer";
    } catch (e) {
      scheduleReconnect();
      return;
    }
    ws.onopen = function () {
      setConnected(true);
      setTimeout(function () { sendCommand("$32", true); }, 600);  // modo laser / CNC
      setTimeout(penLoad, 1000);  // ajustes de la pluma ($41-$44), si la placa los tiene
    };
    ws.onclose = function () {
      setConnected(false);
      scheduleReconnect();
    };
    ws.onerror = function () {
      setConnected(false);
    };
    ws.onmessage = function (e) {
      if (e.data instanceof ArrayBuffer) {
        var bytes = new Uint8Array(e.data);
        var msg = "";
        for (var i = 0; i < bytes.length; i++) msg += String.fromCharCode(bytes[i]);
        wsBuffer += msg;
        var lines = wsBuffer.split(/[\r\n]+/);
        wsBuffer = lines.pop();
        lines.forEach(handleLine);
      }
    };
  }

  function scheduleReconnect() {
    if (reconnectTimer) return;
    reconnectTimer = setTimeout(function () {
      reconnectTimer = null;
      connectWs();
    }, 3000);
  }

  function setConnected(ok) {
    els["conn-status-dot"].style.background = ok ? "#16a34a" : "#dc2626";
    els["conn-status-text"].textContent = ok ? "Conexión establecida" : "Sin conexión";
  }

  // Los informes de estado (<Run|MPos:...|SD:34.5,/archivo.gc>) llegan cada 2 s y llenaban
  // la consola. En modo no verboso se muestran sin el nombre del archivo y solo cuando cambia
  // el estado, cada 5 % de avance de la SD, o como maximo cada 30 s. El modo verboso los
  // muestra todos, tal cual llegan.
  var statusLog = { state: "", bucket: -1, t: 0 };
  function stripSdName(line) {
    return line.replace(/(\|SD:[0-9.]+),[^|>]*/, "$1");
  }
  function shouldLogStatus(line) {
    if (els["console-verbose"].checked) return true;
    var state = line.slice(1, -1).split("|")[0] || "";
    var m = /\|SD:([0-9.]+)/.exec(line);
    var bucket = m ? Math.floor(parseFloat(m[1]) / 5) : -1;
    var now = Date.now();
    if (state !== statusLog.state || bucket !== statusLog.bucket || now - statusLog.t >= 30000) {
      statusLog.state = state;
      statusLog.bucket = bucket;
      statusLog.t = now;
      return true;
    }
    return false;
  }

  function handleLine(line) {
    if (!line) return;
    var isStatus = line[0] === "<" && line[line.length - 1] === ">";
    if (!isStatus) {
      appendConsole(line);
    } else if (shouldLogStatus(line)) {
      appendConsole(els["console-verbose"].checked ? line : stripSdName(line));
    }
    if (isStatus) {
      parseStatusReport(line);
    }
    var m32 = /^\$32=([01])\b/.exec(line.trim());
    if (m32) setMachineMode(m32[1] === "1");
    // Idioma de la pantalla: cualquier respuesta "$40=n" lo actualiza, aunque el recolector de
    // loadLanguage() ya se haya cerrado (otro "ok" podia terminarlo antes de tiempo)
    var m40 = /^\$40=([0-3])\b/.exec(line.trim());
    if (m40) renderLanguage(m40[1]);
    var mPen = /^\$(4[1-4])=(-?[0-9.]+)\b/.exec(line.trim());
    if (mPen) penApplySetting(mPen[1], parseFloat(mPen[2]));
    if (settingsCollector) {
      var trimmed = line.trim();
      var m = /^\$(\d+)=(.+)$/.exec(trimmed);
      if (m) settingsCollector.buffer[m[1]] = m[2].trim();
      if (trimmed === "ok") {
        var cb = settingsCollector.onDone;
        var buf = settingsCollector.buffer;
        settingsCollector = null;
        cb(buf);
      }
    }
    if (pendingCmdResult) {
      var t = line.trim();
      if (t === "ok" || /^error:/i.test(t) || /^ALARM:/i.test(t)) {
        var resultCb = pendingCmdResult.onResult;
        pendingCmdResult = null;
        resultCb(t);
      }
    }
    els["last-update-label"].textContent = "Última actualización: " + new Date().toLocaleTimeString();
  }

  var machineState = "";
  var wco = null;  // Work Coordinate Offset del último informe (para WPos = MPos - WCO)

  // El firmware reporta "Hold:0"/"Hold:1" (y "Door:x"), nunca "Hold" a secas
  // (Report.cpp:856-859). Comparar con === "Hold" hacia que la pausa no se
  // detectara nunca: el boton no mostraba "Reanudar" y "Iniciar trabajo"
  // volvia a lanzar el archivo con $SD/Run en vez de reanudar con "~".
  function isPausedState(state) {
    return typeof state === "string" && state.indexOf("Hold") === 0;
  }
  function isRunningState(state) {
    return state === "Run";
  }

  /* --- estado del trabajo visible ---------------------------------------
   * El punto de color y el texto del boton no dicen si el trabajo esta en
   * curso, pausado, finalizado o detenido: aqui se traduce el estado que
   * reporta el firmware a un texto legible y se acompania del nombre del
   * archivo cuando lo sabemos (los trabajos lanzados desde el LCD no traen
   * nombre, y entonces solo se muestra el estado). */
  var STATE_TEXT = {
    "Idle": "Inactivo",
    "Run": "Trabajo en curso",
    "Hold": "Trabajo en pausa",
    "Alarm": "Alarma",
    "Door": "Puerta abierta",
    "Check": "Modo revisión",
    "Home": "Homing",
    "Sleep": "Reposo",
    "Tool": "Cambio de herramienta"
  };
  var currentJob = null;    // archivo del ultimo trabajo lanzado desde la WebUI
  var pendingJob = null;    // lanzado y todavia no visto en marcha
  var jobEndReason = null;  // "finished" | "stopped" | null
  var jobWasBusy = false;

  function updateJobStatus(state) {
    var box = document.getElementById("job-status");
    var txt = document.getElementById("job-status-text");
    var file = document.getElementById("job-status-file");
    if (!box || !txt) return;

    var base = String(state || "").split(":")[0];
    var busy = base === "Run" || base === "Hold";

    if (base === "Run") {
      if (pendingJob) {                 // era nuestro lanzamiento: guardamos el nombre
        currentJob = pendingJob;
        pendingJob = null;
        jobEndReason = null;
      } else if (jobEndReason) {        // arranco otro trabajo (desde el LCD): nombre desconocido
        currentJob = null;
        jobEndReason = null;
      }
    }
    if (jobWasBusy && !busy && !jobEndReason && currentJob) jobEndReason = "finished";
    jobWasBusy = busy;

    var cls, text;
    if (base === "Run") { cls = "is-run"; text = "Trabajo en curso"; }
    else if (base === "Hold") { cls = "is-paused"; text = "Trabajo en pausa"; }
    else if (base === "Alarm") { cls = "is-alarm"; text = "Alarma"; }
    else if (jobEndReason === "finished") { cls = "is-done"; text = "Trabajo finalizado"; }
    else if (jobEndReason === "stopped") { cls = "is-stopped"; text = "Trabajo detenido"; }
    else { cls = "is-idle"; text = STATE_TEXT[base] || base || "Inactivo"; }

    box.className = "job-status " + cls;
    if (txt.textContent !== text) txt.textContent = text;
    var withFile = !!currentJob && cls !== "is-idle";
    if (file) {
      file.hidden = !withFile;
      if (withFile && file.textContent !== currentJob) file.textContent = currentJob;
    }
  }

  // Formato GRBL: <Idle|MPos:0.000,0.000,0.000|FS:0,0|WCO:0.000,0.000,0.000>
  function parseStatusReport(line) {
    var body = line.slice(1, -1);
    var parts = body.split("|");
    var state = parts[0] || "";
    machineState = state;
    updateRunButton(state);
    updateJobStatus(state);
    els["machine-state-dot"].style.background =
      state === "Alarm" ? "#dc2626" : ((isRunningState(state) || isPausedState(state)) ? "#f59e0b" : "#16a34a");

    var mpos = null;
    var wpos = null;
    parts.slice(1).forEach(function (part) {
      var idx = part.indexOf(":");
      if (idx === -1) return;
      var key = part.slice(0, idx);
      var val = part.slice(idx + 1);
      if (key === "MPos") mpos = val.split(",");
      if (key === "WPos") wpos = val.split(",");
      if (key === "WCO") wco = val.split(",");
      if (key === "Ov") {
        var ov = val.split(",");  // avance, rapido, husillo (%)
        ovrSet("feed", parseInt(ov[0], 10));
        ovrSet("spindle", parseInt(ov[2], 10));
      }
      if (key === "FS") {
        var fs = val.split(",");
        els["stat-feed"].textContent = fs[0] || "0";
        els["stat-spindle"].textContent = fs[1] || "0";
      }
    });

    // El firmware puede mandar WPos directamente; si solo manda MPos la
    // coordenada de trabajo se calcula con el ultimo WCO conocido.
    if (!wpos && mpos && wco) {
      wpos = [0, 1, 2].map(function (i) {
        var v = parseFloat(mpos[i] || 0) - parseFloat(wco[i] || 0);
        return isNaN(v) ? "0" : String(v);
      });
    }

    function fmt(coords, i) {
      var n = parseFloat(coords && coords[i]);
      return isNaN(n) ? "0.000" : n.toFixed(3);
    }
    var machine = mpos || wpos;
    var work = wpos || mpos;
    if (machine) {
      els["stat-mx"].textContent = fmt(machine, 0);
      els["stat-my"].textContent = fmt(machine, 1);
    }
    if (work) {
      els["stat-x"].textContent = fmt(work, 0);
      els["stat-y"].textContent = fmt(work, 1);
      penSetZ(work[2]);
    }
  }

  function appendConsole(line) {
    if (!els["console-verbose"].checked && line.trim() === "") return;
    var div = document.createElement("div");
    div.textContent = line;
    els["console-output"].appendChild(div);
    while (els["console-output"].childNodes.length > 500) {
      els["console-output"].removeChild(els["console-output"].firstChild);
    }
    if (els["console-autoscroll"].checked) {
      els["console-output"].scrollTop = els["console-output"].scrollHeight;
    }
  }

  els["console-clear"].addEventListener("click", function () {
    els["console-output"].innerHTML = "";
  });

  // ---------- Control de tamaño de texto (dos controles, mismo estado) ----------
  var textScale = parseFloat(localStorage.getItem("dlc32-text-scale")) || 1.0;
  function applyTextScale() {
    document.documentElement.style.fontSize = (16 * textScale) + "px";
    var pct = Math.round(textScale * 100) + "%";
    els["text-size-label"].textContent = pct;
    els["text-size-label-2"].textContent = pct;
    localStorage.setItem("dlc32-text-scale", textScale);
  }
  function incTextScale() {
    textScale = Math.min(1.6, Math.round((textScale + 0.1) * 10) / 10);
    applyTextScale();
  }
  function decTextScale() {
    textScale = Math.max(0.8, Math.round((textScale - 0.1) * 10) / 10);
    applyTextScale();
  }
  els["text-size-inc"].addEventListener("click", incTextScale);
  els["text-size-dec"].addEventListener("click", decTextScale);
  els["text-size-inc-2"].addEventListener("click", incTextScale);
  els["text-size-dec-2"].addEventListener("click", decTextScale);
  applyTextScale();

  // ---------- Dark mode ----------
  var ICON_SUN = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="5"></circle><line x1="12" y1="1" x2="12" y2="3"></line><line x1="12" y1="21" x2="12" y2="23"></line><line x1="4.22" y1="4.22" x2="5.64" y2="5.64"></line><line x1="18.36" y1="18.36" x2="19.78" y2="19.78"></line><line x1="1" y1="12" x2="3" y2="12"></line><line x1="21" y1="12" x2="23" y2="12"></line><line x1="4.22" y1="19.78" x2="5.64" y2="18.36"></line><line x1="18.36" y1="5.64" x2="19.78" y2="4.22"></line></svg>';
  var ICON_MOON = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"></path></svg>';
  var darkMode = localStorage.getItem("dlc32-dark-mode") === "1";
  function applyDarkMode() {
    document.body.classList.toggle("dark", darkMode);
    els["theme-toggle-icon"].innerHTML = darkMode ? ICON_MOON : ICON_SUN;
    localStorage.setItem("dlc32-dark-mode", darkMode ? "1" : "0");
  }
  els["theme-toggle"].addEventListener("click", function () {
    darkMode = !darkMode;
    applyDarkMode();
  });
  applyDarkMode();

  // ---------- Envío de comandos GRBL ----------
  function sendCommand(cmd, quiet) {
    if (!cmd || !cmd.trim()) return;
    if (!quiet) appendConsole("[#]" + (cmd === "\u0018" ? "Ctrl-X (reset)" : cmd));
    var url = "/command?commandText=" + encodeURIComponent(cmd);
    fetch(url)
      .then(function (r) {
        // Un 401/500 no lanza excepcion: sin este chequeo los errores de
        // autenticacion o de la placa pasarian silenciosos.
        if (!r.ok) appendConsole("[error] HTTP " + r.status + tr(" al enviar: ") + cmd);
      })
      .catch(function (err) {
        appendConsole("[error] " + err.message);
      });
  }

  els["console-send"].addEventListener("click", function () {
    sendCommand(els["console-input"].value);
    els["console-input"].value = "";
  });
  els["console-input"].addEventListener("keydown", function (e) {
    if (e.key === "Enter") {
      sendCommand(els["console-input"].value);
      els["console-input"].value = "";
    }
  });

  // ---------- Macros de consola (3 botones configurables) ----------
  var MACROS_KEY = "dlc32-console-macros";
  function loadMacros() {
    try { return JSON.parse(localStorage.getItem(MACROS_KEY) || "{}"); } catch (e) { return {}; }
  }
  function saveMacros(macros) { localStorage.setItem(MACROS_KEY, JSON.stringify(macros)); }
  function renderMacros() {
    var macros = loadMacros();
    [1, 2, 3].forEach(function (slot) {
      var m = macros[slot];
      var btn = els["console-macro-" + slot + "-btn"];
      btn.textContent = m ? m.label : "Configurar " + slot;
      btn.title = m ? m.command : "Sin configurar";
    });
  }
  function configureMacro(slot) {
    var macros = loadMacros();
    var existing = macros[slot];
    var label = prompt("Nombre del botón " + slot + ":", existing ? existing.label : "");
    if (label === null) return;
    var command = prompt("Comando a enviar (G-code o $):", existing ? existing.command : "");
    if (command === null) return;
    if (!label.trim() || !command.trim()) {
      delete macros[slot];
    } else {
      macros[slot] = { label: label.trim(), command: command.trim() };
    }
    saveMacros(macros);
    renderMacros();
  }
  [1, 2, 3].forEach(function (slot) {
    els["console-macro-" + slot + "-btn"].addEventListener("click", function () {
      var macros = loadMacros();
      var m = macros[slot];
      if (!m) { configureMacro(slot); return; }
      sendCommand(m.command);
    });
    els["console-macro-" + slot + "-edit"].addEventListener("click", function () { configureMacro(slot); });
  });
  renderMacros();

  // ---------- Jog / acciones rápidas ----------
  function getJogSpeed() { return parseFloat(els["jog-speed-num"].value) || 1500; }
  function getJogDist() { return parseFloat(els["jog-dist-num"].value) || 50; }

  document.querySelectorAll("[data-jog]").forEach(function (btn) {
    btn.addEventListener("click", function () {
      var axisDir = btn.dataset.jog;
      var axis = axisDir[0];
      var sign = axisDir[1] === "+" ? "" : "-";
      var dist = sign + getJogDist();
      sendCommand("$J=G91 " + axis + dist + " F" + getJogSpeed());
    });
  });
  els["jog-stop"].addEventListener("click", function () { sendCommand("!"); });

  function updateVSlider(inputEl, thumbEl, fillEl) {
    var min = parseFloat(inputEl.min);
    var max = parseFloat(inputEl.max);
    var val = parseFloat(inputEl.value);
    var pct = ((val - min) / (max - min)) * 100;
    pct = Math.max(0, Math.min(100, pct));
    thumbEl.style.top = (100 - pct) + "%";
    fillEl.style.height = pct + "%";
  }

  els["jog-speed"].addEventListener("input", function () {
    els["jog-speed-num"].value = this.value;
    updateVSlider(els["jog-speed"], els["jog-speed-thumb"], els["jog-speed-fill"]);
  });
  els["jog-speed-num"].addEventListener("input", function () {
    els["jog-speed"].value = this.value;
    updateVSlider(els["jog-speed"], els["jog-speed-thumb"], els["jog-speed-fill"]);
  });
  els["jog-dist"].addEventListener("input", function () {
    els["jog-dist-num"].value = this.value;
    updateVSlider(els["jog-dist"], els["jog-dist-thumb"], els["jog-dist-fill"]);
  });
  els["jog-dist-num"].addEventListener("input", function () {
    els["jog-dist"].value = this.value;
    updateVSlider(els["jog-dist"], els["jog-dist-thumb"], els["jog-dist-fill"]);
  });
  updateVSlider(els["jog-speed"], els["jog-speed-thumb"], els["jog-speed-fill"]);
  updateVSlider(els["jog-dist"], els["jog-dist-thumb"], els["jog-dist-fill"]);

  // ---------- Pluma (plotter): subir y bajar con el eje Z ----------
  // La tarjeta solo aparece si la placa contesta a $41 (alturas de la pluma): en un firmware sin esos ajustes
  // responde error:3 y la tarjeta se queda oculta. Subir y bajar son movimientos "$J=" en coordenadas de
  // trabajo, igual que en la pantalla de la maquina: Z=0 es la pluma tocando el papel.
  var pen = { known: false, up: 3, down: 0, plunge: 500, draw: 1500, z: null, msgTimer: null };
  var PEN_TOL = 0.3;                                         // mm: margen para decir "arriba" o "abajo"
  var PEN_FIELDS = { "41": "up", "42": "down", "43": "plunge", "44": "draw" };

  function penSay(text, isError) {
    els["pen-msg"].textContent = text;
    els["pen-msg"].classList.toggle("is-error", !!isError);
    clearTimeout(pen.msgTimer);
    pen.msgTimer = setTimeout(function () { els["pen-msg"].textContent = ""; }, 5000);
  }

  function penRenderState() {
    if (!pen.known || pen.z === null) return;
    els["pen-z"].textContent = pen.z.toFixed(3);
    var txt, cls;
    if (pen.z >= pen.up - PEN_TOL)        { txt = "▲ ARRIBA";        cls = "pen-state is-up"; }
    else if (pen.z <= pen.down + PEN_TOL) { txt = "▼ ABAJO";         cls = "pen-state is-down"; }
    else                                  { txt = "● EN MOVIMIENTO"; cls = "pen-state is-moving"; }
    els["pen-state"].textContent = txt;
    els["pen-state"].className = cls;
  }

  function penSetZ(v) {
    var n = parseFloat(v);
    if (isNaN(n)) return;
    pen.z = n;
    penRenderState();
  }

  function penFillInputs() {
    ["up", "down", "plunge", "draw"].forEach(function (k) {
      var el = els["pen-set-" + k];
      if (el && document.activeElement !== el) el.value = pen[k];   // no pisa lo que el usuario esta escribiendo
    });
    penRenderState();
  }

  function penApplySetting(code, n) {
    var k = PEN_FIELDS[code];
    if (!k || isNaN(n)) return;
    pen[k] = n;
    pen.known = true;
    els["pen-card"].hidden = false;
    penFillInputs();
  }

  function penLoad() {
    sendCommand("$41", true);
    setTimeout(function () {
      if (!pen.known) return;                                // sin $41 no hay pluma: no se piden los demas
      sendCommand("$42", true);
      setTimeout(function () { sendCommand("$43", true); }, 250);
      setTimeout(function () { sendCommand("$44", true); }, 500);
    }, 700);
    // la variante de plotter se reconoce por su nombre (V1.0(M+X.PLT...)): ahi se oculta lo del laser
    fetch("/command?commandText=" + encodeURIComponent("[ESP420]"))
      .then(function (r) { return r.ok ? r.text() : ""; })
      .then(function (t) { if (/M\+X\.PLT/.test(t)) document.body.classList.add("plotter"); })
      .catch(function () {});
  }

  function penCanMove() {
    if (isRunningState(machineState) || isPausedState(machineState)) {
      alert(tr("Detén el trabajo antes de mover la pluma."));
      return false;
    }
    if (machineState === "Alarm") {
      alert(tr("La máquina está en alarma. Desbloquéala con $X antes de mover la pluma."));
      return false;
    }
    return true;
  }

  // el boton se desactiva un momento (no se acumulan ordenes) y se pide el estado para ver el resultado enseguida
  function penBusy(btn) {
    btn.disabled = true;
    setTimeout(function () { btn.disabled = false; }, 1200);
    setTimeout(function () { sendCommand("?", true); }, 400);
    setTimeout(function () { sendCommand("?", true); }, 1600);
  }

  function penGoZ(z, feed, btn) {
    if (!penCanMove()) return false;
    sendCommand("$J=G90G21Z" + z.toFixed(2) + "F" + Math.round(feed));
    penBusy(btn);
    return true;
  }

  els["pen-up"].addEventListener("click", function () { penGoZ(pen.up, pen.draw, this); });
  els["pen-down"].addEventListener("click", function () { penGoZ(pen.down, pen.plunge, this); });

  els["pen-origin"].addEventListener("click", function () {   // sube la pluma y va al origen de trabajo, en ese orden
    if (!penGoZ(pen.up, pen.draw, els["pen-up"])) return;
    sendCommand("$J=G90G21X0Y0F" + Math.round(pen.draw));
    penBusy(this);
  });

  els["pen-zero-z"].addEventListener("click", function () {
    if (!penCanMove()) return;
    if (!confirm(tr("¿Poner Z=0 aquí? La pluma debe estar apenas tocando el papel."))) return;
    sendCommand("G92 Z0");
    penBusy(this);
    penSay(tr("Z=0 establecido en la posición actual."), false);
  });

  els["pen-save"].addEventListener("click", function () {
    var v = {
      "41": parseFloat(els["pen-set-up"].value),
      "42": parseFloat(els["pen-set-down"].value),
      "43": parseFloat(els["pen-set-plunge"].value),
      "44": parseFloat(els["pen-set-draw"].value)
    };
    var limits = { "41": [0.5, 50], "42": [-5, 5], "43": [50, 5000], "44": [100, 10000] };
    for (var code in limits) {
      if (isNaN(v[code]) || v[code] < limits[code][0] || v[code] > limits[code][1]) {
        penSay(tr("Revisa los valores: $") + code + " (" + limits[code][0] + " a " + limits[code][1] + ")", true);
        return;
      }
    }
    if (v["41"] <= v["42"]) {
      penSay(tr("La altura de arriba debe ser mayor que la de abajo."), true);
      return;
    }
    var btn = this;
    btn.disabled = true;
    var map = {};
    Object.keys(v).forEach(function (code) { map[code] = String(v[code]); });
    applySettingsValues(map, function (errors) {
      btn.disabled = false;
      if (errors && errors.length) {
        penSay(errors.length + " rechazado(s)", true);
      } else {
        penSay(tr("Ajustes de la pluma guardados."), false);
      }
    });
  });

  els["btn-home"].addEventListener("click", function () { sendCommand("$H"); });
  els["btn-position"].addEventListener("click", function () { sendCommand("?"); });
  els["btn-zero"].addEventListener("click", function () { sendCommand("G92 X0 Y0"); });
  els["btn-goorigin"].addEventListener("click", function () { sendCommand("G90 G0 Z0"); });
  els["btn-reset-controller"].addEventListener("click", function () {
    if (!confirm("¿Reiniciar el controlador? Esto detiene cualquier trabajo en curso (pausado o corriendo).")) return;
    sendCommand(String.fromCharCode(0x18));
  });

  // Láser/Spindle: potencia por porcentaje (0-1000 en unidades GRBL S)
  document.querySelectorAll(".btn-power").forEach(function (btn) {
    btn.addEventListener("click", function () {
      document.querySelectorAll(".btn-power").forEach(function (b) { b.classList.remove("active"); });
      btn.classList.add("active");
      var pct = parseFloat(btn.dataset.power);
      if (pct <= 0) {
        sendCommand("M5");
      } else {
        var sValue = Math.round((pct / 100) * 1000);
        // En modo laser ($32=1) un M3 suelto no enciende nada mientras el modo de movimiento
        // sea G0 (Grbl pasa la potencia a 0). Con G1 en la misma linea dispara en el sitio.
        sendCommand(machineLaser ? "G1 F1000 M3 S" + sValue : "M3 S" + sValue);
      }
    });
  });

  // ---------- SD File ----------
  var sdFiles = [];
  var sdSelected = null;
  var sdPath = "/";        // carpeta abierta de la SD ("/" o "/a/b")

  // Ruta con "/" final, como la espera /upload (path=/a/b/)
  function sdDirParam() { return sdPath === "/" ? "/" : sdPath + "/"; }
  // Ruta completa de un archivo/carpeta de la carpeta abierta
  function sdFull(name) { return (sdPath === "/" ? "" : sdPath) + "/" + name; }
  function sdEnc(p) { return p.split("/").map(encodeURIComponent).join("/"); }
  // La respuesta de /upload es 200 aunque falle la accion: el resultado viene en "status"
  function sdActionFailed(data) {
    return !!(data && data.status && /does not exist|already exists|cannot|error|failed|busy|no sd/i.test(data.status));
  }

  function loadSdFiles() {
    fetch("/upload?path=" + encodeURIComponent(sdDirParam()))
      .then(function (r) { return r.json(); })
      .then(function (data) {
        sdFiles = (data.files || []).filter(function (f) {
          return f.name !== "System Volume Information" && f.name.charAt(0) !== ".";
        }).map(function (f) { f.isDir = f.size === "-1"; return f; });
        renderSdSpace(data);
        renderSdPath();
        renderSdList();
      })
      .catch(function () {
        els["sd-space-label"].textContent = "Error al leer la SD";
      });
  }

  function sdClearSelection() {
    sdSelected = null;
    sdSelectedFile = null;
    els["sd-details-empty"].hidden = false;
    els["sd-details-body"].hidden = true;
  }

  function sdOpenDir(path) {
    sdPath = path || "/";
    sdClearSelection();
    els["sd-search"].value = "";
    loadSdFiles();
  }

  // Miga de pan: SD / carpeta / subcarpeta (cada tramo lleva a esa carpeta)
  function renderSdPath() {
    var box = els["sd-breadcrumb"];
    box.innerHTML = "";
    var acc = "";
    function crumb(label, path, last) {
      var b = document.createElement("button");
      b.type = "button";
      b.className = "sd-crumb" + (last ? " is-last" : "");
      if (path !== "/") b.setAttribute("translate", "no");
      b.textContent = label;
      if (!last) b.addEventListener("click", function () { sdOpenDir(path); });
      box.appendChild(b);
    }
    var parts = sdPath === "/" ? [] : sdPath.substring(1).split("/");
    crumb("SD", "/", parts.length === 0);
    parts.forEach(function (seg, i) {
      acc += "/" + seg;
      var sep = document.createElement("span");
      sep.className = "sd-crumb-sep";
      sep.textContent = "/";
      box.appendChild(sep);
      crumb(seg, acc, i === parts.length - 1);
    });
    var nd = sdFiles.filter(function (f) { return f.isDir; }).length, nf = sdFiles.length - nd;
    els["sd-count"].textContent = nf + " " + tr(nf === 1 ? "archivo" : "archivos") + " · " +
      nd + " " + tr(nd === 1 ? "carpeta" : "carpetas");
  }

  function formatBytes(n) {
    if (n >= 1024 * 1024 * 1024) return (n / (1024 * 1024 * 1024)).toFixed(2) + "GB";
    if (n >= 1024 * 1024) return (n / (1024 * 1024)).toFixed(2) + "MB";
    if (n >= 1024) return (n / 1024).toFixed(2) + "KB";
    return n + "B";
  }

  function renderSdSpace(data) {
    // "Ok" es la respuesta normal; los demas status vienen de respuestas sin
    // "files" ({"status":"No SD Card"} / "Busy", WebServer.cpp:1234-1239),
    // que antes se veian como una lista simplemente vacia, sin aviso.
    if (data.status && data.status !== "Ok") {
      els["sd-space-label"].textContent = data.status;
      els["sd-space-fill"].style.width = "0%";
      els["sd-space-pct"].textContent = "--";
      return;
    }
    var label = "SD " + (data.total || "--") + " | Usado " + (data.used || "--");
    if (data.total && data.used) {
      var free = sizeToBytes(data.total) - sizeToBytes(data.used);
      if (free > 0) label += " | Libre " + formatBytes(free);
    }
    els["sd-space-label"].textContent = label;
    var pct = parseInt(data.occupation || "0", 10);
    els["sd-space-fill"].style.width = pct + "%";
    els["sd-space-pct"].textContent = pct + "%";
  }

  function renderSdList() {
    var query = (els["sd-search"].value || "").toLowerCase();
    var sortMode = els["sd-sort"].value;
    var pageSize = parseInt(els["sd-page-size"].value, 10);

    var list = sdFiles.filter(function (f) {
      return f.name.toLowerCase().indexOf(query) !== -1;
    });

    list.sort(function (a, b) {
      if (a.isDir !== b.isDir) return a.isDir ? -1 : 1;   // las carpetas primero
      if (sortMode === "name-desc") return b.name.localeCompare(a.name);
      if (sortMode === "size-desc") return a.isDir ? a.name.localeCompare(b.name) : sizeToBytes(b.size) - sizeToBytes(a.size);
      if (sortMode === "size-asc") return a.isDir ? a.name.localeCompare(b.name) : sizeToBytes(a.size) - sizeToBytes(b.size);
      return a.name.localeCompare(b.name);
    });

    list = list.slice(0, pageSize);

    var tbody = els["sd-table-body"];
    tbody.innerHTML = "";
    function iconBtn(cls, title, glyph, fn) {
      var b = document.createElement("button");
      b.type = "button";
      b.className = "sd-row-btn " + cls;
      b.title = title;
      b.setAttribute("aria-label", title);
      b.textContent = glyph;
      b.addEventListener("click", function (ev) { ev.stopPropagation(); fn(); });
      return b;
    }
    if (sdPath !== "/") {
      var up = document.createElement("tr");
      up.className = "sd-row-dir sd-row-up";
      up.innerHTML = '<td>&#8593;</td><td>..</td><td></td><td><span class="sd-badge sd-badge-dir">Subir un nivel</span></td>';
      up.addEventListener("click", function () { sdOpenDir(sdPath.substring(0, sdPath.lastIndexOf("/")) || "/"); });
      tbody.appendChild(up);
    }
    list.forEach(function (f) {
      var row = document.createElement("tr");
      row.dataset.name = f.name;
      if (f.isDir) {
        row.className = "sd-row-dir";
        row.innerHTML =
          '<td>&#128193;</td>' +
          '<td translate="no">' + escapeHtml(f.name) + '</td>' +
          '<td>&mdash;</td>' +
          '<td></td>';
        var cell = row.lastChild;
        var badge = document.createElement("span");
        badge.className = "sd-badge sd-badge-dir";
        badge.textContent = "Carpeta";
        cell.appendChild(badge);
        cell.appendChild(iconBtn("sd-row-rename", "Renombrar", "\u270e", function () { sdRenameDir(f.name); }));
        cell.appendChild(iconBtn("sd-row-delete", "Eliminar", "\u2716", function () { sdDeleteDir(f.name); }));
        row.addEventListener("click", function () { sdOpenDir(sdFull(f.name)); });
      } else {
        if (sdSelected === f.name) row.classList.add("selected");
        row.innerHTML =
          '<td>&#128196;</td>' +
          '<td translate="no">' + escapeHtml(f.name) + '</td>' +
          '<td>' + escapeHtml(f.size) + '</td>' +
          '<td><span class="sd-badge">Listo</span></td>';
        row.addEventListener("click", function () { selectSdFile(f); });
      }
      tbody.appendChild(row);
    });
    if (!sdFiles.length) {
      var empty = document.createElement("tr");
      empty.innerHTML = '<td colspan="4" class="sd-empty">Carpeta vacía.</td>';
      tbody.appendChild(empty);
    }
  }

  // ---------- CRUD de carpetas ----------
  function sdCall(query, okMsg, failMsg, onOk) {
    return fetch("/upload?" + query + "&path=" + encodeURIComponent(sdDirParam()))
      .then(function (r) {
        if (!r.ok) throw new Error("HTTP " + r.status);
        return r.json();
      })
      .then(function (data) {
        if (sdActionFailed(data)) throw new Error(data.status);
        if (onOk) onOk();
        loadSdFiles();
        if (okMsg) logSdActivity(okMsg);
      })
      .catch(function (err) { alert(tr(failMsg) + ": " + err.message); });
  }

  function sdCreateDir() {
    var name = prompt(tr("Nombre de la nueva carpeta:"), "");
    if (!name) return;
    name = name.trim().replace(/[\\/]/g, "");
    if (!name) return;
    sdCall("action=createdir&filename=" + encodeURIComponent(name), "Carpeta creada: " + name, "No se pudo crear la carpeta");
  }

  function sdRenameDir(name) {
    var nn = prompt(tr("Nuevo nombre para") + " " + name + ":", name);
    if (!nn) return;
    nn = nn.trim().replace(/[\\/]/g, "");
    if (!nn || nn === name) return;
    sdCall("action=rename&filename=" + encodeURIComponent(name) + "&newname=" + encodeURIComponent(nn),
      "Carpeta renombrada: " + name + " -> " + nn, "No se pudo renombrar");
  }

  function sdDeleteDir(name) {
    if (!confirm(tr("¿Eliminar la carpeta") + " " + name + " " + tr("y todo su contenido?"))) return;
    sdCall("action=deletedir&filename=" + encodeURIComponent(name), "Carpeta eliminada: " + name, "No se pudo eliminar la carpeta");
  }

  function sizeToBytes(sizeStr) {
    if (!sizeStr) return 0;
    var m = /([\d.]+)\s*(KB|MB|GB)?/i.exec(sizeStr);
    if (!m) return 0;
    var n = parseFloat(m[1]);
    var unit = (m[2] || "").toUpperCase();
    if (unit === "KB") return n * 1024;
    if (unit === "MB") return n * 1024 * 1024;
    if (unit === "GB") return n * 1024 * 1024 * 1024;
    return n;
  }

  function escapeHtml(s) {
    return String(s).replace(/[&<>"']/g, function (c) {
      return { "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c];
    });
  }

  var sdPreviewCache = null;
  var sdSelectedFile = null;
  var sdSelectedDir = "/";

  function selectSdFile(f) {
    sdSelected = f.name;
    sdSelectedFile = f;
    sdPreviewCache = null;
    renderSdList();
    els["sd-details-empty"].hidden = true;
    els["sd-details-body"].hidden = false;
    els["sd-detail-name"].textContent = f.name;
    els["sd-detail-size"].textContent = f.size;
    els["sd-detail-modified"].textContent = f.datetime || "--";
    sdSelectedDir = sdPath === "/" ? "/" : sdPath + "/";
    els["sd-preview-body"].hidden = true;
    els["sd-preview-loading"].hidden = true;
    els["sd-preview-load-btn"].hidden = true;
    var pcv = els["sd-preview-canvas"]; if (pcv) pcv.getContext("2d").clearRect(0, 0, pcv.width, pcv.height);
    prevRequest(f);              // se pide sola; si ya esta hecha sale al instante
    renderPreviewPanel();
  }

  // ---------- Vista previa: parseo real de G-code, sin datos inventados ----------
  // Se guardan los tramos de corte/grabado (G1/G2/G3 con el laser encendido), tambien los de S0, con su
  // potencia. Se pintan sobremuestreados y reducidos con suavizado: el tramado de puntos de un raster se
  // convierte en medios tonos y se ve la imagen. Las dimensiones salen de los tramos con S > 0.
  function newGcodePreviewParser() {
    var x = 0, y = 0, relative = false, mode = 0, laserOn = true, sNow = 0;
    var allMinX = Infinity, allMinY = Infinity, allMaxX = -Infinity, allMaxY = -Infinity;
    var bMinX = Infinity, bMinY = Infinity, bMaxX = -Infinity, bMaxY = -Infinity;
    var cap = 65536, n = 0, seg = new Float32Array(cap * 5);
    var points = 0, travel = 0, maxS = 0, hasBurn = false;
    var rest = "";

    function oneLine(raw) {
      var line = raw.split(";")[0].split("(")[0];
      if (!line) return;
      line = line.trim().toUpperCase();
      if (!line) return;
      if (/\bG91\b/.test(line)) relative = true;
      if (/\bG90\b/.test(line)) relative = false;
      if (/\bM[34]\b/.test(line)) laserOn = true;
      if (/\bM5\b/.test(line)) laserOn = false;
      var gm = /\bG0?([0-3])\b/.exec(line);
      if (gm) mode = parseInt(gm[1], 10);
      var sm = /S(-?[\d.]+)/.exec(line);
      if (sm) sNow = parseFloat(sm[1]);
      var xMatch = /X(-?[\d.]+)/.exec(line);
      var yMatch = /Y(-?[\d.]+)/.exec(line);
      if (!xMatch && !yMatch) return;
      if (!gm && /\bG(?:[4-9]|[1-9]\d)\b/.test(line)) return;
      var nx = xMatch ? (relative ? x + parseFloat(xMatch[1]) : parseFloat(xMatch[1])) : x;
      var ny = yMatch ? (relative ? y + parseFloat(yMatch[1]) : parseFloat(yMatch[1])) : y;
      var dx = nx - x, dy = ny - y;
      travel += Math.sqrt(dx * dx + dy * dy);
      points++;
      if (nx < allMinX) allMinX = nx; if (nx > allMaxX) allMaxX = nx;
      if (ny < allMinY) allMinY = ny; if (ny > allMaxY) allMaxY = ny;
      if (mode >= 1 && laserOn) {
        if (n === cap) { cap *= 2; var big = new Float32Array(cap * 5); big.set(seg); seg = big; }
        var o = n * 5;
        seg[o] = x; seg[o + 1] = y; seg[o + 2] = nx; seg[o + 3] = ny; seg[o + 4] = sNow;
        n++;
        if (sNow > 0) {
          hasBurn = true;
          if (sNow > maxS) maxS = sNow;
          if (x < bMinX) bMinX = x; if (x > bMaxX) bMaxX = x; if (y < bMinY) bMinY = y; if (y > bMaxY) bMaxY = y;
          if (nx < bMinX) bMinX = nx; if (nx > bMaxX) bMaxX = nx; if (ny < bMinY) bMinY = ny; if (ny > bMaxY) bMaxY = ny;
        }
      }
      x = nx;
      y = ny;
    }

    return {
      // Procesa un trozo de texto; la ultima linea, si llega cortada, espera al trozo siguiente.
      feed: function (chunk) {
        var text = rest + chunk;
        var from = 0, nl;
        while ((nl = text.indexOf("\n", from)) !== -1) { oneLine(text.substring(from, nl)); from = nl + 1; }
        rest = text.substring(from);
      },
      finish: function () {
        if (rest) { oneLine(rest); rest = ""; }
        var minX = hasBurn ? bMinX : allMinX, maxX = hasBurn ? bMaxX : allMaxX;
        var minY = hasBurn ? bMinY : allMinY, maxY = hasBurn ? bMaxY : allMaxY;
        if (!isFinite(minX)) { minX = 0; maxX = 0; minY = 0; maxY = 0; }
        return { minX: minX, minY: minY, maxX: maxX, maxY: maxY, seg: seg, nseg: n, maxS: maxS || 1000,
                 points: points, travel: travel };
      }
    };
  }

  function parseGcodePreview(text) {
    var p = newGcodePreviewParser();
    p.feed(text);
    return p.finish();
  }

  function renderGcodePreview(data, cv) {
    // Siempre a la misma resolucion, en un lienzo aparte: asi se puede terminar aunque el usuario ya este en otro archivo
    var W = 640, H = 460;
    cv.width = W; cv.height = H;
    var w = data.maxX - data.minX || 1;
    var h = data.maxY - data.minY || 1;
    var pad = 12;
    // Sobremuestreo: hasta x4, limitado a ~4,5 millones de celdas para no agotar la memoria del navegador
    var ss = Math.max(1, Math.min(4, Math.floor(Math.sqrt(4500000 / (W * H)))));
    var SW = W * ss, SH = H * ss;
    var scale = Math.min((SW - 2 * pad * ss) / w, (SH - 2 * pad * ss) / h);
    var offX = (SW - w * scale) / 2;
    var offY = (SH - h * scale) / 2;
    var sum = new Float32Array(SW * SH), cnt = new Uint16Array(SW * SH);
    var seg = data.seg, k, o;
    for (k = 0; k < data.nseg; k++) {
      o = k * 5;
      var ax = (seg[o] - data.minX) * scale + offX, ay = SH - ((seg[o + 1] - data.minY) * scale + offY);
      var bx = (seg[o + 2] - data.minX) * scale + offX, by = SH - ((seg[o + 3] - data.minY) * scale + offY);
      var p = Math.min(1, seg[o + 4] / data.maxS);
      var steps = Math.max(1, Math.ceil(Math.max(Math.abs(bx - ax), Math.abs(by - ay))));
      for (var t = 0; t <= steps; t++) {
        var px = Math.round(ax + (bx - ax) * t / steps), py = Math.round(ay + (by - ay) * t / steps);
        if (px < 0 || py < 0 || px >= SW || py >= SH) continue;
        var idx = py * SW + px;
        sum[idx] += p; if (cnt[idx] < 65535) cnt[idx]++;
      }
    }
    // Reduccion con suavizado: potencia media de las celdas recorridas de cada bloque ss x ss
    var val = new Float32Array(W * H), hist = new Uint32Array(256), seen = 0, i, x, y, bx2, by2;
    for (y = 0; y < H; y++) {
      for (x = 0; x < W; x++) {
        var s2 = 0, c2 = 0;
        for (by2 = 0; by2 < ss; by2++) {
          var row = (y * ss + by2) * SW + x * ss;
          for (bx2 = 0; bx2 < ss; bx2++) { s2 += sum[row + bx2]; c2 += cnt[row + bx2]; }
        }
        i = y * W + x;
        if (c2) { var v = s2 / c2; val[i] = v; hist[Math.min(255, Math.floor(v * 255))]++; seen++; }
        else val[i] = -1;
      }
    }
    // Niveles automaticos: el 99,5 % de la potencia queda como blanco (una imagen suave no sale apagada)
    var hi = 1, acc = 0, lim = seen * 0.995;
    for (i = 0; i < 256; i++) { acc += hist[i]; if (acc >= lim) { hi = Math.max(0.05, (i + 1) / 255); break; } }
    // Como el papel: blanco lo que no se graba y negro lo que si (cuanta mas potencia, mas negro)
    var ctx = cv.getContext("2d"), img = ctx.createImageData(W, H), d = img.data;
    for (i = 0; i < W * H; i++) {
      var q = i * 4, vv = val[i], c;
      if (vv < 0) c = 222;   // fuera del recorrido: gris muy claro
      else {
        var f = Math.min(1, vv / hi);
        f = Math.pow(f, 0.85);
        c = 252 - 244 * f;   // 252 (blanco) -> 8 (negro)
      }
      d[q] = c; d[q + 1] = c; d[q + 2] = c; d[q + 3] = 255;
    }
    ctx.putImageData(img, 0, 0);
  }

  // ---------- Vista previa automatica ----------
  // Al seleccionar un archivo se pide sola (sin boton). Las descargas van de una en una (la placa solo atiende
  // una a la vez), en cola, y siguen aunque se navegue a otro archivo o carpeta: el progreso no se pierde y, al
  // volver, la miniatura ya esta. Se guarda en memoria y en el navegador (localStorage) para la proxima vez.
  var SD_PREV_KEY = "dlc32-sd-preview-bn";   // v2: blanco y negro (las miniaturas viejas eran en negativo)
  var SD_PREV_MAX = 24;
  var prevJobs = {};        // clave -> { state: "queued"|"loading"|"done"|"error", got, total, t0, entry, ctl }
  var prevQueue = [];       // claves en espera
  var prevActive = null;    // clave que se esta descargando

  function prevCacheKey(f, dir) { return (dir === undefined ? sdFull(f.name) : dir + f.name) + "|" + f.size + "|" + (f.datetime || ""); }
  function prevCacheRead() {
    try { return JSON.parse(localStorage.getItem(SD_PREV_KEY) || "[]"); } catch (e) { return []; }
  }
  function prevCacheGet(key) {
    var j = prevJobs[key];
    if (j && j.state === "done") return j.entry;
    var list = prevCacheRead();
    for (var i = 0; i < list.length; i++) if (list[i].k === key) return list[i];
    return null;
  }
  function prevCachePut(entry) {
    var list = prevCacheRead().filter(function (e) { return e.k !== entry.k; });
    list.push(entry);
    while (list.length > SD_PREV_MAX) list.shift();
    while (list.length) {
      try { localStorage.setItem(SD_PREV_KEY, JSON.stringify(list)); return; }
      catch (e) { list.shift(); }   // sin espacio: se descartan las mas viejas
    }
  }

  function showPreviewInfo(d) {
    var w = d.maxX - d.minX || 1, h = d.maxY - d.minY || 1;
    els["sd-preview-dims"].textContent = w.toFixed(1) + " x " + h.toFixed(1) + " mm";
    els["sd-preview-travel"].textContent = (d.travel / 1000).toFixed(2) + " m";
    els["sd-preview-points"].textContent = d.points.toLocaleString();
  }

  function fmtMB(b) { return (b / 1048576).toFixed(1) + " MB"; }

  // Pinta en el panel de detalles el estado del archivo seleccionado (cola, descarga, listo o error)
  function renderPreviewPanel() {
    if (!sdSelectedFile) return;
    var key = prevCacheKey(sdSelectedFile, sdSelectedDir);
    var job = prevJobs[key];
    var entry = prevCacheGet(key);
    var body = els["sd-preview-body"], load = els["sd-preview-loading"], retry = els["sd-preview-load-btn"];
    retry.hidden = true;
    if (entry) {
      load.hidden = true;
      var img = new Image();
      img.onload = function () {
        if (sdSelectedFile && prevCacheKey(sdSelectedFile, sdSelectedDir) !== key) return;
        var cv = els["sd-preview-canvas"];
        cv.width = img.width; cv.height = img.height;
        cv.getContext("2d").drawImage(img, 0, 0);
        sdPreviewCache = { minX: entry.minX, minY: entry.minY, maxX: entry.maxX, maxY: entry.maxY, travel: entry.travel, points: entry.points };
        showPreviewInfo(entry);
        body.hidden = false;
      };
      img.src = entry.png;
      return;
    }
    body.hidden = true;
    sdPreviewCache = null;
    if (!job) { load.hidden = true; return; }
    load.hidden = false;
    if (job.state === "error") {
      load.hidden = true;
      retry.hidden = false;
      retry.textContent = "Error, reintentar";
    } else if (job.state === "queued") {
      els["sd-preview-status"].textContent = tr("En cola") + " · " + (prevQueue.indexOf(key) + 1) + "/" + prevQueue.length;
      els["sd-preview-bar"].style.width = "0%";
    } else {
      var secs = Math.max(0.2, (Date.now() - job.t0) / 1000), rate = job.got / secs;
      var txt = tr("Leyendo archivo...") + " " + fmtMB(job.got);
      if (job.total > 0) {
        var pct = Math.min(100, Math.round(100 * job.got / job.total));
        txt += " / " + fmtMB(job.total) + " (" + pct + " %)";
        els["sd-preview-bar"].style.width = pct + "%";
        if (rate > 0 && job.got > 0) txt += " · " + Math.max(1, Math.round((job.total - job.got) / rate)) + " s";
      } else {
        els["sd-preview-bar"].style.width = "100%";
      }
      txt += " · " + (rate / 1024).toFixed(0) + " KB/s";
      els["sd-preview-status"].textContent = txt;
    }
  }

  // ---------- Miniatura para la pantalla de la maquina (se sube a la SD) ----------
  // La pantalla puede generar la vista previa ella sola, pero con poca RAM y despacio. El navegador ya tiene los
  // datos: construye la misma imagen de 4 bits (0 = fuera del recorrido, 1 = blanco ... 15 = negro) y la sube a
  // /.mx/pv_<hash>.mxp; la pantalla solo la lee. El hash y el formato son los de MKS_preview.cpp (FNV-1a de
  // "ruta" con el tamano exacto del archivo en bytes).
  function mxpHash(path, size) {
    var b = new TextEncoder().encode(path), h = (2166136261 ^ size) >>> 0;
    for (var i = 0; i < b.length; i++) { h ^= b[i]; h = Math.imul(h, 16777619) >>> 0; }
    return ("00000000" + h.toString(16)).slice(-8);
  }

  function buildMxp(data, size) {
    var SIDE = 152;
    var w = data.maxX - data.minX || 0.01, h = data.maxY - data.minY || 0.01;
    var sc = (SIDE - 2) / Math.max(w, h);
    var gw = Math.min(SIDE, Math.floor(w * sc) + 3), gh = Math.min(SIDE, Math.floor(h * sc) + 3);
    var sum = new Float32Array(gw * gh), cnt = new Uint32Array(gw * gh), seg = data.seg, k, o;
    for (k = 0; k < data.nseg; k++) {
      o = k * 5;
      var ax = (seg[o] - data.minX) * sc + 0.5, ay = (gh - 1) - (seg[o + 1] - data.minY) * sc + 0.5;
      var bx = (seg[o + 2] - data.minX) * sc + 0.5, by = (gh - 1) - (seg[o + 3] - data.minY) * sc + 0.5;
      var p = Math.min(1, seg[o + 4] / data.maxS);
      var m = Math.max(Math.abs(bx - ax), Math.abs(by - ay)), steps = m < 1 ? 1 : Math.ceil(m);
      for (var t = 0; t <= steps; t++) {
        var px = Math.floor(ax + (bx - ax) * t / steps), py = Math.floor(ay + (by - ay) * t / steps);
        if (px < 0 || py < 0 || px >= gw || py >= gh) continue;
        sum[py * gw + px] += p; cnt[py * gw + px]++;
      }
    }
    var hist = new Uint32Array(256), seen = 0, i;
    for (i = 0; i < gw * gh; i++) if (cnt[i]) { hist[Math.min(255, Math.floor(sum[i] / cnt[i] * 255))]++; seen++; }
    var hi = 1, acc = 0, lim = seen * 0.995;
    for (i = 0; i < 256; i++) { acc += hist[i]; if (acc >= lim) { hi = Math.max(0.05, (i + 1) / 255); break; } }
    var rowb = (gw + 1) >> 1, out = new Uint8Array(28 + rowb * gh), dv = new DataView(out.buffer);
    dv.setUint32(0, 0x3150584D, true);          // "MXP1"
    dv.setUint32(4, size >>> 0, true);
    dv.setUint16(8, gw, true); dv.setUint16(10, gh, true);
    dv.setFloat32(12, w, true); dv.setFloat32(16, h, true); dv.setFloat32(20, data.travel / 1000, true);
    dv.setUint32(24, data.points >>> 0, true);
    for (var y = 0; y < gh; y++) {
      for (var x = 0; x < gw; x++) {
        var j = y * gw + x, lv = 0;
        if (cnt[j]) { var f = Math.min(1, (sum[j] / cnt[j]) / hi); lv = 1 + Math.floor(14 * Math.pow(f, 0.85) + 0.5); }
        out[28 + y * rowb + (x >> 1)] |= (x & 1) ? lv : (lv << 4);
      }
    }
    return out;
  }

  function uploadMxp(path, size, bytes) {
    if (isRunningState(machineState) || isPausedState(machineState)) return;   // con un trabajo en marcha la SD no se toca
    var name = "pv_" + mxpHash(path, size) + ".mxp", rel = ".mx/" + name;
    fetch("/upload?action=createdir&path=" + encodeURIComponent("/") + "&filename=" + encodeURIComponent(".mx"))
      .catch(function () {})
      .then(function () {
        var fd = new FormData();
        fd.append("path", "/.mx/");
        fd.append(rel + "S", bytes.length);
        fd.append("myfile[]", new Blob([bytes]), rel);
        return fetch("/upload", { method: "POST", body: fd });
      })
      .catch(function () {});   // si falla no pasa nada: la pantalla la genera ella
  }

  function prevFinish(key, name, dir, parser, job) {
    var data = parser.finish();
    var cv = document.createElement("canvas");
    renderGcodePreview(data, cv);
    var entry = { k: key, t: Date.now(), minX: data.minX, minY: data.minY, maxX: data.maxX, maxY: data.maxY,
                  travel: data.travel, points: data.points, png: cv.toDataURL("image/png") };
    job.entry = entry;
    job.state = "done";
    prevCachePut(entry);
    try { uploadMxp(dir + name, job.got, buildMxp(data, job.got)); } catch (e) {}   // para la pantalla de la maquina
  }

  function prevNext() {
    if (prevActive || !prevQueue.length) return;
    var key = prevQueue.shift(), job = prevJobs[key];
    if (!job) return prevNext();
    prevActive = key;
    job.state = "loading";
    job.t0 = Date.now();
    job.got = 0;
    var parser = newGcodePreviewParser();
    var ctl = (typeof AbortController !== "undefined") ? new AbortController() : null;
    job.ctl = ctl;
    var last = 0;
    function done(ok) {
      if (ok) { try { prevFinish(key, job.name, job.dir, parser, job); } catch (e) { job.state = "error"; } }
      else if (job.state !== "cancelled") job.state = "error";
      if (job.state === "cancelled") delete prevJobs[key];
      prevActive = null;
      renderPreviewPanel();
      prevNext();
    }
    fetch("/SD" + sdEnc(job.dir + job.name), ctl ? { signal: ctl.signal } : undefined).then(function (r) {
      if (!r.ok) throw new Error("HTTP " + r.status);
      job.total = parseInt(r.headers.get("Content-Length") || "0", 10) || 0;
      if (!r.body || !r.body.getReader || typeof TextDecoder === "undefined") {
        return r.text().then(function (text) { parser.feed(text); done(true); });
      }
      var reader = r.body.getReader(), dec = new TextDecoder("utf-8");
      function pump() {
        return reader.read().then(function (res) {
          if (res.done) { parser.feed(dec.decode()); done(true); return; }
          job.got += res.value.length;
          parser.feed(dec.decode(res.value, { stream: true }));   // se va leyendo mientras llega
          var now = Date.now();
          if (now - last > 150) { last = now; renderPreviewPanel(); }
          return pump();
        });
      }
      return pump();
    }).catch(function () { done(false); });
  }

  // Pide la vista previa de un archivo (si ya esta hecha o en curso, no hace nada); el seleccionado pasa al frente
  function prevRequest(f) {
    var dir = sdPath === "/" ? "/" : sdPath + "/";
    var key = prevCacheKey(f, dir);
    if (prevCacheGet(key)) return;
    var job = prevJobs[key];
    if (job && (job.state === "loading" || job.state === "queued")) {
      var i = prevQueue.indexOf(key);
      if (i > 0) { prevQueue.splice(i, 1); prevQueue.unshift(key); }   // el que miras va el primero de la cola
      return;
    }
    prevJobs[key] = { state: "queued", got: 0, total: 0, name: f.name, dir: dir };
    prevQueue.unshift(key);
    prevNext();
  }

  // Cancelar la descarga del archivo seleccionado (en curso o en cola) para poder seguir con otro
  els["sd-preview-cancel"].addEventListener("click", function () {
    if (!sdSelectedFile) return;
    var key = prevCacheKey(sdSelectedFile, sdSelectedDir), job = prevJobs[key];
    if (!job) return;
    if (job.state === "queued") {
      var i = prevQueue.indexOf(key);
      if (i >= 0) prevQueue.splice(i, 1);
      delete prevJobs[key];
    } else if (job.state === "loading") {
      job.state = "cancelled";
      if (job.ctl) job.ctl.abort();
    }
    renderPreviewPanel();
  });

  els["sd-preview-load-btn"].addEventListener("click", function () {
    // solo aparece tras un error: reintenta
    if (!sdSelectedFile) return;
    delete prevJobs[prevCacheKey(sdSelectedFile, sdSelectedDir)];
    prevRequest(sdSelectedFile);
    renderPreviewPanel();
  });

  // ---------- Actividad reciente (log real de acciones hechas desde esta interfaz) ----------
  var SD_ACTIVITY_KEY = "dlc32-sd-activity";
  var sdActivityExpanded = false;
  function logSdActivity(text) {
    var list = parseJsonSafe(localStorage.getItem(SD_ACTIVITY_KEY), []);
    if (!Array.isArray(list)) list = [];
    list.unshift({ text: text, time: new Date().toLocaleString() });
    list = list.slice(0, 30);
    localStorage.setItem(SD_ACTIVITY_KEY, JSON.stringify(list));
    renderSdActivity();
  }
  function renderSdActivity() {
    var list = parseJsonSafe(localStorage.getItem(SD_ACTIVITY_KEY), []);
    if (!Array.isArray(list)) list = [];
    if (!list.length) {
      els["sd-activity-list"].innerHTML = '<div class="sd-activity-empty">Sin actividad todavía.</div>';
      return;
    }
    var shown = sdActivityExpanded ? list : list.slice(0, 5);
    els["sd-activity-list"].innerHTML = shown.map(function (item) {
      return '<div class="sd-activity-item"><span>' + escapeHtml(item.text) + "</span><span>" + escapeHtml(item.time) + "</span></div>";
    }).join("");
  }
  els["sd-activity-toggle"].addEventListener("click", function () {
    sdActivityExpanded = !sdActivityExpanded;
    els["sd-activity-toggle"].textContent = sdActivityExpanded ? "Ver menos" : "Ver todo";
    renderSdActivity();
  });
  renderSdActivity();

  els["sd-search"].addEventListener("input", renderSdList);
  els["sd-sort"].addEventListener("change", renderSdList);
  els["sd-page-size"].addEventListener("change", renderSdList);
  els["sd-refresh"].addEventListener("click", loadSdFiles);
  els["sd-newdir-btn"].addEventListener("click", sdCreateDir);

  var RUN_ICON = '<svg viewBox="0 0 24 24" fill="currentColor"><polygon points="5 3 19 12 5 21 5 3"></polygon></svg>';
  var STOP_ICON = '<svg viewBox="0 0 24 24" fill="currentColor"><rect x="5" y="5" width="14" height="14" rx="1"></rect></svg>';

  // Sin trabajo: [Pasadas] [Iniciar]. Con trabajo: el boton pasa a Parar, Pasadas se apaga (gris) y
  // aparece Pausar / Reanudar debajo.
  function updateRunButton(state) {
    var btn = els["sd-run-btn"];
    var active = isRunningState(state) || isPausedState(state);
    btn.classList.toggle("is-stop", active);
    btn.classList.toggle("btn-primary", !active);
    els["sd-run-label"].textContent = active ? "Parar" : "Iniciar trabajo";
    els["sd-run-icon"].innerHTML = active ? STOP_ICON : RUN_ICON;
    els["sd-passes-box"].classList.toggle("is-disabled", active);
    ["sd-passes", "sd-passes-dec", "sd-passes-inc"].forEach(function (id) { els[id].disabled = active; });
    els["sd-pause-row"].hidden = !active;
    els["sd-pause-btn"].textContent = isPausedState(state) ? "Reanudar" : "Pausar";
  }

  // ---------- Pasadas (veces que se repite el archivo) ----------
  function passesValue() {
    var n = parseInt(els["sd-passes"].value, 10);
    if (!(n >= 1)) n = 1;
    if (n > 99) n = 99;
    els["sd-passes"].value = n;
    return n;
  }
  els["sd-passes-dec"].addEventListener("click", function () { els["sd-passes"].value = Math.max(1, passesValue() - 1); });
  els["sd-passes-inc"].addEventListener("click", function () { els["sd-passes"].value = Math.min(99, passesValue() + 1); });
  els["sd-passes"].addEventListener("change", passesValue);

  function stopJob() {
    if (!confirm("¿Detener el trabajo en curso?")) return;
    // Feed hold para decelerar y luego Ctrl-X (0x18): en el firmware
    // mc_reset() apaga el laser, corta I/O y cierra el archivo SD
    // (MotionControl.cpp:532-543).
    sendCommand("!");
    setTimeout(function () { sendCommand("\u0018"); }, 300);
    logSdActivity("Trabajo detenido");
    // Solo tiene sentido marcarlo si había (o se creía que había) trabajo.
    if (currentJob || jobWasBusy) {
      pendingJob = null;
      jobEndReason = "stopped";
      jobWasBusy = false;
      updateJobStatus(machineState);
    }
  }

  els["sd-run-btn"].addEventListener("click", function () {
    if (isRunningState(machineState) || isPausedState(machineState)) { stopJob(); return; }
    if (!sdSelected) return;
    pendingJob = sdSelected;
    jobEndReason = null;
    var passes = passesValue();
    // "$SD/Passes=N" y "$SD/Run=archivo" van en la MISMA peticion (se ejecutan en ese orden): sin esto la
    // placa podria repetir las pasadas que dejo pendientes un trabajo anterior.
    sendCommand("$SD/Passes=" + passes + "\n$SD/Run=" + sdFull(sdSelected));
    logSdActivity("Trabajo iniciado: " + sdSelected + (passes > 1 ? " \u00d7" + passes : ""));
    updateJobStatus("Run");
  });

  els["sd-pause-btn"].addEventListener("click", function () {
    if (isRunningState(machineState)) {
      sendCommand("!");
      logSdActivity("Trabajo pausado");
    } else if (isPausedState(machineState)) {
      sendCommand("~");
      logSdActivity("Trabajo reanudado");
    }
  });

  els["sd-delete-btn"].addEventListener("click", function () {
    if (!sdSelected) return;
    if (!confirm(tr("¿Eliminar ") + sdSelected + tr(" de la SD?"))) return;
    var deletedName = sdSelected;
    fetch("/upload?action=delete&path=" + encodeURIComponent(sdDirParam()) + "&filename=/" + encodeURIComponent(sdSelected))
      .then(function (r) {
        if (!r.ok) throw new Error("HTTP " + r.status);
        sdClearSelection();
        loadSdFiles();
        logSdActivity("Archivo eliminado: " + deletedName);
      })
      .catch(function (err) {
        alert("No se pudo eliminar " + deletedName + ": " + err.message);
      });
  });

  els["sd-download-btn"].addEventListener("click", function () {
    if (!sdSelected) return;
    var a = document.createElement("a");
    a.href = "/SD" + sdEnc(sdFull(sdSelected));
    a.download = sdSelected;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    logSdActivity("Archivo descargado: " + sdSelected);
  });

  els["sd-rename-btn"].addEventListener("click", function () {
    if (!sdSelected) return;
    var newName = prompt("Nuevo nombre para " + sdSelected + ":", sdSelected);
    if (!newName || newName === sdSelected) return;
    var oldName = sdSelected;
    fetch("/upload?action=rename&path=" + encodeURIComponent(sdDirParam()) + "&filename=" + encodeURIComponent(oldName) + "&newname=" + encodeURIComponent(newName))
      .then(function (r) {
        if (!r.ok) throw new Error("HTTP " + r.status);
        sdSelected = newName;
        loadSdFiles();
        logSdActivity("Renombrado: " + oldName + " -> " + newName);
      })
      .catch(function (err) {
        alert("No se pudo renombrar (" + err.message + "). ¿Firmware con soporte de rename?");
      });
  });

  els["sd-frame-btn"].addEventListener("click", function () {
    if (!sdSelected) return;
    if (!sdPreviewCache) { alert('Primero carga la "Vista previa" para conocer el área del trabajo.'); return; }
    var d = sdPreviewCache;
    sendCommand("M5");
    sendCommand("G90");
    sendCommand("G0 X" + d.minX.toFixed(3) + " Y" + d.minY.toFixed(3));
    sendCommand("G0 X" + d.maxX.toFixed(3) + " Y" + d.minY.toFixed(3));
    sendCommand("G0 X" + d.maxX.toFixed(3) + " Y" + d.maxY.toFixed(3));
    sendCommand("G0 X" + d.minX.toFixed(3) + " Y" + d.maxY.toFixed(3));
    sendCommand("G0 X" + d.minX.toFixed(3) + " Y" + d.minY.toFixed(3));
    logSdActivity("Encuadre trazado: " + sdSelected);
  });

  els["sd-upload-btn"].addEventListener("click", function () { els["sd-upload-input"].click(); });
  els["sd-upload-input"].addEventListener("change", function () {
    var files = Array.from(this.files || []);
    if (!files.length) return;
    els["sd-upload-progress"].hidden = false;
    uploadNext(files, 0);
  });

  function uploadOne(file, onProgress) {
    return new Promise(function (resolve, reject) {
      var formData = new FormData();
      // El firmware guarda el archivo con el nombre que recibe: para ir a una carpeta, el nombre lleva la ruta
      var rel = (sdPath === "/" ? "" : sdPath.substring(1) + "/") + file.name;
      formData.append("path", sdDirParam());
      formData.append(rel + "S", file.size);
      formData.append("myfile[]", file, rel);
      var xhr = new XMLHttpRequest();
      xhr.open("POST", "/upload");
      xhr.upload.addEventListener("progress", function (e) {
        if (e.lengthComputable) onProgress(Math.round((e.loaded / e.total) * 100));
      });
      xhr.onload = function () {
        // Sin mirar el status, un 401/500 se colaba como "Archivo subido".
        if (xhr.status >= 200 && xhr.status < 300) {
          resolve();
        } else {
          reject(new Error("HTTP " + xhr.status));
        }
      };
      xhr.onerror = function () { reject(new Error("Error de red")); };
      xhr.send(formData);
    });
  }

  function uploadNext(files, idx) {
    if (idx >= files.length) {
      els["sd-upload-progress"].hidden = true;
      els["sd-upload-progress-fill"].style.width = "0%";
      loadSdFiles();
      return;
    }
    var file = files[idx];
    els["sd-upload-progress-label"].textContent = file.name + " (" + (idx + 1) + " de " + files.length + ")";
    els["sd-upload-progress-fill"].style.width = "0%";
    uploadOne(file, function (pct) {
      els["sd-upload-progress-fill"].style.width = pct + "%";
    })
      .then(function () {
        logSdActivity("Archivo subido: " + file.name);
        uploadNext(files, idx + 1);
      })
      .catch(function (err) {
        // Antes el error se tragaba y se seguia como si hubiera subido.
        logSdActivity("Fallo al subir " + file.name + ": " + err.message);
        alert("No se pudo subir " + file.name + ": " + err.message);
        uploadNext(files, idx + 1);
      });
  }

  // ---------- Info de la placa ----------
  function loadBoardInfo() {
    fetch("/command?commandText=" + encodeURIComponent("[ESP800]"))
      .then(function (r) {
        if (!r.ok) throw new Error("HTTP " + r.status);
        return r.text();
      })
      .then(function (text) {
        boardInfoText = text;
        var fwMatch = /FW version:([^#]+)/.exec(text);
        var hostMatch = /hostname:([^\s#]+)/.exec(text);
        if (fwMatch) {
          els["footer-fw-version"].textContent = "FW " + fwMatch[1].trim();
        }
        if (hostMatch) els["board-hostname"].textContent = hostMatch[1].trim();
        renderAbout();
      })
      .catch(function (err) {
        els["footer-fw-version"].textContent = "FW ?";
      });
    els["pill-ip"].textContent = "IP " + boardHost;
  }

  // ---------- Uptime ----------
  var startTime = Date.now();
  setInterval(function () {
    var secs = Math.floor((Date.now() - startTime) / 1000);
    var h = String(Math.floor(secs / 3600)).padStart(2, "0");
    var m = String(Math.floor((secs % 3600) / 60)).padStart(2, "0");
    var s = String(secs % 60).padStart(2, "0");
    els["uptime-label"].textContent = "Encendido " + h + ":" + m + ":" + s;
  }, 1000);

  // ---------- Settings ----------
  var SETTINGS_CATALOG = {
    "0": { name: "Pulso de paso", unit: "microsegundos", desc: "Duración del pulso de paso. Mínimo 3us.", cat: "movimiento" },
    "1": { name: "Retardo en reposo", unit: "milisegundos", desc: "Retardo antes de deshabilitar motores al detenerse.", cat: "movimiento" },
    "2": { name: "Invertir pulso de paso", unit: "máscara", desc: "Invierte la señal de paso por eje (00000ZYX).", cat: "movimiento" },
    "3": { name: "Invertir dirección de paso", unit: "máscara", desc: "Invierte la señal de dirección por eje (00000ZYX).", cat: "movimiento" },
    "4": { name: "Invertir enable pin", unit: "booleano", desc: "Invierte la señal de habilitación del driver.", cat: "movimiento" },
    "5": { name: "Invertir pines de límite", unit: "booleano", desc: "Invierte todos los pines de entrada de límite.", cat: "limites" },
    "6": { name: "Invertir pin de sonda", unit: "booleano", desc: "Invierte la señal del pin de sonda (probe).", cat: "limites" },
    "10": { name: "Opciones de reporte de estado", unit: "máscara", desc: "Determina qué datos se incluyen en los reportes.", cat: "reportes" },
    "11": { name: "Desviación de unión", unit: "mm", desc: "Qué tan rápido Grbl se mueve entre movimientos consecutivos.", cat: "movimiento" },
    "12": { name: "Tolerancia de arco", unit: "mm", desc: "Precisión de trazado de arcos G2/G3.", cat: "movimiento" },
    "13": { name: "Reportar en pulgadas", unit: "booleano", desc: "Usa pulgadas en vez de mm para posición/velocidad.", cat: "reportes" },
    "20": { name: "Límites suaves", unit: "booleano", desc: "Activa alarma al exceder el espacio de trabajo. Requiere homing.", cat: "limites" },
    "21": { name: "Límites duros", unit: "booleano", desc: "Detiene el movimiento de inmediato al activar un switch.", cat: "limites" },
    "22": { name: "Ciclo de homing", unit: "booleano", desc: "Activa el ciclo de homing. Requiere switches en todos los ejes.", cat: "limites" },
    "23": { name: "Invertir dirección de homing", unit: "máscara", desc: "Busca el switch en dirección negativa por eje (00000ZYX).", cat: "limites" },
    "24": { name: "Velocidad de localización de homing", unit: "mm/min", desc: "Velocidad lenta para ubicar el switch con precisión.", cat: "limites" },
    "25": { name: "Velocidad de búsqueda de homing", unit: "mm/min", desc: "Velocidad rápida para encontrar el switch antes de la fase lenta.", cat: "limites" },
    "26": { name: "Debounce de switch de homing", unit: "milisegundos", desc: "Retardo entre fases del homing.", cat: "limites" },
    "27": { name: "Retiro de homing", unit: "mm", desc: "Distancia de retroceso tras activar el switch.", cat: "limites" },
    "30": { name: "Velocidad máxima de spindle", unit: "RPM", desc: "Velocidad máxima. PWM al 100%.", cat: "laser" },
    "31": { name: "Velocidad mínima de spindle", unit: "RPM", desc: "Velocidad mínima. PWM al 0.4%.", cat: "laser" },
    "32": { name: "Modo láser", unit: "booleano", desc: "Comandos G1/2/3 consecutivos no se detienen al cambiar potencia.", cat: "laser" },
    "28": { name: "Frecuencia PWM del spindle", unit: "Hz", desc: "Frecuencia PWM (requiere reinicio).", cat: "laser" },
    "34": { name: "Valor PWM apagado", unit: "% (float)", desc: "Valor PWM cuando está apagado (requiere reinicio).", cat: "laser" },
    "35": { name: "Valor PWM mínimo", unit: "% (float)", desc: "Valor PWM mínimo (requiere reinicio).", cat: "laser" },
    "36": { name: "Valor PWM máximo", unit: "% (float)", desc: "Valor PWM máximo (requiere reinicio).", cat: "laser" },
    "39": { name: "Sonidos de alerta", unit: "booleano", desc: "Zumbador al terminar un trabajo, al abortarlo por un error y en las alarmas.", cat: "sistema" },
    "40": { name: "Idioma del LCD", unit: "0..3", desc: "Idioma de la pantalla: 0=中文 1=English 2=Português 3=Español. Ver la vista Idioma.", cat: "sistema" },
    "41": { name: "Pluma: altura arriba", unit: "mm", desc: "Altura de la pluma levantada para viajar (plotter).", cat: "pluma" },
    "42": { name: "Pluma: altura abajo", unit: "mm", desc: "Z de dibujo; 0 es la pluma tocando el papel (plotter).", cat: "pluma" },
    "43": { name: "Pluma: velocidad de bajada", unit: "mm/min", desc: "Avance al bajar la pluma (plotter).", cat: "pluma" },
    "44": { name: "Pluma: velocidad de dibujo", unit: "mm/min", desc: "Avance de trazo y de los movimientos manuales (plotter).", cat: "pluma" },
    "80": { name: "Entero de usuario 80", unit: "entero", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "81": { name: "Entero de usuario 81", unit: "entero", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "82": { name: "Entero de usuario 82", unit: "entero", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "83": { name: "Entero de usuario 83", unit: "entero", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "84": { name: "Entero de usuario 84", unit: "entero", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "90": { name: "Flotante de usuario 90", unit: "float", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "91": { name: "Flotante de usuario 91", unit: "float", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "92": { name: "Flotante de usuario 92", unit: "float", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "93": { name: "Flotante de usuario 93", unit: "float", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "94": { name: "Flotante de usuario 94", unit: "float", desc: "Reservado para uso personalizado.", cat: "sistema" },
    "100": { name: "Resolución de eje X", unit: "step/mm", desc: "Pasos por milímetro en X.", cat: "movimiento" },
    "101": { name: "Resolución de eje Y", unit: "step/mm", desc: "Pasos por milímetro en Y.", cat: "movimiento" },
    "102": { name: "Resolución de eje Z", unit: "step/mm", desc: "Pasos por milímetro en Z.", cat: "movimiento" },
    "110": { name: "Velocidad máxima X", unit: "mm/min", desc: "Usada como velocidad rápida (G0).", cat: "movimiento" },
    "111": { name: "Velocidad máxima Y", unit: "mm/min", desc: "Usada como velocidad rápida (G0).", cat: "movimiento" },
    "112": { name: "Velocidad máxima Z", unit: "mm/min", desc: "Usada como velocidad rápida (G0).", cat: "movimiento" },
    "120": { name: "Aceleración X", unit: "mm/s²", desc: "Para no exceder el torque del motor.", cat: "movimiento" },
    "121": { name: "Aceleración Y", unit: "mm/s²", desc: "Para no exceder el torque del motor.", cat: "movimiento" },
    "122": { name: "Aceleración Z", unit: "mm/s²", desc: "Para no exceder el torque del motor.", cat: "movimiento" },
    "130": { name: "Recorrido máximo X", unit: "mm", desc: "Distancia máxima desde el switch de homing.", cat: "movimiento" },
    "131": { name: "Recorrido máximo Y", unit: "mm", desc: "Distancia máxima desde el switch de homing.", cat: "movimiento" },
    "132": { name: "Recorrido máximo Z", unit: "mm", desc: "Distancia máxima desde el switch de homing.", cat: "movimiento" },
    "140": { name: "Corriente motor X", unit: "Amps", desc: "Corriente de operación (drivers SPI/Trinamic).", cat: "sistema" },
    "141": { name: "Corriente motor Y", unit: "Amps", desc: "Corriente de operación (drivers SPI/Trinamic).", cat: "sistema" },
    "142": { name: "Corriente motor Z", unit: "Amps", desc: "Corriente de operación (drivers SPI/Trinamic).", cat: "sistema" },
    "150": { name: "Corriente en reposo X", unit: "%", desc: "Porcentaje de la corriente de operación (SPI/Trinamic).", cat: "sistema" },
    "151": { name: "Corriente en reposo Y", unit: "%", desc: "Porcentaje de la corriente de operación (SPI/Trinamic).", cat: "sistema" },
    "152": { name: "Corriente en reposo Z", unit: "%", desc: "Porcentaje de la corriente de operación (SPI/Trinamic).", cat: "sistema" },
    "160": { name: "Micropasos X", unit: "micros/step", desc: "Microstepping (drivers SPI/Trinamic).", cat: "sistema" },
    "161": { name: "Micropasos Y", unit: "micros/step", desc: "Microstepping (drivers SPI/Trinamic).", cat: "sistema" },
    "162": { name: "Micropasos Z", unit: "micros/step", desc: "Microstepping (drivers SPI/Trinamic).", cat: "sistema" },
    "170": { name: "Stallguard X", unit: "0-255", desc: "Sensibilidad de detección de bloqueo (SPI/Trinamic).", cat: "sistema" },
    "171": { name: "Stallguard Y", unit: "0-255", desc: "Sensibilidad de detección de bloqueo (SPI/Trinamic).", cat: "sistema" },
    "172": { name: "Stallguard Z", unit: "0-255", desc: "Sensibilidad de detección de bloqueo (SPI/Trinamic).", cat: "sistema" }
  };
  var CATEGORY_LABELS = { movimiento: "Movimiento", limites: "Límites", laser: "Láser", pluma: "Pluma", reportes: "Reportes", sistema: "Sistema", otros: "Otros" };
  var CATEGORY_ORDER = ["movimiento", "limites", "laser", "pluma", "reportes", "sistema", "otros"];
  var SETTINGS_BACKUP_KEY = "dlc32-settings-backup";

  var settingsCollector = null;
  var settingsCurrent = {};
  var settingsPending = {};
  var settingsActiveCategory = "";

  // loadSettings() relee $$ de la placa. Al entrar en la vista se pasa
  // preservePending=true: antes se reseteaba settingsPending y un cambio sin
  // guardar desaparecia simplemente por cambiar de pestaña.
  function loadSettings(preservePending) {
    if (!preservePending) settingsPending = {};
    settingsCollector = {
      buffer: {},
      onDone: function (buffer) {
        settingsCurrent = buffer;
        renderSettingsCategories();
        renderSettingsTable();
        updatePendingSummary();
      }
    };
    sendCommand("$$");
    setTimeout(function () {
      if (settingsCollector) {
        var cb = settingsCollector.onDone;
        var buf = settingsCollector.buffer;
        settingsCollector = null;
        cb(buf);
      }
    }, 4000);
  }

  function renderSettingsCategories() {
    var counts = {};
    Object.keys(settingsCurrent).forEach(function (code) {
      var meta = SETTINGS_CATALOG[code];
      var cat = meta ? meta.cat : "otros";
      counts[cat] = (counts[cat] || 0) + 1;
    });
    var total = Object.keys(settingsCurrent).length;
    var html = '<div class="settings-cat-item' + (settingsActiveCategory === "" ? " active" : "") + '" data-cat="">Todas <span class="sd-badge">' + total + "</span></div>";
    CATEGORY_ORDER.forEach(function (cat) {
      if (!counts[cat]) return;
      html += '<div class="settings-cat-item' + (settingsActiveCategory === cat ? " active" : "") + '" data-cat="' + cat + '">' + CATEGORY_LABELS[cat] + ' <span class="sd-badge">' + counts[cat] + "</span></div>";
    });
    els["settings-catlist"].innerHTML = html;
    document.querySelectorAll(".settings-cat-item").forEach(function (el) {
      el.addEventListener("click", function () {
        settingsActiveCategory = el.dataset.cat;
        renderSettingsCategories();
        renderSettingsTable();
      });
    });
    var filterHtml = '<option value="">Todas las categorías</option>';
    CATEGORY_ORDER.forEach(function (cat) {
      if (counts[cat]) filterHtml += '<option value="' + cat + '">' + CATEGORY_LABELS[cat] + "</option>";
    });
    els["settings-category-filter"].innerHTML = filterHtml;
    els["settings-category-filter"].value = settingsActiveCategory;
  }

  function renderSettingsTable() {
    var query = (els["settings-search"].value || "").toLowerCase();
    var codes = Object.keys(settingsCurrent).sort(function (a, b) { return parseInt(a, 10) - parseInt(b, 10); });
    var rows = "";
    codes.forEach(function (code) {
      var meta = SETTINGS_CATALOG[code] || { name: "Parámetro $" + code, unit: "--", desc: "Sin descripción disponible en el catálogo.", cat: "otros" };
      if (settingsActiveCategory && meta.cat !== settingsActiveCategory) return;
      if (query && meta.name.toLowerCase().indexOf(query) === -1 && code.indexOf(query) === -1) return;
      var value = settingsPending.hasOwnProperty(code) ? settingsPending[code] : settingsCurrent[code];
      var changed = settingsPending.hasOwnProperty(code) && settingsPending[code] !== settingsCurrent[code];
      rows +=
        "<tr" + (changed ? ' class="settings-row-changed"' : "") + ">" +
        '<td class="settings-code">$' + code + "</td>" +
        '<td><input type="text" class="settings-value-input" data-code="' + code + '" value="' + escapeHtml(value) + '"></td>' +
        "<td>" + escapeHtml(meta.unit) + "</td>" +
        "<td>" + escapeHtml(meta.name) + "</td>" +
        '<td class="settings-desc">' + escapeHtml(meta.desc) + "</td>" +
        "</tr>";
    });
    els["settings-table-body"].innerHTML = rows || '<tr><td colspan="5" class="settings-empty">Sin resultados.</td></tr>';
    document.querySelectorAll(".settings-value-input").forEach(function (input) {
      input.addEventListener("change", function () {
        var code = this.dataset.code;
        if (this.value === settingsCurrent[code]) delete settingsPending[code];
        else settingsPending[code] = this.value;
        updatePendingSummary();
        renderSettingsTable();
      });
    });
  }

  function updatePendingSummary() {
    var codes = Object.keys(settingsPending);
    els["settings-pending-count"].textContent = codes.length;
    els["settings-save-btn"].disabled = codes.length === 0;
    els["settings-pending-hint"].hidden = codes.length === 0;
    els["settings-pending-list"].innerHTML = codes.map(function (code) {
      var meta = SETTINGS_CATALOG[code];
      return (
        '<div class="settings-pending-row"><span class="settings-code">$' + code + "</span> <span>" + (meta ? escapeHtml(meta.name) : "") +
        '</span> <span class="settings-pending-change">' + escapeHtml(settingsCurrent[code]) + " &rarr; " + escapeHtml(settingsPending[code]) + "</span></div>"
      );
    }).join("");
  }

  // ---------- Vista: Language (idioma del LCD, $40) ----------
  // El firmware acepta 0..3 (SettingsDefinitions.cpp): 0=cn 1=en 2=de 3=es.
  // Al escribir $40 el refresco periodico de MKS_FREERTOS_TASK.cpp recarga
  // las cadenas del LCD, asi que no hace falta reiniciar.
  var LANG_NAMES = {
    "0": "中文 · Chino",
    "1": "English · Inglés",
    "2": "Português · Portugués",
    "3": "Español"
  };

  function renderLanguage(value) {
    var badge = document.getElementById("lang-current");
    if (badge) {
      badge.textContent = value === null ? "Desconocido" : (LANG_NAMES[value] || ("$40=" + value));
    }
    document.querySelectorAll(".lang-option[data-lang]").forEach(function (b) {
      b.classList.toggle("active", value !== null && b.dataset.lang === String(value));
    });
  }

  // Lee $40 con el mismo mecanismo que $$: settingsCollector acumula las
  // lineas "clave=valor" hasta que llega "ok".
  var languageRetried = false;
  function loadLanguage(onDone) {
    var badge = document.getElementById("lang-current");
    if (badge) badge.textContent = "Leyendo…";

    var collector = {
      buffer: {},
      onDone: function (buf) {
        var v = null;
        if (buf["40"] !== undefined) {
          var p = parseInt(buf["40"], 10);
          v = isNaN(p) ? null : String(p);
        }
        if (v === null && !languageRetried) {   // sin respuesta: un reintento antes de decir "Desconocido"
          languageRetried = true;
          setTimeout(function () { loadLanguage(onDone); }, 300);
          return;
        }
        languageRetried = false;
        renderLanguage(v);
        if (onDone) onDone(v);
      }
    };
    settingsCollector = collector;
    sendCommand("$40");
    setTimeout(function () {
      // Solo si seguimos siendo el recolector activo (no pisar a loadSettings)
      if (settingsCollector === collector) {
        settingsCollector = null;
        collector.onDone(collector.buffer);
      }
    }, 2500);
  }

  function setLanguage(value) {
    var msg = document.getElementById("lang-msg");
    if (msg) msg.textContent = "Enviando $40=" + value + "…";
    sendCommandWithResult("$40=" + value, function (result) {
      if (result && /^error:/i.test(result)) {
        if (msg) msg.textContent = "Error del controlador: " + result;
        loadLanguage();
        return;
      }
      if (result === null) {
        if (msg) msg.textContent = "Sin respuesta del controlador (timeout).";
        loadLanguage();
        return;
      }
      // El "ok" del comando no garantiza la escritura: hubo un caso en el que
      // la WebUI anunció el cambio y el controlador se quedó en $40=1. Asi
      // que volvemos a leer $40 y solo entonces damos por bueno el cambio.
      if (msg) msg.textContent = "Escrito. Comprobando $40…";
      loadLanguage(function (v) {
        if (!msg) return;
        if (v === String(value)) {
          msg.textContent = "$40=" + v + " → " + tr(LANG_NAMES[String(v)] || "") +
            tr(". Textos del LCD recargados; si una página no se actualiza, entra y sales de ella.");
        } else {
          msg.textContent = v === null
            ? tr("No se pudo leer $40 tras escribirlo. Vuelve a intentarlo.")
            : tr("Aviso: el controlador se quedó en ") + "$40=" + v +
              tr(". El cambio no se aplicó; vuelve a intentarlo.");
        }
      });
    });
  }

  document.querySelectorAll(".lang-option[data-lang]").forEach(function (btn) {
    btn.addEventListener("click", function () {
      setLanguage(parseInt(btn.dataset.lang, 10));
    });
  });

  // Idioma de la propia WebUI: solo afecta a este navegador (localStorage).
  document.querySelectorAll(".lang-option[data-uilang]").forEach(function (btn) {
    btn.addEventListener("click", function () {
      i18nSetLang(btn.dataset.uilang);
    });
  });

  // ---------- Actualizar WebUI: sube index.html.gz a la memoria interna (SPIFFS) ----------
  (function () {
    var btn = document.getElementById("webui-update-btn");
    var input = document.getElementById("webui-update-input");
    var msg = document.getElementById("webui-update-msg");
    var prog = document.getElementById("webui-update-progress");
    var fill = document.getElementById("webui-update-fill");
    if (!btn || !input) return;

    function say(text, cls) {
      msg.textContent = text;
      msg.className = "about-msg" + (cls ? " " + cls : "");
    }

    function gzipBlob(file) {
      if (typeof CompressionStream === "undefined") return Promise.reject(new Error("nocomp"));
      var stream = file.stream().pipeThrough(new CompressionStream("gzip"));
      return new Response(stream).blob();
    }

    function send(blob) {
      return new Promise(function (resolve, reject) {
        var name = "index.html.gz";
        var fd = new FormData();
        fd.append("path", "/");
        fd.append(name + "S", blob.size);
        fd.append("myfile[]", blob, name);
        var xhr = new XMLHttpRequest();
        xhr.open("POST", "/files");
        xhr.upload.addEventListener("progress", function (e) {
          if (e.lengthComputable) fill.style.width = Math.round((e.loaded / e.total) * 100) + "%";
        });
        xhr.onload = function () {
          if (xhr.status >= 200 && xhr.status < 300) resolve();
          else reject(new Error("HTTP " + xhr.status));
        };
        xhr.onerror = function () { reject(new Error("Error de red")); };
        xhr.send(fd);
      });
    }

    btn.addEventListener("click", function () {
      if (jobWasBusy) { say("Detén el trabajo antes de actualizar la WebUI.", "is-error"); return; }
      input.value = "";
      input.click();
    });

    input.addEventListener("change", function () {
      var file = input.files && input.files[0];
      if (!file) return;
      var lower = file.name.toLowerCase();
      var isGz = /\.gz$/.test(lower);
      if (!isGz && !/\.html?$/.test(lower)) { say("Elige un archivo .gz o .html.", "is-error"); return; }
      if (!confirm("¿Reemplazar la WebUI de la placa por el archivo seleccionado? Si el archivo está dañado tendrás que usar la página de emergencia (?index=yes).")) return;

      var prepare = isGz
        ? file.slice(0, 2).arrayBuffer().then(function (b) {
            var h = new Uint8Array(b);
            if (h[0] !== 0x1f || h[1] !== 0x8b) throw new Error("nogz");
            return file;
          })
        : gzipBlob(file);

      say("Subiendo WebUI…");
      fill.style.width = "0%";
      prog.hidden = false;
      prepare.then(send).then(function () {
        say("WebUI actualizada. Recargando…", "is-ok");
        setTimeout(function () { location.reload(); }, 1500);
      }).catch(function (err) {
        prog.hidden = true;
        if (err && err.message === "nogz") say("El archivo no es un .gz válido.", "is-error");
        else if (err && err.message === "nocomp") say("Tu navegador no puede comprimir el .html: sube un index.html.gz.", "is-error");
        else say("No se pudo actualizar la WebUI", "is-error");
      });
    });
  })();

  var pendingCmdResult = null;
  function sendCommandWithResult(cmd, callback) {
    pendingCmdResult = { onResult: callback };
    sendCommand(cmd);
    setTimeout(function () {
      if (pendingCmdResult) {
        var cb = pendingCmdResult.onResult;
        pendingCmdResult = null;
        cb(null);
      }
    }, 2000);
  }

  function applySettingsValues(map, onDone) {
    var codes = Object.keys(map);
    var errors = [];
    function next(idx) {
      if (idx >= codes.length) { onDone(errors); return; }
      sendCommandWithResult("$" + codes[idx] + "=" + map[codes[idx]], function (result) {
        if (result && /^error:/i.test(result)) errors.push({ code: codes[idx], result: result });
        next(idx + 1);
      });
    }
    next(0);
  }

  function updateValidationBadges(errors) {
    var badge = els["settings-validation-syntax"];
    if (!errors || !errors.length) {
      badge.textContent = "Confirmado por el controlador";
      badge.style.background = "";
      badge.style.color = "";
      els["settings-validation-errors-row"].hidden = true;
    } else {
      badge.textContent = errors.length + " rechazado(s)";
      badge.style.background = "#fee2e2";
      badge.style.color = "#dc2626";
      els["settings-validation-errors-row"].hidden = false;
      els["settings-validation-detail"].textContent = errors.map(function (e) { return "$" + e.code + ": " + e.result; }).join(", ");
    }
  }

  els["settings-search"].addEventListener("input", renderSettingsTable);
  els["settings-category-filter"].addEventListener("change", function () {
    settingsActiveCategory = this.value;
    renderSettingsCategories();
    renderSettingsTable();
  });

  els["settings-save-btn"].addEventListener("click", function () {
    var codes = Object.keys(settingsPending);
    if (!codes.length) return;
    localStorage.setItem(SETTINGS_BACKUP_KEY, JSON.stringify(settingsCurrent));
    applySettingsValues(settingsPending, function (errors) {
      settingsPending = {};
      loadSettings();
      updateValidationBadges(errors);
    });
  });

  els["settings-restore-btn"].addEventListener("click", function () {
    var raw = localStorage.getItem(SETTINGS_BACKUP_KEY);
    if (!raw) { alert("Todavía no hay un respaldo guardado (se crea automáticamente la primera vez que guardas cambios)."); return; }
    if (!confirm("¿Restaurar los valores previos al último \"Guardar cambios\"?")) return;
    var backup = parseJsonSafe(raw, null);
    if (!backup || typeof backup !== "object") { alert("El respaldo local está corrupto; no se restaura."); return; }
    applySettingsValues(backup, function () { loadSettings(); });
  });

  els["settings-export-btn"].addEventListener("click", function () {
    var lines = Object.keys(settingsCurrent)
      .sort(function (a, b) { return parseInt(a, 10) - parseInt(b, 10); })
      .map(function (c) { return "$" + c + "=" + settingsCurrent[c]; });
    var blob = new Blob([lines.join("\n")], { type: "text/plain" });
    var url = URL.createObjectURL(blob);
    var a = document.createElement("a");
    a.href = url;
    a.download = "dlc32-settings-" + Date.now() + ".txt";
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
  });

  els["settings-import-btn"].addEventListener("click", function () { els["settings-import-input"].click(); });
  els["settings-import-input"].addEventListener("change", function () {
    var file = this.files && this.files[0];
    this.value = "";
    if (!file) return;
    var reader = new FileReader();
    reader.onload = function () {
      var map = {};
      String(reader.result).split(/\r?\n/).forEach(function (line) {
        var m = /^\$(\d+)=(.+)$/.exec(line.trim());
        if (m) map[m[1]] = m[2].trim();
      });
      var codes = Object.keys(map);
      if (!codes.length) { alert("El archivo no tiene líneas $N=valor válidas."); return; }
      if (!confirm("Se van a aplicar " + codes.length + " parámetros a la placa. ¿Continuar?")) return;
      applySettingsValues(map, function (errors) {
        loadSettings();
        updateValidationBadges(errors);
        logSdActivity && logSdActivity("Configuración importada (" + codes.length + " parámetros)");
      });
    };
    reader.readAsText(file);
  });

  els["settings-defaults-btn"].addEventListener("click", function () {
    if (!confirm('Esto restaura TODOS los parámetros $ a los valores de fábrica del firmware (comando $RST=$). ¿Continuar?')) return;
    sendCommand("$RST=$");
    setTimeout(loadSettings, 1000);
  });

  // ---------- Init ----------
  i18nStart();
  i18nSetLang(UI_LANG);
  connectWs();
  loadBoardInfo();
  setInterval(function () {
    if (ws && ws.readyState === WebSocket.OPEN) sendCommand("?", true);  // sondeo: sin eco en la consola
  }, 2000);
})();
