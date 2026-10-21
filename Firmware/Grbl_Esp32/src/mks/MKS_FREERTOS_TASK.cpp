#include "MKS_FREERTOS_TASK.h"
#include "MKS_draw_boot.h"
#include "MKS_draw_fileview.h"
#include "MKS_draw_language.h"   // mc_language_init: recarga de $40 en caliente
#include "MKS_draw_pen.h"        // plotter: pantalla de la pluma

// 8 KB: con 5 KB la lista de archivos (SD + FAT + snprintf con decimales) desbordaba la pila y corrompia el heap
// ("CORRUPT HEAP ... abort()" al pulsar Grabado). El minimo medido en reposo (1,6 KB) no sirve: hay que medir bajo carga.
#define DISP_TASK_STACK                 4096*2
#define DISP_TASK_PRO                   2
#define DISP_TASK_CORE                  1

#define FRAME_TASK_STACK                4096*2
#define FRAME_TASK_PRO                  3          // 尝试巡边的线程与lvgl线程同优先级
#define FRAME_TASK_CORE                 1

TaskHandle_t lv_disp_tcb = NULL;
TaskHandle_t frame_task_tcb = NULL;

#define USE_DelayUntil

static void mks_page_data_updata(void);

#ifdef ENABLE_JOB_DIAG
// Diagnóstico: maximo tiempo que la tarea LVGL (prioridad 2, por encima
// del lazo de protocolo en prioridad 1) mantiene ocupado el core 1 sin ceder.
// Si ese tiempo supera la duracion del buffer del planificador (~119 ms con
// archivos de segmentos cortos), el lazo no puede alimentar el machine, el
// planificador se vacia y el movimiento se para. Protocol.cpp lo incluye en
// su linea "[diag] hueco ...".
volatile int64_t diag_lvgl_max = 0;
int64_t           diag_lvgl_t0 = 0;
#endif  // ENABLE_JOB_DIAG

IRAM_ATTR void lvgl_disp_task(void *parg) { 

#if defined(USE_DelayUntil)
    TickType_t       xLastWakeTime;
    const TickType_t xDisplayFrequency = 5;                  
    xLastWakeTime = xTaskGetTickCount();   
#endif

    bool logo_flag = true;   
    mks_lvgl_init();
    MEMLOG("tras mks_lvgl_init");

    mks_global_style_init();
    MEMLOG("tras estilos globales");

    mks_boot_begin();   // logo 4 s con la version; si se mantiene pulsado, QR del repositorio
    MEMLOG("tras pantalla de arranque");

    // 创建二值量
    is_fram_need = xSemaphoreCreateBinary();
    // la tarea del marco (8 KB de pila) ya no se crea al arrancar: nace al pulsar Marco y se borra al terminar
    mks_grbl.wifi_connect_enable = true;

    while(1) {

#ifdef ENABLE_JOB_DIAG
        diag_lvgl_t0 = (int64_t)xTaskGetTickCount();
#endif  // ENABLE_JOB_DIAG

        if(logo_flag == true) {
            lv_task_handler();

            if(mks_boot_backlight_due()) {
                 LCD_BLK_ON;
            }

            if(mks_boot_tick()) {
                mks_lv_clean_ui();

                if(mks_updata.updata_flag == UD_HAD_FILE) {
                    mks_draw_updata();
                }else {
                    if(mks_grbl.is_test_mode == true) {
                        mks_draw_test_ui();
                    }else{
                        mks_draw_ready();
                    }
                }
                MEMLOG("tras dibujar pantalla principal");
                logo_flag = false;
            }
        }else {
            lv_task_handler();
            mks_page_data_updata();
        }

#ifdef ENABLE_JOB_DIAG
        {
            int64_t diag_d = (int64_t)xTaskGetTickCount() - diag_lvgl_t0;
            if (diag_d > diag_lvgl_max) {
                diag_lvgl_max = diag_d;
            }
        }
#endif  // ENABLE_JOB_DIAG
#if defined(USE_DelayUntil)
    vTaskDelayUntil(&xLastWakeTime, xDisplayFrequency); //使用相对延时，保证时间精准
#else
        vTaskDelay(5); // 5ms 绝对延时
#endif   
    }
}

