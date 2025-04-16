#pragma once

#include <cstdio>
#include <cstdint>
#include <string>

// Debug modes
#define DEBUG_MODE 1
#define DEBUG_STRUCT_INFO 1

// Debug printf wrapper macros - only produce output in debug mode
#if defined(DEBUG) || defined(_DEBUG) || defined(DEBUG_MODE)
#define FMT_PRINT(...) ::printf("[%s:%d] ", __FILE__, __LINE__), ::printf(__VA_ARGS__)
#define FMT_PRINTLN(...)                          \
    do                                            \
    {                                             \
        ::printf("[%s:%d] ", __FILE__, __LINE__); \
        ::printf(__VA_ARGS__);                    \
        ::printf("\n");                           \
    } while (0)
#else
#define FMT_PRINT(...)
#define FMT_PRINTLN(...)
#endif

namespace stream_buffer
{
    namespace utils
    {

        // BCD conversion utility
        /**
         * @brief Decode BCD (Binary Coded Decimal) data to an integer
         * @param data Pointer to BCD data
         * @param length Length of the BCD data in bytes
         * @return Decoded numeric value, or -1 if decoding failed
         */
        long long decode_bcd(const void *data, size_t length);

        // Hex dump utility for debugging binary data
        void hex_dump(const void *data, size_t size);

    } // namespace utils
} // namespace stream_buffer