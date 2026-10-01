#include "MKS_draw_language.h"

mc_lg_muilt_t mc_language;
LANGUAGE_PAGE_T language_page;

// Carga en mc_language las cadenas que van a usar los labels del LCD.
// Depende de mks_grbl.language, que arranca desde $40 (mks_grbl_parg_init)
// y cambia al pulsar un boton de esta pagina o al escribir $40 a distancia
// (refresco periodico en MKS_FREERTOS_TASK.cpp).
//
// Corregido (bug original): aqui ponia "=" (asignacion) y no "==", asi que
// esta funcion cargaba SIEMPRE ingles y ademas pisaba mks_grbl.language.
// Chino y aleman no tienen fichero propio en este arbol (solo language_en.h
// y ahora language_es.h), asi que comparten las cadenas EN: si no, los
// punteros quedarian NULL y las etiquetas se quedarian con el "Text" por
// defecto de LVGL.
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

	} else {  // SimpleChinese / English / Deutsch -> cadenas EN

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
}


enum {
	ID_L_BACK,
	ID_L_CN,
	ID_L_EN,
	ID_L_DE,
	ID_L_ES,
};

static uint8_t get_event(lv_obj_t* obj) {

    if(obj == language_page.imgbtn_back)                		return ID_L_BACK;
    else if(obj == language_page.imgbtn_simple_cn)           	return ID_L_CN;
	else if(obj == language_page.imgbtn_en)						return ID_L_EN;
	else if(obj == language_page.imgbtn_de)						return ID_L_DE;
	else if(obj == language_page.imgbtn_es)						return ID_L_ES;
	return 255;  // sin coincidencia: el id basura podia mandar $40=2 (idioma)
}

/*
 *	language num:
 *	0------cn
 *	1------en
 *	2------de
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
			mks_grbl.language = Deutsch;
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
// imgbtn_de no se crea en draw_language(), por eso no se toca: pasarle NULL
// hundiria el LVGL en lv_btn_set_style.
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
	language_btn_set_selected(language_page.imgbtn_es,        language == 3);
	// language == 2 (Deutsch): sin boton en pantalla, ninguno destacado.
}


char str_language_ch[10] = "$40=0\n";
char str_language_en[10] = "$40=1\n";
char str_language_de[10] = "$40=2\n"; 
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
			case ID_L_DE: 
				set_language(2); 
				set_language_btn_style(2);
				MKS_GRBL_CMD_SEND(str_language_de);
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
	language_page.imgbtn_simple_cn = mks_lv_btn_set(mks_global.mks_src, language_page.imgbtn_simple_cn, 130, 50, 20, 110, event_handler);
	language_page.imgbtn_en = mks_lv_btn_set(mks_global.mks_src, language_page.imgbtn_en, 130, 50, 20, 110 + 60, event_handler);
	language_page.imgbtn_es = mks_lv_btn_set(mks_global.mks_src, language_page.imgbtn_es, 130, 50, 20, 110 + 120, event_handler);

	language_num = language_select->get();

	set_language_btn_style(language_num);
	
	language_page.label_back = label_for_imgbtn_name(mks_global.mks_src, language_page.label_back, language_page.imgbtn_back, 0, 0, mc_language.back);
	language_page.label_simple_cn = label_for_btn_name(language_page.imgbtn_simple_cn, language_page.label_simple_cn , 0, 0, "中文");
	language_page.label_en = label_for_btn_name(language_page.imgbtn_en, language_page.label_en, 0, 0, "English");
	language_page.label_es = label_for_btn_name(language_page.imgbtn_es, language_page.label_es, 0, 0, "Español");
	mks_ui_page.mks_ui_page = MKS_UI_LANGUAGE;
}

void mks_clear_language(void) {
    lv_obj_clean(mks_global.mks_src);
}
