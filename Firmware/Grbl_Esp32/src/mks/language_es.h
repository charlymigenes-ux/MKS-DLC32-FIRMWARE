#ifndef __language_es_h
#define __language_es_h

/* Traducción al español del LCD (MKS DLC32).
 *
 * Nota de fuente: las fuentes originales del LCD (lv_font_roboto_16 y dlc32Font)
 * solo traen ASCII y unos caracteres chinos. Las tildes, la ñ, ¿ y ¡ salen de
 * lv_pic/font_latin.c (roboto16Latin y dlc32FontLatin), que añaden el rango
 * Latin-1 y delegan todo lo demás en la fuente original.
 */

/* 公共 / común */
#define BACK_ES                 "Atrás"
#define YES_ES                  "Sí"
#define NO_ES                   "No"

/* 主页 / portada */
#define CONTROL_ES              "Control"
#define SCULPTURE_ES            "Tallado"
#define TOOL_ES                 "Herramientas"
#define MPOS_ES                 "Origen"
#define WPOS_ES                 "Trabajo"
#define WIFI_CONNECT_ES         "Conectado"
#define WIFI_DISCONNECT_ES      "Sin conexión"

/* 控制界面 / control */
#define XY_CLEAR_ES             "Borrar XY"
#define Z_CLEAR_ES              "Borrar Z"
#define KNIFE_ES                "Cuchilla"
#define NEXT_ES                 "Siguiente"
#define UP_ES                   "Subir"
#define COOLING_ES              "Refrigerante"
#define POSITION_ES             "Posición"
#define SPEED_HIGH_ES           "Vel. alta"
#define SPEED_MID_ES            "Vel. media"
#define SPEED_LOW_ES            "Vel. baja"
#define SPINDLE_ES              "Husillo"
#define CARVE_ES                "Tallar"

/* 文件界面 / archivos */
#define DIS_NO_SDCARD_ES        "Sin tarjeta SD"

/* 雕刻界面 / tallado */
#define HOLD_ES                 "Pausa"
#define CYCLE_ES                "Ciclo"
#define STOP_ES                 "Parar"
#define ADJUST_ES               "Ajuste"
#define SPINDLE_SPPED_ES        "Velocidad del husillo:"
#define FEED_RATE_ES            "Avance:"
#define RAPID_SPEED_ES          "Vel. rápida"
#define CARVE_TIMES_ES          "tiempo:"

/* 提示 / avisos */
#define DIS_STOP_CARVE_ES       "¿Quieres parar el tallado?"
#define DIS_HOMEING_ES          "Homing..."
#define DIS_NO_HARD_HOME_ES     "Homing duro no habilitado..."
#define DIS_HOME_SUCCEED_ES     "¡Homing completado!"
#define DIS_HOME_FAIL_ES        "Fallo de homing"
#define DIS_PROBE_SET_ES        "Configurando sonda..."
#define DIS_PROBE_SECCEED_ES    "¡Sonda correcta!"
#define DIS_PROBE_FAIL_ES       "Fallo de sonda"
#define DIS_HARD_LIMIT_ES       "¡Límite duro!"
#define DIS_SOFT_LIMIT_ES       "¡Límite suave!"
#define DIS_UNLOCK_ES           "¡Por favor, desbloquea!"
#define DIS_WAIT_MC_STOP_ES     "Espera a que la máquina pare"
#define DIS_POS_SUCCEED_ES      "Posición alcanzada"

/* Botones y popups (antes literales fijos en el codigo de cada pagina) */
#define CANCEL_ES               "Cancelar"
#define FRAME_ES                "Marco"
#define CARVE_FILE_SURE_ES      "¿Tallar este archivo?"
#define PAUSE_ES                "Pausar"
#define START_ES                "Iniciar"
#define CONFIRM_ES              "Confirmar"
#define ADD_ES                  "Aumentar"
#define REDUCE_ES               "Reducir"
#define EXIT_ES                 "Salir"
#define LANGUAGE_ES             "Idioma"
#define SCANF_ES                "Escanear"
#define RECONNECT_ES            "Reconectar"
#define CONNECT_ES              "Conectar"
#define PASSWORD_ES             "Contraseña:"
#define Z_HOME_ES               "Z inicio"
#define UPDATE_TITLE_ES         "Actualizando..."

