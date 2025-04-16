#pragma once

#include "core/buffer.h"
#include "core/thread_sync.h"
#include "network/multicast.h"
#include "common/types.h"
#include <atomic>
#include <memory>
#include <functional>

namespace stream_buffer
{
    namespace core
    {

        /**
         * @brief Message processor interface for processing buffered messages
         */
        class IMessageProcessor
        {
        public:
            virtual ~IMessageProcessor() = default;
            virtual bool ProcessMessage(char *data, size_t length) = 0;
        };

        /**
         * @brief Buffer processing class with dependency injection
         */
        class BufferProcessor
        {
        public:
            /**
             * @brief Constructor with dependency injection
             *
             * @param config Network configuration
             * @param buffer_size Buffer size in bytes
             * @param network_receiver Network receiver implementation
             * @param message_processor Message processor implementation
             */
            BufferProcessor(
                const common::MulticastConfig &config,
                size_t buffer_size = common::constants::DEFAULT_BUFFER_SIZE * common::constants::MEGA_BYTE,
                std::unique_ptr<network::INetworkReceiver> network_receiver = nullptr,
                std::unique_ptr<IMessageProcessor> message_processor = nullptr);

            /**
             * @brief Destructor
             */
            ~BufferProcessor();

            // Prevent copying
            BufferProcessor(const BufferProcessor &) = delete;
            BufferProcessor &operator=(const BufferProcessor &) = delete;

            /**
             * @brief Run the buffer processor
             */
            void Run();

            /**
             * @brief Stop the buffer processor
             */
            void Stop();

        private:
            // Thread functions
            static void *ReceiveThreadFunction(void *arg);
            static void *ProcessThreadFunction(void *arg);

            // Thread management
            void StartThreads();
            void JoinThreads();

            // Member variables
            std::unique_ptr<Buffer> buffer_;
            std::unique_ptr<ThreadSync> sync_;
            std::unique_ptr<network::INetworkReceiver> network_receiver_;
            std::unique_ptr<IMessageProcessor> message_processor_;

            common::MulticastConfig config_;
            pthread_t receive_thread_id_;
            pthread_t process_thread_id_;
            std::atomic<bool> running_{false};
            int socket_id_{-1};
        };

    } // namespace core
} // namespace stream_buffer