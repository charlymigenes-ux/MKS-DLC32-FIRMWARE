#ifndef __language_pt_h
#define __language_pt_h

/* Traducción al portugués (Brasil) del LCD (MKS DLC32).
 *
 * Nota de fuente: las fuentes originales del LCD (lv_font_roboto_16 y dlc32Font)
 * solo traen ASCII y unos caracteres chinos. Las tildes y la ç salen de
 * lv_pic/font_latin.c (roboto16Latin y dlc32FontLatin), que añaden el rango
 * Latin-1 y delegan todo lo demás en la fuente original.
 */

/* 公共 / común */
#define BACK_PT                 "Voltar"
#define YES_PT                  "Sim"
#define NO_PT                   "Não"

/* 主页 / portada */
#define CONTROL_PT              "Controle"
#define SCULPTURE_PT            "Gravação"
#define TOOL_PT                 "Ferramentas"
#define MPOS_PT                 "Origem"
#define WPOS_PT                 "Trabalho"
#define WIFI_CONNECT_PT         "Conectado"
#define WIFI_DISCONNECT_PT      "Sem conexão"

/* 控制界面 / control */
#define XY_CLEAR_PT             "Zerar XY"
#define Z_CLEAR_PT              "Zerar Z"
#define KNIFE_PT                "Lâmina"
#define NEXT_PT                 "Próximo"
#define UP_PT                   "Subir"
#define COOLING_PT              "Refrigeração"
#define POSITION_PT             "Posição"
#define SPEED_HIGH_PT           "Vel. alta"
#define SPEED_MID_PT            "Vel. média"
#define SPEED_LOW_PT            "Vel. baixa"
#define SPINDLE_PT              "Spindle"
#define CARVE_PT                "Gravar"

/* 文件界面 / archivos */
#define DIS_NO_SDCARD_PT        "Sem cartão SD"

/* 雕刻界面 / tallado */
#define HOLD_PT                 "Pausa"
#define CYCLE_PT                "Ciclo"
#define STOP_PT                 "Parar"
#define ADJUST_PT               "Ajuste"
#define SPINDLE_SPPED_PT        "Velocidade do spindle:"
#define FEED_RATE_PT            "Avanço:"
#define RAPID_SPEED_PT          "Vel. rápida"
#define CARVE_TIMES_PT          "vezes:"

/* 提示 / avisos */
#define DIS_STOP_CARVE_PT       "Quer parar a gravação?"
#define DIS_HOMEING_PT          "Homing..."
#define DIS_NO_HARD_HOME_PT     "Homing físico desativado..."
#define DIS_HOME_SUCCEED_PT     "Homing concluído!"
#define DIS_HOME_FAIL_PT        "Falha no homing"
#define DIS_PROBE_SET_PT        "Configurando sonda..."
#define DIS_PROBE_SECCEED_PT    "Sonda ok!"
#define DIS_PROBE_FAIL_PT       "Falha na sonda"
#define DIS_HARD_LIMIT_PT       "Limite físico!"
#define DIS_SOFT_LIMIT_PT       "Limite de software!"
#define DIS_UNLOCK_PT           "Por favor, desbloqueie!"
#define DIS_WAIT_MC_STOP_PT     "Aguarde a máquina parar"
#define DIS_POS_SUCCEED_PT      "Posição alcançada"

/* Botones y popups (antes literales fijos en el codigo de cada pagina) */
#define CANCEL_PT               "Cancelar"
#define FRAME_PT                "Moldura"
#define CARVE_FILE_SURE_PT      "Gravar este arquivo?"
#define PAUSE_PT                "Pausar"
#define START_PT                "Iniciar"
#define CONFIRM_PT              "Confirmar"
#define ADD_PT                  "Aumentar"
#define REDUCE_PT               "Reduzir"
#define EXIT_PT                 "Sair"
#define LANGUAGE_PT             "Idioma"
#define SCANF_PT                "Buscar"
#define RECONNECT_PT            "Reconectar"
#define CONNECT_PT              "Conectar"
#define PASSWORD_PT             "Senha:"
#define Z_HOME_PT               "Z início"
#define UPDATE_TITLE_PT         "Atualizando..."

