#define _DEFAULT_SOURCE

#include "trace.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TIMESTAMP_LENGHT 35

static void timestampISO8601(char *buffer, size_t bufferSize, const struct timespec *timestamp) {
    struct tm localtime;
    char date[32];      // YYYY-MM-DDTHH:MM:SS
    char timezone1[6];  // +HHMM
    char timezone2[7];  // +HH:MM
    int result;
    size_t sizeResult;

    // Parameter validation
    if (buffer == NULL) {
        return;
    }
    memset(buffer, 0, bufferSize);
    if (bufferSize < TIMESTAMP_LENGHT) {
        return;
    }

    // convert to local time
    localtime_r(&timestamp->tv_sec, &localtime);

    // YYYY-MM-DDTHH:MM:SS
    sizeResult = strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%S", &localtime);
    if (sizeResult <= 0) {
        return;
    }

    // timezone as +HHMM, then make +HH:MM
    strncpy(timezone2, "+00:00", sizeof(timezone2));
    sizeResult = strftime(timezone1, sizeof(timezone1), "%z", &localtime);
    if (sizeResult != 5) {
        return;
    }
    timezone2[0] = timezone1[0];
    timezone2[1] = timezone1[1];
    timezone2[2] = timezone1[2];
    timezone2[4] = timezone1[3];
    timezone2[5] = timezone1[4];

    // output YYYY-MM-DDTHH:MM:SS,nnnnnnnnn+HH:MM
    result = snprintf(buffer, bufferSize, "%s,%09ld%s", date, (long)timestamp->tv_nsec, timezone2);
    if (result < TIMESTAMP_LENGHT) {
        memset(buffer, 0, bufferSize);  // its not right, dont ship it
        return;
    }

    return;
}

static void timestampRelative(char *buffer, size_t bufferSize, const struct timespec *timestamp0,
                              const struct timespec *timestamp1) {
    int offset = 0;
    struct timespec t0, t1, timediff;
    char sign = '+';

    t0 = *timestamp0;
    t1 = *timestamp1;

    // get sign
    if (t0.tv_sec > t1.tv_sec || (t0.tv_sec == t1.tv_sec && t0.tv_nsec > t1.tv_nsec)) {
        t0 = *timestamp1;
        t1 = *timestamp0;
        sign = '-';
    }

    // normalize difference
    timediff.tv_sec = t1.tv_sec - t0.tv_sec;
    timediff.tv_nsec = t1.tv_nsec - t0.tv_nsec;
    if (timediff.tv_nsec < 0) {
        timediff.tv_sec -= 1;
        timediff.tv_nsec += 1000000000L;
    }

    if (timediff.tv_sec > 0) {
        snprintf(buffer, bufferSize, "%c%ld,%09ldns", sign, timediff.tv_sec, timediff.tv_nsec);
        return;
    }

    snprintf(buffer, bufferSize, "%c%ld", sign, timediff.tv_nsec);
    return;
}

void _trace(const char *file, int line, const char *func, const char *format, ...) {
    static struct timespec lastTimestamp = {0};
    struct timespec currentTimestamp;
    char timeString[TIMESTAMP_LENGHT + 1];
    va_list args;

    // get current time
    clock_gettime(CLOCK_REALTIME, &currentTimestamp);

    if (currentTimestamp.tv_sec > lastTimestamp.tv_sec) {
        // output absolute timestamp
        timestampISO8601(timeString, sizeof(timeString), &currentTimestamp);
        fprintf(stdout, "%s - ", timeString);
    } else {
        if (lastTimestamp.tv_sec == 0) {
            lastTimestamp = currentTimestamp;
        }
        // output relative timestamp
        timestampRelative(timeString, sizeof(timeString), &lastTimestamp, &currentTimestamp);
        fprintf(stdout, "%29s       - ", timeString);
    }
    lastTimestamp = currentTimestamp;

    // output file:line function()
    fprintf(stdout, "%s:%d - %s() - ", file, line, func);

    // output formatted message
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);

    // output: newline
    fputc('\n', stdout);
    fflush(stdout);
}
