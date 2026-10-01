/**
 * Diccionario español -> inglés de la WebUI.
 *
 * El español es SIEMPRE el texto fuente: el HTML y todos los textos que escribe
 * app.js siguen en español y aqui se declara como se dicen en ingles. Asi:
 *
 *   - si falta una traduccion, el texto se queda en espanol (nunca desaparece);
 *   - cambiar la interfaz a ingles es reversible al 100 %;
 *   - anadir un idioma mas seria otro fichero con la misma forma.
 *
 * Busqueda de i18nLookup(texto), en este orden:
 *   1. igualdad exacta;
 *   2. prefijo  -> "Ultima actualizacion: 12:00:01"  (cambia solo la cabecera)
 *   3. sufijo   -> "3 rechazado(s)"                  (cambia solo el final)
 *   4. troceado por "|" -> "SD 8MB | Usado 1MB | Libre 7MB"
 *
 * Prefijo y sufijo exigen frontera de palabra ("Estado" no traduce
 * "Estado_final.gcode"), que es lo que evita corromper nombres de archivo.
 *
 * Para ver cuantos textos quitan sin cubrir:  node webui/check_i18n.js
 */
var I18N_EN = {
  /* ---------- cabecera, navegacion y pie ---------- */
  "Acerca de": "About",
  "Archivos SD": "SD Files",
  "Parámetros": "Settings",
  "Idioma": "Language",
  "Panel de control del accesorio": "Accessory control panel",
  "Firmware abierto": "Open firmware",
  "Cambiar tema": "Switch theme",
  "Próximamente": "Coming soon",

  /* ---------- vista Acerca de y panel lateral ---------- */
  "Contraer panel": "Collapse panel",
  "Expandir panel": "Expand panel",
  "ACERCA DE": "ABOUT",
  "Firmware Grbl_ESP32 para la controladora MKS DLC32, con interfaz web integrada y pantalla táctil. Pensado para láser y CNC de uso general.": "Grbl_ESP32 firmware for the MKS DLC32 controller, with a built-in web interface and touchscreen. Designed for general-purpose laser and CNC machines.",
  "Destino": "Target",
  "Nombre de host": "Hostname",
  "Dirección IP": "IP address",
  "Versión de la WebUI": "WebUI version",
  "CRÉDITOS Y LICENCIA": "CREDITS AND LICENSE",
  "Repositorio de esta versión": "Repository for this version",
  "Código fuente, descargas y registro de cambios.": "Source code, downloads and change log.",
  "Firmware original en el que se basa esta versión.": "Original firmware this version is based on.",
  "Núcleo de control de movimiento para ESP32.": "Motion control core for ESP32.",
  "Intérprete de G-code y planificador de movimiento originales.": "Original G-code interpreter and motion planner.",
  "Biblioteca gráfica de la pantalla táctil.": "Touchscreen graphics library.",
  "Este programa es software libre: puedes redistribuirlo y modificarlo bajo los términos de la": "This program is free software: you can redistribute it and modify it under the terms of the",
  ". Se distribuye sin ninguna garantía.": ". It is distributed without any warranty.",
  "Conectando…": "Connecting…",
  "Conexión establecida": "Connected",
  "Sin conexión": "Disconnected",
  "Encendido": "Uptime",
  "Última actualización:": "Last update:",

  /* ---------- vista Control ---------- */
  "ESTADO DE MÁQUINA": "MACHINE STATUS",
  "JOG / CONTROL MANUAL": "JOG / MANUAL CONTROL",
  "Distancia (mm)": "Distance (mm)",
  "Velocidad (mm/min)": "Speed (mm/min)",
  "Z arriba": "Z up",
  "Z abajo": "Z down",
  "Agrandar texto": "Increase text size",
  "Reducir texto": "Decrease text size",
  "ACCIONES RÁPIDAS": "QUICK ACTIONS",
  "Origen": "Home",
  "Posición": "Position",
  "LÁSER / SPINDLE": "LASER / SPINDLE",
  "ACCIONES SECUNDARIAS": "SECONDARY ACTIONS",
  "Establecer cero (X,Y)": "Set zero (X,Y)",
  "Ir a origen (Z)": "Go to origin (Z)",
  "Reiniciar controlador": "Restart controller",
  "CONSOLA EN VIVO": "LIVE CONSOLE",
  "Modo verboso": "Verbose mode",
  "Limpiar": "Clear",
  "Enviar": "Send",
  "Escribe un comando...": "Type a command...",
  "Configurar": "Configure",
  "Nombre del botón": "Button name",
  "Comando a enviar (G-code o $):": "Command to send (G-code or $):",
  "al enviar:": "when sending:",
  "Error del controlador:": "Controller error:",
  "Enviando $40=": "Sending $40=",
  ". Textos del LCD recargados; si una página no se actualiza, entra y sales de ella.":
    ". LCD texts reloaded; if a page does not update, leave it and come back.",
  "Sin respuesta del controlador (timeout).": "No response from the controller (timeout).",
  "Escrito. Comprobando $40…": "Written. Checking $40…",
  "No se pudo leer $40 tras escribirlo. Vuelve a intentarlo.": "Could not read $40 after writing it. Try again.",
  "Aviso: el controlador se quedó en": "Warning: the controller stayed at",
  ". El cambio no se aplicó; vuelve a intentarlo.": ". The change was not applied; try again.",
  "Pausar": "Pause",
  "Reanudar": "Resume",

  /* estado del trabajo (tarjeta ESTADO DE MÁQUINA) */
  "Inactivo": "Idle",
  "Trabajo en curso": "Job running",
  "Trabajo en pausa": "Job paused",
  "Trabajo finalizado": "Job finished",
  "Trabajo detenido": "Job stopped",
  "Alarma": "Alarm",
  "Puerta abierta": "Door open",
  "Modo revisión": "Check mode",
  "Reposo": "Sleep",
  "Cambio de herramienta": "Tool change",

  /* ---------- vista Archivos SD ---------- */
  "ARCHIVOS SD": "SD FILES",
  "Actualizar": "Refresh",
  "Subir archivo": "Upload file",
  "Buscar archivo...": "Search file...",
  "Ordenar por": "Sort by",
  "Nombre (A-Z)": "Name (A-Z)",
  "Nombre (Z-A)": "Name (Z-A)",
  "Tamaño (mayor)": "Size (largest)",
  "Tamaño (menor)": "Size (smallest)",
  "Todos": "All",
  "Mostrar": "Show",
  "Archivo": "File",
  "Estado": "Status",
  "Tamaño": "Size",
  "Modificado": "Modified",
  "Listo": "Ready",
  "DETALLES DEL ARCHIVO": "FILE DETAILS",
  "Selecciona un archivo de la lista.": "Select a file from the list.",
  "Iniciar trabajo": "Start job",
  "Cargar vista previa": "Load preview",
  "Encuadrar": "Fit",
  "Detener": "Stop",
  "Renombrar": "Rename",
  "Eliminar": "Delete",
  "VISTA PREVIA DEL TRABAJO": "JOB PREVIEW",
  "Leyendo archivo...": "Reading file...",
  "Dimensiones": "Dimensions",
  "Recorrido estimado": "Estimated travel",
  "Puntos totales": "Total points",
  "Leyendo…": "Reading…",
  "Usado": "Used",
  "Libre": "Free",
  "ACTIVIDAD RECIENTE": "RECENT ACTIVITY",
  "Ver todo": "Show all",
  "Ver menos": "Show less",
  "Sin actividad todavía.": "No activity yet.",
  "Error al leer la SD": "Error reading the SD card",
  "Error, reintentar": "Error, retry",
  "Error de red": "Network error",
  "¿Eliminar": "Delete",
  "de la SD?": "from the SD?",
  "¿Eliminar fichero.gcode de la SD?": "Delete fichero.gcode from the SD?",
  "No se pudo eliminar": "Could not delete",
  "No se pudo subir": "Could not upload",
  "No se pudo renombrar (": "Could not rename (",
  "). ¿Firmware con soporte de rename?": "). Firmware with rename support?",
  "Archivo descargado:": "File downloaded:",
  "Archivo eliminado:": "File deleted:",
  "Archivo subido:": "File uploaded:",
  "Archivo subido": "File uploaded",
  "Nuevo nombre para": "New name for",
  "Renombrado:": "Renamed:",
  "Fallo al subir": "Upload failed",
  "Encuadre trazado:": "Job bounding box:",
  "Trabajo pausado": "Job paused",
  "Trabajo reanudado": "Job resumed",
  "Trabajo iniciado:": "Job started:",
  "Primero carga la \"Vista previa\" para conocer el área del trabajo.": "Load the \"Preview\" first to know the work area.",

  /* ---------- vista Parámetros ---------- */
  "CONFIGURACIÓN DEL CONTROLADOR": "CONTROLLER CONFIGURATION",
  "✓ Guardar cambios": "✓ Save changes",
  "↻ Restaurar respaldo": "↻ Restore backup",
  "⇩ Exportar configuración": "⇩ Export configuration",
  "⇪ Importar configuración": "⇪ Import configuration",
  "↺ Valores por defecto": "↺ Factory defaults",
  "APLICAR CON SEGURIDAD": "APPLY SAFELY",
  "Buscar parámetro...": "Search parameter...",
  "Todas las categorías": "All categories",
  "Todas": "All",
  "PARÁMETROS": "PARAMETERS",
  "Parámetro $": "Parameter $",
  "Parámetro": "Parameter",
  "Código": "Code",
  "Descripción": "Description",
  "Unidad": "Unit",
  "Valor": "Value",
  "Detalle": "Details",
  "RESUMEN DE CAMBIOS": "CHANGES SUMMARY",
  "Los cambios no se han aplicado aún. Presiona \"Guardar cambios\" para enviarlos al controlador.":
    "The changes have not been applied yet. Press \"Save changes\" to send them to the controller.",
  "ESTADO DE VALIDACIÓN": "VALIDATION STATUS",
  "Sin verificar": "Not checked",
  "Última confirmación del controlador": "Last controller confirmation",
  "Confirmado por el controlador": "Confirmed by the controller",
  "Cada \"Guardar cambios\" respalda automáticamente los valores anteriores. Si algo sale mal, usa \"Restaurar respaldo\".":
    "Every \"Save changes\" automatically backs up the previous values. If something goes wrong, use \"Restore backup\".",
  "Sin descripción disponible en el catálogo.": "No description available in the catalog.",
  "rechazado(s)": "rejected",
  "Movimiento": "Motion",
  "Límites": "Limits",
  "Láser": "Laser",
  "Reportes": "Reports",
  "Sistema": "System",
  "Otros": "Others",
  "Todavía no hay un respaldo guardado (se crea automáticamente la primera vez que guardas cambios).":
    "There is no backup yet (one is created automatically the first time you save changes).",
  "¿Restaurar los valores previos al último \"Guardar cambios\"?":
    "Restore the values from before the last \"Save changes\"?",
  "El respaldo local está corrupto; no se restaura.": "The local backup is corrupt; it was not restored.",
  "El archivo no tiene líneas $N=valor válidas.": "The file has no valid $N=value lines.",
  "Se van a aplicar": "These will apply",
  "parámetros a la placa. ¿Continuar?": "parameters to the board. Continue?",
  "Esto restaura TODOS los parámetros $ a los valores de fábrica del firmware (comando $RST=$). ¿Continuar?":
    "This resets ALL $ parameters to the firmware factory values (command $RST=$). Continue?",
  "Configuración importada (": "Configuration imported (",
  "parámetros)": "parameters)",
  "¿Detener el trabajo en curso?": "Stop the job in progress?",
  "¿Reiniciar el controlador? Esto detiene cualquier trabajo en curso (pausado o corriendo).":
    "Restart the controller? This stops any job in progress (paused or running).",

  /* ---------- catálogo $ (nombres, unidades y descripciones) ---------- */
  "Pulso de paso": "Step pulse time",
  "Retardo en reposo": "Step idle delay",
  "Invertir pulso de paso": "Step pulse invert",
  "Invertir dirección de paso": "Step direction invert",
  "Invertir enable pin": "Invert enable pin",
  "Invertir pines de límite": "Invert limit pins",
  "Invertir pin de sonda": "Invert probe pin",
  "Opciones de reporte de estado": "Status report options",
  "Desviación de unión": "Junction deviation",
  "Tolerancia de arco": "Arc tolerance",
  "Reportar en pulgadas": "Report in inches",
  "Límites suaves": "Soft limits",
  "Límites duros": "Hard limits",
  "Ciclo de homing": "Homing cycle",
  "Invertir dirección de homing": "Invert homing direction",
  "Velocidad de localización de homing": "Homing locate speed",
  "Velocidad de búsqueda de homing": "Homing search speed",
  "Debounce de switch de homing": "Homing switch debounce",
  "Retiro de homing": "Homing pull-off",
  "Velocidad máxima de spindle": "Max spindle speed",
  "Velocidad mínima de spindle": "Min spindle speed",
  "Modo láser": "Laser mode",
  "Frecuencia PWM del spindle": "Spindle PWM frequency",
  "Valor PWM apagado": "PWM off value",
  "Valor PWM mínimo": "PWM min value",
  "Valor PWM máximo": "PWM max value",
  "Idioma del LCD": "LCD language",
  "Idioma de la pantalla: 0=中文 1=English 2=Deutsch 3=Español. Ver la vista Idioma.":
    "Screen language: 0=中文 1=English 2=Deutsch 3=Spanish. See the Language view.",
  "Entero de usuario 80": "User integer 80",
  "Entero de usuario 81": "User integer 81",
  "Entero de usuario 82": "User integer 82",
  "Entero de usuario 83": "User integer 83",
  "Entero de usuario 84": "User integer 84",
  "Flotante de usuario 90": "User float 90",
  "Flotante de usuario 91": "User float 91",
  "Flotante de usuario 92": "User float 92",
  "Flotante de usuario 93": "User float 93",
  "Flotante de usuario 94": "User float 94",
  "Resolución de eje X": "Axis resolution X",
  "Resolución de eje Y": "Axis resolution Y",
  "Resolución de eje Z": "Axis resolution Z",
  "Velocidad máxima X": "Max rate X",
  "Velocidad máxima Y": "Max rate Y",
  "Velocidad máxima Z": "Max rate Z",
  "Aceleración X": "Acceleration X",
  "Aceleración Y": "Acceleration Y",
  "Aceleración Z": "Acceleration Z",
  "Recorrido máximo X": "Max travel X",
  "Recorrido máximo Y": "Max travel Y",
  "Recorrido máximo Z": "Max travel Z",
  "Corriente motor X": "Motor current X",
  "Corriente motor Y": "Motor current Y",
  "Corriente motor Z": "Motor current Z",
  "Corriente en reposo X": "Idle current X",
  "Corriente en reposo Y": "Idle current Y",
  "Corriente en reposo Z": "Idle current Z",
  "Micropasos X": "Microsteps X",
  "Micropasos Y": "Microsteps Y",
  "Micropasos Z": "Microsteps Z",
  "Duración del pulso de paso. Mínimo 3us.": "Step pulse duration. Minimum 3us.",
  "Retardo antes de deshabilitar motores al detenerse.": "Delay before disabling the motors when stopping.",
  "Invierte la señal de paso por eje (00000ZYX).": "Inverts the step signal per axis (00000ZYX).",
  "Invierte la señal de dirección por eje (00000ZYX).": "Inverts the direction signal per axis (00000ZYX).",
  "Invierte la señal de habilitación del driver.": "Inverts the driver enable signal.",
  "Invierte todos los pines de entrada de límite.": "Inverts all limit input pins.",
  "Invierte la señal del pin de sonda (probe).": "Inverts the probe pin signal.",
  "Determina qué datos se incluyen en los reportes.": "Determines which data is included in reports.",
  "Qué tan rápido Grbl se mueve entre movimientos consecutivos.": "How fast Grbl moves between consecutive moves.",
  "Precisión de trazado de arcos G2/G3.": "G2/G3 arc drawing accuracy.",
  "Usa pulgadas en vez de mm para posición/velocidad.": "Uses inches instead of mm for position and speed.",
  "Activa alarma al exceder el espacio de trabajo. Requiere homing.": "Raises an alarm when leaving the work area. Requires homing.",
  "Detiene el movimiento de inmediato al activar un switch.": "Stops motion immediately when a switch is triggered.",
  "Activa el ciclo de homing. Requiere switches en todos los ejes.": "Runs the homing cycle. Requires switches on every axis.",
  "Busca el switch en dirección negativa por eje (00000ZYX).": "Looks for the switch in the negative direction per axis (00000ZYX).",
  "Velocidad lenta para ubicar el switch con precisión.": "Slow speed to locate the switch precisely.",
  "Velocidad rápida para encontrar el switch antes de la fase lenta.": "Fast speed to find the switch before the slow phase.",
  "Retardo entre fases del homing.": "Delay between homing phases.",
  "Distancia de retroceso tras activar el switch.": "Backoff distance after triggering the switch.",
  "Distancia máxima desde el switch de homing.": "Maximum distance from the homing switch.",
  "Velocidad máxima. PWM al 100%.": "Maximum speed. PWM at 100%.",
  "Velocidad mínima. PWM al 0.4%.": "Minimum speed. PWM at 0.4%.",
  "Comandos G1/2/3 consecutivos no se detienen al cambiar potencia.": "Consecutive G1/2/3 commands do not stop when the power changes.",
  "Frecuencia PWM (requiere reinicio).": "PWM frequency (requires restart).",
  "Valor PWM cuando está apagado (requiere reinicio).": "PWM value when off (requires restart).",
  "Valor PWM mínimo (requiere reinicio).": "PWM minimum value (requires restart).",
  "Valor PWM máximo (requiere reinicio).": "PWM maximum value (requires restart).",
  "Reservado para uso personalizado.": "Reserved for custom use.",
  "Pasos por milímetro en X.": "Steps per millimetre in X.",
  "Pasos por milímetro en Y.": "Steps per millimetre in Y.",
  "Pasos por milímetro en Z.": "Steps per millimetre in Z.",
  "Usada como velocidad rápida (G0).": "Used as the rapid rate (G0).",
  "Para no exceder el torque del motor.": "To stay within the motor torque.",
  "Corriente de operación (drivers SPI/Trinamic).": "Operating current (SPI/Trinamic drivers).",
  "Porcentaje de la corriente de operación (SPI/Trinamic).": "Percentage of the operating current (SPI/Trinamic).",
  "Micropasos (drivers SPI/Trinamic).": "Microstepping (drivers SPI/Trinamic).",
  "Sensibilidad de detección de bloqueo (SPI/Trinamic).": "Stall detection sensitivity (SPI/Trinamic).",
  "microsegundos": "microseconds",
  "milisegundos": "milliseconds",
  "máscara": "mask",
  "booleano": "boolean",
  "entero": "integer",

  /* ---------- vista Idioma ---------- */
  "IDIOMAS": "LANGUAGES",
  "Aquí se cambian dos cosas: el idioma de": "There are two things to change here: the language of",
  "esta interfaz web": "this web interface",
  "(se guarda en tu navegador) y el de la": "(stored in your browser) and that of the",
  "pantalla LCD del equipo": "the machine's LCD screen",
  "(parámetro": "(setting",
  "del controlador). Ninguna de las dos necesita botón de guardar.":
    "on the controller). Neither of them needs a save button.",
  "IDIOMA DE ESTA WEBUI": "THIS WEBUI LANGUAGE",
  "Interfaz en español": "Interface in Spanish",
  "Interfaz en inglés": "Interface in English",
  "Alcance": "Scope",
  "Menús, tablas, avisos y mensajes de esta web. No toca la consola ni la pantalla LCD.":
    "Menus, tables, notices and messages on this site. It does not touch the console or the LCD screen.",
  "Idioma de la interfaz": "Interface language",
  "IDIOMA DE LA PANTALLA (LCD)": "LCD SCREEN LANGUAGE",
  "Chino": "Chinese",
  "Inglés": "English",
  "Alemán": "German",
  "Español": "Spanish",
  "Idioma actual": "Current language",
  "Última operación": "Last operation",
  "Desconocido": "Unknown",
  "中文 · Chino": "中文 · Chinese",
  "English · Inglés": "English",
  "Deutsch · Alemán": "Deutsch · German",
  "NOTAS": "NOTES",
  "• El cambio de la pantalla es inmediato: el controlador recarga los textos del LCD en menos de un segundo. Si la página abierta no cambia de texto, entra y sale de esa página.":
    "• The screen change is immediate: the controller reloads the LCD texts in under a second. If the open page does not update its text, leave the page and come back.",
  "• 中文 y Deutsch muestran los textos en inglés: en el firmware solo existen":
    "• 中文 and Deutsch show the English text: the firmware only ships",
  "(inglés) y": "(English) and",
  "(español).": "(Spanish).",
  "• El botón 中文 de la pantalla puede salir con un recuadro: la fuente del LCD":
    "• The 中文 button on the screen may show an empty box: the LCD font",
  "no incluye glifos chinos (sí tiene tildes, ñ, ¿ y ¡).":
    " has no Chinese glyphs (it does have accents, ñ, ¿ and ¡).",
  "• El idioma de la pantalla se guarda en NVS: sobrevive a los reinicios. El de esta web se guarda en el navegador (localStorage) y no afecta a nadie más.":
    "• The screen language is stored in NVS: it survives reboots. This site's is stored in the browser (localStorage) and affects nobody else."
};

