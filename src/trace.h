#ifndef TRACE_H
#define TRACE_H

/*  TRACE subsystem

    TRACE is a small, standalone logging facility intended to be safe to call from anywhere in the codebase.

    The TRACE(...) macro records a single log entry that includes precise call-site context (__FILE__, __LINE__, and
    __func__) together with a printf-style formatted message. Format strings are validated at compile time via the
   printf format attribute, and callers must not include a trailing newline, as TRACE always emits exactly one line.

    Each trace entry is written to stdout and, if configured, duplicated to a log file set via set_trace_file(). Every
    line begins with a local ISO-8601 timestamp including nanoseconds and a numeric timezone offset, followed by the
    call-site information and the formatted message.

    The output format is:
        YYYY-MM-DDTHH:MM:SS,nnnnnnnnn+HH:MM file:line - function - message

    TRACE returns 0 on success and -1 on internal failure. When an internal error occurs, details are reported to
    stderr. This behavior is intentional while the module is under development.

    The trace module depends only on standard C/POSIX libraries and trace.h. Other modules may depend on trace, but
    trace itself remains fully standalone and must not affect application logic beyond producing diagnostic output.

    TODO:
    Performance optimizations may be introduced later, such as reducing function calls or minimizing per-call overhead,
    once behavior and stability are fully validated.
*/

#include <stdarg.h>

int set_trace_file(const char *path);
void close_trace_file(void);

int _trace_va(const char *file, int line, const char *function, const char *format, va_list arguments);

int _trace(const char *file, int line, const char *function, const char *format, ...)
    __attribute__((format(printf, 4, 5)));

#define TRACE(...) _trace(__FILE__, __LINE__, __func__, __VA_ARGS__)

#endif /* TRACE_H */
