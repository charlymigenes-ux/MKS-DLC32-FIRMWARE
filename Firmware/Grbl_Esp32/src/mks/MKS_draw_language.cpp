#include "MKS_draw_language.h"
#include <string.h>

mc_lg_muilt_t mc_language;

// Textos sueltos de las paginas (los que no estan en language_es.h): la clave es el texto en
// espanol tal como lo escriben los T()/hdr_T()/... de cada pagina. Sin entrada, el portugues
// cae al ingles.
static const struct { const char* es; const char* pt; } mks_pt_table[] = {
	{ "Acerca de", "Sobre" },
	{ "Activo", "Ativo" },
	{ "Al tocar", "Ao tocar" },
	{ "Alertas", "Alertas" },
	{ "Basado en", "Baseado em" },
	{ "Buscando redes", "Buscando redes" },
	{ "Buscar redes", "Buscar redes" },
	{ "Cerrar la conexión actual", "Encerrar a conexão atual" },
	{ "Conectando", "Conectando" },
	{ "Conexión de la red", "Conexão de rede" },
	{ "Datos del microcontrolador", "Dados do microcontrolador" },
	{ "Desconectando", "Desconectando" },
	{ "Desconectar", "Desconectar" },
	{ "Elegir", "Escolher" },
	{ "Elige una red", "Escolha uma rede" },
	{ "Fin de trabajo, error y alarma", "Fim do trabalho, erro e alarme" },
	{ "Identificador del chip", "Identificador do chip" },
	{ "Idioma de la pantalla ($40)", "Idioma da tela ($40)" },
	{ "Información del sistema y del hardware", "Informações do sistema e do hardware" },
	{ "Intensidad de la señal", "Intensidade do sinal" },
	{ "Licencia", "Licença" },
	{ "Memoria del programa", "Memória do programa" },
	{ "Modelo de la placa", "Modelo da placa" },
	{ "No disponible en esta compilación", "Indisponível nesta compilação" },
	{ "No se encontraron redes", "Nenhuma rede encontrada" },
	{ "Pitido al tocar la pantalla", "Bipe ao tocar na tela" },
	{ "Placa", "Placa" },
	{ "Procesador", "Processador" },
	{ "Red", "Rede" },
	{ "Redes disponibles", "Redes disponíveis" },
	{ "Reloj y temperatura", "Relógio e temperatura" },
	{ "Señal", "Sinal" },
	{ "Sin conexión", "Sem conexão" },
	{ "Software libre", "Software livre" },
	{ "Sonido al tocar y alertas", "Som ao tocar e alertas" },
	{ "Toca el enlace para ver el QR", "Toque no link para ver o QR" },
	{ "Toca para cerrar", "Toque para fechar" },
	{ "Versión instalada", "Versão instalada" },
	{ "Toca el QR para continuar", "Toque no QR para continuar" },
	{ "Archivos SD", "Arquivos SD" },
	{ "Aire", "Ar" },
	{ "Sonda Z", "Sonda Z" },
	{ "Cero XY", "Zero XY" },
	{ "Cero XYZ", "Zero XYZ" },
	{ "Cero Z", "Zero Z" },
	{ "Más", "Mais" },
	{ "Menos", "Menos" },
	{ "Red: ", "Rede: " },
	{ "Herramienta", "Ferramenta" },
	{ "Configuraci\xc3\xb3n", "Configura\xc3\xa7\xc3\xb5" "es" },
	{ "Señal: ", "Sinal: " },
};

const char* mks_tr(const char* es, const char* en) {
	if(mks_grbl.language == Espanol) return es;
	if(mks_grbl.language == Portugues) {
		for(size_t i = 0; i < sizeof(mks_pt_table) / sizeof(mks_pt_table[0]); i++) {
			if(strcmp(es, mks_pt_table[i].es) == 0) return mks_pt_table[i].pt;
		}
	}
	return en;
}
LANGUAGE_PAGE_T language_page;

