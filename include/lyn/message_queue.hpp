#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>

namespace lyn {
struct MessageQueueException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

template<class C>
class MessageQueue {
public:
    using value_type = C;
    using queue_t = std::queue<C>;

    MessageQueue() : m_cv(), m_mtx(), m_queue(), m_alive(true) {}
    MessageQueue(const MessageQueue&) = delete; // no copies
    MessageQueue(MessageQueue&& rhs) = default;
    MessageQueue& operator=(const MessageQueue&) = delete; // no copies
    MessageQueue& operator=(MessageQueue&& rhs) = default;
    virtual ~MessageQueue() { shutdown(); }

    inline typename queue_t::size_type size() const { return m_queue.size(); }
    void shutdown() {
        if(m_alive) {
            m_alive = false;
            m_cv.notify_all();
        }
    }
    void push(const C& msg) {
        if(!m_alive) throw MessageQueueException(std::string("MessageQueue::push shutdown"));
        {
            std::lock_guard<std::mutex> guard(m_mtx);
            m_queue.push(msg);
        }
        m_cv.notify_one();
    }
    void push(C&& msg) {
        if(!m_alive) throw MessageQueueException(std::string("MessageQueue::push shutdown"));
        {
            std::lock_guard<std::mutex> guard(m_mtx);
            m_queue.push(std::move(msg));
        }
        m_cv.notify_one();
    }
    template<class... Args>
    void emplace(Args&&... args) {
        if(!m_alive) throw MessageQueueException(std::string("MessageQueue::emplace shutdown"));
        {
            std::lock_guard<std::mutex> guard(m_mtx);
            m_queue.emplace(std::forward<Args>(args)...);
        }
        m_cv.notify_one();
    }
    auto pop() { // blocking pop
        std::unique_lock<std::mutex> lock(m_mtx);
        while(m_alive && m_queue.empty()) m_cv.wait(lock);
        if(!m_alive) throw MessageQueueException(std::string("MessageQueue::pop shutdown"));
        auto msg = std::move(m_queue.front());
        m_queue.pop();
        return msg;
    }
    bool pop(C& fill) { // polling pop
        if(!m_alive) throw MessageQueueException(std::string("MessageQueue::pop shutdown"));
        std::lock_guard<std::mutex> guard(m_mtx);
        if(m_queue.empty()) return false;
        fill = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
    queue_t pop_all() { // getting the whole queue, blocking
        queue_t replacement;
        std::unique_lock<std::mutex> lock(m_mtx);
        while(m_alive && m_queue.empty()) m_cv.wait(lock);
        if(!m_alive) throw MessageQueueException(std::string("MessageQueue::pop_all shutdown"));
        replacement.swap(m_queue);
        return replacement;
    }
    bool pop_all(queue_t& fill) { // getting the whole queue, polling
        if(!m_alive) throw MessageQueueException(std::string("MessageQueue::pop_all shutdown"));
        std::lock_guard<std::mutex> guard(m_mtx);
        if(m_queue.empty()) return false;
        fill.swap(m_queue);
        return true;
    }

private:
    std::condition_variable m_cv;
    mutable std::mutex m_mtx;
    queue_t m_queue;
    std::atomic<bool> m_alive;
};
} // namespace lyn
