#include "core/buffer_processor.h"
#include "utils/debug.h"
#include "processing/tfe_processor.h"
#include <iostream>
#include <cstring>

namespace stream_buffer
{
    namespace core
    {

        // TFE message processor implementation
        class TFEMessageProcessor : public core::IMessageProcessor
        {
        public:
            explicit TFEMessageProcessor(Buffer *buffer) : buffer_(buffer) {}

            bool ProcessMessage(char *data, size_t length) override
            {
                (void)data;   // Unused parameter
                (void)length; // Unused parameter

                size_t processed = buffer_->ProcessPendingData();
                return processed != static_cast<size_t>(common::constants::PROCESS_FAILED);
            }

        private:
            Buffer *buffer_;
        };

        BufferProcessor::BufferProcessor(
            const common::MulticastConfig &config,
            size_t buffer_size,
            std::unique_ptr<network::INetworkReceiver> network_receiver,
            std::unique_ptr<core::IMessageProcessor> message_processor)
            : buffer_(std::make_unique<Buffer>(buffer_size, std::make_unique<processing::TFEProcessor>())),
              sync_(std::make_unique<ThreadSync>()),
              config_(config)
        {
            // Create default implementations if not provided
            if (!network_receiver)
            {
                // Will be initialized in Run() when socket is created
                network_receiver_ = nullptr;
            }
            else
            {
                network_receiver_ = std::move(network_receiver);
            }

            if (!message_processor)
            {
                message_processor_ = std::make_unique<TFEMessageProcessor>(buffer_.get());
            }
            else
            {
                message_processor_ = std::move(message_processor);
            }
        }

        BufferProcessor::~BufferProcessor()
        {
            Stop();
        }

        void BufferProcessor::Run()
        {
            // Create socket
            socket_id_ = network::CreateSocket(config_);

            if (socket_id_ < 0)
            {
                FMT_PRINT("Failed to create socket\n");
                return;
            }

            // Join multicast group
            if (network::JoinMulticastGroup(
                    socket_id_,
                    config_.group_ip,
                    config_.port,
                    config_.interface_name,
                    config_.interface_ip) < 0)
            {

                FMT_PRINT("Failed to join multicast group\n");
                close(socket_id_);
                return;
            }

            // Create network receiver with socket
            network_receiver_ = std::make_unique<network::MulticastReceiver>(socket_id_);

            // Start processing
            running_ = true;
            StartThreads();

            // Wait for user input to stop
            FMT_PRINT("Running... press Enter to exit\n");
            std::cin.get();

            // Clean up
            Stop();
        }

        void BufferProcessor::Stop()
        {
            if (running_)
            {
                running_ = false;
                JoinThreads();
                if (socket_id_ >= 0)
                {
                    close(socket_id_);
                    socket_id_ = -1;
                }
            }
        }

        void BufferProcessor::StartThreads()
        {
            pthread_create(&receive_thread_id_, nullptr, ReceiveThreadFunction, this);
            pthread_create(&process_thread_id_, nullptr, ProcessThreadFunction, this);
        }

        void BufferProcessor::JoinThreads()
        {
            pthread_join(receive_thread_id_, nullptr);
            pthread_join(process_thread_id_, nullptr);
        }

        void *BufferProcessor::ReceiveThreadFunction(void *arg)
        {
            auto *processor = static_cast<BufferProcessor *>(arg);

            while (processor->running_)
            {
                processor->sync_->Lock();

                // Check if buffer needs to be reset or compacted
                if (processor->buffer_->IsEmpty())
                {
                    processor->buffer_->Reset();
                }
                else if (processor->buffer_->ShouldCompact())
                {
                    processor->buffer_->CompactBuffer();
                }

                // Get available space
                size_t avail = processor->buffer_->GetAvailableSize();

                processor->sync_->Unlock();

                // Receive data if space available
                if (avail > 0)
                {
                    int received = processor->network_receiver_->ReceiveData(
                        processor->buffer_->GetBufferEndPtr(), avail);

                    if (received > 0)
                    {
                        processor->sync_->Lock();
                        processor->buffer_->AppendData(received);
                        processor->sync_->Signal();
                        processor->sync_->Unlock();
                    }
                    else if (errno == EINTR || errno == EAGAIN)
                    {
                        // Handle temporary errors with select
                        timeval timeout = {.tv_sec = 1, .tv_usec = 0};
                        fd_set read_set;
                        FD_ZERO(&read_set);
                        FD_SET(processor->socket_id_, &read_set);
                        select(processor->socket_id_ + 1, &read_set, nullptr, nullptr, &timeout);
                        continue;
                    }
                    else if (received < 0)
                    {
                        // Fatal error
                        FMT_PRINT("Socket error: %s\n", strerror(errno));
                        processor->running_ = false;
                        break;
                    }
                }
            }

            return nullptr;
        }

        void *BufferProcessor::ProcessThreadFunction(void *arg)
        {
            auto *processor = static_cast<BufferProcessor *>(arg);

            while (processor->running_)
            {
                processor->sync_->Lock();

                while (processor->buffer_->HasPendingData() && processor->running_)
                {
                    // Get data to process
                    size_t queued = processor->buffer_->GetQueuedSize();
                    char *data = processor->buffer_->GetBufferTopPtr();

                    // Unlock during processing
                    processor->sync_->Unlock();

                    // Process message
                    bool success = processor->message_processor_->ProcessMessage(data, queued);

                    // Relock for buffer updates
                    processor->sync_->Lock();

                    if (success)
                    {
                        size_t processed = processor->buffer_->GetQueuedSize() - queued;
                        if (processed > 0)
                        {
                            processor->buffer_->RemoveProcessedData(processed);
                            FMT_PRINT("Processed bytes: %zu, Top=%zu, End=%zu, Queued=%zu\n",
                                      processed,
                                      processor->buffer_->GetBufferTop(),
                                      processor->buffer_->GetBufferEnd(),
                                      processor->buffer_->GetQueuedSize());
                        }
                    }
                    else
                    {
                        FMT_PRINT("Processing error\n");
                        processor->running_ = false;
                        break;
                    }
                }

                // Wait for more data if none available
                if (processor->running_)
                {
                    processor->sync_->Wait();
                }

                processor->sync_->Unlock();
            }

            return nullptr;
        }

    } // namespace core
} // namespace stream_buffer