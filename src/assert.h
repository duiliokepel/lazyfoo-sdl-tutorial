#ifndef ASSERTX_H
#define ASSERTX_H

/**
 * ASSERT subsystem
 *
 * Goal: keep the happy path readable while still doing defensive checks.
 *
 * Usage:
 *     ASSERT(condition, "reason") { cleanup(); return -1; }
 *
 * Behavior:
 * - On failure: logs context (file/line/function/expression + message), then
 * runs the caller-provided action block.
 * - Failure is expected to be rare, so the branch is marked unlikely.
 * - Can be used with else:
 *     ASSERT(condition, "reason") { ... } else { ... }
 * - Sets g_assert_errno with errno for usage with strerror() or similar.
 */

#include <errno.h>
#include <stdbool.h>

#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(condition) __builtin_expect(!!(condition), 1)
    #define UNLIKELY(condition) __builtin_expect(!!(condition), 0)
#else
    #define LIKELY(condition) (condition)
    #define UNLIKELY(condition) (condition)
#endif

extern _Thread_local int g_assert_errno;

int _trace_assert_failed(const char *file, int line, const char *function,
                         const char *expression, const char *reason_format, ...)
    __attribute__((format(printf, 5, 6)));

int _trace_loopbound_exceeded(const char *file, int line, const char *function,
                              const char *expression, const char *reason_format,
                              ...) __attribute__((format(printf, 5, 6)));

static inline int _assert_loopbound(const char *file, int line,
                                    const char *function,
                                    const char *expression,
                                    unsigned long *iteration_counter,
                                    const unsigned long max_iterations,
                                    bool *bound_tripped) {
    if (UNLIKELY(*iteration_counter >= max_iterations)) {
        if (bound_tripped) { *bound_tripped = true; }
        _trace_loopbound_exceeded(
            file, line, function, expression,
            "Iteration counter=[%lu] reached max iterations=[%lu]",
            *iteration_counter, max_iterations);
        return 0;
    }
    (*iteration_counter)++;
    return 1;
}

#define ASSERT(expression, ...)                                          \
    if (UNLIKELY(!(expression)) &&                                       \
        (g_assert_errno = errno,                                         \
         _trace_assert_failed(__FILE__, __LINE__, __func__, #expression, \
                              __VA_ARGS__),                              \
         1))

#define LOOPBOUND(condition, iteration_counter, max_iterations, bound_tripped) \
    (_assert_loopbound(__FILE__, __LINE__, __func__, #condition,               \
                       &(iteration_counter), max_iterations,  \
                       bound_tripped) &&                                       \
     (condition))
#endif  // ASSERTX_H
