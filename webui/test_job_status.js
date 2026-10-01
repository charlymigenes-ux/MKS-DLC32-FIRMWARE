/**
 * Prueba de la maquina de estados del panel de trabajo de la WebUI.
 *
 * Extrae de webui/js/app.js el bloque STATE_TEXT + updateJobStatus() y lo
 * ejecuta contra un DOM de mentira, alimentandolo con los informes que manda
 * el firmware (<Run|...>, <Hold:0|...>, <Idle|...>...). Asi se comprueban los
 * cinco estados (en curso, pausa, finalizado, detenido, inactivo) sin tener
 * que arrancar un trabajo real en la maquina.
 *
 *   node webui/test_job_status.js
 */
var fs = require("fs");
var path = require("path");

var appjs = path.join(__dirname, "js", "app.js");
var src = fs.readFileSync(appjs, "utf8");

var i0 = src.indexOf("var STATE_TEXT");
var i1 = src.indexOf("function updateJobStatus");
if (i0 < 0 || i1 < 0 || i1 < i0) {
  console.error("no se localizo el bloque en app.js");
  process.exit(2);
}

// fin de la funcion: emparejando llaves
var j = src.indexOf("{", i1), k = j, depth = 0;
for (; k < src.length; k++) {
  if (src[k] === "{") depth++;
  else if (src[k] === "}") { depth--; if (depth === 0) { k++; break; } }
}
var bloque = src.slice(i0, k);

var modulo = [
  "var elements = {",
  '  "job-status": {className: ""},',
  '  "job-status-text": {textContent: ""},',
  '  "job-status-file": {textContent: "", hidden: true}',
  "};",
  "var document = {",
  "  getElementById: function (id) { return elements[id] || null; }",
  "};",
  bloque,
  "module.exports = {",
  "  updateJobStatus: updateJobStatus,",
  "  set: function (n, v) { eval(n + \" = v\"); },",
  "  get: function () { return {currentJob: currentJob, pendingJob: pendingJob,",
  "    jobEndReason: jobEndReason, jobWasBusy: jobWasBusy}; },",
  "  els: elements",
  "};"
].join("\n");

var m = {exports: {}};
new Function("module", "exports", modulo)(m, m.exports);
var api = m.exports;

function txt()  { return api.els["job-status-text"].textContent; }
function cls()  { return api.els["job-status"].className; }
function file() { return api.els["job-status-file"].hidden ? null : api.els["job-status-file"].textContent; }

var fallos = 0;
function chk(nombre, esperado, real) {
  var ok = real === esperado;
  if (!ok) fallos++;
  console.log((ok ? "  ok   " : "  FALLO") + " " + nombre +
              "  -> " + JSON.stringify(real) + (ok ? "" : "  (esperado " + JSON.stringify(esperado) + ")"));
}

function informe(linea) {
  // igual que parseStatusReport: <Run|MPos:...> -> primer trozo sin el "<"
  api.updateJobStatus(String(linea).replace(/^</, "").split("|")[0]);
}

console.log("A) recien cargado");
informe("<Idle|MPos:0,0,0|FS:0,0>");
chk("texto", "Inactivo", txt());
chk("clase", "job-status is-idle", cls());
chk("archivo", null, file());

console.log("B) la WebUI lanza diseño.gcode");
api.set("pendingJob", "diseno.gcode");
api.set("jobEndReason", null);
informe("<Run|MPos:10,0,0|FS:1000,800>");
chk("texto", "Trabajo en curso", txt());
chk("clase", "job-status is-run", cls());
chk("archivo", "diseno.gcode", file());

console.log("C) se pausa (Hold:0)");
informe("<Hold:0|MPos:10,0,0|FS:0,0>");
chk("texto", "Trabajo en pausa", txt());
chk("clase", "job-status is-paused", cls());
chk("archivo", "diseno.gcode", file());

console.log("D) se reanuda");
informe("<Run|MPos:11,0,0|FS:1000,800>");
chk("texto", "Trabajo en curso", txt());
chk("archivo", "diseno.gcode", file());

console.log("E) termina solo");
informe("<Idle|MPos:20,0,0|FS:0,0>");
chk("texto", "Trabajo finalizado", txt());
chk("clase", "job-status is-done", cls());
chk("archivo", "diseno.gcode", file());

console.log("F) luego se arranca otro trabajo desde el LCD (nombre desconocido)");
api.set("pendingJob", null);
informe("<Run|MPos:0,0,0|FS:900,300>");
chk("texto", "Trabajo en curso", txt());
chk("archivo", null, file());

console.log("G) ese trabajo acaba: no se puede saber si acabo o se paro");
informe("<Idle|MPos:5,0,0|FS:0,0>");
chk("texto", "Inactivo", txt());
chk("clase", "job-status is-idle", cls());

console.log("H) se detiene un trabajo de la WebUI");
api.set("currentJob", "mariposa.gcode");
api.set("pendingJob", null);
api.set("jobEndReason", "stopped");
api.set("jobWasBusy", false);
informe("<Idle|MPos:7,0,0|FS:0,0>");
chk("texto", "Trabajo detenido", txt());
chk("clase", "job-status is-stopped", cls());
chk("archivo", "mariposa.gcode", file());

console.log("I) alarma");
api.set("currentJob", null);
api.set("jobEndReason", null);
informe("<Alarm|MPos:7,0,0|FS:0,0>");
chk("texto", "Alarma", txt());
chk("clase", "job-status is-alarm", cls());

console.log("J) otros estados de Grbl");
informe("<Door:0|MPos:0,0,0|FS:0,0>");
chk("Door", "Puerta abierta", txt());
informe("<Check|MPos:0,0,0|FS:0,0>");
chk("Check", "Modo revisión", txt());
informe("<Sleep|MPos:0,0,0|FS:0,0>");
chk("Sleep", "Reposo", txt());

console.log("K) arranque de un trabajo nuevo tras \"detenido\"");
api.set("currentJob", "otro.gcode");
api.set("pendingJob", "otro.gcode");
api.set("jobEndReason", null);
informe("<Run|MPos:0,0,0|FS:800,400>");
chk("texto", "Trabajo en curso", txt());
chk("archivo", "otro.gcode", file());

console.log("");
console.log(fallos === 0 ? "TODO CORRECTO" : (fallos + " FALLOS"));
process.exit(fallos === 0 ? 0 : 1);
