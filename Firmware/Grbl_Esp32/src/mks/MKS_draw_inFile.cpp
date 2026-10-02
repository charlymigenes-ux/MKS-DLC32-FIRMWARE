#include "MKS_draw_inFile.h"
#include "MKS_draw_language.h"   // mc_language: textos del LCD

inFILE_PAGE_T infile_page;

LV_IMG_DECLARE(X_N);			
LV_IMG_DECLARE(X_P);			
LV_IMG_DECLARE(Y_N);			
LV_IMG_DECLARE(Y_P);
LV_IMG_DECLARE(back);

LV_IMG_DECLARE(png_infile_cave);
LV_IMG_DECLARE(png_infile_pos);
LV_IMG_DECLARE(png_infile_frame);

LV_IMG_DECLARE(Positionting);
LV_IMG_DECLARE(png_pos_pre);
	
LV_IMG_DECLARE(png_m_up);
LV_IMG_DECLARE(png_m_right);		
LV_IMG_DECLARE(png_m_left);		
LV_IMG_DECLARE(png_m_down);

LV_IMG_DECLARE(png_infile_pos_pre);
LV_IMG_DECLARE(png_infile_frame_pre);
LV_IMG_DECLARE(png_infile_cave_pre);

LV_IMG_DECLARE(png_m_z_n);			
LV_IMG_DECLARE(png_m_z_n_pre);		
LV_IMG_DECLARE(png_m_z_p);			
LV_IMG_DECLARE(png_m_z_p_pre);
LV_IMG_DECLARE(png_home_pre);
LV_IMG_DECLARE(png_hhome_pre);		
LV_IMG_DECLARE(png_unlock_pre);		
LV_IMG_DECLARE(png_pos_pre);	
LV_IMG_DECLARE(png_m_up);
LV_IMG_DECLARE(png_m_right);		
LV_IMG_DECLARE(png_m_left);		
LV_IMG_DECLARE(png_m_down);
LV_IMG_DECLARE(png_back_pre);	




enum {

	ID_INF_UP,
	ID_INF_DOWN,
	ID_INF_LEFT,
	ID_INF_RIGHT,
	ID_INF_Z_UP,
	ID_INF_Z_DOWN,
	ID_INF_STEP,
	ID_INF_SPEED,
	ID_INF_XY_POS,
	ID_INF_Z_POS,
	ID_INF_CARVE,
	ID_INF_BACK,
	ID_INF_XY_HOME,
	ID_INF_Z_HOME,
	ID_INF_L_NEXT,
	ID_INF_L_UP,
	ID_INF_KNIFE,
};

static void disp_imgbtn(void);
static void disp_imgbtn_1(void);
static void disp_imgbtn_1_del(void);
static void disp_imgbtn_2(void);
static void disp_imgbtn_2_del(void);
static void disp_label(void);
static void disp_btn(void);
static void inf_event(lv_obj_t* obj, lv_event_t event);
static uint8_t get_id(lv_obj_t* obj) {

    if      (obj == move_page.y_n)  			return ID_INF_UP;
    else if (obj == move_page.y_p)     			return ID_INF_DOWN;
    else if (obj == move_page.x_n)				return ID_INF_LEFT;
    else if (obj == move_page.x_p) 				return ID_INF_RIGHT;
	else if (obj == move_page.z_n) 				return ID_INF_Z_UP;
	else if (obj == move_page.z_p) 				return ID_INF_Z_DOWN;
	else if (obj == move_page.xy_home)     		return ID_INF_XY_HOME;
	else if (obj == move_page.z_home)     		return ID_INF_Z_HOME;
	else if (obj == infile_page.btn_cancle)   	return ID_INF_BACK;
	else if (obj == move_page.btn_len)     		return ID_INF_STEP;
	else if (obj == move_page.btn_speed)		return ID_INF_SPEED;
	else if (obj == infile_page.btn_sculpture)	return ID_INF_CARVE; 
	else if (obj == move_page.next)				return ID_INF_L_NEXT;
	else if (obj == move_page.up)				return ID_INF_L_UP;
	else if (obj == move_page.xy_clear)			return ID_INF_XY_POS;
	else if (obj == move_page.z_clear)			return ID_INF_Z_POS;
	else if (obj == move_page.knife)			return ID_INF_KNIFE;
	return 255;  // sin coincidencia: sin accion
}

static void event_handler_cave_yes(lv_obj_t* obj, lv_event_t event) {

	if (event == LV_EVENT_RELEASED) {
		mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
		cavre_popup_del();
		lv_obj_clean(mks_global.mks_src);
		start_print();
	}
}

