#include "lyn/thread.hpp"

#include <iostream>
#include <thread>

// ping-pong example

lyn::thread::cv_mtx_pair cvmtx;
int state = 0;

void a_thread() {
    lyn::thread::wait_for_then( cvmtx, [] { return state == 1; }, [] {
        std::cout << "thread: pong\n";
        return ++state;
    });

    // unguarded work

    lyn::thread::guard_then_notify_using<lyn::thread::notifier_of_one>(cvmtx, [] {
        std::cout << "thread: ping\n";
        return ++state;
    });
}

int main() {
    auto th = std::thread(a_thread);

    lyn::thread::guard_then_notify_using<lyn::thread::notifier_of_one>(cvmtx, [] {
        std::cout << "main: ping\n";
        return ++state;
    });

    // unguarded work

    lyn::thread::wait_for_then( cvmtx, [] { return state == 3; }, [] {
        std::cout << "main: pong\n";
        return ++state;
    });

    th.join();
}
