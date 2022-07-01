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
#include <future>
#include <mutex>
#include <queue>
#include <type_traits>
#include <utility>

namespace lyn {
namespace mq {

    template<class EventType = std::function<void()>, class Clock = std::chrono::steady_clock,
             class TimePoint = std::chrono::time_point<Clock>>
    class timer_queue {
    public:
        using event_type = EventType;
        using clock_type = Clock;
        using duration = typename Clock::duration;
        using time_point = TimePoint;

    private:
        struct TimedEvent {
            bool operator<(const TimedEvent& rhs) const { return rhs.StartTime < StartTime; }
            time_point StartTime;
            event_type m_event;
        };

    public:
        struct queue_type : std::priority_queue<TimedEvent> {
            using std::priority_queue<TimedEvent>::pop;

            bool pop(event_type& ev) {
                if(this->empty()) return false;
                ev = std::move(this->top().m_event); // extract event
                this->pop();
                return true;
            }
        };

        explicit timer_queue(duration now_delay) : m_now_delay(std::move(now_delay)) {}

        timer_queue() : timer_queue(std::chrono::nanoseconds(0)) {}
        timer_queue(const timer_queue&) = delete;
        timer_queue(timer_queue&&) = delete;
        timer_queue& operator=(const timer_queue&) = delete;
        timer_queue& operator=(timer_queue&&) = delete;
        ~timer_queue() { shutdown(); }

        void shutdown() {
            m_shutdown = true;
            m_cv.notify_all();
        }
        void clear() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue = queue_type{};
        }
        void restart() { m_shutdown = false; }

        bool is_open() const { return !m_shutdown; }
        bool operator!() const { return m_shutdown; }
        explicit operator bool() const { return !m_shutdown; }

        // synchronize requires EventType to return `void` and be capable of captures.
        template<class R, class Func>
        R synchronize(Func&& func) {
            std::promise<R> p;
            std::future<R> f = p.get_future();

            if constexpr(std::is_same_v<void, R>) {
                emplace_do_urgently([&p, func = std::forward<Func>(func)](auto&&... args) {
                    func(std::forward<decltype(args)>(args)...);
                    p.set_value();
                });

                f.wait();
            } else {
                emplace_do_urgently([&p, func = std::forward<Func>(func)](auto&&... args) {
                    p.set_value(func(std::forward<decltype(args)>(args)...));
                });

                f.wait();
                return f.get();
            }
        }

        template<class... Args>
        void emplace_do_in(duration dur, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_queue.emplace(TimedEvent{clock_type::now() + dur, std::forward<Args>(args)...});
            m_cv.notify_all();
        }

        template<class TP, class... Args>
        void emplace_do_at(TP&& tp, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_mutex);

            m_queue.emplace(TimedEvent{std::forward<TP>(tp), std::forward<Args>(args)...});
            m_seq += std::chrono::nanoseconds(1);
            m_cv.notify_all();
        }

        template<class... Args>
        void emplace_do(Args&&... args) {
            emplace_do_in(m_now_delay, std::forward<Args>(args)...);
        }

        template<class... Args>
        void emplace_do_urgently(Args&&... args) {
            emplace_do_at(m_seq, std::forward<Args>(args)...);
        }

        bool wait_pop(event_type& ev) {
            std::unique_lock<std::mutex> lock(m_mutex);

            while((m_queue.empty() || clock_type::now() < m_queue.top().StartTime) && not m_shutdown) {
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

        bool wait_pop_all(queue_type& in_out) {
            in_out = queue_type{}; // make sure it's empty
            std::unique_lock<std::mutex> lock(m_mutex);

            while((m_queue.empty() || clock_type::now() < m_queue.top().StartTime) && not m_shutdown) {
                if(m_queue.empty()) {
                    m_cv.wait(lock);
                } else {
                    auto st = m_queue.top().StartTime;
                    m_cv.wait_until(lock, st);
                }
            }
            if(m_shutdown) return false;

            auto now = clock_type::now();
            while(!m_queue.empty() && now >= m_queue.top().StartTime) {
                in_out.emplace(std::move(m_queue.top()));
                m_queue.pop();
            }

            return true;
        }

    protected:
        // These methods violate the timing aspect and extracts queued events including those that expires in the
        // future. One possible use-case is when writing tests that don't care about the timing.
        bool wait_pop_future(event_type& ev) {
            std::unique_lock<std::mutex> lock(m_mutex);

            while(m_queue.empty() && not m_shutdown) {
                m_cv.wait(lock);
            }
            if(m_shutdown) return false; // time to quit

            ev = std::move(m_queue.top().m_event); // extract event
            m_queue.pop();

            return true;
        }

        bool wait_pop_all_future(queue_type& in_out) {
            in_out = queue_type{}; // make sure it's empty
            std::unique_lock<std::mutex> lock(m_mutex);

            while(m_queue.empty() && not m_shutdown) {
                m_cv.wait(lock);
            }
            if(m_shutdown) return false;

            std::swap(in_out, m_queue);

            return true;
        }

        duration get_now_delay() const { return m_now_delay; }
        time_point get_seq() const { return m_seq; }

    private:
        queue_type m_queue{};
        mutable std::mutex m_mutex{};
        std::condition_variable m_cv{};
        std::atomic<bool> m_shutdown{};
        duration m_now_delay;
        // m_seq is used for to make sure events are executed in the order they are put in the queue which can be used
        // to extend the queue with adding a bunch of elements at the same time and to guarantee them to be extracted
        // together in the exact order they were put in.
        time_point m_seq{};
    };
} // namespace mq
} // namespace lyn