static void event_handler_cave_no(lv_obj_t* obj, lv_event_t event) {

	if (event == LV_EVENT_RELEASED) {
		cavre_popup_del();
	}
}




static void set_cooling(lv_obj_t* obj, lv_event_t event) {

	if (event == LV_EVENT_RELEASED) {
		CoolantState state;
		state = coolant_get_state();
		if(state.Flood == 1) {
			MKS_GRBL_CMD_SEND("M9\n");
		}
		else{
			MKS_GRBL_CMD_SEND("M8\n");
		}
	}
}

static void event_handler_back(void) {

	mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
	lv_obj_clean(mks_global.mks_src);
	mks_draw_ready();
}

static void event_handler_sure(void) {
	char temp[128];
	memset(temp, 0, sizeof(temp));
	memcpy(temp, mks_file_list.filename_str[mks_file_list.file_choose], sizeof(temp));
	if(temp[0]=='/') temp[0] = ' ';
	mks_draw_cavre_popup(temp, event_handler_cave_yes, event_handler_cave_no);
}


static void event_handler_com_info(lv_obj_t* obj, lv_event_t event){

	if (event == LV_EVENT_RELEASED) {
		common_popup_com_del();
	}
}


static void event_handler_carve_set(lv_obj_t* obj, lv_event_t event){

	if (event == LV_EVENT_RELEASED) {
		infile_clean_obj(mks_global.mks_src_3);
		mks_draw_freaure();
	}
}

static void event_henadle_pupup_com(lv_obj_t* obj, lv_event_t event) { 

	if (event == LV_EVENT_RELEASED) {
		common_popup_com_del();
	}
}

static void set_xy_pos(lv_obj_t* obj, lv_event_t event) {
	if(event != LV_EVENT_RELEASED) return;
	if(sys.state == State::Idle && mks_get_motor_status() ) {
		MKS_GRBL_CMD_SEND("G92X0Y0\n");
		mks_draw_common_popup_info_com(mc_language.dis_info, mc_language.dis_pos_succeed, " ", event_henadle_pupup_com);
	}else {
		mks_draw_common_popup_info_com(mc_language.dis_warning, mc_language.dis_wait_mc_stop, " ", event_henadle_pupup_com);
	}
}

static void set_z_pos(lv_obj_t* obj, lv_event_t event) {
	if(event != LV_EVENT_RELEASED) return;
	if(sys.state == State::Idle && mks_get_motor_status() ) {
		MKS_GRBL_CMD_SEND("G92Z0\n");
		mks_draw_common_popup_info_com(mc_language.dis_info, mc_language.dis_pos_succeed, " ", event_henadle_pupup_com);
	}else {
		mks_draw_common_popup_info_com(mc_language.dis_warning, mc_language.dis_wait_mc_stop, " ", event_henadle_pupup_com);
	}
}

static void set_xyz_pos(lv_obj_t* obj, lv_event_t event) {

	if(event == LV_EVENT_RELEASED ) {
		set_click_status(false);

		if(sys.state == State::Idle && mks_get_motor_status() ) {
			MKS_GRBL_CMD_SEND("G92X0Y0Z0\n");
			mks_draw_common_popup_info_com(mc_language.dis_info, mc_language.dis_pos_succeed, " ", event_henadle_pupup_com);
		}else {
			mks_draw_common_popup_info_com(mc_language.dis_warning, mc_language.dis_wait_mc_stop, " ", event_henadle_pupup_com);
		}
	}
}

static void set_knife() {

	if(probe_invert->get()) {
		MKS_GRBL_CMD_SEND("G21 G91 G38.2 Z-50 F80\n");
		mks_draw_common_pupup_info(mc_language.dis_info, mc_language.dis_probe_set, " ");
		probe_run.status = PROBE_STAR;

	}else {
		set_click_status(true);
		mks_draw_common_popup_info_com(mc_language.dis_info, mc_language.dis_setting_error, mc_language.dis_set_6_1, event_henadle_pupup_com);
	}
}

static void event_handler_len_set(void){

	if (mks_grbl.move_dis == M_0_1_MM) {
		mks_grbl.move_dis = M_1_MM;
		lv_label_set_text(move_page.label_len, "1mm");
	}else if(mks_grbl.move_dis == M_1_MM) {
		mks_grbl.move_dis = M_10_MM;
		lv_label_set_text(move_page.label_len, "10mm");
	}else if(mks_grbl.move_dis == M_10_MM) {
		mks_grbl.move_dis = M_0_1_MM;
		lv_label_set_text(move_page.label_len, "0.1mm");
	}
}

