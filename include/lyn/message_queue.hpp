#pragma once

#include "lyn/thread.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>

namespace lyn {
namespace mq {
struct message_queue_exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

template<class C>
class message_queue {
public:
    using value_type = C;
    using queue_t = std::queue<C>;

    message_queue() : m_cv(), m_mtx(), m_queue(), m_alive(true) {}
    message_queue(const message_queue&) = delete; // no copies
    message_queue(message_queue&& rhs) = default;
    message_queue& operator=(const message_queue&) = delete; // no copies
    message_queue& operator=(message_queue&& rhs) = default;
    virtual ~message_queue() { shutdown(); }

    inline typename queue_t::size_type size() const { return m_queue.size(); }
    void shutdown() {
        if(m_alive) {
            m_alive = false;
            m_cv.notify_all();
        }
    }
    void push(const C& msg) {
        if(!m_alive) throw message_queue_exception(std::string("message_queue::push shutdown"));
        lyn::thread::guard_then_notify_using<lyn::thread::notifier_of_one>(m_mtx, m_cv, [&]
        {
            m_queue.push(msg);
        });
    }
    void push(C&& msg) {
        if(!m_alive) throw message_queue_exception(std::string("message_queue::push shutdown"));
        lyn::thread::guard_then_notify_using<lyn::thread::notifier_of_one>(m_mtx, m_cv, [&]
        {
            m_queue.push(std::move(msg));
        });
    }
    template<class... Args>
    void emplace(Args&&... args) {
        if(!m_alive) throw message_queue_exception(std::string("message_queue::emplace shutdown"));
        lyn::thread::guard_then_notify_using<lyn::thread::notifier_of_one>(m_mtx, m_cv, [&]
        {
            m_queue.emplace(std::forward<Args>(args)...);
        });
    }
    auto pop() { // blocking pop
        std::unique_lock<std::mutex> lock(m_mtx);
        while(m_alive && m_queue.empty()) m_cv.wait(lock);
        if(!m_alive) throw message_queue_exception(std::string("message_queue::pop shutdown"));
        auto msg = std::move(m_queue.front());
        m_queue.pop();
        return msg;
    }
    bool pop(C& fill) { // polling pop
        if(!m_alive) throw message_queue_exception(std::string("message_queue::pop shutdown"));
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
        if(!m_alive) throw message_queue_exception(std::string("message_queue::pop_all shutdown"));
        replacement.swap(m_queue);
        return replacement;
    }
    bool pop_all(queue_t& fill) { // getting the whole queue, polling
        if(!m_alive) throw message_queue_exception(std::string("message_queue::pop_all shutdown"));
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
} // namespace mq
} // namespace lyn