/**
 * Traduce un texto de la interfaz (siempre en español) al inglés.
 * Si no hay entrada, devuelve el texto tal cual.
 */
function i18nLookup(text) {
  if (typeof text !== "string" || !text) return text;
  if (I18N_EN[text] !== undefined) return I18N_EN[text];
  if (i18nLookup.cache[text] !== undefined) return i18nLookup.cache[text];

  // Se trabaja sin los espacios de los bordes y se vuelven a pegar al final.
  var m = /^(\s*)([\s\S]*?)(\s*)$/.exec(text);
  var core = m[2];

  // El HTML puede dejar saltos e indentacion dentro del texto: si con el texto
  // plano hay clave, se usa esa (el resultado conserva los bordes originales).
  if (I18N_EN[core] === undefined && /\s{2,}|\n|\t/.test(core)) {
    var flat = core.replace(/\s+/g, " ");
    if (I18N_EN[flat] !== undefined) core = flat;
  }

  var out = i18nCore(core);
  out = out === core ? text : m[1] + out + m[3];
  i18nLookup.cache[text] = out;
  return out;
}
i18nLookup.cache = {};

function i18nIsWord(ch) {
  return ch !== "" && /[\w\u00C0-\u024F]/.test(ch);
}

/** Prefijo o sufijo mas largo que encaje en la frontera de palabra. */
function i18nMatch(c, mode) {
  var best = null;
  for (var k in I18N_EN) {
    if (k.length < 4 || k.length >= c.length) continue;
    if (mode === "prefix") {
      if (c.lastIndexOf(k, 0) !== 0) continue;
      if (i18nIsWord(k.charAt(k.length - 1)) && i18nIsWord(c.charAt(k.length))) continue;
    } else {
      if (c.indexOf(k, c.length - k.length) !== c.length - k.length) continue;
      var before = c.charAt(c.length - k.length - 1);
      if (i18nIsWord(k.charAt(0)) && i18nIsWord(before)) continue;
    }
    if (best === null || k.length > best.length) best = k;
  }
  return best;
}

function i18nOnce(c) {
  if (I18N_EN[c] !== undefined) return I18N_EN[c];
  var out = c;
  var p = i18nMatch(c, "prefix");
  if (p !== null) out = I18N_EN[p] + c.slice(p.length);
  var s = i18nMatch(out, "suffix");
  if (s !== null && out.length > s.length) out = out.slice(0, out.length - s.length) + I18N_EN[s];
  return out;
}

/** i18nOnce + troceado por "|" (lineas como "SD 8MB | Usado 1MB | Libre 7MB"). */
function i18nCore(c) {
  var once = i18nOnce(c);
  if (once !== c || c.indexOf("|") === -1) return once;

  var segs = c.split("|");
  var rebuilt = [];
  var any = false;
  for (var i = 0; i < segs.length; i++) {
    var m = /^(\s*)([\s\S]*?)(\s*)$/.exec(segs[i]);
    var t = i18nOnce(m[2]);
    if (t !== m[2]) any = true;
    rebuilt.push(m[1] + t + m[3]);
  }
  return any ? rebuilt.join("|") : c;
}
