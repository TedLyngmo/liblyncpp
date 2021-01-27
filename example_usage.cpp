#include "lyn/thread.hpp"

#include <iostream>
#include <thread>

// ping-pong example

std::mutex mtx;
std::condition_variable cv;
int state = 0;

void a_thread() {
    lyn::thread::wait_for_then( mtx, cv, [] { return state == 1; }, [] {
        std::cout << "thread: pong\n";
        return ++state;
    });

    // unguarded work

    lyn::thread::guard_then_notify_using<lyn::thread::notifier_of_one>(mtx, cv, [] {
        std::cout << "thread: ping\n";
        return ++state;
    });
}

int main() {
    auto th = std::jthread(a_thread);

    lyn::thread::guard_then_notify_using<lyn::thread::notifier_of_one>(mtx, cv, [] {
        std::cout << "main: ping\n";
        return ++state;
    });

    // unguarded work

    lyn::thread::wait_for_then( mtx, cv, [] { return state == 3; }, [] {
        std::cout << "main: pong\n";
        return ++state;
    });
}
