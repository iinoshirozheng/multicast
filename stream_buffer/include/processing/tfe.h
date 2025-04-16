#pragma once

#include "utils/debug.h"
#include <cstdint>
#include <cstring>
#include <string>

namespace stream_buffer
{
    namespace processing
    {
        namespace tfe
        {

            // Packet constants
            constexpr uint8_t ESC_CODE = 0x1B;       // Escape code marker (ASCII ESC)
            constexpr uint8_t CHECKSUM_CODE = 0x0D;  // Checksum value (typically CR)
            constexpr uint8_t TERMINAL_CODE = 0x0A;  // Terminal code (typically LF)
            constexpr size_t CHECK_SUM_SIZE = 1;     // Checksum size in bytes
            constexpr size_t TERMINAL_CODE_SIZE = 2; // Terminal code size in bytes
            constexpr size_t MAX_BODY_SIZE = 4096;   // Maximum allowed body size

#pragma pack(push, 1) // Disable alignment for accurate packet structure

            /**
             * @brief TFE Protocol Header Structure
             *
             * The header contains information about the message type,
             * timing, sequence, and body length.
             */
            struct Header
            {
                char esc_code;               // X(1)   -> 1 byte, ASCII 27 (ESC)
                char transmission_code;      // X(1)   -> "1" futures, "4" options
                char message_kind;           // X(1)   -> "1" data message
                uint8_t information_time[6]; // 9(12)  -> 6 bytes BCD (hhmmssmmmuuu)
                uint8_t information_seq[4];  // 9(8)   -> 4 bytes BCD
                uint8_t version_no;          // 9(2)   -> 1 byte BCD
                uint8_t body_length[2];      // 9(4)   -> 2 bytes BCD

                /**
                 * @brief Print header information for debugging
                 */
                void Print() const
                {
                    FMT_PRINT("TFE Header:\n");
                    FMT_PRINT("  ESC Code: 0x%02X\n", static_cast<uint8_t>(esc_code));
                    FMT_PRINT("  Trans Code: %c\n", transmission_code);
                    FMT_PRINT("  Message Kind: %c\n", message_kind);
                    FMT_PRINT("  Info Time: %lld\n", utils::decode_bcd(information_time, sizeof(information_time)));
                    FMT_PRINT("  Info Seq: %lld\n", utils::decode_bcd(information_seq, sizeof(information_seq)));
                    FMT_PRINT("  Version No: %u\n", static_cast<unsigned>(utils::decode_bcd(&version_no, sizeof(version_no))));
                    FMT_PRINT("  Body Length: %u\n", static_cast<unsigned>(utils::decode_bcd(body_length, sizeof(body_length))));
                }

                /**
                 * @brief Validate header fields
                 * @return true if header is valid, false otherwise
                 */
                bool IsValid() const
                {
                    // Check escape code
                    if (static_cast<uint8_t>(esc_code) != ESC_CODE)
                    {
                        FMT_PRINT("Invalid escape code: 0x%02X\n", static_cast<uint8_t>(esc_code));
                        return false;
                    }

                    // Check transmission code
                    // if (transmission_code != '1' && transmission_code != '4')
                    // {
                    //     FMT_PRINT("Invalid transmission code: %c\n", transmission_code);
                    //     return false;
                    // }

                    // // Check message kind
                    // if (message_kind != '1')
                    // {
                    //     FMT_PRINT("Invalid message kind: %c\n", message_kind);
                    //     return false;
                    // }

                    // // Check body length
                    long long body_size = utils::decode_bcd(body_length, sizeof(body_length));
                    if (body_size == -1LL || body_size > static_cast<long long>(MAX_BODY_SIZE))
                    {
                        FMT_PRINT("Invalid body length: %lld\n",
                                  body_size == -1LL ? 0LL : body_size);
                        return false;
                    }

                    return true;
                }

                /**
                 * @brief Get decoded body length
                 * @return Decoded body length or 0 on error
                 */
                uint32_t GetBodyLength() const
                {
                    long long length = utils::decode_bcd(body_length, sizeof(body_length));
                    if (length == -1LL)
                    {
                        return 0;
                    }
                    return static_cast<uint32_t>(length);
                }
            };

