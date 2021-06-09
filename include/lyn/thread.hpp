#pragma once

#include <condition_variable>
#include <mutex>
#include <utility>

namespace lyn {
namespace thread {
    // -------------------------------------------------------------------------
    struct cv_mtx_pair {
        std::condition_variable cv;
        std::mutex mtx;
    };
    // -------------------------------------------------------------------------
    struct notifier_of_one {
        ~notifier_of_one() { cv.notify_one(); }
        std::condition_variable& cv;
    };

    struct notifier_of_all {
        ~notifier_of_all() { cv.notify_all(); }
        std::condition_variable& cv;
    };
    // -------------------------------------------------------------------------
    template<class NotifierType, class Func>
    decltype(auto) guard_then_notify_using(cv_mtx_pair& cvmtx, Func&& func) {
        NotifierType notifier{cvmtx.cv};
        std::lock_guard<std::mutex> lock(cvmtx.mtx);
        return func();
    }

    template<class NotifierType, class Func>
    decltype(auto) guard_then_notify_using(std::mutex& mtx, std::condition_variable& cv, Func&& func) {
        NotifierType notifier{cv};
        std::lock_guard<std::mutex> lock(mtx);
        return func();
    }
    // -------------------------------------------------------------------------
    template<class Cond, class Func>
    decltype(auto) wait_for_then(cv_mtx_pair& cvmtx, Cond&& cond, Func&& func) {
        std::unique_lock<std::mutex> lock(cvmtx.mtx);
        cvmtx.cv.wait(lock, std::forward<Cond>(cond));
        return func();
    }

    template<class Cond, class Func>
    decltype(auto) wait_for_then(std::mutex& mtx, std::condition_variable& cv, Cond&& cond, Func&& func) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, std::forward<Cond>(cond));
        return func();
    }
    // -------------------------------------------------------------------------
    template<bool AutoReset = true>
    class event {
    public:
        template<class Func = void(*)()>
        decltype(auto) do_then_set(Func&& func = []{}) {
            notifier_of_one notifier{m_cvmtx.cv};
            std::lock_guard<std::mutex> lock(m_cvmtx.mtx);
            event_state_setter evs{*this, true};
            return func();
        }

        template<class Func = void(*)()>
        decltype(auto) wait_for_signal_then(Func&& func = []{}) {
            notifier_of_one notifier{m_cvmtx.cv};
            std::unique_lock<std::mutex> lock(m_cvmtx.mtx);
            while(not m_state) m_cvmtx.cv.wait(lock);
            if(AutoReset) {
                event_state_setter evs{*this, false};
                return func();
            }
            return func();
        }

        template<class Func = void(*)()>
        decltype(auto) wait_for_reset_then(Func&& func = []{}) {
            std::unique_lock<std::mutex> lock(m_cvmtx.mtx);
            while(m_state) m_cvmtx.cv.wait(lock);
            return func();
        }

    private:
        struct event_state_setter {
            ~event_state_setter() {
                ev.m_state = state;
            }
            event& ev;
            bool state;
        };

        cv_mtx_pair m_cvmtx;
        bool m_state = false;
    };


} // namespace thread
} // namespace lyn
