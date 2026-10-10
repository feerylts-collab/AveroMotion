#pragma once

#include "Core/AveroTypes.h"
#include <cassert>

namespace Avero::Core
{
    // Макросы для ассертов
    // В Debug режиме они проверяют условие и выводят сообщение
    // В Release режиме они отключены для производительности

#ifdef _DEBUG
#define AVERO_ASSERT(condition, message) \
            do { \
                if (!(condition)) { \
                    assert((condition) && (message)); \
                } \
            } while (0)

#define AVERO_ASSERT_MSG(condition, ...) \
            do { \
                if (!(condition)) { \
                    assert((condition)); \
                } \
            } while (0)
#else
#define AVERO_ASSERT(condition, message) ((void)0)
#define AVERO_ASSERT_MSG(condition, ...) ((void)0)
#endif

// Специальные ассерты для разных случаев
#define AVERO_NOT_IMPLEMENTED() AVERO_ASSERT(false, "Not implemented yet")
#define AVERO_UNREACHABLE() AVERO_ASSERT(false, "Unreachable code reached")
#define AVERO_CHECK_NULL(ptr) AVERO_ASSERT((ptr) != nullptr, "Null pointer")
}