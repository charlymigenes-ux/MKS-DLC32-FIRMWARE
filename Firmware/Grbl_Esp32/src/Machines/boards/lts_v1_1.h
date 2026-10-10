// Perfil de placa: MKS LTS V1.1 (la placa de la TwoTrees TTS-55 PRO).
//
// Un perfil define SOLO lo que cambia entre placas. Todo lo comun (I2S, motores, limites,
// LCD, SD, zumbador) esta en i2s_out_xyz_mks_dlc32.h.
//
// Estado: PROBADO en una TTS-55 PRO de 5 W (pantalla, WebUI, laser, SD y pitidos).
// Origen de los pines: no hay documentacion oficial de MKS para la LTS; son los que funcionan con este
// firmware. La SONDA (GPIO2) NO esta verificada.
#pragma once

#define BOARD_NAME          "Board:MKS LTS V1.1"

#define SPINDLE_OUTPUT_PIN  GPIO_NUM_22
#define LASER_OUTPUT_PIN    GPIO_NUM_22
#define PROBE_PIN           GPIO_NUM_2