static void event_handler_speed(void){

	if(mks_grbl.move_speed == LOW_SPEED) {
		mks_grbl.move_speed = MID_SPEED;
		mks_lv_label_updata(move_page.label_speed, mc_language.speed_mid);
	}else if(mks_grbl.move_speed == MID_SPEED) {
		mks_grbl.move_speed = HIGHT_SPEED;
		mks_lv_label_updata(move_page.label_speed, mc_language.speed_high);
	}else if(mks_grbl.move_speed == HIGHT_SPEED) {
		mks_grbl.move_speed = LOW_SPEED;
		mks_lv_label_updata(move_page.label_speed, mc_language.speed_low);
	}
}


static void event_handler(lv_obj_t* obj, lv_event_t event) {

	uint8_t id = get_id(obj);

	if(event == LV_EVENT_PRESSED) { 

    }
	
	if((event == LV_EVENT_RELEASED) || (event == LV_EVENT_PRESS_LOST)) {

		switch(id) {
			case ID_INF_UP:	move_ctrl('Y', 1); break;
			case ID_INF_DOWN: move_ctrl('Y', 0); break;
			case ID_INF_LEFT: move_ctrl('X', 0); break;
			case ID_INF_RIGHT: move_ctrl('X', 1); break;
			case ID_INF_Z_UP: move_ctrl('Z', 1); break;
			case ID_INF_Z_DOWN: move_ctrl('Z', 0); break;
			// case ID_INF_XY_POS: set_xy_pos(); break;
			// case ID_INF_Z_POS: set_z_pos(); break;
			case ID_INF_BACK: grbl_send(CLIENT_SERIAL, "into back\n"); event_handler_back(); break;
			case ID_INF_STEP: event_handler_len_set(); break;
			case ID_INF_SPEED: event_handler_speed(); break;
			case ID_INF_CARVE: event_handler_sure(); break; 
			case ID_INF_L_NEXT	:	disp_imgbtn_1_del(); disp_imgbtn_2(); break;
			case ID_INF_L_UP	: 	disp_imgbtn_2_del();  disp_imgbtn_1(); break;
			// case ID_INF_XY_POS:	set_xy_pos();		break;
			// case ID_INF_Z_POS :	set_z_pos(); 		break;
			case ID_INF_KNIFE: set_knife(); break;
		}
	}
}

// =========================================================================================
// Pantalla tras elegir el archivo: posicionar el cabezal y empezar el trabajo.
// 480x320: barra y=5 (Atras + nombre y tamano del archivo), franja de coordenadas y=38,
// cruceta XY (3x3 de 68x52) + columna Z + columna de ajustes a y=68, y abajo el boton
// grande de iniciar a y=244.
// =========================================================================================
extern lv_obj_t* pos_strip;   // MKS_draw_move.cpp: franja X / Y / Z que reajusta move_pos_update()

#define INF_COL_CARD   LV_COLOR_MAKE(0x1F, 0x23, 0x33)
#define INF_COL_SIDE   LV_COLOR_MAKE(0x18, 0x1B, 0x28)
#define INF_COL_TRACK  LV_COLOR_MAKE(0x3F, 0x46, 0x66)
#define INF_COL_EMER   LV_COLOR_MAKE(0x2D, 0xE0, 0xA7)
#define INF_COL_MUTED  LV_COLOR_MAKE(0x9A, 0xA3, 0xC0)

enum { IB_UP, IB_DOWN, IB_LEFT, IB_RIGHT, IB_XY0, IB_ZUP, IB_ZDOWN, IB_Z0, IB_STEP, IB_SPEED, IB_PROBE,
       IB_AIR, IB_XYZ0, IB_START, IB_BACK, IB_COUNT };

static lv_style_t inf_btn_style, inf_btn_pr_style, inf_start_style, inf_strip_style;
static lv_style_t inf_text_style, inf_dark_style, inf_muted_style, inf_sym_style, inf_symhome_style;
static lv_obj_t*  inf_btn[IB_COUNT];
static char       inf_name_txt[64], inf_size_txt[16];

static const char* inf_T(const char* es, const char* en, const char* ch) {
	return mks_grbl.language == SimpleChinese ? ch : (mks_grbl.language == Espanol ? es : en);
}

static void inf_name_fit(char* out, size_t n, const char* name, int max_cp) {   // UTF-8 sin partir caracteres
	size_t o = 0;
	int cp = 0;
	for(size_t i = 0; name[i] != '\0' && o + 1 < n; i++) {
		if(((uint8_t)name[i] & 0xC0) != 0x80) {
			if(cp == max_cp) {
				if(o + 4 < n) { out[o++] = '.'; out[o++] = '.'; out[o++] = '.'; }
				break;
			}
			cp++;
		}
		out[o++] = name[i];
	}
	out[o] = '\0';
}

