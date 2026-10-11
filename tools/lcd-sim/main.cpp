// Simulador del LCD: ejecuta las pantallas reales del firmware sobre LVGL y vuelca PNG (via PPM).
// uso: ./sim <directorio_salida> <idioma: es|en|zh|pt> [escena ...]   (sin escenas = todas)
#include "mks/MKS_draw_lvgl.h"
#include "mks/MKS_LVGL.h"
#include "mks/MKS_draw_frame.h"
#include "mks/MKS_draw_boot.h"
#include "mks/MKS_draw_qr.h"
#include "mks/MKS_draw_fileview.h"
#include "mks/MKS_draw_carving.h"
#include <string>
#include <vector>

extern uint32_t sim_millis;
extern bool sim_wifi_on;
extern SDState sim_sd_state;
extern float sim_percent;

static uint16_t fb[480 * 320];
static lv_disp_buf_t disp_buf;
static lv_color_t buf1[480 * 40];
static int touch_x = 0, touch_y = 0;
static bool touch_down = false;

static void flush_cb(lv_disp_drv_t* drv, const lv_area_t* a, lv_color_t* c) {
    for (int y = a->y1; y <= a->y2; y++)
        for (int x = a->x1; x <= a->x2; x++) fb[y * 480 + x] = c++->full;
    lv_disp_flush_ready(drv);
}
static bool touch_cb(lv_indev_drv_t*, lv_indev_data_t* d) {
    d->point.x = touch_x; d->point.y = touch_y;
    d->state = touch_down ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    return false;
}

static void run(int ms) {
    for (int i = 0; i < ms / 5; i++) { sim_millis += 5; lv_task_handler(); }
}
static void tap(int x, int y) {          // toque real: pulsar, soltar
    touch_x = x; touch_y = y; touch_down = true; run(60);
    touch_down = false; run(120);
}

static void save(const std::string& dir, const std::string& name) {
    std::string path = dir + "/" + name + ".ppm";
    FILE* f = fopen(path.c_str(), "wb");
    fprintf(f, "P6\n480 320\n255\n");
    for (int i = 0; i < 480 * 320; i++) {
        uint16_t v = fb[i];
        uint8_t px[3] = { (uint8_t)(((v >> 11) & 31) * 255 / 31), (uint8_t)(((v >> 5) & 63) * 255 / 63), (uint8_t)((v & 31) * 255 / 31) };
        fwrite(px, 1, 3, f);
    }
    fclose(f);
    printf("escena %s\n", name.c_str());
}

static void to_ready() {   // vuelve a la pantalla principal limpiando la actual
    mks_lv_clean_ui();
    mks_draw_ready();
    ready_data_updata();
    run(200);
}

