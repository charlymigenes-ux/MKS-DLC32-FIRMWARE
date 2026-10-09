#include "MKS_TS35.h"

TFT_eSPI tft = TFT_eSPI(); 

void tft_LCD_Fill() {
    tft.fillRect(0, 0,480, 320, TFT_COLOR_RED);
}

void tft_TS35_init() {
    tft.begin();
    tft.setRotation(1);
    // tft.fillScreen(tft.color565(0X1A, 0X1A, 0X1A));
    tft.initDMA();
    delay_ms(100);
}   

void ts35_beep_init() { 
    pinMode(BEEPER, OUTPUT);
    digitalWrite(BEEPER, LOW);
}

void ts35_beep_on(void) {
    if(beep_status->get()) digitalWrite(BEEPER, HIGH);
}

static const uint16_t* beep_pat   = NULL;   // duraciones en ms, alternando encendido / apagado
static uint8_t         beep_len   = 0;
static uint8_t         beep_idx   = 0;
static uint32_t        beep_t0    = 0;
static uint32_t        beep_last_error = 0;

void ts35_beep_off(void) {
    if (beep_pat != NULL) return;   // un patron en curso es dueño del zumbador (la LCD llama a esto en cada lectura sin toque)
    digitalWrite(BEEPER, LOW);
}

// pi-pi-pi---piiii: trabajo terminado | dos tonos largos: error que aborta el trabajo | tres largos: alarma
static const uint16_t PAT_DONE[]  = { 120, 90, 120, 90, 120, 90, 450 };
static const uint16_t PAT_ERROR[] = { 350, 150, 350 };
static const uint16_t PAT_ALARM[] = { 600, 150, 600, 150, 600 };

void ts35_beep_play(ts35_beep_kind_t kind) {
    if (!alert_sound_status->get()) return;   // $39
    uint32_t now = millis();
    if (kind == BEEP_ERROR) {
        if (beep_pat != NULL || (beep_last_error != 0 && now - beep_last_error < 3000)) return;   // no machacar un sonido ni repetir errores
        beep_last_error = now;
    }
    const uint16_t* pat = kind == BEEP_DONE ? PAT_DONE : (kind == BEEP_ERROR ? PAT_ERROR : PAT_ALARM);
    uint8_t         len = kind == BEEP_DONE ? sizeof(PAT_DONE) / 2 : (kind == BEEP_ERROR ? sizeof(PAT_ERROR) / 2 : sizeof(PAT_ALARM) / 2);
    beep_idx = 0;
    beep_len = len;
    beep_t0  = now;
    beep_pat = pat;
    digitalWrite(BEEPER, HIGH);
}

void ts35_beep_tick(void) {
    if (beep_pat == NULL) return;
    if (millis() - beep_t0 < beep_pat[beep_idx]) return;
    beep_idx++;
    beep_t0 = millis();
    if (beep_idx >= beep_len) {
        beep_pat = NULL;
        digitalWrite(BEEPER, LOW);
        return;
    }
    digitalWrite(BEEPER, (beep_idx % 2 == 0) ? HIGH : LOW);
}


#define LED_RESOLUTION      10
#define LED_CHANNEL         14
void bsp_led_init(void) { 
#ifdef LED_PIN
    ledcSetup(LED_CHANNEL, 1000, LED_RESOLUTION); // 设置通道14 ,分辨率为10（2^10 = 1024）
    ledcAttachPin(LED_PIN, LED_CHANNEL);  // 将通道与对应的引脚连接
#endif
}

void bsp_set_duty(uint32_t duty) {
#ifdef LED_PIN
    ledcWrite(LED_CHANNEL, duty);
#endif
}

// 0-255
void bsp_led_pwm_speed(uint8_t speed) {
    uint32_t duty = 1024;
    duty = (duty * (255 - speed)) / 255;
    bsp_set_duty(duty);
}


uint32_t led_count = 0;;
uint8_t led_dir_flag = 1;
void bsp_led_run(void) {

    if(led_dir_flag) {
        led_count = led_count + 15;
        if(led_count >= 255) {
            led_dir_flag = 0;
        }
    }else{
        led_count = led_count - 15;
        if(led_count == 0) {
            led_dir_flag = 1;
        }
    }
    bsp_led_pwm_speed(led_count);
}










