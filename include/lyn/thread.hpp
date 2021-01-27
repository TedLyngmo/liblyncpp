#include <condition_variable>
#include <mutex>
#include <type_traits>

namespace lyn {
namespace thread {
// ----------------------------------------------------------------------------------
struct notifier_of_one {
    ~notifier_of_one() { cv.notify_one(); }
    std::condition_variable& cv;
};

struct notifier_of_all {
    ~notifier_of_all() { cv.notify_all(); }
    std::condition_variable& cv;
};
// ----------------------------------------------------------------------------------
template<class NotifierType, class Func,
         std::enable_if_t<std::is_invocable_v<Func>, int> = 0>
decltype(auto) guard_then_notify_using(std::mutex& mtx, std::condition_variable& cv,
                                       Func func) {
    NotifierType notifier{cv};
    std::lock_guard<std::mutex> lock(mtx);
    return func();
}
// ----------------------------------------------------------------------------------
template<class Cond, class Func,
         std::enable_if_t<std::is_invocable_v<Cond> && std::is_invocable_v<Func>,
                          int> = 0>
decltype(auto) wait_for_then(std::mutex& mtx, std::condition_variable& cv, Cond cond,
                   Func func) {
    std::unique_lock<std::mutex> ul(mtx);
    cv.wait(ul, cond);
    return func();
}
// ----------------------------------------------------------------------------------
} // namespace thread
} // namespace lyn
