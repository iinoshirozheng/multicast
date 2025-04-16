#include "processing/tfe_processor.h"
#include "utils/debug.h"
#include "common/types.h"

namespace stream_buffer
{
    namespace processing
    {

        // Process a TFE message from the buffer
        size_t TFEProcessor::ProcessMessage(const char *message, size_t length)
        {
            if (!message || length < sizeof(tfe::Header))
            {
                FMT_PRINT("Invalid message or insufficient data (length: %zu)\n", length);
                return 0;
            }

            const auto *header = reinterpret_cast<const tfe::Header *>(message);

            // Use new header validation function
            if (!header->IsValid())
            {
                FMT_PRINT("Invalid TFE header\n");
                size_t new_header_pos = FindNextHeader(message, length);
                FMT_PRINT("Skipping data, next potential header at offset: %zu\n", new_header_pos);
                return new_header_pos;
            }

            // Get body size using the new GetBodyLength method
            uint32_t body_size = header->GetBodyLength();
            if (body_size == 0)
            {
                FMT_PRINT("Invalid body length in TFE header\n");
                size_t new_header_pos = FindNextHeader(message, length);
                return new_header_pos;
            }

            // Calculate total packet size using the utility function
            size_t total_size = tfe::CalculatePacketSize(body_size);

            // Check if we have enough data for the complete packet
            if (length < total_size)
            {
                FMT_PRINT("Incomplete packet: expected %zu bytes, got %zu\n", total_size, length);
                return 0; // Not enough data yet, wait for more
            }

            // Validate checksum if needed
            if (!tfe::ValidateChecksum(message, total_size - tfe::TERMINAL_CODE_SIZE))
            {
                FMT_PRINT("Invalid checksum\n");
                // Try to recover by finding the next valid header
                size_t new_header_pos = FindNextHeader(message, length);
                return new_header_pos;
            }

            // Print header information
            header->Print();

            // Process specific message types
            if (header->transmission_code == '1' &&
                header->message_kind == '1')
            {
                // Future products I010
                if (body_size >= sizeof(tfe::BodyI010))
                {
                    const auto *body = reinterpret_cast<const tfe::BodyI010 *>(message + sizeof(tfe::Header));

                    // Directly process the body since IsValid() is always true
                    body->Print();

                    // Here you would add more processing logic for the specific message type
                    FMT_PRINT("Processing product: %s\n", body->GetProductId().c_str());
                }
                else
                {
                    FMT_PRINT("Body size too small for I010: %u bytes\n", body_size);
                }
            }
            else
            {
                FMT_PRINT("Unhandled message type: Trans=%c Kind=%c\n",
                          header->transmission_code, header->message_kind);
            }

            // Return total bytes processed
            return total_size;
        }

        // Find the next packet header in the buffer
        size_t TFEProcessor::FindNextHeader(const char *data, size_t length)
        {
            if (!data || length == 0)
            {
                return 0;
            }

            for (size_t i = 0; i < length; ++i)
            {
                if (static_cast<uint8_t>(data[i]) == tfe::ESC_CODE)
                {
                    // Check if we have enough data for a potential header
                    if (i + sizeof(tfe::Header) <= length)
                    {
                        const auto *possible_header = reinterpret_cast<const tfe::Header *>(data + i);
                        // Do a quick validation of key fields
                        if (possible_header->transmission_code == '1' ||
                            possible_header->transmission_code == '4')
                        {
                            FMT_PRINT("Found potential header at offset %zu\n", i);
                            return i;
                        }
                    }
                    else
                    {
                        // Found ESC but not enough data to validate header
                        FMT_PRINT("Found ESC code at offset %zu but not enough data for header\n", i);
                        return i;
                    }
                }
            }

            // No potential header found
            return length;
        }

    } // namespace processing
} // namespace stream_buffer