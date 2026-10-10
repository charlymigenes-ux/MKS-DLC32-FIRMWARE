// Perfil de placa: MKS DLC32 V2.x (V2.0 y V2.1).
//
// Un perfil define SOLO lo que cambia entre placas. Todo lo comun (I2S, motores, limites,
// LCD, SD, zumbador) esta en i2s_out_xyz_mks_dlc32.h.
//
// Estado: EXPERIMENTAL, SIN PROBAR en una placa V2.x con este firmware (la unica prueba se colgo al
// arrancar y la causa esta sin determinar).
//
// Fuente de los pines: esquemático oficial de MKS (makerbase-mks/MKS-DLC32, hardware/MKS DLC32 V2.1_003/
// MKS DLC32 V2.1_003 SCH.pdf, hoja ESP32.sch):
//   - net "LC" (laser/husillo, borne SPINDLE y TTL) -> IO32
//   - net "Probe"                                   -> IO22
//   - la tarjeta SD usa SCK 14, DO 12, DI 13, CS 15, DET 39 (igual que la LTS; el PDF de pines dice
//     "DO: IO2" por error, manda el esquematico)
//   - IO2 es un LED azul de estado: NO usar IO2 como entrada (en la LTS se usa para la sonda)
//   - IO0 va al conector "I2C interface" (SDA) con pull-up: no se puede usar como salida
//   - la placa no entra sola en modo de descarga desde el USB (hace falta SDA a GND al arrancar)
#pragma once

#define BOARD_NAME          "Board:MKS DLC32 V2.1"

#define SPINDLE_OUTPUT_PIN  GPIO_NUM_32
#define LASER_OUTPUT_PIN    GPIO_NUM_32
#define PROBE_PIN           GPIO_NUM_22
