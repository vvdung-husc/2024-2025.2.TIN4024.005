#pragma once
#include <Arduino.h>

bool IsReady(unsigned long &ulTimer, unsigned long millisecond); // Thay uint32_t -> unsigned long
String StringFormat(const char* fmt, ...);
