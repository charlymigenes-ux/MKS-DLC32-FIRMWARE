// Stub minimo de Arduino.h para compilar las pantallas del LCD en el PC.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef __cplusplus
#include <string>
typedef bool boolean;
typedef uint8_t byte;
#define PROGMEM
#define IRAM_ATTR
#define portTICK_RATE_MS 1
extern "C" uint32_t millis(void);
inline void delay(uint32_t) {}
inline void delay_ms(uint32_t) {}
class String : public std::string {
  public:
    String() {}
    String(const char* s) : std::string(s ? s : "") {}
    String(const std::string& s) : std::string(s) {}
    const char* c_str() const { return std::string::c_str(); }
    void trim() {}
    int toInt() const { return atoi(c_str()); }
    String& operator+=(const char* s) { append(s); return *this; }
};
class IPAddress {
  public:
    String toString() const { return String("192.168.0.61"); }
};
#else
uint32_t millis(void);
#endif