static void inf_styles_init(void) {
	lv_style_copy(&inf_btn_style, &lv_style_plain_color);
	inf_btn_style.body.main_color   = INF_COL_CARD;
	inf_btn_style.body.grad_color   = INF_COL_CARD;
	inf_btn_style.body.radius       = 10;
	inf_btn_style.body.border.width = 0;
	lv_style_copy(&inf_btn_pr_style, &inf_btn_style);
	inf_btn_pr_style.body.main_color = INF_COL_EMER;
	inf_btn_pr_style.body.grad_color = INF_COL_EMER;
	lv_style_copy(&inf_start_style, &inf_btn_style);          // boton grande de iniciar
	inf_start_style.body.main_color = INF_COL_EMER;
	inf_start_style.body.grad_color = INF_COL_EMER;
	lv_style_copy(&inf_strip_style, &inf_btn_style);          // franja de coordenadas
	inf_strip_style.body.main_color = INF_COL_SIDE;
	inf_strip_style.body.grad_color = INF_COL_SIDE;

	lv_style_copy(&inf_text_style, &lv_style_plain);
	inf_text_style.text.font  = mc_font();
	inf_text_style.text.color = LV_COLOR_WHITE;
	lv_style_copy(&inf_dark_style, &inf_text_style);
	inf_dark_style.text.color = LV_COLOR_MAKE(0x08, 0x0C, 0x18);
	lv_style_copy(&inf_muted_style, &inf_text_style);
	inf_muted_style.text.color = INF_COL_MUTED;
	lv_style_copy(&inf_sym_style, &inf_text_style);            // flechas: simbolos de Roboto
	inf_sym_style.text.font = &lv_font_roboto_22;
	lv_style_copy(&inf_symhome_style, &inf_sym_style);
	inf_symhome_style.text.color = INF_COL_EMER;
}

static lv_obj_t* inf_mkbtn(int id, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, const char* text,
						   const lv_style_t* label_style, const lv_style_t* rel_style) {
	lv_obj_t* b = lv_btn_create(mks_global.mks_src, NULL);
	lv_obj_set_size(b, w, h);
	lv_obj_set_pos(b, x, y);
	lv_btn_set_style(b, LV_BTN_STYLE_REL, (lv_style_t*)rel_style);
	lv_btn_set_style(b, LV_BTN_STYLE_PR, &inf_btn_pr_style);
	lv_obj_set_event_cb(b, inf_event);
	lv_obj_t* l = lv_label_create(b, NULL);
	lv_label_set_style(l, LV_LABEL_STYLE_MAIN, (lv_style_t*)label_style);
	lv_label_set_text(l, text);
	inf_btn[id] = b;
	return b;
}

static void inf_event(lv_obj_t* obj, lv_event_t event) {
	if(event != LV_EVENT_RELEASED) return;
	for(int i = 0; i < IB_COUNT; i++) {
		if(obj != inf_btn[i]) continue;
		switch(i) {
			case IB_UP:    move_ctrl('Y', 1); break;
			case IB_DOWN:  move_ctrl('Y', 0); break;
			case IB_LEFT:  move_ctrl('X', 0); break;
			case IB_RIGHT: move_ctrl('X', 1); break;
			case IB_ZUP:   move_ctrl('Z', 1); break;
			case IB_ZDOWN: move_ctrl('Z', 0); break;
			case IB_XY0:   set_xy_pos(obj, event); break;
			case IB_Z0:    set_z_pos(obj, event); break;
			case IB_XYZ0:  set_xyz_pos(obj, event); break;
			case IB_STEP:  event_handler_len_set(); break;
			case IB_SPEED: event_handler_speed(); break;
			case IB_PROBE: set_knife(); break;
			case IB_AIR:   set_cooling(obj, event); break;
			case IB_START: event_handler_sure(); break;
			case IB_BACK:
				// atras: a la lista de archivos (antes iba al menu principal)
				mks_ui_page.mks_ui_page = MKS_UI_PAGE_LOADING;
				lv_obj_clean(mks_global.mks_src);
				mks_draw_craving();
				break;
		}
		return;
	}
}