// Carga en mc_language las cadenas que van a usar los labels del LCD.
// Depende de mks_grbl.language, que arranca desde $40 (mks_grbl_parg_init)
// y cambia al pulsar un boton de esta pagina o al escribir $40 a distancia
// (refresco periodico en MKS_FREERTOS_TASK.cpp).
//
// Corregido (bug original): aqui ponia "=" (asignacion) y no "==", asi que
// esta funcion cargaba SIEMPRE ingles y ademas pisaba mks_grbl.language.
// El portugues tiene language_pt.h. El ingles se usa tambien como respaldo (si no, los punteros
// quedarian NULL y las etiquetas se quedarian con el "Text" por defecto de LVGL).
// El chino (language_ch.h) se aplica encima del ingles al final de mc_language_init().
// Chino simplificado: se aplica sobre el ingles (los textos que la fuente no puede escribir se
// quedan en ingles). Los textos salen de language_ch.h.
static void mc_language_apply_chinese(void) {
	mc_language.spindle = SPINDLE_LASER_CH;
	mc_language.spindle_speed = SPINDLE_SPEED_LASER_CH;
	mc_language.spindle_speed_fmt = SPINDLE_SPEED_FMT_LASER_CH;
	mc_language.gauge_spindle = GAUGE_SPINDLE_LASER_CH;
	mc_language.feed_rate = FEED_RATE_LASER_CH;
	mc_language.feed_rate_fmt = FEED_RATE_FMT_LASER_CH;
	mc_language.gauge_feed = GAUGE_FEED_LASER_CH;
	mc_language.job_feed_real = JOB_FEED_REAL_LASER_CH;
	mc_language.back = BACK_CH;
	mc_language.yes = YES_CH;
	mc_language.no = NO_CH;
	mc_language.control = CONTROL_CH;
	mc_language.sculpture = SCULPTURE_CH;
	mc_language.tool = TOOL_CH;
	mc_language.Mpos = MPOS_CH;
	mc_language.Wpos = WPOS_CH;
	mc_language.wifi_connect = WIFI_CONNECT_CH;
	mc_language.wifi_disconnect = WIFI_DISCONNECT_CH;
	mc_language.xy_clear = XY_CLEAR_CH;
	mc_language.z_clear = Z_CLEAR_CH;
	mc_language.knife = KNIFE_CH;
	mc_language.next = NEXT_CH;
	mc_language.up = UP_CH;
	mc_language.cooling = COOLING_CH;
	mc_language.position = POSITION_CH;
	mc_language.speed_high = SPEED_HIGH_CH;
	mc_language.speed_mid = SPEED_MID_CH;
	mc_language.speed_low = SPEED_LOW_CH;
	mc_language.spindle = SPINDLE_CH;
	mc_language.carve = CARVE_CH;
	mc_language.dis_no_sd_card = DIS_NO_SDCARD_CH;
	mc_language.hold = HOLD_CH;
	mc_language.cycle = CYCLE_CH;
	mc_language.stop = STOP_CH;
	mc_language.adjust = ADJUST_CH;
	mc_language.spindle_speed = SPINDLE_SPPED_CH;
	mc_language.feed_rate = FEED_RATE_CH;
	mc_language.rapid_speed = RAPID_SPEED_CH;
	mc_language.carve_times = CARVE_TIMES_CH;
	mc_language.dis_stop_print = DIS_STOP_CARVE_CH;
	mc_language.dis_homing = DIS_HOMEING_CH;
	mc_language.dis_no_hard_homing = DIS_NO_HARD_HOME_CH;
	mc_language.dis_homing_succeed = DIS_HOME_SUCCEED_CH;
	mc_language.dis_homing_fail = DIS_HOME_FAIL_CH;
	mc_language.dis_probe_succeed = DIS_PROBE_SECCEED_CH;
	mc_language.dis_probe_fail = DIS_PROBE_FAIL_CH;
	mc_language.cancel = CANCEL_CH;
	mc_language.frame = FRAME_CH;
	mc_language.carve_file_sure = CARVE_FILE_SURE_CH;
	mc_language.pause = PAUSE_CH;
	mc_language.start = START_CH;
	mc_language.confirm = CONFIRM_CH;
	mc_language.add = ADD_CH;
	mc_language.reduce = REDUCE_CH;
	mc_language.exit = EXIT_CH;
	mc_language.language = LANGUAGE_CH;
	mc_language.scanf = SCANF_CH;
	mc_language.reconnect = RECONNECT_CH;
	mc_language.connect = CONNECT_CH;
	mc_language.password = PASSWORD_CH;
	mc_language.z_home = Z_HOME_CH;
	mc_language.update_title = UPDATE_TITLE_CH;
	mc_language.dis_info = INFO_CH;
	mc_language.dis_warning = WARNING_CH;
	mc_language.dis_error = ERROR_CH;
	mc_language.dis_unlock_success = UNLOCK_SUCCESS_CH;
	mc_language.dis_setting_error = SETTING_ERROR_CH;
	mc_language.dis_set_6_1 = SET_6_1_CH;
	mc_language.dis_wait_idle = WAIT_IDLE_CH;
	mc_language.dis_file_too_big = FILE_TOO_BIG_CH;
	mc_language.dis_continue_sure = CONTINUE_SURE_CH;
	mc_language.dis_file_loading = FILE_LOADING_CH;
	mc_language.dis_loading_file = LOADING_FILE_CH;
	mc_language.dis_sd_busy = SD_BUSY_CH;
	mc_language.dis_running = RUNNING_CH;
	mc_language.dis_print_stop_sure = PRINT_STOP_SURE_CH;
	mc_language.dis_print_done = PRINT_DONE_CH;
	mc_language.wifi_scanning = WIFI_SCANNING_CH;
	mc_language.wifi_connecting = WIFI_CONNECTING_CH;
	mc_language.wifi_disconnecting = WIFI_DISCONNECTING_CH;
	mc_language.wifi_pwd_prompt = WIFI_PWD_PROMPT_CH;
	mc_language.wifi_status_on = WIFI_STATUS_ON_CH;
	mc_language.wifi_status_off = WIFI_STATUS_OFF_CH;
	mc_language.dis_update_succeed = UPDATE_SUCCEED_CH;
	mc_language.dis_update_restart = UPDATE_RESTART_CH;
	mc_language.dis_update_fail = UPDATE_FAIL_CH;
	mc_language.dis_update_fail_help = UPDATE_FAIL_HELP_CH;
	mc_language.power_fmt = POWER_FMT_CH;
	mc_language.speed_fmt = SPEED_FMT_CH;
	mc_language.spindle_speed_fmt = SPINDLE_SPEED_FMT_CH;
	mc_language.feed_rate_fmt = FEED_RATE_FMT_CH;
	mc_language.rapid_fmt = RAPID_FMT_CH;
	mc_language.gauge_feed = GAUGE_FEED_CH;
	mc_language.gauge_spindle = GAUGE_SPINDLE_CH;
	mc_language.gauge_rapid = GAUGE_RAPID_CH;
	mc_language.job_elapsed = JOB_ELAPSED_CH;
	mc_language.job_left = JOB_LEFT_CH;
	mc_language.job_feed_real = JOB_FEED_REAL_CH;
	mc_language.job_power_real = JOB_POWER_REAL_CH;
	mc_language.resume = RESUME_CH;
	mc_language.tool_board = TOOL_BOARD_CH;
	mc_language.test_title = TESTING_CH;
	mc_language.probe_check = PROBE_CHECK_CH;
	mc_language.x_limit_check = X_LIMIT_CHECK_CH;
	mc_language.y_limit_check = Y_LIMIT_CHECK_CH;
	mc_language.z_limit_check = Z_LIMIT_CHECK_CH;
	mc_language.sd_check = SD_CHECK_CH;
	mc_language.i2c_check = I2C_CHECK_CH;
	mc_language.cpu_temp = CPU_TEMP_CH;
	mc_language.test_warning = TEST_WARNING_CH;
}