/* Avisos y mensajes de dialogo */
#define INFO_PT                 "Informação"
#define WARNING_PT              "Aviso"
#define ERROR_PT                "Erro"
#define UNLOCK_SUCCESS_PT       "Desbloqueio concluído!"
#define SETTING_ERROR_PT        "Erro de ajuste!"
#define SET_6_1_PT              "Defina $6=1!"
#define WAIT_IDLE_PT            "Aguarde a máquina ficar livre"
#define FILE_TOO_BIG_PT         "O arquivo é grande demais"
#define CONTINUE_SURE_PT        "Quer continuar?"
#define FILE_LOADING_PT         "Carregando arquivo..."
#define LOADING_FILE_PT         "Carregando arquivo..."
#define SD_BUSY_PT              "O cartão SD está ocupado..."
#define RUNNING_PT              "Em andamento..."
#define PRINT_STOP_SURE_PT      "Quer parar o trabalho?"
#define PRINT_DONE_PT           "Arquivo concluído!"
#define WIFI_SCANNING_PT        "Buscando wifi..."
#define WIFI_CONNECTING_PT      "Conectando wifi..."
#define WIFI_DISCONNECTING_PT   "Desconectando wifi..."
#define WIFI_PWD_PROMPT_PT      "Digite a senha para conectar"
#define WIFI_STATUS_ON_PT       "WIFI:Conectado"
#define WIFI_STATUS_OFF_PT      "WIFI:Sem conexão"
#define UPDATE_SUCCEED_PT       "Atualização concluída"
#define UPDATE_RESTART_PT       "Reinicie a máquina!"
#define UPDATE_FAIL_PT          "Falha na atualização"
#define UPDATE_FAIL_HELP_PT     "Verifique mkscfg.txt ou o cartão SD"

/* Formatos con %d */
#define POWER_FMT_PT            "Potência:%d%%"
#define SPEED_FMT_PT            "Velocidade:%d%%"
#define SPINDLE_SPEED_FMT_PT    "Vel. do spindle: %d%%"
#define FEED_RATE_FMT_PT        "Avanço: %d%%"
#define RAPID_FMT_PT            "Vel. rápida: %d%%"
#define GAUGE_FEED_PT            "Avanço"
#define GAUGE_SPINDLE_PT         "Spindle"
#define GAUGE_RAPID_PT           "Rápido"
#define JOB_ELAPSED_PT           "Tempo"
#define JOB_LEFT_PT              "Restante"
#define JOB_FEED_REAL_PT         "Avanço"
#define JOB_POWER_REAL_PT        "Potência"
#define RESUME_PT                "Retomar"
#define TOOL_BOARD_PT            "Placa"
// Rotulos con el modo laser activo ($32=1): potencia y velocidad en vez de husillo y avance
#define SPINDLE_LASER_PT               "Laser"
#define SPINDLE_SPEED_LASER_PT         "Potência do laser:"
#define SPINDLE_SPEED_FMT_LASER_PT     "Potência: %d%%"
#define GAUGE_SPINDLE_LASER_PT         "Potência"
#define FEED_RATE_LASER_PT             "Velocidade:"
#define FEED_RATE_FMT_LASER_PT         "Velocidade: %d%%"
#define GAUGE_FEED_LASER_PT            "Velocidade"
#define JOB_FEED_REAL_LASER_PT         "Velocidade"

/* Pagina de pruebas (test) */
#define TESTING_PT              "Testando..."
#define PROBE_CHECK_PT          "Sonda:"
#define X_LIMIT_CHECK_PT        "Limite X:"
#define Y_LIMIT_CHECK_PT        "Limite Y:"
#define Z_LIMIT_CHECK_PT        "Limite Z:"
#define SD_CHECK_PT             "SD:OK"
#define I2C_CHECK_PT            "I2C:"
#define CPU_TEMP_PT             "Temp. CPU:"
#define TEST_WARNING_PT         "#FF0000 Aviso!#"

#endif