void mks_draw_inFile(char *fn) {

	inf_styles_init();
	for(int i = 0; i < IB_COUNT; i++) inf_btn[i] = NULL;

	//	record the file name / size (the carving popup and the frame screen use them)
	memset(frame_ctrl.file_name, 0, sizeof(frame_ctrl.file_name));
	memcpy(frame_ctrl.file_name, fn, 128);
	frame_ctrl.file_size = mks_file_list.file_size[mks_file_list.file_choose];

	// barra superior: Atras, nombre del archivo y su tamano
	lv_obj_t* back = inf_mkbtn(IB_BACK, 8, 5, 84, 28, mc_language.back, &inf_text_style, &inf_btn_style);
	move_page.Back = back;   // los avisos (popups) la habilitan o deshabilitan con set_click_status()

	const char* shown = (fn[0] == '/' || fn[0] == ' ') ? fn + 1 : fn;
	inf_name_fit(inf_name_txt, sizeof(inf_name_txt), shown, 28);
	lv_obj_t* name = lv_label_create(mks_global.mks_src, NULL);
	lv_label_set_style(name, LV_LABEL_STYLE_MAIN, &inf_text_style);
	lv_label_set_text(name, inf_name_txt);
	lv_obj_set_pos(name, 102, 9);

	uint32_t bytes = frame_ctrl.file_size;
	if(bytes < 1024)         snprintf(inf_size_txt, sizeof(inf_size_txt), "%u B", (unsigned)bytes);
	else if(bytes < 1048576) snprintf(inf_size_txt, sizeof(inf_size_txt), "%.1f KB", bytes / 1024.0f);
	else                     snprintf(inf_size_txt, sizeof(inf_size_txt), "%.2f MB", bytes / 1048576.0f);
	lv_obj_t* size = lv_label_create(mks_global.mks_src, NULL);
	lv_label_set_style(size, LV_LABEL_STYLE_MAIN, &inf_muted_style);
	lv_label_set_text(size, inf_size_txt);
	lv_obj_align(size, NULL, LV_ALIGN_IN_TOP_RIGHT, -14, 9);

	// franja de coordenadas (X, Y y Z a todo el ancho; move_pos_update() las actualiza)
	pos_strip = lv_obj_create(mks_global.mks_src, NULL);
	lv_obj_set_size(pos_strip, 460, 24);
	lv_obj_set_pos(pos_strip, 10, 38);
	lv_obj_set_style(pos_strip, &inf_strip_style);
	move_page.label_xpos = label_for_text(pos_strip, move_page.label_xpos, pos_strip, -153, 0, LV_ALIGN_CENTER, "X:0");
	move_page.label_ypos = label_for_text(pos_strip, move_page.label_ypos, pos_strip, 0, 0, LV_ALIGN_CENTER, "Y:0");
	move_page.label_zpos = label_for_text(pos_strip, move_page.label_zpos, pos_strip, 153, 0, LV_ALIGN_CENTER, "Z:0");

	// cruceta XY: la tecla del centro pone el cero de X e Y en la posicion actual
	inf_mkbtn(IB_UP,    82,  68, 68, 52, LV_SYMBOL_UP,    &inf_sym_style, &inf_btn_style);
	inf_mkbtn(IB_LEFT,   8, 126, 68, 52, LV_SYMBOL_LEFT,  &inf_sym_style, &inf_btn_style);
	inf_mkbtn(IB_XY0,   82, 126, 68, 52, "XY=0",          &inf_text_style, &inf_btn_style);
	inf_mkbtn(IB_RIGHT, 156, 126, 68, 52, LV_SYMBOL_RIGHT, &inf_sym_style, &inf_btn_style);
	inf_mkbtn(IB_DOWN,  82, 184, 68, 52, LV_SYMBOL_DOWN,  &inf_sym_style, &inf_btn_style);

	// columna Z: subir, cero de Z y bajar
	inf_mkbtn(IB_ZUP,   234,  68, 68, 52, LV_SYMBOL_UP,   &inf_sym_style, &inf_btn_style);
	inf_mkbtn(IB_Z0,    234, 126, 68, 52, "Z=0",          &inf_text_style, &inf_btn_style);
	inf_mkbtn(IB_ZDOWN, 234, 184, 68, 52, LV_SYMBOL_DOWN, &inf_sym_style, &inf_btn_style);

	// ajustes: paso, velocidad y sonda de Z
	lv_obj_t* step = inf_mkbtn(IB_STEP,  312,  68, 160, 52, "", &inf_text_style, &inf_btn_style);
	lv_obj_t* spd  = inf_mkbtn(IB_SPEED, 312, 126, 160, 52, "", &inf_text_style, &inf_btn_style);
	inf_mkbtn(IB_PROBE, 312, 184, 160, 52, inf_T("Sonda Z", "Z probe", "Z\xe6\xa3\x80\xe6\xb5\x8b"), &inf_text_style, &inf_btn_style);   // Z检测

	// los manejadores del paso y la velocidad actualizan estas etiquetas
	move_page.label_len = lv_obj_get_child(step, NULL);
	move_page.label_speed = lv_obj_get_child(spd, NULL);
	if(mks_grbl.move_dis == M_0_1_MM)      lv_label_set_text(move_page.label_len, "0.1mm");
	else if(mks_grbl.move_dis == M_1_MM)   lv_label_set_text(move_page.label_len, "1mm");
	else                                   lv_label_set_text(move_page.label_len, "10mm");
	if(mks_grbl.move_speed == LOW_SPEED)        lv_label_set_text(move_page.label_speed, mc_language.speed_low);
	else if(mks_grbl.move_speed == MID_SPEED)   lv_label_set_text(move_page.label_speed, mc_language.speed_mid);
	else                                        lv_label_set_text(move_page.label_speed, mc_language.speed_high);

	// fila inferior: aire, cero de los tres ejes y el boton grande de iniciar
	inf_mkbtn(IB_AIR,   8, 244, 100, 68, inf_T("Aire", "Air", "M8"), &inf_text_style, &inf_btn_style);
	inf_mkbtn(IB_XYZ0, 114, 244, 100, 68, "XYZ=0", &inf_text_style, &inf_btn_style);
	lv_obj_t* go = inf_mkbtn(IB_START, 220, 244, 252, 68, mc_language.start, &inf_dark_style, &inf_start_style);
	lv_btn_set_style(go, LV_BTN_STYLE_PR, &inf_btn_pr_style);

	mks_ui_page.mks_ui_page = MKS_UI_inFile;
}

