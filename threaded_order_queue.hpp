// threaded_order_queue.hpp
// -----------------------------
// A thread-safe queue for order submission.
// Uses mutex + condition_variable for producer/consumer behavior.
// Used to decouple input and matching threads.
// -----------------------------

#ifndef THREADED_ORDER_QUEUE_HPP
#define THREADED_ORDER_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class ThreadedQueue {
public:
    // Push a new item into the queue (called by producer thread)
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (shutdown_) throw std::runtime_error("queue is shut down");
        queue_.push(item);
        cv_.notify_one();
    }

    // Pop an item from the queue (blocks if empty)
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]() { return !queue_.empty() || shutdown_; });

        if (shutdown_ && queue_.empty())
            return std::nullopt; // Shutdown signal and nothing left

        T item = queue_.front();
        queue_.pop();
        return item;
    }

    // Signal shutdown — wakes up any waiting threads
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }

private:
    std::queue<T> queue_;              // Underlying data container
    std::mutex mutex_;                 // Guards queue access
    std::condition_variable cv_;       // Signals when new items are available
    bool shutdown_ = false;            // Set to true when shutting down
};

#endif // THREADED_ORDER_QUEUE_HPP
