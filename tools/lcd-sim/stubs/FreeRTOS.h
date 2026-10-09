#pragma once
#include "Arduino.h"
typedef void* TaskHandle_t;
typedef void* SemaphoreHandle_t;
typedef int BaseType_t;
#define pdPASS 1
#define pdTRUE 1
inline BaseType_t xSemaphoreGive(SemaphoreHandle_t) { return 1; }
