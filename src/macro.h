#pragma once
#include <string.h>
#include <assert.h>
#include "sake.h"

#define SAKE_ASSERT(x) \
    if (!(x)) { \
        SAKE_LOG_ERROR(SAKE_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << sake::util::BacktraceToString(100, 2, "   "); \
        assert(x); \
    }

#define SAKE_ASSERT2(x, w) \
    if (!(x)) { \
        SAKE_LOG_ERROR(SAKE_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << sake::util::BacktraceToString(100, 2, "   "); \
        assert(x); \
    }
