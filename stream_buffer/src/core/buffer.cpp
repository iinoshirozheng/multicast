#include "core/buffer.h"
#include "utils/debug.h"
#include <cstring>

namespace stream_buffer
{
    namespace core
    {

        Buffer::Buffer(size_t buffer_size, std::unique_ptr<IBufferProcessor> processor)
            : capacity_(buffer_size), processor_(std::move(processor))
        {
            InitializeBuffer(buffer_size);
            FMT_PRINT("Buffer Size: %zu\n", capacity_);
        }

        Buffer::~Buffer() = default;

        Buffer::Buffer(Buffer &&other) noexcept
            : top_(other.top_),
              end_(other.end_),
              capacity_(other.capacity_),
              buffer_(std::move(other.buffer_)),
              processor_(std::move(other.processor_))
        {
            other.top_ = 0;
            other.end_ = 0;
            other.capacity_ = 0;
        }

        Buffer &Buffer::operator=(Buffer &&other) noexcept
        {
            if (this != &other)
            {
                top_ = other.top_;
                end_ = other.end_;
                capacity_ = other.capacity_;
                buffer_ = std::move(other.buffer_);
                processor_ = std::move(other.processor_);

                other.top_ = 0;
                other.end_ = 0;
                other.capacity_ = 0;
            }
            return *this;
        }

        size_t Buffer::GetUsedSize() const
        {
            return top_;
        }

        size_t Buffer::GetQueuedSize() const
        {
            return end_ - top_;
        }

        size_t Buffer::GetAvailableSize() const
        {
            return capacity_ - end_;
        }

        size_t Buffer::GetTotalCapacity() const
        {
            return capacity_;
        }

        char *Buffer::GetBufferEndPtr() const
        {
            return &buffer_[end_];
        }

        char *Buffer::GetBufferTopPtr() const
        {
            return &buffer_[top_];
        }

        size_t Buffer::GetBufferTop() const
        {
            return top_;
        }

        size_t Buffer::GetBufferEnd() const
        {
            return end_;
        }

        bool Buffer::IsEmpty() const
        {
            return end_ == top_;
        }

        bool Buffer::ShouldCompact() const
        {
            return GetQueuedSize() < GetUsedSize();
        }

        bool Buffer::HasPendingData() const
        {
            return end_ > top_;
        }

        void Buffer::InitializeBuffer(size_t size)
        {
            top_ = 0;
            end_ = 0;
            buffer_ = std::make_unique<char[]>(size);
        }

        void Buffer::Reset()
        {
            top_ = 0;
            end_ = 0;
        }

        void Buffer::CompactBuffer()
        {
            size_t queued = GetQueuedSize();
            FMT_PRINT("Buffer::CompactBuffer() top=%zu end=%zu queued=%zu\n", top_, end_, queued);

            if (top_ > 0 && queued > 0)
            {
                std::memmove(&buffer_[0], &buffer_[top_], queued);
                end_ = queued;
                top_ = 0;
            }
        }

        void Buffer::AppendData(size_t data_size)
        {
            end_ += data_size;
        }

        void Buffer::RemoveProcessedData(size_t bytes_processed)
        {
            top_ += bytes_processed;
        }

        size_t Buffer::ProcessPendingData()
        {
            if (!processor_)
            {
                return 0;
            }

            return processor_->ProcessMessage(GetBufferTopPtr(), GetQueuedSize());
        }

    } // namespace core
} // namespace stream_buffer