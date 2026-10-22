#pragma once

/*
    sculpfun_s30.h
    Maquina para la placa Sculpfun S30 / S30 Pro (ESP32-D0WD, 4 MB, "De By LYz 20221115"), sin pantalla.

    Los pines salen del SculpFun.h oficial (Sculpfun-design/S30-Series-Software-Firmware) y de la lectura
    de los ajustes `$` de una S30 Pro real. Los motores van por GPIO directos (no hay registros I2S).

    Estado: SIN PROBAR con motores ni laser conectados; solo se verifico que compila.
*/

#ifdef ENABLE_SD_CARD
    #undef ENABLE_SD_CARD
#endif

#define NO_LCD

#define MACHINE_NAME                "SculpFun S30"
#define BOARD_NAME                  "Board:Sculpfun S30 Pro"
#define FW_NAME                     "V1.0(M+X.S30.20261022)"

#ifdef N_AXIS
    #undef N_AXIS
#endif
#define N_AXIS 3     // como el firmware de Sculpfun: Z existe en los informes pero sin motor

#define HTTP_UPLOAD_BUFLEN 1024*10

#define ENABLE_SOFTWARE_DEBOUNCE

// Motores (GPIO directos). El enable es comun a X e Y.
#define X_STEP_PIN                  GPIO_NUM_27
#define X_DIRECTION_PIN             GPIO_NUM_26
#define Y_STEP_PIN                  GPIO_NUM_33
#define Y_DIRECTION_PIN             GPIO_NUM_32
#define STEPPERS_DISABLE_PIN        GPIO_NUM_25

// Finales de carrera (GPIO 34 y 35 son solo entrada)
#define X_LIMIT_PIN                 GPIO_NUM_34
#define Y_LIMIT_PIN                 GPIO_NUM_35

// Laser: ver MKS DLC32, la subclase Laser lee $32 y permite M4
#define SPINDLE_TYPE                SpindleType::LASER
#define SPINDLE_OUTPUT_PIN          GPIO_NUM_4
#define LASER_OUTPUT_PIN            SPINDLE_OUTPUT_PIN

// Refrigerante / bomba de aire como en el firmware de Sculpfun
#define COOLANT_FLOOD_PIN           GPIO_NUM_16
#define COOLANT_MIST_PIN            GPIO_NUM_17

// Sin zumbador, sin LCD, sin sonda
#define BEEPER                      UNDEFINED_PIN
#define LCD_SCK                     UNDEFINED_PIN
#define LCD_MISO                    UNDEFINED_PIN
#define LCD_MOSI                    UNDEFINED_PIN
#define LCD_RS                      UNDEFINED_PIN
#define LCD_EN                      UNDEFINED_PIN
#define LCD_RST                     UNDEFINED_PIN
#define LCD_CS                      UNDEFINED_PIN
#define TOUCH_CS                    UNDEFINED_PIN
#define IIC_SCL                     UNDEFINED_PIN
#define IIC_SDA                     UNDEFINED_PIN

// === Valores por defecto (solo se aplican tras $RST=*; los ajustes viven en NVS)
#define DEFAULT_STEP_PULSE_MICROSECONDS     10
#define DEFAULT_STEPPER_IDLE_LOCK_TIME      25
#define DEFAULT_DIRECTION_INVERT_MASK       4
#define DEFAULT_HARD_LIMIT_ENABLE           1
#define DEFAULT_HOMING_ENABLE               1
#define DEFAULT_HOMING_CYCLE_0              (bit(X_AXIS) | bit(Y_AXIS))
#define DEFAULT_HOMING_CYCLE_1              0
#define DEFAULT_HOMING_DIR_MASK             7
#define DEFAULT_HOMING_FEED_RATE            700.0
#define DEFAULT_HOMING_SEEK_RATE            4000.0
#define DEFAULT_HOMING_PULLOFF              1.0

#define DEFAULT_SPINDLE_RPM_MAX             1000.0
#define DEFAULT_SPINDLE_RPM_MIN             0.0
#define DEFAULT_LASER_MODE                  1
#define DEFAULT_SPINDLE_FREQ                1000
#define DEFAULT_LASER_FULL_POWER            1000
#define DEFAULT_SPINDLE_MAX_VALUE           1000
#define DEFAULT_SPINDLE_MIN_VALUE           0

#define DEFAULT_X_STEPS_PER_MM 80.0
#define DEFAULT_Y_STEPS_PER_MM 80.0
#define DEFAULT_Z_STEPS_PER_MM 250.0

#define DEFAULT_X_MAX_RATE 6000.0
#define DEFAULT_Y_MAX_RATE 6000.0
#define DEFAULT_Z_MAX_RATE 1000.0

#define DEFAULT_X_ACCELERATION 1000.0
#define DEFAULT_Y_ACCELERATION 1000.0
#define DEFAULT_Z_ACCELERATION 1000.0

#define DEFAULT_X_MAX_TRAVEL 400.0
#define DEFAULT_Y_MAX_TRAVEL 410.0
#define DEFAULT_Z_MAX_TRAVEL 200.0

#define DEFAULT_BEEP_STATUS                 0
#define DEFAULT_LANGUAGE_STATUS             1
