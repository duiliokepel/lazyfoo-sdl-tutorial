#ifndef ASSERTX_H
#define ASSERTX_H

/*  ASSERT subsystem

    This module exists to reduce defensive-programming clutter and keep error handling readable at the call site.

    Instead of repeating patterns like:

        if (condition == false) {
            TRACE("reason");
            cleanup();
            return -1;
        }

    the ASSERT macro allows the same logic to be expressed in a single, linear statement:

        ASSERT(condition == true, cleanup(); return -1;, "reason");

    The intent is to keep the normal execution path visually dominant, while still providing detailed error reporting,
    precise call-site context (file, line, function), and explicit failure handling.

    ASSERT logs failures through the TRACE subsystem and then executes the caller-provided action block. The action is
    fully controlled by the caller and may be NO_ACTION when no recovery is needed.
*/

#include <stdbool.h>

#define NO_ACTION \
    do {          \
    } while (0);

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(condition) __builtin_expect(!!(condition), 1)
#define UNLIKELY(condition) __builtin_expect(!!(condition), 0)
#else
#define LIKELY(condition) (condition)
#define UNLIKELY(condition) (condition)
#endif

int _trace_assert_failed(const char *file, int line, const char *function, const char *expression,
                         const char *reason_format, ...) __attribute__((format(printf, 5, 6)));

#define ASSERT(expression, action, ...)                                                   \
    do {                                                                                  \
        if (UNLIKELY((expression) == false)) {                                            \
            _trace_assert_failed(__FILE__, __LINE__, __func__, #expression, __VA_ARGS__); \
            { action; }                                                                   \
        }                                                                                 \
    } while (0);

#endif  // ASSERTX_H