            /**
             * @brief Futures product structure (I010)
             *
             * Contains information about a futures product.
             */
            struct BodyI010
            {
                char prod_id_s[10];                   // X(10) product code
                uint8_t reference_price[5];           // 9(9)  -> 5 bytes BCD
                char prod_kind;                       // X(1)
                uint8_t decimal_locator;              // 9(1)
                uint8_t strike_price_decimal_locator; // 9(1)
                uint8_t begin_date[4];                // 9(8) -> 4 bytes BCD
                uint8_t end_date[4];                  // 9(8)
                uint8_t flow_group;                   // 9(2) -> 1 byte BCD
                uint8_t delivery_date[4];             // 9(8)
                char dynamic_banding;                 // X(1)

                /**
                 * @brief Print body information for debugging
                 */
                void Print() const
                {
                    FMT_PRINT("\nBodyI010 Size: %zu\n", sizeof(BodyI010));

                    // Create null-terminated copy of product ID
                    char prod_id_copy[11] = {0};
                    std::memcpy(prod_id_copy, prod_id_s, sizeof(prod_id_s));
                    FMT_PRINT("Product ID: %s\n", prod_id_copy);

                    FMT_PRINT("Reference Price: %lld\n", utils::decode_bcd(reference_price, sizeof(reference_price)));
                    FMT_PRINT("Product Kind: %c\n", prod_kind);
                    FMT_PRINT("Decimal Locator: %u\n", static_cast<unsigned>(utils::decode_bcd(&decimal_locator, sizeof(decimal_locator))));
                    FMT_PRINT("Strike Price Decimal Locator: %u\n",
                              static_cast<unsigned>(utils::decode_bcd(&strike_price_decimal_locator, sizeof(strike_price_decimal_locator))));
                    FMT_PRINT("Begin Date: %lld\n", utils::decode_bcd(begin_date, sizeof(begin_date)));
                    FMT_PRINT("End Date: %lld\n", utils::decode_bcd(end_date, sizeof(end_date)));
                    FMT_PRINT("Flow Group: %u\n", static_cast<unsigned>(utils::decode_bcd(&flow_group, sizeof(flow_group))));
                    FMT_PRINT("Delivery Date: %lld\n", utils::decode_bcd(delivery_date, sizeof(delivery_date)));
                    FMT_PRINT("Dynamic Banding: %c\n", dynamic_banding);
                }

                /**
                 * @brief Get product ID as a string
                 * @return Product ID string
                 */
                std::string GetProductId() const
                {
                    return std::string(prod_id_s, sizeof(prod_id_s));
                }

                /**
                 * @brief Validate body fields
                 * @return true if body is valid, false otherwise
                 */
                bool IsValid() const
                {
                    // Check reference price
                    if (utils::decode_bcd(reference_price, sizeof(reference_price)) == -1LL)
                    {
                        return false;
                    }

                    // Check dates
                    if (utils::decode_bcd(begin_date, sizeof(begin_date)) == -1LL ||
                        utils::decode_bcd(end_date, sizeof(end_date)) == -1LL ||
                        utils::decode_bcd(delivery_date, sizeof(delivery_date)) == -1LL)
                    {
                        return false;
                    }

                    return true;
                }
            };

#pragma pack(pop)

            /**
             * @brief Calculate packet total size from body size
             * @param body_size Size of packet body
             * @return Total packet size including header, body, checksum and terminal code
             */
            inline size_t CalculatePacketSize(size_t body_size)
            {
                return sizeof(Header) + body_size + CHECK_SUM_SIZE + TERMINAL_CODE_SIZE;
            }

            /**
             * @brief Validate packet checksum
             * @param data Packet data
             * @param length Packet length excluding terminal code
             * @return true if checksum is valid, false otherwise
             */
            inline bool ValidateChecksum(const char *data, size_t length)
            {
                if (!data || length < sizeof(Header) + CHECK_SUM_SIZE)
                {
                    return false;
                }

                // Simple checksum validation - in a real implementation this would be more complex
                uint8_t checksum = data[length - 1];
                return checksum == CHECKSUM_CODE;
            }

            // Ensure structures have the expected sizes
            static_assert(sizeof(Header) == 16, "TFE::Header struct size mismatch!");
            static_assert(sizeof(BodyI010) == 32, "TFE::BodyI010 struct size mismatch!");

        } // namespace tfe
    } // namespace processing
} // namespace stream_buffer