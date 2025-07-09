// ------------
// A timestamped printf
// ------------

#include <stdarg.h>
#include <stdio.h>
#include <time.h>


// Timestamped printf
void tsprintf(const char *format, ...) {
    va_list args;
    struct tm tm_now;
    char timebuf[20];
    time_t now;
    
    now = time(NULL);
    localtime_r(&now, &tm_now);  // thread safe
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm_now);
    printf("[%s] ", timebuf);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
