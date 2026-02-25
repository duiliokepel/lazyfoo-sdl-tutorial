#include "assert.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "trace.h"

#define SET_BOLD_MODE "\x1b[1m"
#define SET_COLOR_RED "\x1b[31m"
#define SET_COLOR_YELLOW "\x1b[33m"
#define RESET_ALL_MODES "\x1b[0m"

_Thread_local int g_assert_errno = 0;

int _trace_assert_failed(const char *file, int line, const char *function,
                         const char *expression, const char *reason_format,
                         ...) {
    int return_code = 0;
    char format[1024];
    va_list arguments;

    if (file == NULL) {
        TRACE("Argument file should not be NULL");
        return -1;
    }
    if (function == NULL) {
        TRACE("Argument function should not be NULL");
        return -1;
    }
    if (expression == NULL) {
        TRACE("Argument expression should not be NULL");
        return -1;
    }
    if (reason_format == NULL) {
        TRACE("Argument reason_format should not be NULL");
        return -1;
    }

    return_code = snprintf(format, sizeof(format),
                           SET_BOLD_MODE SET_COLOR_RED
                           "Assertion failed" RESET_ALL_MODES ": (%s) - %s",
                           expression, reason_format);
    if (return_code < 0) {
        int error_num = errno;
        TRACE("snprintf error=[%s]", strerror(error_num));
        return -1;
    }
    if ((size_t)return_code >= sizeof(format)) {
        TRACE("return_code=[%d] exceeds size of format=[%zu]", return_code,
              sizeof(format));
        return -1;
    }

    va_start(arguments, reason_format);
    return_code = _trace_va(file, line, function, format, arguments);
    va_end(arguments);
    if (return_code < 0) {
        fprintf(stderr, "%s:%d - %s - _trace_va error\n", __FILE__, __LINE__,
                __func__);
        return -1;
    }

    return 0;
}

int _trace_loopbound_exceeded(const char *file, int line, const char *function,
                              const char *expression, const char *reason_format,
                              ...) {
    int return_code = 0;
    char format[1024];
    va_list arguments;

    if (file == NULL) {
        TRACE("Argument file should not be NULL");
        return -1;
    }
    if (function == NULL) {
        TRACE("Argument function should not be NULL");
        return -1;
    }
    if (expression == NULL) {
        TRACE("Argument expression should not be NULL");
        return -1;
    }
    if (reason_format == NULL) {
        TRACE("Argument reason_format should not be NULL");
        return -1;
    }

    return_code = snprintf(format, sizeof(format),
                           SET_BOLD_MODE SET_COLOR_YELLOW
                           "Loop exceeded bound" RESET_ALL_MODES ": (%s) - %s",
                           expression, reason_format);
    if (return_code < 0) {
        int error_num = errno;
        TRACE("snprintf error=[%s]", strerror(error_num));
        return -1;
    }
    if ((size_t)return_code >= sizeof(format)) {
        TRACE("return_code=[%d] exceeds size of format=[%zu]", return_code,
              sizeof(format));
        return -1;
    }

    va_start(arguments, reason_format);
    return_code = _trace_va(file, line, function, format, arguments);
    va_end(arguments);
    if (return_code < 0) {
        fprintf(stderr, "%s:%d - %s - _trace_va error\n", __FILE__, __LINE__,
                __func__);
        return -1;
    }

    return 0;
}
