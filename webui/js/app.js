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
    if (savedLang === "en" || savedLang === "es") UI_LANG = savedLang;
  } catch (e) {}

  var I18N_ATTRS = ["placeholder", "title", "alt", "aria-label"];

  // El diccionario y el buscador viven en js/i18n.js, que se inlina antes que
  // este fichero. Si faltara, la interfaz sigue funcionando: simplemente no
  // cambia de idioma.
  var i18nDict = typeof i18nLookup === "function" ? i18nLookup : function (t) { return t; };

  function tr(text) {
    if (UI_LANG !== "en" || typeof text !== "string") return text;
    return i18nDict(text);
  }

  function i18nTextNode(node) {
    var cur = node.nodeValue;
    if (typeof cur !== "string") return;

    if (UI_LANG !== "en") {
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

    if (UI_LANG !== "en") {
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
    if (node.tagName === "SCRIPT" || node.tagName === "STYLE") return;
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
    if (lang !== "en" && lang !== "es") return;
    UI_LANG = lang;
    try { localStorage.setItem("dlc32.lang", lang); } catch (e) {}
    i18nApplyAll();
    document.querySelectorAll(".lang-option[data-uilang]").forEach(function (b) {
      b.classList.toggle("active", b.dataset.uilang === lang);
    });
    var badge = document.getElementById("uilang-current");
    if (badge) badge.textContent = lang === "en" ? "English" : "Español";
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

  function handleLine(line) {
    if (!line) return;
    appendConsole(line);
    if (line[0] === "<" && line[line.length - 1] === ">") {
      parseStatusReport(line);
    }
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
  function sendCommand(cmd) {
    if (!cmd || !cmd.trim()) return;
    appendConsole("[#]" + (cmd === "\u0018" ? "Ctrl-X (reset)" : cmd));
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
        sendCommand("M3 S" + sValue);
      }
    });
  });

  // ---------- SD File ----------
  var sdFiles = [];
  var sdSelected = null;

  function loadSdFiles() {
    fetch("/upload?path=/")
      .then(function (r) { return r.json(); })
      .then(function (data) {
        sdFiles = (data.files || []).filter(function (f) { return f.size !== "-1"; });
        renderSdSpace(data);
        renderSdList();
      })
      .catch(function () {
        els["sd-space-label"].textContent = "Error al leer la SD";
      });
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
      if (sortMode === "name-asc") return a.name.localeCompare(b.name);
      if (sortMode === "name-desc") return b.name.localeCompare(a.name);
      if (sortMode === "size-desc") return sizeToBytes(b.size) - sizeToBytes(a.size);
      if (sortMode === "size-asc") return sizeToBytes(a.size) - sizeToBytes(b.size);
      return 0;
    });

    list = list.slice(0, pageSize);

    var tbody = els["sd-table-body"];
    tbody.innerHTML = "";
    list.forEach(function (f) {
      var row = document.createElement("tr");
      row.dataset.name = f.name;
      if (sdSelected === f.name) row.classList.add("selected");
      row.innerHTML =
        '<td>&#128196;</td>' +
        '<td>' + escapeHtml(f.name) + '</td>' +
        '<td>' + escapeHtml(f.size) + '</td>' +
        '<td><span class="sd-badge">Listo</span></td>';
      row.addEventListener("click", function () { selectSdFile(f); });
      tbody.appendChild(row);
    });
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

  function selectSdFile(f) {
    sdSelected = f.name;
    sdPreviewCache = null;
    renderSdList();
    els["sd-details-empty"].hidden = true;
    els["sd-details-body"].hidden = false;
    els["sd-detail-name"].textContent = f.name;
    els["sd-detail-size"].textContent = f.size;
    els["sd-detail-modified"].textContent = f.datetime || "--";
    els["sd-preview-body"].hidden = true;
    els["sd-preview-load-btn"].hidden = false;
    els["sd-preview-load-btn"].textContent = "Cargar vista previa";
    els["sd-preview-svg"].innerHTML = "";
  }

  // ---------- Vista previa: parseo real de G-code, sin datos inventados ----------
  function parseGcodePreview(text) {
    var lines = text.split(/\r?\n/);
    var x = 0, y = 0, relative = false;
    var minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
    var points = [[0, 0]];
    var travel = 0;
    lines.forEach(function (raw) {
      var line = raw.split(";")[0].split("(")[0].trim();
      if (!line) return;
      if (/\bG91\b/.test(line)) relative = true;
      if (/\bG90\b/.test(line)) relative = false;
      if (!/\bG0?[0-3]\b/.test(line)) return;
      var xMatch = /X(-?[\d.]+)/.exec(line);
      var yMatch = /Y(-?[\d.]+)/.exec(line);
      if (!xMatch && !yMatch) return;
      var nx = xMatch ? (relative ? x + parseFloat(xMatch[1]) : parseFloat(xMatch[1])) : x;
      var ny = yMatch ? (relative ? y + parseFloat(yMatch[1]) : parseFloat(yMatch[1])) : y;
      travel += Math.sqrt((nx - x) * (nx - x) + (ny - y) * (ny - y));
      x = nx;
      y = ny;
      minX = Math.min(minX, x);
      maxX = Math.max(maxX, x);
      minY = Math.min(minY, y);
      maxY = Math.max(maxY, y);
      points.push([x, y]);
    });
    if (!isFinite(minX)) { minX = 0; maxX = 0; minY = 0; maxY = 0; }
    return { minX: minX, minY: minY, maxX: maxX, maxY: maxY, points: points, travel: travel };
  }

  function renderGcodePreview(data) {
    var w = data.maxX - data.minX || 1;
    var h = data.maxY - data.minY || 1;
    var scale = Math.min(180 / w, 180 / h);
    var offX = 10 + (180 - w * scale) / 2;
    var offY = 10 + (180 - h * scale) / 2;
    function mapPt(p) {
      var px = (p[0] - data.minX) * scale + offX;
      var py = 200 - ((p[1] - data.minY) * scale + offY);
      return px.toFixed(1) + "," + py.toFixed(1);
    }
    var pts = data.points;
    if (pts.length > 4000) {
      var step = Math.ceil(pts.length / 4000);
      var sampled = [];
      for (var i = 0; i < pts.length; i += step) sampled.push(pts[i]);
      pts = sampled;
    }
    var d = "M" + pts.map(mapPt).join(" L");
    els["sd-preview-svg"].innerHTML = '<path d="' + d + '"></path>';
    els["sd-preview-dims"].textContent = w.toFixed(1) + " x " + h.toFixed(1) + " mm";
    els["sd-preview-travel"].textContent = (data.travel / 1000).toFixed(2) + " m";
    els["sd-preview-points"].textContent = data.points.length.toLocaleString();
  }

  els["sd-preview-load-btn"].addEventListener("click", function () {
    if (!sdSelected) return;
    els["sd-preview-load-btn"].hidden = true;
    els["sd-preview-loading"].hidden = false;
    fetch("/SD/" + encodeURIComponent(sdSelected))
      .then(function (r) { return r.text(); })
      .then(function (text) {
        sdPreviewCache = parseGcodePreview(text);
        renderGcodePreview(sdPreviewCache);
        els["sd-preview-loading"].hidden = true;
        els["sd-preview-body"].hidden = false;
      })
      .catch(function () {
        els["sd-preview-loading"].hidden = true;
        els["sd-preview-load-btn"].hidden = false;
        els["sd-preview-load-btn"].textContent = "Error, reintentar";
      });
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

  function updateRunButton(state) {
    var btn = els["sd-run-btn"];
    btn.classList.remove("is-running", "is-paused");
    if (isRunningState(state)) {
      els["sd-run-label"].textContent = "Pausar";
      btn.classList.add("is-running");
    } else if (isPausedState(state)) {
      els["sd-run-label"].textContent = "Reanudar";
      btn.classList.add("is-paused");
    } else {
      els["sd-run-label"].textContent = "Iniciar trabajo";
    }
  }

  els["sd-run-btn"].addEventListener("click", function () {
    if (isRunningState(machineState)) {
      sendCommand("!");
      logSdActivity("Trabajo pausado");
      return;
    }
    if (isPausedState(machineState)) {
      sendCommand("~");
      logSdActivity("Trabajo reanudado");
      return;
    }
    if (!sdSelected) return;
    pendingJob = sdSelected;
    jobEndReason = null;
    sendCommand("$SD/Run=" + sdSelected);
    logSdActivity("Trabajo iniciado: " + sdSelected);
    updateJobStatus("Run");
  });

  els["sd-stop-btn"].addEventListener("click", function () {
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
  });
  els["sd-delete-btn"].addEventListener("click", function () {
    if (!sdSelected) return;
    if (!confirm(tr("¿Eliminar ") + sdSelected + tr(" de la SD?"))) return;
    var deletedName = sdSelected;
    fetch("/upload?action=delete&filename=/" + encodeURIComponent(sdSelected))
      .then(function (r) {
        if (!r.ok) throw new Error("HTTP " + r.status);
        sdSelected = null;
        els["sd-details-empty"].hidden = false;
        els["sd-details-body"].hidden = true;
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
    a.href = "/SD/" + encodeURIComponent(sdSelected);
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
    fetch("/upload?action=rename&filename=" + encodeURIComponent(oldName) + "&newname=" + encodeURIComponent(newName))
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
      var fullName = "/" + file.name;
      formData.append("path", "/");
      formData.append(fullName + "S", file.size);
      formData.append("myfile[]", file, file.name);
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
    "33": { name: "Frecuencia PWM del spindle", unit: "% (float)", desc: "Frecuencia PWM (requiere reinicio).", cat: "laser" },
    "34": { name: "Valor PWM apagado", unit: "% (float)", desc: "Valor PWM cuando está apagado (requiere reinicio).", cat: "laser" },
    "35": { name: "Valor PWM mínimo", unit: "% (float)", desc: "Valor PWM mínimo (requiere reinicio).", cat: "laser" },
    "36": { name: "Valor PWM máximo", unit: "% (float)", desc: "Valor PWM máximo (requiere reinicio).", cat: "laser" },
    "40": { name: "Idioma del LCD", unit: "0..3", desc: "Idioma de la pantalla: 0=中文 1=English 2=Deutsch 3=Español. Ver la vista Idioma.", cat: "sistema" },
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
  var CATEGORY_LABELS = { movimiento: "Movimiento", limites: "Límites", laser: "Láser", reportes: "Reportes", sistema: "Sistema", otros: "Otros" };
  var CATEGORY_ORDER = ["movimiento", "limites", "laser", "reportes", "sistema", "otros"];
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
    "2": "Deutsch · Alemán",
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
    if (ws && ws.readyState === WebSocket.OPEN) sendCommand("?");
  }, 2000);
})();
