#ifndef ASSERTX_H
#define ASSERTX_H

#include <stdbool.h>

#include "trace.h"

#define C_ASSERT(expression) ((expression) ? true : (TRACE("ASSERT FAILED: (%s)", #expression), false))

#endif  // ASSERTX_H
