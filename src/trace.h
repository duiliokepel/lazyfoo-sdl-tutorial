#ifndef TRACE_H
#define TRACE_H

#include <stdarg.h>

void _trace(const char *file, int line, const char *func, const char *format, ...);

#define TRACE(...) _trace(__FILE__, __LINE__, __func__, __VA_ARGS__)

#endif /* TRACE_H */