const lv_font_t* mc_font(void) {
	// Roboto con Latin-1 para ES/EN/DE; con chino hace falta la fuente que trae los glifos chinos
	return (mks_grbl.language == SimpleChinese) ? &dlc32FontLatin : &roboto16Latin;
}

// Con el modo laser activo ($32=1) los rotulos de potencia y velocidad hablan de laser
// (Potencia / Velocidad); sin el, de husillo y avance. Se aplica tras cargar el idioma y se
// repite cuando cambia $32 (ver mks_page_data_updata). Las paginas ya dibujadas conservan el
// texto anterior hasta que se vuelven a pintar.
static void mc_language_apply_mode(void) {
	if(!laser_mode->get()) return;

	if(mks_grbl.language == SimpleChinese) {
		mc_language.spindle = SPINDLE_LASER_CH;
		mc_language.spindle_speed = SPINDLE_SPEED_LASER_CH;
		mc_language.spindle_speed_fmt = SPINDLE_SPEED_FMT_LASER_CH;
		mc_language.gauge_spindle = GAUGE_SPINDLE_LASER_CH;
		mc_language.feed_rate = FEED_RATE_LASER_CH;
		mc_language.feed_rate_fmt = FEED_RATE_FMT_LASER_CH;
		mc_language.gauge_feed = GAUGE_FEED_LASER_CH;
		mc_language.job_feed_real = JOB_FEED_REAL_LASER_CH;
	} else if(mks_grbl.language == Espanol) {
		mc_language.spindle = SPINDLE_LASER_ES;
		mc_language.spindle_speed = SPINDLE_SPEED_LASER_ES;
		mc_language.spindle_speed_fmt = SPINDLE_SPEED_FMT_LASER_ES;
		mc_language.gauge_spindle = GAUGE_SPINDLE_LASER_ES;
		mc_language.feed_rate = FEED_RATE_LASER_ES;
		mc_language.feed_rate_fmt = FEED_RATE_FMT_LASER_ES;
		mc_language.gauge_feed = GAUGE_FEED_LASER_ES;
		mc_language.job_feed_real = JOB_FEED_REAL_LASER_ES;
	} else if(mks_grbl.language == Portugues) {
		mc_language.spindle = SPINDLE_LASER_PT;
		mc_language.spindle_speed = SPINDLE_SPEED_LASER_PT;
		mc_language.spindle_speed_fmt = SPINDLE_SPEED_FMT_LASER_PT;
		mc_language.gauge_spindle = GAUGE_SPINDLE_LASER_PT;
		mc_language.feed_rate = FEED_RATE_LASER_PT;
		mc_language.feed_rate_fmt = FEED_RATE_FMT_LASER_PT;
		mc_language.gauge_feed = GAUGE_FEED_LASER_PT;
		mc_language.job_feed_real = JOB_FEED_REAL_LASER_PT;
	} else {
		mc_language.spindle = SPINDLE_LASER_EN;
		mc_language.spindle_speed = SPINDLE_SPEED_LASER_EN;
		mc_language.spindle_speed_fmt = SPINDLE_SPEED_FMT_LASER_EN;
		mc_language.gauge_spindle = GAUGE_SPINDLE_LASER_EN;
		mc_language.feed_rate = FEED_RATE_LASER_EN;
		mc_language.feed_rate_fmt = FEED_RATE_FMT_LASER_EN;
		mc_language.gauge_feed = GAUGE_FEED_LASER_EN;
		mc_language.job_feed_real = JOB_FEED_REAL_LASER_EN;
	}
}

