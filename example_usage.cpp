#include "lyn_threads.hpp"

#include <iostream>
#include <thread>

// ping-pong example

std::mutex mtx;
std::condition_variable cv;
int state = 0;

void a_thread() {
    lyn::wait_for_then( mtx, cv, [] { return state == 1; }, [] {
        std::cout << "thread: pong\n";
        return ++state;
    });

    // unguarded work

    lyn::guard_then_notify_using<lyn::notifier_of_one>(mtx, cv, [] {
        std::cout << "thread: ping\n";
        return ++state;
    });
}

int main() {
    auto th = std::jthread(a_thread);

    lyn::guard_then_notify_using<lyn::notifier_of_one>(mtx, cv, [] {
        std::cout << "main: ping\n";
        return ++state;
    });

    // unguarded work

    lyn::wait_for_then( mtx, cv, [] { return state == 3; }, [] {
        std::cout << "main: pong\n";
        return ++state;
    });
}