static void disp_imgbtn(void) {

	move_page.Back = lv_imgbtn_creat_mks(mks_global.mks_src_1, move_page.Back, &png_back_pre, &back, LV_ALIGN_IN_TOP_LEFT, 10, 5 , event_handler);

	disp_imgbtn_1();

	move_page.y_n = lv_imgbtn_creat_mks(mks_global.mks_src_2, move_page.y_n, &png_up_pre, &png_up, LV_ALIGN_IN_TOP_LEFT, 88, 10, event_handler);
    move_page.y_p = lv_imgbtn_creat_mks(mks_global.mks_src_2, move_page.y_p, &png_down_pre, &png_down, LV_ALIGN_IN_TOP_LEFT, 88, 138, event_handler);
    move_page.x_n = lv_imgbtn_creat_mks(mks_global.mks_src_2, move_page.x_n, &png_left_pre, &png_left, LV_ALIGN_IN_TOP_LEFT, 10, 74, event_handler);
    move_page.x_p = lv_imgbtn_creat_mks(mks_global.mks_src_2, move_page.x_p, &png_right_pre, &png_right, LV_ALIGN_IN_TOP_LEFT, 166, 74, event_handler);
	move_page.z_n = lv_imgbtn_creat_mks(mks_global.mks_src_2, move_page.z_n, &png_z_up_pre, &png_z_up, LV_ALIGN_IN_TOP_LEFT, 244, 10, event_handler);
	move_page.z_p = lv_imgbtn_creat_mks(mks_global.mks_src_2, move_page.z_p, &png_z_down_pre, &png_z_down, LV_ALIGN_IN_TOP_LEFT, 244, 138, event_handler);

	move_page.xy_home = lv_imgbtn_creat_mks(mks_global.mks_src_2, move_page.xy_home, &png_xyhome_pre, &png_xyhome, LV_ALIGN_IN_TOP_LEFT, 88, 74, event_handler);
	move_page.z_home = lv_imgbtn_creat_mks(mks_global.mks_src_2, move_page.z_home, &png_z_home_pre, &png_z_home, LV_ALIGN_IN_TOP_LEFT, 244, 74, event_handler);

	// infile_page.btn_sure_print = lv_imgbtn_creat_mks(mks_global.mks_src_1, infile_page.btn_sure_print, &png_infile_cave_pre, &png_infile_cave, LV_ALIGN_IN_LEFT_MID, 370,-15, event_handler);
	infile_page.btn_cancle = lv_imgbtn_creat_mks(mks_global.mks_src_1, infile_page.btn_cancle, &back, &back, LV_ALIGN_IN_LEFT_MID,10, -15 , event_handler);
}

static void disp_up_set(lv_obj_t* obj, lv_event_t event) {

	if(event != LV_EVENT_RELEASED) return;
	disp_imgbtn_2_del();  
	disp_imgbtn_1();
}