void mc_language_init(void) {

	if(mks_grbl.language == Espanol) {

		mc_language.back = BACK_ES;
		mc_language.yes = YES_ES;
		mc_language.no = NO_ES;

		mc_language.control = CONTROL_ES;
		mc_language.sculpture = SCULPTURE_ES;
		mc_language.tool = TOOL_ES;
		mc_language.Mpos = MPOS_ES;
		mc_language.Wpos = WPOS_ES;
		mc_language.wifi_connect = WIFI_CONNECT_ES;
		mc_language.wifi_disconnect = WIFI_DISCONNECT_ES;

		mc_language.xy_clear = XY_CLEAR_ES;
		mc_language.z_clear = Z_CLEAR_ES;
		mc_language.knife = KNIFE_ES;
		mc_language.next = NEXT_ES;
		mc_language.up = UP_ES;
		mc_language.cooling = COOLING_ES;
		mc_language.position = POSITION_ES;
		mc_language.speed_high = SPEED_HIGH_ES;
		mc_language.speed_mid = SPEED_MID_ES;
		mc_language.speed_low = SPEED_LOW_ES;
		mc_language.spindle = SPINDLE_ES;
		mc_language.carve = CARVE_ES;

		mc_language.dis_no_sd_card = DIS_NO_SDCARD_ES;

		mc_language.hold = HOLD_ES;
		mc_language.cycle = CYCLE_ES;
		mc_language.stop = STOP_ES;
		mc_language.adjust = ADJUST_ES;
		mc_language.spindle_speed = SPINDLE_SPPED_ES;
		mc_language.feed_rate = FEED_RATE_ES;
		mc_language.rapid_speed = RAPID_SPEED_ES;
		mc_language.carve_times = CARVE_TIMES_ES;

		mc_language.dis_stop_print = DIS_STOP_CARVE_ES;
		mc_language.dis_homing = DIS_HOMEING_ES;
		mc_language.dis_no_hard_homing = DIS_NO_HARD_HOME_ES;
		mc_language.dis_homing_succeed = DIS_HOME_SUCCEED_ES;
		mc_language.dis_homing_fail = DIS_HOME_FAIL_ES;
		mc_language.dis_probe_set = DIS_PROBE_SET_ES;
		mc_language.dis_probe_succeed = DIS_PROBE_SECCEED_ES;
		mc_language.dis_probe_fail = DIS_PROBE_FAIL_ES;

		/* botones y popups */
		mc_language.cancel = CANCEL_ES;
		mc_language.frame = FRAME_ES;
		mc_language.carve_file_sure = CARVE_FILE_SURE_ES;
		mc_language.pause = PAUSE_ES;
		mc_language.start = START_ES;
		mc_language.confirm = CONFIRM_ES;
		mc_language.add = ADD_ES;
		mc_language.reduce = REDUCE_ES;
		mc_language.exit = EXIT_ES;
		mc_language.language = LANGUAGE_ES;
		mc_language.scanf = SCANF_ES;
		mc_language.reconnect = RECONNECT_ES;
		mc_language.connect = CONNECT_ES;
		mc_language.password = PASSWORD_ES;
		mc_language.z_home = Z_HOME_ES;
		mc_language.update_title = UPDATE_TITLE_ES;

		/* avisos y mensajes */
		mc_language.dis_info = INFO_ES;
		mc_language.dis_warning = WARNING_ES;
		mc_language.dis_error = ERROR_ES;
		mc_language.dis_pos_succeed = DIS_POS_SUCCEED_ES;
		mc_language.dis_wait_mc_stop = DIS_WAIT_MC_STOP_ES;
		mc_language.dis_unlock = DIS_UNLOCK_ES;
		mc_language.dis_unlock_success = UNLOCK_SUCCESS_ES;
		mc_language.dis_setting_error = SETTING_ERROR_ES;
		mc_language.dis_set_6_1 = SET_6_1_ES;
		mc_language.dis_wait_idle = WAIT_IDLE_ES;
		mc_language.dis_file_too_big = FILE_TOO_BIG_ES;
		mc_language.dis_continue_sure = CONTINUE_SURE_ES;
		mc_language.dis_file_loading = FILE_LOADING_ES;
		mc_language.dis_loading_file = LOADING_FILE_ES;
		mc_language.dis_sd_busy = SD_BUSY_ES;
		mc_language.dis_running = RUNNING_ES;
		mc_language.dis_print_stop_sure = PRINT_STOP_SURE_ES;
		mc_language.dis_print_done = PRINT_DONE_ES;
		mc_language.wifi_scanning = WIFI_SCANNING_ES;
		mc_language.wifi_connecting = WIFI_CONNECTING_ES;
		mc_language.wifi_disconnecting = WIFI_DISCONNECTING_ES;
		mc_language.wifi_pwd_prompt = WIFI_PWD_PROMPT_ES;
		mc_language.wifi_status_on = WIFI_STATUS_ON_ES;
		mc_language.wifi_status_off = WIFI_STATUS_OFF_ES;
		mc_language.dis_update_succeed = UPDATE_SUCCEED_ES;
		mc_language.dis_update_restart = UPDATE_RESTART_ES;
		mc_language.dis_update_fail = UPDATE_FAIL_ES;
		mc_language.dis_update_fail_help = UPDATE_FAIL_HELP_ES;

		/* formatos con %d */
		mc_language.power_fmt = POWER_FMT_ES;
		mc_language.speed_fmt = SPEED_FMT_ES;
		mc_language.spindle_speed_fmt = SPINDLE_SPEED_FMT_ES;
		mc_language.feed_rate_fmt = FEED_RATE_FMT_ES;
		mc_language.rapid_fmt = RAPID_FMT_ES;
		mc_language.gauge_feed = GAUGE_FEED_ES;
		mc_language.gauge_spindle = GAUGE_SPINDLE_ES;
		mc_language.gauge_rapid = GAUGE_RAPID_ES;
		mc_language.job_elapsed = JOB_ELAPSED_ES;
		mc_language.job_left = JOB_LEFT_ES;
		mc_language.job_feed_real = JOB_FEED_REAL_ES;
		mc_language.job_power_real = JOB_POWER_REAL_ES;
		mc_language.resume = RESUME_ES;
		mc_language.tool_board = TOOL_BOARD_ES;

		/* pagina de pruebas */
		mc_language.test_title = TESTING_ES;
		mc_language.probe_check = PROBE_CHECK_ES;
		mc_language.x_limit_check = X_LIMIT_CHECK_ES;
		mc_language.y_limit_check = Y_LIMIT_CHECK_ES;
		mc_language.z_limit_check = Z_LIMIT_CHECK_ES;
		mc_language.sd_check = SD_CHECK_ES;
		mc_language.i2c_check = I2C_CHECK_ES;
		mc_language.cpu_temp = CPU_TEMP_ES;
		mc_language.test_warning = TEST_WARNING_ES;

	} else if(mks_grbl.language == Portugues) {

		mc_language.back = BACK_PT;
		mc_language.yes = YES_PT;
		mc_language.no = NO_PT;

		mc_language.control = CONTROL_PT;
		mc_language.sculpture = SCULPTURE_PT;
		mc_language.tool = TOOL_PT;
		mc_language.Mpos = MPOS_PT;
		mc_language.Wpos = WPOS_PT;
		mc_language.wifi_connect = WIFI_CONNECT_PT;
		mc_language.wifi_disconnect = WIFI_DISCONNECT_PT;

		mc_language.xy_clear = XY_CLEAR_PT;
		mc_language.z_clear = Z_CLEAR_PT;
		mc_language.knife = KNIFE_PT;
		mc_language.next = NEXT_PT;
		mc_language.up = UP_PT;
		mc_language.cooling = COOLING_PT;
		mc_language.position = POSITION_PT;
		mc_language.speed_high = SPEED_HIGH_PT;
		mc_language.speed_mid = SPEED_MID_PT;
		mc_language.speed_low = SPEED_LOW_PT;
		mc_language.spindle = SPINDLE_PT;
		mc_language.carve = CARVE_PT;

		mc_language.dis_no_sd_card = DIS_NO_SDCARD_PT;

		mc_language.hold = HOLD_PT;
		mc_language.cycle = CYCLE_PT;
		mc_language.stop = STOP_PT;
		mc_language.adjust = ADJUST_PT;
		mc_language.spindle_speed = SPINDLE_SPPED_PT;
		mc_language.feed_rate = FEED_RATE_PT;
		mc_language.rapid_speed = RAPID_SPEED_PT;
		mc_language.carve_times = CARVE_TIMES_PT;

		mc_language.dis_stop_print = DIS_STOP_CARVE_PT;
		mc_language.dis_homing = DIS_HOMEING_PT;
		mc_language.dis_no_hard_homing = DIS_NO_HARD_HOME_PT;
		mc_language.dis_homing_succeed = DIS_HOME_SUCCEED_PT;
		mc_language.dis_homing_fail = DIS_HOME_FAIL_PT;
		mc_language.dis_probe_set = DIS_PROBE_SET_PT;
		mc_language.dis_probe_succeed = DIS_PROBE_SECCEED_PT;
		mc_language.dis_probe_fail = DIS_PROBE_FAIL_PT;

		/* botones y popups */
		mc_language.cancel = CANCEL_PT;
		mc_language.frame = FRAME_PT;
		mc_language.carve_file_sure = CARVE_FILE_SURE_PT;
		mc_language.pause = PAUSE_PT;
		mc_language.start = START_PT;
		mc_language.confirm = CONFIRM_PT;
		mc_language.add = ADD_PT;
		mc_language.reduce = REDUCE_PT;
		mc_language.exit = EXIT_PT;
		mc_language.language = LANGUAGE_PT;
		mc_language.scanf = SCANF_PT;
		mc_language.reconnect = RECONNECT_PT;
		mc_language.connect = CONNECT_PT;
		mc_language.password = PASSWORD_PT;
		mc_language.z_home = Z_HOME_PT;
		mc_language.update_title = UPDATE_TITLE_PT;

		/* avisos y mensajes */
		mc_language.dis_info = INFO_PT;
		mc_language.dis_warning = WARNING_PT;
		mc_language.dis_error = ERROR_PT;
		mc_language.dis_pos_succeed = DIS_POS_SUCCEED_PT;
		mc_language.dis_wait_mc_stop = DIS_WAIT_MC_STOP_PT;
		mc_language.dis_unlock = DIS_UNLOCK_PT;
		mc_language.dis_unlock_success = UNLOCK_SUCCESS_PT;
		mc_language.dis_setting_error = SETTING_ERROR_PT;
		mc_language.dis_set_6_1 = SET_6_1_PT;
		mc_language.dis_wait_idle = WAIT_IDLE_PT;
		mc_language.dis_file_too_big = FILE_TOO_BIG_PT;
		mc_language.dis_continue_sure = CONTINUE_SURE_PT;
		mc_language.dis_file_loading = FILE_LOADING_PT;
		mc_language.dis_loading_file = LOADING_FILE_PT;
		mc_language.dis_sd_busy = SD_BUSY_PT;
		mc_language.dis_running = RUNNING_PT;
		mc_language.dis_print_stop_sure = PRINT_STOP_SURE_PT;
		mc_language.dis_print_done = PRINT_DONE_PT;
		mc_language.wifi_scanning = WIFI_SCANNING_PT;
		mc_language.wifi_connecting = WIFI_CONNECTING_PT;
		mc_language.wifi_disconnecting = WIFI_DISCONNECTING_PT;
		mc_language.wifi_pwd_prompt = WIFI_PWD_PROMPT_PT;
		mc_language.wifi_status_on = WIFI_STATUS_ON_PT;
		mc_language.wifi_status_off = WIFI_STATUS_OFF_PT;
		mc_language.dis_update_succeed = UPDATE_SUCCEED_PT;
		mc_language.dis_update_restart = UPDATE_RESTART_PT;
		mc_language.dis_update_fail = UPDATE_FAIL_PT;
		mc_language.dis_update_fail_help = UPDATE_FAIL_HELP_PT;

		/* formatos con %d */
		mc_language.power_fmt = POWER_FMT_PT;
		mc_language.speed_fmt = SPEED_FMT_PT;
		mc_language.spindle_speed_fmt = SPINDLE_SPEED_FMT_PT;
		mc_language.feed_rate_fmt = FEED_RATE_FMT_PT;
		mc_language.rapid_fmt = RAPID_FMT_PT;
		mc_language.gauge_feed = GAUGE_FEED_PT;
		mc_language.gauge_spindle = GAUGE_SPINDLE_PT;
		mc_language.gauge_rapid = GAUGE_RAPID_PT;
		mc_language.job_elapsed = JOB_ELAPSED_PT;
		mc_language.job_left = JOB_LEFT_PT;
		mc_language.job_feed_real = JOB_FEED_REAL_PT;
		mc_language.job_power_real = JOB_POWER_REAL_PT;
		mc_language.resume = RESUME_PT;
		mc_language.tool_board = TOOL_BOARD_PT;

		/* pagina de pruebas */
		mc_language.test_title = TESTING_PT;
		mc_language.probe_check = PROBE_CHECK_PT;
		mc_language.x_limit_check = X_LIMIT_CHECK_PT;
		mc_language.y_limit_check = Y_LIMIT_CHECK_PT;
		mc_language.z_limit_check = Z_LIMIT_CHECK_PT;
		mc_language.sd_check = SD_CHECK_PT;
		mc_language.i2c_check = I2C_CHECK_PT;
		mc_language.cpu_temp = CPU_TEMP_PT;
		mc_language.test_warning = TEST_WARNING_PT;

	} else {  // SimpleChinese / English -> cadenas EN

		mc_language.back = BACK_EN;
		mc_language.yes = YES_EN;
		mc_language.no = NO_EN;

		mc_language.control = CONTROL_EN;
		mc_language.sculpture = SCULPTURE_EN;
		mc_language.tool = TOOL_EN;
		mc_language.Mpos = MPOS_EN;
		mc_language.Wpos = WPOS_EN;
		mc_language.wifi_connect = WIFI_CONNECT_EN;
		mc_language.wifi_disconnect = WIFI_DISCONNECT_EN;

		mc_language.xy_clear = XY_CLEAR_EN;
		mc_language.z_clear = Z_CLEAR_EN;
		mc_language.knife = KNIFE_EN;
		mc_language.next = NEXT_EN;
		mc_language.up = UP_EN;
		mc_language.cooling = COOLING_EN;
		mc_language.position = POSITION_EN;
		mc_language.speed_high = SPEED_HIGH_EN;
		mc_language.speed_mid = SPEED_MID_EN;
		mc_language.speed_low = SPEED_LOW_EN;
		mc_language.spindle = SPINDLE_EN;
		mc_language.carve = CARVE_EN;

		mc_language.dis_no_sd_card = DIS_NO_SDCARD_EN;

		mc_language.hold = HOLD_EN;
		mc_language.cycle = CYCLE_EN;
		mc_language.stop = STOP_EN;
		mc_language.adjust = ADJUST_EN;
		mc_language.spindle_speed = SPINDLE_SPPED_EN;
		mc_language.feed_rate = FEED_RATE_EN;
		mc_language.rapid_speed = RAPID_SPEED_EN;
		mc_language.carve_times = CARVE_TIMES_EN;

		mc_language.dis_stop_print = DIS_STOP_CARVE_EN;
		mc_language.dis_homing = DIS_HOMEING_EN;
		mc_language.dis_no_hard_homing = DIS_NO_HARD_HOME_EN;
		mc_language.dis_homing_succeed = DIS_HOME_SUCCEED_EN;
		mc_language.dis_homing_fail = DIS_HOME_FAIL_EN;
		mc_language.dis_probe_set = DIS_PROBE_SET;
		mc_language.dis_probe_succeed = DIS_PROBE_SECCEED_EN;
		mc_language.dis_probe_fail = DIS_PROBE_FAIL_EN;

		/* botones y popups */
		mc_language.cancel = CANCEL_EN;
		mc_language.frame = FRAME_EN;
		mc_language.carve_file_sure = CARVE_FILE_SURE_EN;
		mc_language.pause = PAUSE_EN;
		mc_language.start = START_EN;
		mc_language.confirm = CONFIRM_EN;
		mc_language.add = ADD_EN;
		mc_language.reduce = REDUCE_EN;
		mc_language.exit = EXIT_EN;
		mc_language.language = LANGUAGE_EN;
		mc_language.scanf = SCANF_EN;
		mc_language.reconnect = RECONNECT_EN;
		mc_language.connect = CONNECT_EN;
		mc_language.password = PASSWORD_EN;
		mc_language.z_home = Z_HOME_EN;
		mc_language.update_title = UPDATE_TITLE_EN;

		/* avisos y mensajes */
		mc_language.dis_info = INFO_EN;
		mc_language.dis_warning = WARNING_EN;
		mc_language.dis_error = ERROR_EN;
		mc_language.dis_pos_succeed = DIS_POS_SUCCEED;
		mc_language.dis_wait_mc_stop = DIS_WAIT_MC_STOP;
		mc_language.dis_unlock = DIS_UNLOCK;
		mc_language.dis_unlock_success = UNLOCK_SUCCESS_EN;
		mc_language.dis_setting_error = SETTING_ERROR_EN;
		mc_language.dis_set_6_1 = SET_6_1_EN;
		mc_language.dis_wait_idle = WAIT_IDLE_EN;
		mc_language.dis_file_too_big = FILE_TOO_BIG_EN;
		mc_language.dis_continue_sure = CONTINUE_SURE_EN;
		mc_language.dis_file_loading = FILE_LOADING_EN;
		mc_language.dis_loading_file = LOADING_FILE_EN;
		mc_language.dis_sd_busy = SD_BUSY_EN;
		mc_language.dis_running = RUNNING_EN;
		mc_language.dis_print_stop_sure = PRINT_STOP_SURE_EN;
		mc_language.dis_print_done = PRINT_DONE_EN;
		mc_language.wifi_scanning = WIFI_SCANNING_EN;
		mc_language.wifi_connecting = WIFI_CONNECTING_EN;
		mc_language.wifi_disconnecting = WIFI_DISCONNECTING_EN;
		mc_language.wifi_pwd_prompt = WIFI_PWD_PROMPT_EN;
		mc_language.wifi_status_on = WIFI_STATUS_ON_EN;
		mc_language.wifi_status_off = WIFI_STATUS_OFF_EN;
		mc_language.dis_update_succeed = UPDATE_SUCCEED_EN;
		mc_language.dis_update_restart = UPDATE_RESTART_EN;
		mc_language.dis_update_fail = UPDATE_FAIL_EN;
		mc_language.dis_update_fail_help = UPDATE_FAIL_HELP_EN;

		/* formatos con %d */
		mc_language.power_fmt = POWER_FMT_EN;
		mc_language.speed_fmt = SPEED_FMT_EN;
		mc_language.spindle_speed_fmt = SPINDLE_SPEED_FMT_EN;
		mc_language.feed_rate_fmt = FEED_RATE_FMT_EN;
		mc_language.rapid_fmt = RAPID_FMT_EN;
		mc_language.gauge_feed = GAUGE_FEED_EN;
		mc_language.gauge_spindle = GAUGE_SPINDLE_EN;
		mc_language.gauge_rapid = GAUGE_RAPID_EN;
		mc_language.job_elapsed = JOB_ELAPSED_EN;
		mc_language.job_left = JOB_LEFT_EN;
		mc_language.job_feed_real = JOB_FEED_REAL_EN;
		mc_language.job_power_real = JOB_POWER_REAL_EN;
		mc_language.resume = RESUME_EN;
		mc_language.tool_board = TOOL_BOARD_EN;

		/* pagina de pruebas */
		mc_language.test_title = TESTING_EN;
		mc_language.probe_check = PROBE_CHECK_EN;
		mc_language.x_limit_check = X_LIMIT_CHECK_EN;
		mc_language.y_limit_check = Y_LIMIT_CHECK_EN;
		mc_language.z_limit_check = Z_LIMIT_CHECK_EN;
		mc_language.sd_check = SD_CHECK_EN;
		mc_language.i2c_check = I2C_CHECK_EN;
		mc_language.cpu_temp = CPU_TEMP_EN;
		mc_language.test_warning = TEST_WARNING_EN;
	}

	if(mks_grbl.language == SimpleChinese) mc_language_apply_chinese();

	mc_language_apply_mode();
}


