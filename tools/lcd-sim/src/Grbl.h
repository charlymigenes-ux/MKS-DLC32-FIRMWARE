// Stub de Grbl.h para el simulador: solo lo que usan las pantallas del LCD. Las pantallas reales
// (MKS_draw_*.cpp) se compilan sin cambios; aqui se finge el resto del firmware.
#pragma once
#include "Arduino.h"
#include "SPI.h"
#include "FS.h"
#include "SD.h"
#include "FreeRTOS.h"
#include <vector>

#define MAX_N_AXIS 3
#define BOARD_NAME "Board:MKS LTS V1.1"
#define FW_NAME "V1.0(M+X.8M.20261001)"

// ---- Config.h: limites de los overrides
namespace FeedOverride { const int Default = 100, Max = 200, Min = 10, CoarseIncrement = 10, FineIncrement = 1; }
namespace RapidOverride { const int Default = 100, Medium = 50, Low = 25, ExtraLow = 5; }
namespace SpindleSpeedOverride { const int Default = 100, Max = 200, Min = 10, CoarseIncrement = 10, FineIncrement = 1; }

// ---- System.h
enum class State : uint8_t { Idle = 0, Alarm, CheckMode, Homing, Cycle, Hold, Jog, SafetyDoor, Sleep };
typedef uint8_t Percent;
typedef struct { volatile State state; bool abort; bool soft_limit; uint32_t spindle_speed; } system_t;
extern system_t sys;
extern volatile Percent sys_rt_f_override, sys_rt_r_override, sys_rt_s_override;
float* system_get_mpos();
void mpos_to_wpos(float* position);
extern bool stepper_idle;
float st_get_realtime_rate();
struct CoolantState { uint8_t Mist : 1; uint8_t Flood : 1; };
CoolantState coolant_get_state();

// ---- SDCard.h
enum class SDState : uint8_t { Idle = 0, NotPresent = 1, Busy = 2, BusyPrinting = 2, BusyUploading = 3, BusyParsing = 4 };
SDState get_sd_state(bool refresh);
extern bool SD_ready_next;
extern uint32_t sd_current_line_number;
void mks_listDir(fs::FS& fs, const char* dirname, uint8_t levels);
uint16_t mks_countDir(fs::FS& fs, const char* dirname, uint8_t levels);
bool openFile(fs::FS& fs, const char* path);
boolean closeFile();
float sd_report_perc_complete();
uint32_t sd_get_current_line_number();
void sd_get_current_filename(char* name);
bool sd_file_check(const char* path);
boolean readFileLine(char* line, int maxlen);

// ---- Serial.h
#define CLIENT_SERIAL 0
#define CLIENT_ALL 7
void serila_write_into_buffer(uint8_t* data);
void grbl_send(uint8_t client, const char* text);
void grbl_sendf(uint8_t client, const char* format, ...);

// ---- Settings
struct SimSetting {
    float v;
    SimSetting(float x = 0) : v(x) {}
    float get() const { return v; }
};
struct FlagSetting : SimSetting { using SimSetting::SimSetting; bool get() const { return v != 0; } };
struct IntSetting : SimSetting { using SimSetting::SimSetting; int32_t get() const { return (int32_t)v; } };
struct FloatSetting : SimSetting { using SimSetting::SimSetting; };
extern FlagSetting *probe_invert, *hard_limits, *homing_enable, *laser_mode, *beep_status, *alert_sound_status;
extern IntSetting* language_select;
extern FloatSetting* rpm_max;

// ---- ESP / WiFi / hora
struct EspStub {
    const char* getChipModel() { return "ESP32-D0WD"; }
    uint8_t getChipRevision() { return 1; }
    uint32_t getFlashChipSize() { return 8 * 1024 * 1024; }
    uint64_t getEfuseMac() { return 0x0C8B951C0F9CULL; }
    uint32_t getCpuFreqMHz() { return 240; }
};
extern EspStub ESP;
float temperatureRead();
void configTzTime(const char* tz, const char* s1, const char* s2 = 0);
String trim(const char* s);
