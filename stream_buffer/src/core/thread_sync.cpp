#include "core/thread_sync.h"
#include <stdexcept>

namespace stream_buffer
{
    namespace core
    {

        ThreadSync::ThreadSync()
        {
            if (pthread_mutex_init(&mutex_, nullptr) != 0)
            {
                throw std::runtime_error("Failed to initialize mutex");
            }

            if (pthread_cond_init(&condition_, nullptr) != 0)
            {
                pthread_mutex_destroy(&mutex_);
                throw std::runtime_error("Failed to initialize condition variable");
            }
        }

        ThreadSync::~ThreadSync()
        {
            pthread_mutex_destroy(&mutex_);
            pthread_cond_destroy(&condition_);
        }

        void ThreadSync::Lock()
        {
            pthread_mutex_lock(&mutex_);
        }

        void ThreadSync::Unlock()
        {
            pthread_mutex_unlock(&mutex_);
        }

        void ThreadSync::Signal()
        {
            pthread_cond_signal(&condition_);
        }

        void ThreadSync::Wait()
        {
            pthread_cond_wait(&condition_, &mutex_);
        }

        // ScopedLock implementation
        ScopedLock::ScopedLock(ThreadSync &sync) : sync_(sync)
        {
            sync_.Lock();
        }

        ScopedLock::~ScopedLock()
        {
            sync_.Unlock();
        }

    } // namespace core
} // namespace stream_buffer