uint8_t count_updata = 0;
uint8_t fram_count = 0;
static void mks_page_data_updata(void) { 

    ts35_beep_tick();   // sonidos de fin de trabajo / error (patrones del zumbador)
#ifdef ENABLE_MEM_DIAG
    {   // cada 5 s durante los primeros 3 minutos: linea de tiempo (aqui se ve cuando entra el WiFi)
        static uint32_t last_mem = 0;
        uint32_t        now_ms   = millis();
        if (now_ms < 180000 && now_ms - last_mem >= 5000) {
            last_mem = now_ms;
            char tag[32];
            snprintf(tag, sizeof(tag), "t=%us", (unsigned)(now_ms / 1000));
            MEMLOG(tag);
        }
    }
#endif

    // $40 se puede cambiar desde la pantalla (set_language) o a distancia, con
    // la WebUI escribiendo "$40=n". Aqui se detecta el cambio y se recargan las
    // cadenas del LCD; sin esto mc_language solo se actualizaba al pulsar los
    // botones de la pagina de Idiomas. Se comprueba en cada ciclo (5 ms); el
    // coste es un entero en RAM. OJO: los labels ya dibujados mantienen el texto
    // anterior hasta que se repinte su pagina.
    static int32_t last_language = -1;
    int32_t cur_language = language_select->get();

    if(cur_language != last_language) {
        last_language = cur_language;
        if((cur_language >= SimpleChinese) && (cur_language <= Espanol)) {
            mks_grbl.language = (GRBL_Language)cur_language;
        }
        mc_language_init();
    }

    // $32 (modo laser) tambien cambia los rotulos (Potencia/Velocidad frente a Husillo/Avance).
    static int32_t last_laser = -1;
    int32_t        cur_laser  = laser_mode->get() ? 1 : 0;
    if(cur_laser != last_laser) {
        last_laser = cur_laser;
        mc_language_init();
    }

    // Un trabajo lanzado desde la WebUI (o el serie) no pasa por la LCD: si hay una pantalla "normal"
    // abierta, saltar solo a la de trabajo en curso (200 ms de confirmacion para no reaccionar a un parpadeo).
    {
        static uint8_t job_seen = 0;
        mks_ui_page_t  pg       = mks_ui_page.mks_ui_page;
        bool           otra     = (pg == MKS_UI_Ready) || (pg == MKS_UI_Control) || (pg == MKS_UI_Caving) ||
                        (pg == MKS_UI_inFile) || (pg == MKS_UI_FileView) || (pg == MKS_UI_Tool) || (pg == MKS_UI_LANGUAGE) || (pg == MKS_UI_Adjust) || (pg == MKS_UI_Pen);
        if(otra && get_sd_state(false) == SDState::BusyPrinting) {
            if(++job_seen >= 40) {
                job_seen = 0;
                mks_lv_clean_ui();
                mks_job_goto_print();
            }
        } else {
            job_seen = 0;
        }
    }

    if(mks_ui_page.mks_ui_page == MKS_UI_PAGE_LOADING) {
        /* Do not updata */
        return ;
    }
    else if(mks_ui_page.mks_ui_page == MKS_UI_TEST) {
        if((count_updata == 200) || (count_updata > 200) ) { 
            draw_testing();
        }
    }
    else if (mks_ui_page.mks_ui_page == MKS_UI_Ready) {  //只有在当前页面才更新数据

        if((count_updata == 20) || (count_updata > 20) ) {        // 20*5=100ms
            if(SD_ready_next == false) ready_data_updata();
            count_updata = 0;
        }
    }
    else if(mks_ui_page.mks_ui_page == MKS_UI_Pring) { // 雕刻界面更新数据

        if((count_updata == 200) || (count_updata > 200) ) { // 200*5=1000ms = 1s

            if(SD_ready_next == false) {
                mks_print_data_updata();
            } 
            count_updata = 0;
        }
    }
    else if(mks_ui_page.mks_ui_page == MKS_UI_inFile) { // 雕刻界面更新数据

        if((count_updata == 200) || (count_updata > 200) ) { // 200*5=1000ms = 1s
            move_pos_update();
            probe_check();
        }
    }
    else if(mks_ui_page.mks_ui_page == MKS_UI_Pen) {          // plotter: coordenadas y estado de la pluma (cada 200 ms)
        if(count_updata >= 40) {
            mks_pen_update();
            count_updata = 0;
        }
    }
    else if(mks_ui_page.mks_ui_page == MKS_UI_FileView) {    // vista previa del archivo
        mks_fileview_poll();
    }
    else if(mks_ui_page.mks_ui_page == MKS_UI_Control) {  //控制界面

        if((count_updata == 20) || (count_updata > 20) ) { // 20*5=100ms = 1s
            hard_home_check();
            soft_home_check();
            move_pos_update();
            probe_check();
            count_updata = 0;
        }
    }
    else if(mks_ui_page.mks_ui_page == MKS_UI_Frame) { 
        
        if((count_updata == 100) || (count_updata > 100) ) { // 100*5=100ms = 1s
            if(frame_ctrl.is_begin_run == true) {
                    if(mks_get_frame_status() == true) {

                        if(fram_count == 10) {
                            if(mks_grbl.popup_1_flag != true){
                                frame_finsh_popup();
                                // void mks_draw_cavre_popup();
                            }
                            frame_ctrl.is_begin_run = false;
                            fram_count = 0;
                        }
                        fram_count++;
                    }else {
                        fram_count = 0;
                    }
            }
        }
    }
#if defined(ENABLE_WIFI)
    else if(mks_ui_page.mks_ui_page == MKS_UI_Wifi) {

        if((mks_wifi.wifi_scanf_status == wifi_scanf_begin) || (mks_wifi.wifi_scanf_status == wifi_scanf_waitting)) {
            mks_wifi_scanf();
        }else if(mks_wifi.wifi_scanf_status == wifi_scanf_succeed) {
            mks_lv_clean_ui();
            mks_draw_wifi_show();
            mks_wifi.wifi_scanf_status = wifi_none;
        }else if(mks_wifi.wifi_scanf_status == wifi_connecting) {
            if(mks_grbl.wifi_connect_status == true) {          // 正在连接状态
                if(mks_get_wifi_status() == true) {             // 确认连接上
                    mks_lv_clean_ui();
                    mks_draw_wifi();
                    mks_wifi.wifi_scanf_status = wifi_none;
                }
            }
        }
        else if(mks_wifi.wifi_scanf_status == wifi_scanf_fail) {
            mks_lv_clean_ui();
            mks_draw_wifi();
            mks_wifi.wifi_scanf_status = wifi_none;
        }
        else if(mks_wifi.wifi_scanf_status == wifi_disconnecting) {
            
            if(mks_get_wifi_status() == false) {  // 确认断开
                mks_wifi.wifi_scanf_status = wifi_scanf_begin;   // 重新扫描
                mks_lv_clean_ui();
                mks_draw_wifi();
            }
        }
        count_updata = 0;
    }
#endif
    else if(mks_ui_page.mks_ui_page == MKS_UI_UPDATA) {
        if(mks_updata.updata_flag == UD_UPDATA_ING) {
            mks_cfg_find();
        }
        else if(mks_updata.updata_flag == UD_UPDATA_FINSH) {
            // 更新完成弹窗
            mks_draw_common_pupup_info(mc_language.dis_info, mc_language.dis_update_succeed, mc_language.dis_update_restart);
            mks_cfg_rename(CFG_FILE_PATG2);
            mks_updata.updata_flag = UD_NONE;
        }
        else if(mks_updata.updata_flag == UD_UPDATA_FAIL) {
            // 更新失败弹窗
            mks_draw_common_pupup_info(mc_language.dis_error, mc_language.dis_update_fail, mc_language.dis_update_fail_help);
            mks_updata.updata_flag = UD_NONE;
        }
    }
    count_updata++;
}

