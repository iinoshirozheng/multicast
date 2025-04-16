#pragma once

#include <pthread.h>

namespace stream_buffer
{
    namespace core
    {

        /**
         * Thread-safe synchronization class for thread coordination
         */
        class ThreadSync
        {
        public:
            ThreadSync();
            ~ThreadSync();

            // Prevent copying
            ThreadSync(const ThreadSync &) = delete;
            ThreadSync &operator=(const ThreadSync &) = delete;

            // Thread synchronization operations
            void Lock();
            void Unlock();
            void Signal();
            void Wait();

        private:
            pthread_mutex_t mutex_;
            pthread_cond_t condition_;
        };

        /**
         * RAII-style lock guard
         */
        class ScopedLock
        {
        public:
            explicit ScopedLock(ThreadSync &sync);
            ~ScopedLock();

            // Prevent copying
            ScopedLock(const ScopedLock &) = delete;
            ScopedLock &operator=(const ScopedLock &) = delete;

        private:
            ThreadSync &sync_;
        };

    } // namespace core
} // namespace stream_buffer