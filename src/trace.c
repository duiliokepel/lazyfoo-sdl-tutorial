#define _DEFAULT_SOURCE

#include "trace.h"

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define TIMESTAMP_SIZE 36  // YYYY-MM-DDTHH:MM:SS,nnnnnnnnn+HH:MM

static char trace_path[PATH_MAX] = "";
static FILE *trace_file = NULL;

int set_trace_file(const char *path) {
    size_t length = 0;

    if (path == NULL) {
        fprintf(stderr, "%s:%d - %s - Argument path should not be NULL\n", __FILE__, __LINE__, __func__);
        return -1;
    }

    // Check path length then copý it
    length = strnlen(path, sizeof(trace_path));
    if (length >= sizeof(trace_path)) {
        fprintf(stderr, "%s:%d - %s - length of path exceeds PATH_MAX=[%zu]\n", __FILE__, __LINE__, __func__,
                sizeof(trace_path));
        return -1;
    }
    memcpy(trace_path, path, length + 1);

    // Close file if it is open
    if (trace_file != NULL) {
        fclose(trace_file);
        trace_file = NULL;
    }

    // Open file on new path
    trace_file = fopen(trace_path, "ab");
    if (trace_file == NULL) {
        int error_num = errno;
        fprintf(stderr, "%s:%d - %s - Failed to open file \"%s\" error=[%s]\n", __FILE__, __LINE__, __func__,
                trace_path, strerror(error_num));
        return -1;
    }

    return 0;
}

void close_trace_file(void) {
    if (trace_file != NULL) {
        fclose(trace_file);
        trace_file = NULL;
    }
    return;
}

static int timestampISO8601(char *buffer, const size_t buffer_size, const struct timespec *timestamp) {
    struct tm localtime;
    struct tm *return_tm_pointer = NULL;
    char date[32];      // YYYY-MM-DDTHH:MM:SS
    char timezone1[6];  // +HHMM
    char timezone2[7];  // +HH:MM
    int return_code;
    size_t return_size;

    // Parameter validation
    if (buffer == NULL) {
        fprintf(stderr, "%s:%d - %s - Argument buffer should not be NULL\n", __FILE__, __LINE__, __func__);
        return -1;
    }
    if (timestamp == NULL) {
        fprintf(stderr, "%s:%d - %s - Argument timestamp should not be NULL\n", __FILE__, __LINE__, __func__);
        return -1;
    }
    if (buffer_size < TIMESTAMP_SIZE) {
        fprintf(stderr, "%s:%d - %s - Argument buffer_size=[%zu] should not be smaller than TIMESTAMP_SIZE=[%d]\n",
                __FILE__, __LINE__, __func__, buffer_size, TIMESTAMP_SIZE);
        return -1;
    }

    memset(buffer, 0, buffer_size);

    // convert to local time
    return_tm_pointer = localtime_r(&timestamp->tv_sec, &localtime);
    if (return_tm_pointer == NULL) {
        int error_num = errno;
        fprintf(stderr, "%s:%d - %s - localtime_r error=[%s]\n", __FILE__, __LINE__, __func__, strerror(error_num));
        return -1;
    }
    // YYYY-MM-DDTHH:MM:SS
    return_size = strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%S", &localtime);
    if (return_size <= 0) {
        fprintf(stderr, "%s:%d - %s - Error generating timestamp in strftime\n", __FILE__, __LINE__, __func__);
        return -1;
    }

    // timezone as +HHMM, then make +HH:MM
    strncpy(timezone2, "+00:00", sizeof(timezone2));
    return_size = strftime(timezone1, sizeof(timezone1), "%z", &localtime);
    if (return_size != 5) {
        fprintf(stderr, "%s:%d - %s - Error generating timezone in strftime\n", __FILE__, __LINE__, __func__);
        return -1;
    }
    timezone2[0] = timezone1[0];
    timezone2[1] = timezone1[1];
    timezone2[2] = timezone1[2];
    timezone2[3] = ':';
    timezone2[4] = timezone1[3];
    timezone2[5] = timezone1[4];
    timezone2[6] = '\0';

    // output YYYY-MM-DDTHH:MM:SS,nnnnnnnnn+HH:MM
    return_code = snprintf(buffer, buffer_size, "%s,%09ld%s", date, (long)timestamp->tv_nsec, timezone2);
    if (return_code < 0) {
        int error_num = errno;
        fprintf(stderr, "%s:%d - %s - snprintf error=[%s]\n", __FILE__, __LINE__, __func__, strerror(error_num));
        memset(buffer, 0, buffer_size);
        return -1;
    }
    if ((size_t)return_code >= buffer_size) {
        fprintf(stderr, "%s:%d - %s - return_code=[%i] should not be bigger than buffer_size=[%zu]\n", __FILE__,
                __LINE__, __func__, return_code, buffer_size);
        memset(buffer, 0, buffer_size);
        return -1;
    }

    return 0;
}