/* Avisos y mensajes de dialogo */
#define INFO_ES                 "Información"
#define WARNING_ES              "Aviso"
#define ERROR_ES                "Error"
#define UNLOCK_SUCCESS_ES       "¡Desbloqueo correcto!"
#define SETTING_ERROR_ES        "¡Error de ajuste!"
#define SET_6_1_ES              "¡Pon $6=1!"
#define WAIT_IDLE_ES            "Espera a que la máquina esté libre"
#define FILE_TOO_BIG_ES         "El archivo es demasiado grande"
#define CONTINUE_SURE_ES        "¿Quieres continuar?"
#define FILE_LOADING_ES         "Cargando archivo..."
#define LOADING_FILE_ES         "Cargando archivo..."
#define SD_BUSY_ES              "La SD está ocupada..."
#define RUNNING_ES              "En marcha..."
#define PRINT_STOP_SURE_ES      "¿Quieres parar la impresión?"
#define PRINT_DONE_ES           "¡Archivo impreso!"
#define WIFI_SCANNING_ES        "Buscando wifi..."
#define WIFI_CONNECTING_ES      "Conectando wifi..."
#define WIFI_DISCONNECTING_ES   "Desconectando wifi..."
#define WIFI_PWD_PROMPT_ES      "Introduce la contraseña para conectar"
#define WIFI_STATUS_ON_ES       "WIFI:Conectado"
#define WIFI_STATUS_OFF_ES      "WIFI:Sin conexión"
#define UPDATE_SUCCEED_ES       "Actualización correcta"
#define UPDATE_RESTART_ES       "¡Reinicia la máquina!"
#define UPDATE_FAIL_ES          "Fallo de actualización"
#define UPDATE_FAIL_HELP_ES     "Revisa mkscfg.txt o la tarjeta SD"

/* Formatos con %d */
#define POWER_FMT_ES            "Potencia:%d%%"
#define SPEED_FMT_ES            "Velocidad:%d%%"
#define SPINDLE_SPEED_FMT_ES    "Vel. del husillo: %d%%"
#define FEED_RATE_FMT_ES        "Avance: %d%%"
#define RAPID_FMT_ES            "Vel. rápida: %d%%"
#define GAUGE_FEED_ES            "Avance"
#define GAUGE_SPINDLE_ES         "Husillo"
#define GAUGE_RAPID_ES           "Rápido"
#define JOB_ELAPSED_ES           "Tiempo"
#define JOB_LEFT_ES              "Restante"
#define JOB_FEED_REAL_ES         "Avance"
#define JOB_POWER_REAL_ES        "Potencia"
#define RESUME_ES                "Reanudar"
#define TOOL_BOARD_ES            "Placa"
// Rotulos con el modo laser activo ($32=1): potencia y velocidad en vez de husillo y avance
#define SPINDLE_LASER_ES               "Láser"
#define SPINDLE_SPEED_LASER_ES         "Potencia del láser:"
#define SPINDLE_SPEED_FMT_LASER_ES     "Potencia: %d%%"
#define GAUGE_SPINDLE_LASER_ES         "Potencia"
#define FEED_RATE_LASER_ES             "Velocidad:"
#define FEED_RATE_FMT_LASER_ES         "Velocidad: %d%%"
#define GAUGE_FEED_LASER_ES            "Velocidad"
#define JOB_FEED_REAL_LASER_ES         "Velocidad"

/* Pagina de pruebas (test) */
#define TESTING_ES              "Probando..."
#define PROBE_CHECK_ES          "Sonda:"
#define X_LIMIT_CHECK_ES        "Límite X:"
#define Y_LIMIT_CHECK_ES        "Límite Y:"
#define Z_LIMIT_CHECK_ES        "Límite Z:"
#define SD_CHECK_ES             "SD:OK"
#define I2C_CHECK_ES            "I2C:"
#define CPU_TEMP_ES             "Temp. CPU:"
#define TEST_WARNING_ES         "#FF0000 ¡Aviso!#"

#endif
