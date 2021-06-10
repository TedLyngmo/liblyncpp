#pragma once

/*
 * Copied from:
 * https://github.com/TedLyngmo/liblyncpp/blob/main/include/lyn/thread.hpp
 * "This is free and unencumbered software released into the public domain."
*/

#include <chrono>
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
    struct notifier_of_none {
        notifier_of_none(std::condition_variable&) {}
    };
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
    namespace detail {
        // partial specializations for manual / automatic reset event types
        template<bool, class T> struct event_impl;

        // manual reset
        template<class T> struct event_impl<false, T> {
            using set_notifier = notifier_of_all; // everyone gets notified when set
            using reset_notifier = notifier_of_none; // there is no reset when waiting

            struct event_state_resetter { // noop in manual reset
                event_state_resetter(T&) {}
            };

            /**
             * \brief Set the state to non-signaled
             *        Notifies all waiting thread that the event is non-signaled
             *
             * \param[in] An optional functor to invoke while the event is locked
             *
             * \return decltype(in)
             */
            template<class Func = void(*)()>
            decltype(auto) reset(Func&& func = []{}) {
                auto This = static_cast<T*>(this);
                set_notifier notifier{This->m_cvmtx.cv};
                std::lock_guard<std::mutex> lock(This->m_cvmtx.mtx);
                class T::event_state_setter evs{*This, false};
                return func();
            }
        };

        // automatic reset
        template<class T> struct event_impl<true, T> {
            using set_notifier = notifier_of_one;
            using reset_notifier = notifier_of_all; // wait_for_reset needs this

            struct event_state_resetter {
                ~event_state_resetter() { ev.m_state = false; }
                T& ev;
            };
        };
    } // namespace detail

    template<bool AutoReset>
    class event : public detail::event_impl<AutoReset, event<AutoReset>> {
        using impl = detail::event_impl<AutoReset, event<AutoReset>>;
        friend impl;
    public:
        // g++ < 11.1 complains "does not declare anything" unless typename
        //            is used instead of "using" below.
        using set_notifier = typename impl::set_notifier;
        using reset_notifier = typename impl::reset_notifier;
        using event_state_resetter = typename impl::event_state_resetter;

        /**
         * \brief Set the state to signaled
         *
         * \param[in] An optional functor to invoke while the event is locked
         *
         * \return decltype(in)
         *
         * event<false> : Notifies all waiting threads that the event is
         *                signaled
         * event<true>  : Notifies one waiting thread that the event is signaled
         */
        template<class Func = void(*)()>
        decltype(auto) set(Func&& func = []{}) {
            set_notifier notifier{m_cvmtx.cv};
            std::lock_guard<std::mutex> lock(m_cvmtx.mtx);
            event_state_setter evs{*this};
            return func();
        }

        /**
         * \brief Wait for the state to be signaled
         *
         * \param[in] An optional functor to invoke after the event is
         *            signaled and while the event is locked
         *
         * \return decltype(in)
         *
         * event<false> : Does not set the state to unsignaled and does
         *                not notify other threads waiting on the event.
         * event<true>  : Sets the state to signaled after func() has 
         *                been invoked and notifies all.
         */
        template<class Func = void(*)()>
        decltype(auto) wait(Func&& func = []{}) {
            reset_notifier notifier{m_cvmtx.cv};
            std::unique_lock<std::mutex> lock(m_cvmtx.mtx);
            while(not m_state) m_cvmtx.cv.wait(lock);
            event_state_resetter evs{*this};
            return func();
        }

        /**
         * \brief Perform a synchronized operation.
         *        Does not care about the state of the event.
         *
         * \param[in] An functor to invoke while the event is locked.
         *
         * \return decltype(in)
         */
        template<class Func>
        decltype(auto) synchronize(Func&& func) {
            std::lock_guard<std::mutex> lock(m_cvmtx.mtx);
            return func();
        }

        /**
         * \brief Wait for the event to be set to non-signaled
         *
         * \param[in] An optional functor to invoke after the event is
         *
         * \return decltype(in)
         */
        template<class Func = void(*)()>
        decltype(auto) wait_for_reset(Func&& func = []{}) {
            std::unique_lock<std::mutex> lock(m_cvmtx.mtx);
            while(m_state) m_cvmtx.cv.wait(lock);
            return func();
        }

        /**
         * \brief Wait for the event to be signaled until a certain time point
         *
         * \param[in] An optional functor to invoke after the event is signaled
         *
         * \return bool : true:  The event was signaled before timeout_time and
         *                       the optional functor was invoked.
         *                false: Waiting timed out and the functor was *not* invoked.
         */
        template<class Clock, class Duration, class Func = void(*)()>
        bool wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time, Func&& func = []{}) {
            reset_notifier notifier{m_cvmtx.cv};
            std::unique_lock<std::mutex> lock(m_cvmtx.mtx);
            if(m_cvmtx.cv.wait_until(lock, timeout_time, [this]{ return m_state; })) {
                event_state_resetter evs{*this};
                func();
                return true; // m_state
            }
            return false; // m_state
        }

        /**
         * \brief Wait for the event to be signaled for a certain duration
         *
         * \param[in] An optional functor to invoke after the event is signaled
         *
         * \return bool : true:  The event was signaled before rel_time and
         *                       the optional functor was invoked.
         *                false: Waiting timed out and the functor was *not* invoked.
         */
        template<class Rep, class Period, class Func = void(*)()>
        bool wait_for(const std::chrono::duration<Rep, Period>& rel_time, Func&& func = []{}) {
            return wait_until(std::chrono::steady_clock::now() + rel_time, std::forward<Func>(func));
        }

    private:
        struct event_state_setter {
            ~event_state_setter() {
                ev.m_state = state;
            }
            event& ev;
            bool state = true;
        };

        cv_mtx_pair m_cvmtx;
        bool m_state = false;
    };

} // namespace thread
} // namespace lyn