int _trace_va(const char *file, int line, const char *function, const char *format, va_list arguments) {
    int return_code = 0;
    struct timespec timestamp;
    char time_string[TIMESTAMP_SIZE];

    // Validate arguments
    if (file == NULL) {
        fprintf(stderr, "%s:%d - %s - Argument file should not be NULL\n", __FILE__, __LINE__, __func__);
        return -1;
    }
    if (function == NULL) {
        fprintf(stderr, "%s:%d - %s - Argument function should not be NULL\n", __FILE__, __LINE__, __func__);
        return -1;
    }
    if (format == NULL) {
        fprintf(stderr, "%s:%d - %s - Argument format should not be NULL\n", __FILE__, __LINE__, __func__);
        return -1;
    }

    // get current time
    return_code = clock_gettime(CLOCK_REALTIME, &timestamp);
    if (return_code != 0) {
        int error_num = errno;
        fprintf(stderr, "%s:%d - %s - clock_gettime error=[%s]\n", __FILE__, __LINE__, __func__, strerror(error_num));
        return -1;
    }

    return_code = timestampISO8601(time_string, sizeof(time_string), &timestamp);
    if (return_code < 0) {
        fprintf(stderr, "%s:%d - %s - timestampISO8601 error\n", __FILE__, __LINE__, __func__);
        return -1;
    }

    // output to trace file
    if (trace_file != NULL) {
        va_list arguments_file;

        // output YYYY-MM-DDTHH:MM:SS,nnnnnnnnn+HH:MM file:line - function -
        return_code = fprintf(trace_file, "%s - %s:%d - %s - ", time_string, file, line, function);
        if (return_code < 0) {
            int error_num = errno;
            fprintf(stderr, "%s:%d - %s - fprintf error=[%s]\n", __FILE__, __LINE__, __func__, strerror(error_num));
            return -1;
        }

        // Copy va_list and output message to file
        va_copy(arguments_file, arguments);
        return_code = vfprintf(trace_file, format, arguments_file);
        if (return_code < 0) {
            int error_num = errno;
            fprintf(stderr, "%s:%d - %s - vfprintf error=[%s]\n", __FILE__, __LINE__, __func__, strerror(error_num));
            va_end(arguments_file);
            return -1;
        }
        va_end(arguments_file);

        return_code = fputc('\n', trace_file);
        if (return_code == EOF) {
            int error_num = errno;
            fprintf(stderr, "%s:%d - %s - fputc error=[%s]\n", __FILE__, __LINE__, __func__, strerror(error_num));
            return -1;
        }
    }

    // output to stdout
    {
        va_list arguments_stdout;

        // output YYYY-MM-DDTHH:MM:SS,nnnnnnnnn+HH:MM file:line - function -
        return_code = fprintf(stdout, "%s - %s:%d - %s - ", time_string, file, line, function);
        if (return_code < 0) {
            int error_num = errno;
            fprintf(stderr, "%s:%d - %s - fprintf error=[%s]\n", __FILE__, __LINE__, __func__, strerror(error_num));
            return -1;
        }

        // Copy va_list and output message to stdout
        va_copy(arguments_stdout, arguments);
        return_code = vfprintf(stdout, format, arguments_stdout);
        if (return_code < 0) {
            int error_num = errno;
            fprintf(stderr, "%s:%d - %s - vfprintf error=[%s]\n", __FILE__, __LINE__, __func__, strerror(error_num));
            va_end(arguments_stdout);
            return -1;
        }
        va_end(arguments_stdout);

        return_code = fputc('\n', stdout);
        if (return_code == EOF) {
            int error_num = errno;
            fprintf(stderr, "%s:%d - %s - fputc error=[%s]\n", __FILE__, __LINE__, __func__, strerror(error_num));
            return -1;
        }
    }

    return 0;
}

int _trace(const char *file, int line, const char *function, const char *format, ...) {
    int return_code = 0;
    va_list arguments;

    va_start(arguments, format);
    return_code = _trace_va(file, line, function, format, arguments);
    va_end(arguments);
    if (return_code < 0) {
        fprintf(stderr, "%s:%d - %s - _trace_va error\n", __FILE__, __LINE__, __func__);
        return -1;
    }

    return 0;
}
