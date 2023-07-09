// debug.cpp

#include "spinach.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>

namespace debug
{
   void log(const char *format, ...)
   {
      char message[1024] = {};
      va_list args;
      va_start(args, format);
      vsprintf_s(message, format, args);
      va_end(args);
      puts(message);
   }
} // !debug