enum {
	ID_L_BACK,
	ID_L_CN,
	ID_L_EN,
	ID_L_PT,
	ID_L_ES,
};

static uint8_t get_event(lv_obj_t* obj) {

    if(obj == language_page.imgbtn_back)                		return ID_L_BACK;
    else if(obj == language_page.imgbtn_simple_cn)           	return ID_L_CN;
	else if(obj == language_page.imgbtn_en)						return ID_L_EN;
	else if(obj == language_page.imgbtn_pt)						return ID_L_PT;
	else if(obj == language_page.imgbtn_es)						return ID_L_ES;
	return 255;  // sin coincidencia: el id basura podia mandar $40=2 (idioma)
}

/*
 *	language num:
 *	0------cn
 *	1------en
 *	2------pt
 *	3------es
 */
static void set_language(uint8_t language) {

	switch(language) {

		case 0:
			mks_grbl.language = SimpleChinese;
		break;

		case 1:
			mks_grbl.language = English;
		break;

		case 2:
			mks_grbl.language = Portugues;
		break;

		case 3:
			mks_grbl.language = Espanol;
		break;
	}
	mc_language_init();
}

static void enent_handler_back(void) {

	mks_clear_language();
	mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
	mks_ui_page.wait_count = DEFAULT_UI_COUNT;
	mks_draw_tool();   // pone mks_ui_page = MKS_UI_Tool
}