static void disp_down_set(lv_obj_t* obj, lv_event_t event) {

	if(event != LV_EVENT_RELEASED) return;
	disp_imgbtn_1_del(); 
	disp_imgbtn_2();
}

static void disp_imgbtn_1(void) {

	move_page.xy_clear = lv_imgbtn_creat_mks(mks_global.mks_src_1, move_page.xy_clear, &png_xyclear_pre, &png_xyclear, LV_ALIGN_IN_TOP_LEFT, 170, 5, set_xy_pos);
	move_page.z_clear = lv_imgbtn_creat_mks(mks_global.mks_src_1, move_page.z_clear, &png_zclear_pre, &png_zclear, LV_ALIGN_IN_TOP_LEFT, 240, 5, set_z_pos);
	move_page.knife = lv_imgbtn_creat_mks(mks_global.mks_src_1, move_page.knife, &png_knife_pre, &png_knife, LV_ALIGN_IN_TOP_LEFT, 310, 5, event_handler);
	move_page.next = lv_imgbtn_creat_mks(mks_global.mks_src_1, move_page.next, &png_l_next_pre, &png_l_next, LV_ALIGN_IN_TOP_LEFT, 380, 5, disp_down_set);

	move_page.label_xy_clear = label_for_imgbtn_name(mks_global.mks_src_1, move_page.label_xy_clear, move_page.xy_clear, 0, 0, mc_language.xy_clear);
	move_page.label_z_clear = label_for_imgbtn_name(mks_global.mks_src_1, move_page.label_z_clear, move_page.z_clear, 0, 0, mc_language.z_clear);
	move_page.label_knife = label_for_imgbtn_name(mks_global.mks_src_1, move_page.label_knife, move_page.knife, 0, 0, mc_language.knife);
	move_page.label_next = label_for_imgbtn_name(mks_global.mks_src_1, move_page.label_next, move_page.next, 0, 0, mc_language.next);
}

static void disp_imgbtn_1_del(void) {
	lv_obj_del(move_page.xy_clear);
	lv_obj_del(move_page.z_clear);
	lv_obj_del(move_page.knife);
	lv_obj_del(move_page.next);

	lv_obj_del(move_page.label_xy_clear);
	lv_obj_del(move_page.label_z_clear);
	lv_obj_del(move_page.label_knife);
	lv_obj_del(move_page.label_next);
}

static void disp_imgbtn_2(void) {
	move_page.up = lv_imgbtn_creat_mks(mks_global.mks_src_1, move_page.up, &png_l_up_pre, &png_l_up, LV_ALIGN_IN_TOP_LEFT, 170, 5, disp_up_set);
	move_page.cooling = lv_imgbtn_creat_mks(mks_global.mks_src_1, move_page.cooling, &png_cooling_pre, &png_cooling, LV_ALIGN_IN_TOP_LEFT, 240, 5, set_cooling);
	move_page.position = lv_imgbtn_creat_mks(mks_global.mks_src_1, move_page.position, &png_position_pre, &png_position, LV_ALIGN_IN_TOP_LEFT, 310, 5, set_xyz_pos);

	move_page.label_cooling = label_for_imgbtn_name(mks_global.mks_src_1, move_page.label_cooling, move_page.cooling, 0, 0, mc_language.cooling);
	move_page.label_position = label_for_imgbtn_name(mks_global.mks_src_1, move_page.label_position, move_page.position, 0, 0, mc_language.position);
	move_page.label_up = label_for_imgbtn_name(mks_global.mks_src_1, move_page.label_up, move_page.up, 0, 0, mc_language.up);
}

static void disp_imgbtn_2_del(void) {
	lv_obj_del(move_page.cooling);
	lv_obj_del(move_page.position);
	lv_obj_del(move_page.up);

	lv_obj_del(move_page.label_cooling);
	lv_obj_del(move_page.label_position);
	lv_obj_del(move_page.label_up);
}