int main(int argc, char** argv) {
    std::string dir = argc > 1 ? argv[1] : "out";
    std::string lang = argc > 2 ? argv[2] : "es";
    std::vector<std::string> want;
    for (int i = 3; i < argc; i++) want.push_back(argv[i]);
    auto on = [&](const char* n) { if (want.empty()) return true; for (auto& w : want) if (w == n) return true; return false; };

    extern IntSetting* language_select;
    language_select->v = lang == "zh" ? 0 : (lang == "en" ? 1 : (lang == "pt" ? 2 : 3));

    lv_init();
    lv_disp_buf_init(&disp_buf, buf1, NULL, 480 * 40);
    lv_disp_drv_t dd; lv_disp_drv_init(&dd);
    dd.hor_res = 480; dd.ver_res = 320; dd.flush_cb = flush_cb; dd.buffer = &disp_buf;
    lv_disp_drv_register(&dd);
    lv_indev_drv_t id; lv_indev_drv_init(&id);
    id.type = LV_INDEV_TYPE_POINTER; id.read_cb = touch_cb;
    lv_indev_drv_register(&id);

    // arranque igual que lvgl_disp_task
    if (language_select->get() == 0) mks_grbl.language = SimpleChinese;
    else if (language_select->get() == 1) mks_grbl.language = English;
    else if (language_select->get() == 2) mks_grbl.language = Portugues;
    else mks_grbl.language = Espanol;
    mc_language_init();
    mks_grbl.move_dis = M_10_MM; mks_grbl.move_speed = HIGHT_SPEED;
    mks_grbl.wifi_connect_enable = true;
    mks_global_style_init();
    mks_boot_begin();
    run(300);
    if (on("00_logo")) save(dir, "00_logo");
    // mantener pulsado el logo: aparece el QR; el contador de 4 s se detiene hasta tocarlo
    touch_x = 240; touch_y = 160; touch_down = true; run(250); printf("[boot] a los 250 ms pulsado, QR abierto: %d\n", (int)mks_qr_popup_is_open()); run(450); printf("[boot] a los 700 ms pulsado, QR abierto: %d (debe ser 0: se abre al soltar)\n", (int)mks_qr_popup_is_open()); touch_down = false; run(150); printf("[boot] al soltar, QR abierto: %d\n", (int)mks_qr_popup_is_open());
    if (on("00b_arranque_qr")) save(dir, "00b_arranque_qr");
    bool cont = false;
    for (int i = 0; i < 1000 && !cont; i++) cont = mks_boot_tick();       // con el QR a la vista no avanza
    printf("[boot] con el QR abierto, ¿continua solo? %s\n", cont ? "SI (mal)" : "no (bien)");
    tap(240, 100);                                                           // tocar el QR: continua
    cont = false;
    for (int i = 0; i < 5 && !cont; i++) cont = mks_boot_tick();
    printf("[boot] tras tocar el QR, ¿continua? %s\n", cont ? "SI (bien)" : "no (mal)");

    // sin hora sincronizada (sin red/NTP) el reloj no se muestra: se ve la barra solo con el Wifi
    mks_lv_clean_ui(); mks_draw_ready(); run(200);
    if (on("01b_principal_sin_hora")) save(dir, "01b_principal_sin_hora");
    to_ready();
    if (on("01_principal")) save(dir, "01_principal");

    // --- Control (mosaico de la izquierda) y popup del laser
    tap(80, 250);                                   // mosaico Control
    move_pos_update(); run(200);
    if (on("02_control")) save(dir, "02_control");
    tap(413, 40);                                   // "Mas": segunda pagina de la cabecera
    if (on("02b_control_mas")) save(dir, "02b_control_mas");
    tap(98, 40);                                    // "Menos": vuelve a la primera
    tap(400, 278);                                  // boton Laser: abre el popup de potencia
    if (on("03_laser_popup")) save(dir, "03_laser_popup");
    tap(286, 148);                                  // 30 %: el laser se enciende y el popup SE QUEDA abierto
    if (on("03b_laser_30")) save(dir, "03b_laser_30");
    tap(240, 214);                                  // Atras con el laser encendido: lo apaga y cierra
    if (on("03c_laser_atras")) save(dir, "03c_laser_atras");

    // --- Lista de la SD y pantalla del archivo
    to_ready();
    tap(240, 250);                                  // mosaico Tallado
    run(300);
    if (on("04_sd_pagina1")) save(dir, "04_sd_pagina1");
    if (on("05_sd_pagina2")) { tap(358, 294); run(300); save(dir, "05_sd_pagina2"); }   // Siguiente
    if (on("06_sd_pagina3")) { tap(358, 294); run(300); save(dir, "06_sd_pagina3"); }
    extern bool sim_pv_hold;
    sim_pv_hold = true;                             // primera fila: abre la vista previa; primero "generando"
    tap(100, 82); run(300); mks_fileview_poll(); run(200);
    if (on("07a_vista_generando")) save(dir, "07a_vista_generando");
    sim_pv_hold = false;
    tap(100, 82 + 0); run(10);
    mks_lv_clean_ui(); mks_draw_fileview(mks_file_list.filename_str[mks_file_list.file_choose]); run(300); mks_fileview_poll(); run(300);
    if (on("07b_vista_previa")) save(dir, "07b_vista_previa");
    tap(330, 270); run(300);                        // Posicionar: la pantalla de mover y poner el cero
    move_pos_update(); run(200);
    if (on("07_archivo")) save(dir, "07_archivo");

    // --- Herramientas
    to_ready();
    tap(400, 250);                                  // mosaico Herramientas
    run(300);
    if (on("08_herramientas")) save(dir, "08_herramientas");
    // barra lateral de Herramientas (en orden): Atras 56, Wifi 95, Beeper 134, Idioma 173, Herramienta 212, Placa 251, Acerca de 290
    struct { const char* n; int y; } secs[] = { { "09_wifi", 95 }, { "11_beeper", 134 }, { "10_idioma", 173 }, { "08b_herramienta", 212 }, { "12_placa", 251 }, { "13_acerca", 290 } };
    for (auto& sc : secs) {
        tap(55, sc.y); run(300);
        if (on(sc.n)) save(dir, sc.n);
    }
    // Acerca de: pulsar el enlace abre el QR
    tap(240, 225); run(200);
    if (on("13b_acerca_qr")) save(dir, "13b_acerca_qr");
    tap(240, 160); run(200);

    // --- Trabajo en curso
    to_ready();
    sim_sd_state = SDState::BusyPrinting;
    sys.state = State::Cycle;
    mks_grbl.is_mks_ts35_flag = true;
    mks_lv_clean_ui();
    mks_job_goto_print();
    mks_print_data_updata(); run(300);
    if (on("14_trabajo")) save(dir, "14_trabajo");
    return 0;
}
