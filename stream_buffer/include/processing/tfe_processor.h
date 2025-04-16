#pragma once

#include "core/buffer.h"
#include "processing/tfe.h"
#include <cstdint>

namespace stream_buffer
{
    namespace processing
    {

        // TFE packet processor implementation
        class TFEProcessor : public core::IBufferProcessor
        {
        public:
            TFEProcessor() = default;
            ~TFEProcessor() override = default;

            // Process a TFE message from the buffer
            size_t ProcessMessage(const char *message, size_t length) override;

        private:
            // Find the next packet header in the buffer
            size_t FindNextHeader(const char *data, size_t length);
        };

    } // namespace processing
} // namespace stream_buffer