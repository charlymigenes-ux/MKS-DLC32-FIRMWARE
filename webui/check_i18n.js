/**
 * Comprueba que la WebUI no tenga textos en español sin traducir.
 *
 *   node webui/check_i18n.js
 *
 * Extrae los textos de index.html (nodos de texto + placeholder/title/alt) y
 * los literales de js/app.js, y para cada uno mira si i18nLookup() lo cambia.
 * Los que no se quedan (y no son tecnicos) se listan como "sin cubrir".
 *
 * Sale con codigo 1 si hay algo sin cubrir, para poder usarlo en CI.
 *
 * Ojo: sin "use strict" a proposito, para que el eval de i18n.js deje
 * I18N_EN e i18nLookup en el ambito de este script.
 */

var fs = require("fs");
var path = require("path");

var dir = __dirname;
var i18nSrc = fs.readFileSync(path.join(dir, "js", "i18n.js"), "utf8");
eval(i18nSrc); // define I18N_EN e i18nLookup

/* Cosas que son tecnicas o estan ya en ingles y no se tocan. */
var SKIP = new Set([
  // marcadores / unidades / codigos
  "A+", "A-", "EN", "ES", "FW --", "FW ?", "FW", "IP --", "IP", "HTTP",
  "mm", "mm/min", "mm/s²", "µm", "RPM", "Amps", "step/mm", "micros/step",
  "0-255", "0..3", "%", "% (float)", "float", "X", "Y", "MX", "MY",
  "Z+", "Z-", "F (Feed)", "S (Spindle/Light)", "MPos", "WPos",
  "$40=0", "$40=1", "$40=2", "$40=3", "→", "—", "⇩", "⌖", "▲", "▼", "✎",
  "中文", "Deutsch", "English", "MKS DLC32", "MKS DLC32 - ESP32-WEB", "ESP32-WEB",
  "MKS DLC32 Firmware", "MKS DLC32 (Makerbase)", "Grbl_ESP32", "Grbl", "LVGL",
  "GNU General Public License v3",
  "Control", "Autoscroll", "OFF", "ON", "Macro 1", "Macro 2", "Macro 3",
  "dlc32Font", "language_en.h", "language_es.h", "SD", "Uptime",
  // nombres oficiales de Grbl (se dejan en ingles en los dos idiomas)
  "Step pulse time", "Step idle delay", "Step pulse invert",
  "Step direction invert", "Stallguard X", "Stallguard Y", "Stallguard Z",
  // literales de codigo / respuestas del firmware
  "Ok", "ok", "error", "files", "No SD Card", "Busy", "active",
  "use strict", "G0 X", "G90 G0 Z0", "G92 X0 Y0", "M3 S", "Ctrl-X (reset)",
  "[error]", "[error] HTTP",
  // comentarios y restos que no llegan al DOM
  "Ultima actualizacion: 12:00:01", "Archivo subido",
  "¿Eliminar fichero.gcode de la SD?",
  // catValue del catalogo (solo se usa como clave interna)
  "otros", "limites", "reportes", "movimiento", "laser", "sistema"
]);

function isCodey(s) {
  return /^[\d\s.,:%+\-*/×÷<>#$°{}()[\]|]*$/.test(s) || s.length < 2 ||
    /^[a-z0-9]+(-[a-z0-9]+)+$/.test(s);          // ids/selectores: "stat-y"
}

function looksSpanish(s) {
  if (/[áéíóúñ¿¡ÁÉÍÓÚÑ]/.test(s)) return true;
  if (s.length < 5) return false;                // no confiar en "en", "de"...
  return /\b(de|la|el|los|las|un|una|unos|unas|por|para|con|sin|del|al|se|en|que|y|o|no|mas|más|esta|este|esto|estos|estas|desde|hasta|sobre|cuando|donde|como|cómo|qué|cual|cuál|hoy|son|está|estan|están|hay|puede|debe|todo|todos|todas|también|tambien|cada|otro|otra)\b/.test(s);
}

function normalize(s) {
  // El DOM ya decodifica las entidades y quita las etiquetas: aqui hacemos lo
  // mismo para que la comparacion sea con el texto tal y como se ve en pantalla.
  return s
    .replace(/&#(\d+);/g, function (_, n) { return String.fromCharCode(parseInt(n, 10)); })
    .replace(/&[a-z]+;/gi, " ")
    .replace(/<[^>]*>/g, " ")
    .replace(/\s+/g, " ")
    .trim();
}

function check(label, s) {
  var t = normalize(s);
  if (!t || isCodey(t) || SKIP.has(t)) return null;
  if (i18nLookup(t) !== t) return null;      // tiene traduccion
  if (!looksSpanish(t)) return null;         // no pinta aqui (ingles/tecnico)
  return { from: label, text: t };
}

function extractHtml(file) {
  var html = fs.readFileSync(file, "utf8");
  html = html.replace(/<script[\s\S]*?<\/script>/gi, " ")
             .replace(/<style[\s\S]*?<\/style>/gi, " ");
  var out = [];
  html.replace(/<[^>]+>/g, "\n").split("\n").forEach(function (chunk) {
    var r = check("index.html", chunk);
    if (r) out.push(r);
  });
  var attrRe = /\b(placeholder|title|alt|aria-label)="([^"]*)"/g;
  var m;
  while ((m = attrRe.exec(html)) !== null) {
    var r2 = check("index.html@" + m[1], m[2]);
    if (r2) out.push(r2);
  }
  return out;
}

function extractJs(file) {
  var src = fs.readFileSync(file, "utf8");
  var lits = src.match(/'(?:[^'\\\n]|\\.)*'|"(?:[^"\\\n]|\\.)*"/g) || [];
  var out = [];
  lits.forEach(function (lit) {
    var s = lit.slice(1, -1)
      .replace(/\\'/g, "'").replace(/\\"/g, '"').replace(/\\n/g, " ");
    var r = check("app.js", s);
    if (r) out.push(r);
  });
  return out;
}

function findDuplicates() {
  var seen = new Map();
  var dups = [];
  i18nSrc.split("\n").forEach(function (line) {
    var m = /^\s*"((?:[^"\\]|\\.)*)":/.exec(line);
    if (!m) return;
    if (seen.has(m[1])) dups.push(m[1]);
    seen.set(m[1], true);
  });
  return dups;
}

var missing = extractHtml(path.join(dir, "index.html"))
  .concat(extractJs(path.join(dir, "js", "app.js")));

var dups = findDuplicates();
var keys = Object.keys(I18N_EN);

console.log("claves del diccionario: " + keys.length);
if (dups.length) {
  console.log("CLAVES DUPLICADAS (" + dups.length + "):");
  dups.forEach(function (k) { console.log("  ! " + k); });
}
if (missing.length) {
  console.log("SIN CUBRIR (" + missing.length + "):");
  missing.forEach(function (r) { console.log("  - [" + r.from + "] " + r.text); });
} else {
  console.log("sin cubrir: 0");
}
process.exit(missing.length || dups.length ? 1 : 0);
