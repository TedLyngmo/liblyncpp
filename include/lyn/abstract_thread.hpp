#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
namespace lyn {
namespace thread {
    // A base class for thread object wrappers
    class abstract_thread {
    public:
        abstract_thread(const abstract_thread&) = delete;            // no copies
        abstract_thread& operator=(const abstract_thread&) = delete; // no copies

        // Must be implemented and must call terminate_and_join() in the most derived class
        virtual ~abstract_thread() = default;

        virtual void start() {
            if(joinable())
                throw std::runtime_error("thread already running");
            else {
                std::unique_lock<std::mutex> lock(m_mtx);
                m_terminated = true;
                // start thread and wait for it to signal that setup has been done
                m_th = std::thread(&abstract_thread::proxy, this);
                m_cv.wait(lock, [this] { return m_terminated == false; });
            }
        }
        inline bool joinable() const { return m_th.joinable(); }
        inline void join() {
            if(joinable()) m_th.join();
        }
        inline void terminate() { m_terminated = true; }
        inline void terminate_and_join() {
            terminate();
            join();
        }
        inline bool terminated() const { return m_terminated; }

    protected:
        abstract_thread() = default;

        // override if thread specific setup needs to be done before start() returns
        virtual void setup_in_thread() {}
        // must be overridden in derived classes
        virtual void execute() = 0;

    private:
        std::atomic<bool> m_terminated{};
        std::thread m_th{};
        std::condition_variable m_cv{};
        std::mutex m_mtx{};

        void proxy() { // executed in the thread
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                setup_in_thread(); // call setup function
                m_terminated = false;
            }
            m_cv.notify_one();
            execute(); // run thread code in derived class
        }
    };
} // namespace thread
} // namespace lyn
