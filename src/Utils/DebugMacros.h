#pragma once

#include <iostream>
#include <cstdlib>
#include <sstream>
#ifdef _MSC_VER
#include <intrin.h>
#endif


#ifdef NDEBUG
#define ASSERT(condition, ...) {}
#else
#define ASSERT(condition, ...) \
        if (!(condition)) { \
            std::ostringstream oss; \
            oss << "Assertion failed: (" << #condition << "), function " << __FUNCTION__ \
                << ", file " << __FILE__ << ", line " << __LINE__ << "."; \
            if constexpr(sizeof(__VA_ARGS__) > 0) { \
                oss << " Message: " << __VA_ARGS__; \
            } \
            std::cerr << oss.str() << std::endl; \
            DebugBreak(); \
            std::abort(); \
        } \
        else \
        { \
        }
#endif


inline void DebugBreak() {
#ifdef _MSC_VER
    __debugbreak();  // For MSVC compiler
#else
    std::abort();  // Fallback for other compilers
#endif
}