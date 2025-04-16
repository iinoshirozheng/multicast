#pragma once

#include <cstddef>
#include <memory>
#include "common/types.h"

namespace stream_buffer
{
    namespace core
    {

        // Interface for buffer processing strategies
        class IBufferProcessor
        {
        public:
            virtual ~IBufferProcessor() = default;
            virtual size_t ProcessMessage(const char *message, size_t length) = 0;
        };

        // StreamBuffer class with clear responsibility and improved interface
        class Buffer
        {
        public:
            // Constructor with dependency injection for processor
            explicit Buffer(
                size_t buffer_size = common::constants::DEFAULT_BUFFER_SIZE * common::constants::MEGA_BYTE,
                std::unique_ptr<IBufferProcessor> processor = nullptr);

            ~Buffer();

            // No copy constructor/assignment to prevent resource leaks
            Buffer(const Buffer &) = delete;
            Buffer &operator=(const Buffer &) = delete;

            // Move constructor/assignment for efficient resource transfer
            Buffer(Buffer &&) noexcept;
            Buffer &operator=(Buffer &&) noexcept;

            // Buffer state queries
            [[nodiscard]] size_t GetUsedSize() const;
            [[nodiscard]] size_t GetQueuedSize() const;
            [[nodiscard]] size_t GetAvailableSize() const;
            [[nodiscard]] size_t GetTotalCapacity() const;

            // Buffer pointers for direct access
            [[nodiscard]] char *GetBufferEndPtr() const;
            [[nodiscard]] char *GetBufferTopPtr() const;
            [[nodiscard]] size_t GetBufferTop() const;
            [[nodiscard]] size_t GetBufferEnd() const;

            // State predicates
            [[nodiscard]] bool IsEmpty() const;
            [[nodiscard]] bool ShouldCompact() const;
            [[nodiscard]] bool HasPendingData() const;

            // Buffer operations
            void CompactBuffer();
            void AppendData(size_t data_size);
            void RemoveProcessedData(size_t bytes_processed);
            size_t ProcessPendingData();
            void Reset();

        private:
            size_t top_ = 0;
            size_t end_ = 0;
            size_t capacity_ = 0;

            std::unique_ptr<char[]> buffer_;
            std::unique_ptr<IBufferProcessor> processor_;

            void InitializeBuffer(size_t size);
        };

    } // namespace core
} // namespace stream_buffer