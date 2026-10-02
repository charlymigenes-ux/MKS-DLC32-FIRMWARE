#ifndef __mks_draw_language_h
#define __mks_draw_language_h

#include "MKS_draw_lvgl.h"
#include "MKS_LVGL.h"


typedef struct {

    lv_obj_t *imgbtn_back;
    lv_obj_t *imgbtn_simple_cn;         // 简体中文
    lv_obj_t *imgbtn_en;                // 英语
    lv_obj_t *imgbtn_de;                // 德语
    lv_obj_t *imgbtn_es;                // español ($40=3)

    lv_obj_t *label_back;
    lv_obj_t *label_simple_cn;         // 简体中文
    lv_obj_t *label_en;                // 英语
    lv_obj_t *label_de;                // 德语
    lv_obj_t *label_es;                // español


}LANGUAGE_PAGE_T;


typedef struct {

    /* 公共 */
    const char *back;
    const char *yes;
    const char *no;

    /* 主页 */
    const char *control;
    const char *sculpture;
    const char *tool;
    const char *Mpos;
    const char *Wpos;

    const char *wifi_connect;
    const char *wifi_disconnect;

    /* 控制界面 */
    const char *xy_clear;
    const char *z_clear;
    const char *knife;
    const char *next;
    const char *up;
    const char *cooling;
    const char *position;
    const char *speed_high;
    const char *speed_mid;
    const char *speed_low;
    const char *spindle;
    const char *carve;

    /* 文件界面 */
    const char *dis_no_sd_card;

    /* 雕刻界面 */
    const char *hold;
    const char *cycle;
    const char *stop;
    const char *adjust;
    const char *spindle_speed;
    const char *feed_rate;
    const char *rapid_speed;
    const char *carve_times;


    
    
    
    /* 提示语 */
    const char *dis_stop_print;
    const char *dis_homing;
    const char *dis_no_hard_homing;
    const char *dis_homing_succeed;
    const char *dis_homing_fail;
    const char *dis_probe_set;
    const char *dis_probe_succeed;
    const char *dis_probe_fail;

    /* Botones y popups que antes estaban fijos en ingles en cada pagina
     * (Yes/Cancel/Back/Pause/Stop/Frame/...): ahora se cuelgan de aqui para
     * que sigan el idioma elegido en $40. */
    const char *cancel;
    const char *frame;
    const char *carve_file_sure;
    const char *pause;
    const char *start;
    const char *confirm;
    const char *add;
    const char *reduce;
    const char *exit;
    const char *language;
    const char *scanf;
    const char *reconnect;
    const char *connect;
    const char *password;
    const char *z_home;
    const char *update_title;

    /* Avisos y mensajes de dialogo */
    const char *dis_info;
    const char *dis_warning;
    const char *dis_error;
    const char *dis_pos_succeed;
    const char *dis_wait_mc_stop;
    const char *dis_unlock;
    const char *dis_unlock_success;
    const char *dis_setting_error;
    const char *dis_set_6_1;
    const char *dis_wait_idle;
    const char *dis_file_too_big;
    const char *dis_continue_sure;
    const char *dis_file_loading;
    const char *dis_loading_file;
    const char *dis_sd_busy;
    const char *dis_running;
    const char *dis_print_stop_sure;
    const char *dis_print_done;
    const char *wifi_scanning;
    const char *wifi_connecting;
    const char *wifi_disconnecting;
    const char *wifi_pwd_prompt;
    const char *wifi_status_on;
    const char *wifi_status_off;
    const char *dis_update_succeed;
    const char *dis_update_restart;
    const char *dis_update_fail;
    const char *dis_update_fail_help;

    /* Formatos con %d (overrides de velocidad/potencia, etc.) */
    const char *power_fmt;
    const char *speed_fmt;
    const char *spindle_speed_fmt;
    const char *feed_rate_fmt;
    const char *rapid_fmt;
    const char *gauge_feed;       // rotulos bajo los indicadores de la pantalla de trabajo
    const char *gauge_spindle;
    const char *gauge_rapid;
    const char *job_elapsed;      // datos del trabajo
    const char *job_left;
    const char *job_feed_real;
    const char *job_power_real;
    const char *resume;           // boton Pausar/Reanudar de la pantalla de trabajo
    const char *tool_board;       // fila "Placa" de la pantalla de Herramientas

    /* Pagina de pruebas (test) */
    const char *test_title;
    const char *probe_check;
    const char *x_limit_check;
    const char *y_limit_check;
    const char *z_limit_check;
    const char *sd_check;
    const char *i2c_check;
    const char *cpu_temp;
    const char *test_warning;
}mc_lg_muilt_t;
extern mc_lg_muilt_t mc_language;

// Rellena mc_language con las cadenas del LCD. OJO: antes solo se llamaba desde
// set_language() (al cambiar el idioma desde la pantalla), nunca al arrancar.
void mc_language_init(void);
const lv_font_t* mc_font(void);   // fuente de texto segun el idioma: con chino, la que trae los glifos chinos


void draw_language(void);
void mks_clear_language(void);

#endif