// // tskNO_AFFINIT 
IRAM_ATTR void disp_task_init(void) {

    xTaskCreatePinnedToCore(lvgl_disp_task,     // task
                            "lvglTask",         // name for task
                            DISP_TASK_STACK,    // size of task stack
                            NULL,               // parameters
                            DISP_TASK_PRO,      // priority
                            // nullptr,
                            &lv_disp_tcb,
                            DISP_TASK_CORE      // must run the task on same core
                                                // core
    );
}

/*-------------------------------------------------------------------------------------------------*/

IRAM_ATTR void frame_task(void *parg) {

    grbl_send(CLIENT_SERIAL,"Creat frame task succeed\n");
    // 定义一个信号量返回值
    BaseType_t sem_receive = pdPASS;

    while(1) {  

        sem_receive = xSemaphoreTake(is_fram_need, portMAX_DELAY);

        if(sem_receive == pdTRUE) {
            // 触发信号量
            grbl_send(CLIENT_SERIAL,"receive sem succeed\n");
            vTaskSuspend(lv_disp_tcb); // 挂起任务da
            mks_run_frame(frame_ctrl.file_name);
            vTaskResume(lv_disp_tcb);   // 恢复任务
        }
        frame_task_tcb = NULL;   // tarea de un solo uso: libera sus 8 KB de pila
        vTaskDelete(NULL);
    }
}

void frame_task_ensure(void) {
    if(frame_task_tcb == NULL) frame_task_init();
}

IRAM_ATTR void frame_task_init(void ) {

    xTaskCreatePinnedToCore(frame_task,             // task
                            "Frame task",           // name for task
                            FRAME_TASK_STACK,       // size of task stack
                            NULL,                   // parameters
                            FRAME_TASK_PRO,         // priority
                            // nullptr,
                            &frame_task_tcb,
                            FRAME_TASK_CORE         // must run the task on same core
                                                    // core
    );
}

/*-------------------------------------------测试模式----------------------------------------------*/

#define TEST_CODE_TASK_STACK                4096
#define TEST_CODE_TASK_PRO                  3          
#define TEST_CODE_TASK_CORE                 1

TaskHandle_t test_code_handle = NULL;


void dlc32_test_debug(void *parg) {

    while(1) {
    
    }
}

void test_code_task_init() {

    xTaskCreatePinnedToCore(dlc32_test_debug,             // task
                            "Test code",           // name for task
                            TEST_CODE_TASK_STACK,       // size of task stack
                            NULL,                   // parameters
                            TEST_CODE_TASK_PRO,         // priority
                            // nullptr,
                            &test_code_handle,
                            TEST_CODE_TASK_CORE         // must run the task on same core
                                                    // core
    );


}
