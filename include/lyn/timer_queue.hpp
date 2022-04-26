/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
*/

// Original: https://github.com/TedLyngmo/liblyncpp

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

namespace lyn {
namespace mq {
template<class EventType = std::function<void()>, class Clock = std::chrono::steady_clock,
         class TimePoint = std::chrono::time_point<Clock>>
class timer_queue {
public:
    using event_type = EventType;
    using clock_type = Clock;
    using time_point = TimePoint;

private:
    struct TimedEvent {
        bool operator<(const TimedEvent& rhs) const { return rhs.StartTime < StartTime; }
        time_point StartTime;
        event_type m_event;
    };

public:
    using queue_type = std::priority_queue<TimedEvent>;

    ~timer_queue() { shutdown(); }

    void shutdown() {
        m_shutdown = true;
        m_cv.notify_all();
    }

    // add a new Event to the queue
    template<class... Args>
    void emplace_do_now(Args&&... args) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_queue.emplace(TimedEvent{m_seq, std::forward<Args>(args)...});
        m_seq += std::chrono::nanoseconds(1);
        m_cv.notify_all();
    }

    template<class TP, class... Args>
    void emplace_do_at(TP&& tp, Args&&... args) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_queue.emplace(TimedEvent{std::forward<TP>(tp), std::forward<Args>(args)...});
        m_cv.notify_all();
    }

    bool wait_pop(event_type& ev) {
        std::unique_lock<std::mutex> lock(m_mutex);

        while(not m_shutdown && (m_queue.empty() || clock_type::now() < m_queue.top().StartTime)) {
            if(m_queue.empty()) {
                m_cv.wait(lock);
            } else {
                auto st = m_queue.top().StartTime;
                m_cv.wait_until(lock, st);
            }
        }
        if(m_shutdown) return false; // time to quit

        ev = std::move(m_queue.top().m_event); // extract event
        m_queue.pop();

        return true;
    }

private:
    queue_type m_queue{};
    mutable std::mutex m_mutex{};
    std::condition_variable m_cv{};
    std::atomic<bool> m_shutdown{};
    // m_seq is used for to make sure events are executed in the order they are put in the queue which can be used to
    // extend the queue with adding a bunch of elements at the same time and to guarantee them to be extracted together
    // in the exact order they were put in.
    time_point m_seq{};
};
} // namespace mq
} // namespace lyn
