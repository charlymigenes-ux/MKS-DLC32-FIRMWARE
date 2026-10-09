// Implementacion falsa del resto del firmware para el simulador de pantallas.
#include "Grbl.h"
#include "mks/MKS_draw_lvgl.h"
#include "mks/MKS_LVGL.h"
#include "mks/MKS_SDCard.h"
#include "mks/MKS_ctrl.h"
#include <stdarg.h>
#include <string>
#include <vector>

// ---- estado que controla el simulador (main.cpp lo cambia por escena)
uint32_t sim_millis = 0;
bool     sim_wifi_on = true;
SDState  sim_sd_state = SDState::Idle;
float    sim_mpos[3] = { 125.4f, 87.0f, 0.0f };
float    sim_wco[3]  = { 100.0f, 80.0f, 0.0f };
float    sim_percent = 42.0f;
uint32_t sim_line    = 18342;
bool     sim_flood   = false;
extern "C" uint32_t millis(void) { return sim_millis; }

// ---- globales que en el firmware viven en MKS_LVGL.cpp, MKS_draw_lvgl.cpp y Settings
GRBL_CRTL      mks_grbl;
LVGL_UI_PAGE_t mks_ui_page;
lv_obj_t*      mks_src = NULL;
SdCard         tf;
void SdCard::writeFile(const char*, const char*) {}
void SdCard::writeFile_line(const char*, const char*, uint32_t) {}
system_t sys = { State::Idle, false, false, 0 };
volatile Percent sys_rt_f_override = 100, sys_rt_r_override = 100, sys_rt_s_override = 100;
bool stepper_idle = true;
bool SD_ready_next = false;
uint32_t sd_current_line_number = 0;
fs::FS SD;
EspStub ESP;
static FlagSetting f_probe(0), f_hard(1), f_home(1), f_laser(1), f_beep(1), f_alert(1);
static IntSetting  i_lang(3);
static FloatSetting r_rpm(1000);
FlagSetting *probe_invert = &f_probe, *hard_limits = &f_hard, *homing_enable = &f_home, *laser_mode = &f_laser,
            *beep_status = &f_beep, *alert_sound_status = &f_alert;
IntSetting*   language_select = &i_lang;
FloatSetting* rpm_max = &r_rpm;

// ---- posiciones, SD y comandos
float* system_get_mpos() { return sim_mpos; }
void mpos_to_wpos(float* p) { for (int i = 0; i < 3; i++) p[i] -= sim_wco[i]; }
float st_get_realtime_rate() { return 1200.0f; }
CoolantState coolant_get_state() { CoolantState c; c.Mist = 0; c.Flood = sim_flood; return c; }
SDState get_sd_state(bool) { return sim_sd_state; }
SDState mks_readSD_Status(void) { return sim_sd_state; }
bool openFile(fs::FS&, const char*) { return true; }
boolean closeFile() { return true; }
float sd_report_perc_complete() { return sim_percent; }
uint32_t sd_get_current_line_number() { return sim_line; }
void sd_get_current_filename(char* n) { strcpy(n, "virgenRosas75x130.gc"); }
bool sd_file_check(const char*) { return true; }
boolean readFileLine(char*, int) { return false; }
void serila_write_into_buffer(uint8_t* d) { printf("[cmd] %s", (const char*)d); }
void grbl_send(uint8_t, const char* t) { printf("[grbl] %s", t); }
void grbl_sendf(uint8_t, const char* f, ...) { va_list a; va_start(a, f); printf("[grbl] "); vprintf(f, a); va_end(a); }
float temperatureRead() { return 48.3f; }
void configTzTime(const char*, const char*, const char*) {}
String trim(const char* s) { return String(s); }
bool mks_get_wifi_status(void) { return sim_wifi_on; }
void BLTOUCH_push_down(void) {}
void ts35_beep_on(void) {}
void ts35_beep_off(void) {}

// Tarjeta SD falsa: mismos recorridos por paginas que SDCard.cpp (mks_listDir / mks_countDir)
struct FakeFile { const char* name; uint32_t size; };
static const FakeFile fake_files[] = {
    { "virgenRosas75x130.gc", 8683028 }, { "guadaclasica75x130.gc", 5432115 }, { "cena2-75x130.gc", 3774512 },
    { "trescruces75x130.gc", 2706432 },  { "Sagrado75x130INV.gc", 2884096 },  { "CristoCloseUp130x75.gc", 1331200 },
    { "teponi\xc3\xb1" "a202x270.gc", 25730000 }, { "efelante120x120.gc", 253400 }, { "buda120x120.gc", 201100 },
    { "CirsCruz_120x120.gc", 1500000 },  { "logo_taller.gc", 48200 },       { "prueba_linea.gc", 1024 },
};
uint16_t mks_countDir(fs::FS&, const char*, uint8_t) { return sizeof(fake_files) / sizeof(fake_files[0]); }
void mks_listDir(fs::FS&, const char*, uint8_t) {
    uint16_t n = sizeof(fake_files) / sizeof(fake_files[0]);
    for (uint16_t i = 0; i < n; i++) {
        if (mks_file_list.file_count >= (mks_file_list.file_page * MKS_FILE_NUM) - MKS_FILE_NUM && mks_file_list.file_count < mks_file_list.file_page * MKS_FILE_NUM) {
            strcpy(mks_file_list.filename_str[mks_file_list.file_begin_num], fake_files[i].name);
            mks_file_list.file_size[mks_file_list.file_begin_num] = fake_files[i].size;
            draw_filexx(mks_file_list.file_begin_num, mks_file_list.filename_str[mks_file_list.file_begin_num]);
            mks_file_list.file_begin_num++;
        }
        mks_file_list.file_count++;
        if (mks_file_list.file_count >= mks_file_list.file_page * MKS_FILE_NUM) return;
    }
}