static void disp_btn(void) {

	/* 按键样式 */
	lv_style_copy(&infile_page.btn_color, &lv_style_scr);
    infile_page.btn_color.body.main_color = LV_COLOR_MAKE(0x17, 0x1A, 0x26);
    infile_page.btn_color.body.grad_color = LV_COLOR_MAKE(0x17, 0x1A, 0x26);
    infile_page.btn_color.body.opa = LV_OPA_COVER;//设置背景色完全不透明
    infile_page.btn_color.text.color = LV_COLOR_WHITE;
	infile_page.btn_color.body.radius = 10;

	lv_style_copy(&infile_page.btn_press_color, &lv_style_scr);
    infile_page.btn_press_color.body.main_color = LV_COLOR_MAKE(0x3F, 0x47, 0x66);
    infile_page.btn_press_color.body.grad_color = LV_COLOR_MAKE(0x3F, 0x47, 0x66);
    infile_page.btn_press_color.body.opa = LV_OPA_COVER;//设置背景色完全不透明
    infile_page.btn_press_color.text.color = LV_COLOR_WHITE;
	infile_page.btn_press_color.body.radius = 10;

	move_page.btn_len = mks_lv_btn_set_for_aglin_screen(mks_global.mks_src_3, move_page.btn_len, 110, 52, LV_ALIGN_IN_TOP_LEFT, 10, 10, event_handler);
	move_page.btn_speed = mks_lv_btn_set_for_aglin_screen(mks_global.mks_src_3, move_page.btn_speed, 110, 52, LV_ALIGN_IN_TOP_LEFT, 10, 74, event_handler);
	infile_page.btn_sculpture = mks_lv_btn_set_for_aglin_screen(mks_global.mks_src_3, infile_page.btn_sculpture, 110, 52, LV_ALIGN_IN_TOP_LEFT, 10, 138, event_handler);		

	lv_btn_set_style(move_page.btn_len, LV_BTN_STYLE_REL, &infile_page.btn_color);
	lv_btn_set_style(move_page.btn_len, LV_BTN_STYLE_PR, &infile_page.btn_press_color);

	lv_btn_set_style(move_page.btn_speed, LV_BTN_STYLE_REL, &infile_page.btn_color);
	lv_btn_set_style(move_page.btn_speed,LV_BTN_STYLE_PR,&infile_page.btn_press_color);

	lv_btn_set_style(infile_page.btn_sculpture, LV_BTN_STYLE_REL, &infile_page.btn_color);
	lv_btn_set_style(infile_page.btn_sculpture,LV_BTN_STYLE_PR,&infile_page.btn_press_color);
}

static void disp_label(void) {

	label_for_imgbtn_name(mks_global.mks_src_1, move_page.Label_back, move_page.Back, 0, 0, mc_language.back);
	
	move_page.label_xpos = label_for_text(mks_global.mks_src_1, move_page.label_xpos, NULL, 93, 5, LV_ALIGN_IN_TOP_LEFT,  	"X:0");
	move_page.label_ypos = label_for_text(mks_global.mks_src_1, move_page.label_ypos, NULL, 93, 36, LV_ALIGN_IN_TOP_LEFT,	"Y:0");
	move_page.label_zpos = label_for_text(mks_global.mks_src_1, move_page.label_zpos, NULL, 93, 66, LV_ALIGN_IN_TOP_LEFT,  	"Z:0");

	if(mks_grbl.move_dis == M_0_1_MM) {
		move_page.label_len = mks_lvgl_long_sroll_label_with_wight_set_center(move_page.btn_len, move_page.label_len, 0, 0, "0.1mm", 50);
	}else if(mks_grbl.move_dis == M_1_MM) {
		move_page.label_len = mks_lvgl_long_sroll_label_with_wight_set_center(move_page.btn_len, move_page.label_len, 0, 0, "1mm", 50);
	}else if(mks_grbl.move_dis == M_10_MM) {
		move_page.label_len = mks_lvgl_long_sroll_label_with_wight_set_center(move_page.btn_len, move_page.label_len, 0, 0, "10mm", 50);
	}
	
	if(mks_grbl.move_speed == LOW_SPEED) {
		move_page.label_speed = mks_lvgl_long_sroll_label_with_wight_set_center(move_page.btn_speed, move_page.label_speed, 0, 0, mc_language.speed_low, 100); //l:500, m:1000, h:2000
	}else if(mks_grbl.move_speed == MID_SPEED) {
		move_page.label_speed = mks_lvgl_long_sroll_label_with_wight_set_center(move_page.btn_speed, move_page.label_speed, 0, 0, mc_language.speed_mid, 100);
	}else if(mks_grbl.move_speed == HIGHT_SPEED) {
		move_page.label_speed = mks_lvgl_long_sroll_label_with_wight_set_center(move_page.btn_speed, move_page.label_speed, 0, 0, mc_language.speed_high, 100);
	}	

	infile_page.label_sculpture = mks_lvgl_long_sroll_label_with_wight_set_center(infile_page.btn_sculpture, infile_page.label_sculpture, 0, 0, mc_language.sculpture, 100);
}

void infile_clean_obj(lv_obj_t *obj_src) {
	lv_obj_clean(obj_src);
}