// Marca el boton del idioma activo. OJO: el puntero de estilo se guarda en el
// objeto, asi que hay que pasar SIEMPRE &mks_global... (nunca una copia local).
// Si un boton no existe (NULL) se ignora: pasarle NULL hundiria el LVGL.
static void language_btn_set_selected(lv_obj_t* btn, bool selected) {

	if(btn == NULL) return;

	const lv_style_t* style = selected ? &mks_global.language_btn_pr_style
	                                   : &mks_global.language_btn_rel_style;
	lv_btn_set_style(btn, LV_BTN_STYLE_REL, (lv_style_t*)style);
	lv_btn_set_style(btn, LV_BTN_STYLE_PR,  (lv_style_t*)style);
}

void set_language_btn_style(uint8_t language) {

	language_btn_set_selected(language_page.imgbtn_simple_cn, language == 0);
	language_btn_set_selected(language_page.imgbtn_en,        language == 1);
	language_btn_set_selected(language_page.imgbtn_pt,        language == 2);
	language_btn_set_selected(language_page.imgbtn_es,        language == 3);
}


char str_language_ch[10] = "$40=0\n";
char str_language_en[10] = "$40=1\n";
char str_language_pt[10] = "$40=2\n"; 
char str_language_es[10] = "$40=3\n";
static void event_handler(lv_obj_t* obj, lv_event_t event) {

	uint8_t id = get_event(obj);

    if(event == LV_EVENT_PRESSED) { ts35_beep_on(); }

    if((event == LV_EVENT_RELEASED) || (event == LV_EVENT_PRESS_LOST))  {

        ts35_beep_off();

        switch(id) {
			case ID_L_BACK: enent_handler_back(); break;
			case ID_L_CN: 
				set_language(0); 
				set_language_btn_style(0);
				MKS_GRBL_CMD_SEND(str_language_ch);
			break;
			case ID_L_EN: 
				set_language(1); 
				set_language_btn_style(1);
				MKS_GRBL_CMD_SEND(str_language_en);
			break;
			case ID_L_PT: 
				set_language(2); 
				set_language_btn_style(2);
				MKS_GRBL_CMD_SEND(str_language_pt);
			break;
			case ID_L_ES:
				set_language(3);
				set_language_btn_style(3);
				MKS_GRBL_CMD_SEND(str_language_es);
			break;
        }
    }
}

