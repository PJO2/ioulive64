// ------------
// A timestamped printf
// ------------

#include <stdarg.h>
#include <stdio.h>
#include <time.h>


// Timestamped printf
void tsprintf(const char *format, ...) {
    va_list args;
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timebuf[20];

    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_now);
    printf("[%s] ", timebuf);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
