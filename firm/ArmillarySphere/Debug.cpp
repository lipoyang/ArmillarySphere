#include <Arduino.h>
#include "Debug.h"

void debug_print(char *format, ...)
{
  static char buff[256];
  
  va_list args;
  va_start(args , format);
  vsprintf(buff, format, args);
  va_end(args);

  Serial.print(buff);
}