void draw_language(void) {

	uint8_t language_num = 0;

	mks_global.mks_src_1 = lv_obj_create(mks_global.mks_src, NULL);
	lv_obj_set_size(mks_global.mks_src_1, about_src1_x_size, about_src1_y_size);
    lv_obj_set_pos(mks_global.mks_src_1, about_src1_x, about_src1_y);
    lv_obj_set_style(mks_global.mks_src_1, &mks_global.mks_src_1_style);

	// tool_page.imgbtn_back = lv_imgbtn_creat_mks(mks_global.mks_src_1, tool_page.imgbtn_back, &png_back_pre, &back, LV_ALIGN_IN_LEFT_MID, 10, -15, event_handler);

	language_page.imgbtn_back = lv_imgbtn_creat_mks(mks_global.mks_src_1, language_page.imgbtn_back, &png_back_pre, &back, LV_ALIGN_IN_LEFT_MID, 10, -15, event_handler);

	// Botones 130x50 en columna izquierda. La pantalla es 480x320 y la cabecera
	// (about_src1) ocupa y=10..100, asi que caben las filas y=110, 170 y 230.
	// Cuatro filas de 42 px con paso de 48 (y=108..294).
	language_page.imgbtn_simple_cn = mks_lv_btn_set(mks_global.mks_src, language_page.imgbtn_simple_cn, 130, 42, 20, 108, event_handler);
	language_page.imgbtn_en = mks_lv_btn_set(mks_global.mks_src, language_page.imgbtn_en, 130, 42, 20, 108 + 48, event_handler);
	language_page.imgbtn_pt = mks_lv_btn_set(mks_global.mks_src, language_page.imgbtn_pt, 130, 42, 20, 108 + 96, event_handler);
	language_page.imgbtn_es = mks_lv_btn_set(mks_global.mks_src, language_page.imgbtn_es, 130, 42, 20, 108 + 144, event_handler);

	language_num = language_select->get();

	set_language_btn_style(language_num);
	
	language_page.label_back = label_for_imgbtn_name(mks_global.mks_src, language_page.label_back, language_page.imgbtn_back, 0, 0, mc_language.back);
	language_page.label_simple_cn = label_for_btn_name(language_page.imgbtn_simple_cn, language_page.label_simple_cn , 0, 0, "中文");
	language_page.label_en = label_for_btn_name(language_page.imgbtn_en, language_page.label_en, 0, 0, "English");
	language_page.label_pt = label_for_btn_name(language_page.imgbtn_pt, language_page.label_pt, 0, 0, "Portugu\xc3\xaas");
	language_page.label_es = label_for_btn_name(language_page.imgbtn_es, language_page.label_es, 0, 0, "Español");
	mks_ui_page.mks_ui_page = MKS_UI_LANGUAGE;
}

void mks_clear_language(void) {
    lv_obj_clean(mks_global.mks_src);
}
