#include "utils/debug.h"
#include <iomanip>
#include <iostream>
#include <limits>

namespace stream_buffer
{
    namespace utils
    {

        // Optimized BCD data to integer conversion
        long long decode_bcd(const void *data, size_t length)
        {
            if (!data || length == 0)
            {
                return -1LL;
            }

            // Check for potential overflow - each byte can represent at most 2 decimal digits
            // long long can represent up to 19 decimal digits (2^63-1 is approximately 9.2*10^18)
            if (length > 9)
            { // 9 bytes = 18 digits max
                FMT_PRINT("BCD data too large, potential overflow: %zu bytes\n", length);
                return -1LL;
            }

            const uint8_t *bytes = static_cast<const uint8_t *>(data);
            long long result = 0;
            constexpr long long max_value = std::numeric_limits<long long>::max();
            constexpr long long max_div_100 = max_value / 100;

            for (size_t i = 0; i < length; ++i)
            {
                uint8_t byte = bytes[i];
                uint8_t high = (byte >> 4) & 0x0F;
                uint8_t low = byte & 0x0F;

                // Validate each nibble is a valid BCD digit (0-9)
                if (high > 9 || low > 9)
                {
                    FMT_PRINT("Invalid BCD byte: 0x%02X\n", byte);
                    return -1LL;
                }

                // Check for potential overflow before multiplying by 100
                if (result > max_div_100)
                {
                    FMT_PRINT("BCD value too large, overflow detected\n");
                    return -1LL;
                }

                result = result * 100 + high * 10 + low;
            }

            return result;
        }

        // Print hex dump of binary data with optimized formatting
        void hex_dump(const void *data, size_t size)
        {
            if (!data || size == 0)
            {
                return;
            }

            const uint8_t *bytes = static_cast<const uint8_t *>(data);
            std::cout << "Hex dump of " << size << " bytes:" << std::endl;

            char ascii[17] = {0};

            for (size_t i = 0; i < size; ++i)
            {
                // Print offset at beginning of line
                if (i % 16 == 0)
                {
                    // Print ASCII representation of previous line
                    if (i > 0)
                    {
                        std::cout << "  " << ascii << std::endl;
                    }

                    // Print new line offset
                    std::cout << std::setw(4) << std::setfill('0') << std::hex << i << ": ";
                }

                // Print hex byte
                std::cout << std::setw(2) << std::setfill('0') << std::hex
                          << static_cast<int>(bytes[i]) << " ";

                // Store ASCII representation
                ascii[i % 16] = (bytes[i] >= 32 && bytes[i] <= 126) ? bytes[i] : '.';
                ascii[(i % 16) + 1] = 0;

                // Add extra space at halfway point
                if ((i + 1) % 8 == 0 && (i + 1) % 16 != 0)
                {
                    std::cout << " ";
                }
            }

            // Print padding spaces for last line if needed
            size_t remaining = size % 16;
            if (remaining != 0)
            {
                // Calculate padding needed
                for (size_t i = 0; i < (16 - remaining) * 3 + (remaining <= 8 ? 1 : 0); ++i)
                {
                    std::cout << " ";
                }

                // Print ASCII representation of last line
                std::cout << "  " << ascii << std::endl;
            }

            std::cout << std::dec; // Reset to decimal mode
        }

    } // namespace utils
} // namespace stream_buffer