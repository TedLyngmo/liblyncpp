#include "lyn/thread.hpp"

#include <iostream>
#include <thread>

// event example

lyn::thread::event<true> ev; // true = auto reset
int state = 0;

void a_thread() {
    // waits for ev to be signaled
    // runs the lambda
    // sets ev to non-signaled
    ev.wait_then([]{
        std::cout << "second: " << ++state << '\n';
    });

    ev.wait_then([]{
        std::cout << "fourth: " << ++state << '\n';
    });

    // unguarded work

}

int main() {
    auto th = std::thread(a_thread);

    // runs the lambda
    // sets ev to signaled
    ev.do_then_set([]{
        std::cout << "first " << ++state << '\n';
    });

    ev.wait_for_reset_then([]{
        std::cout << "reset\n";
    });

    ev.do_then_set([]{
        std::cout << "third " << ++state << '\n';
    });

    th.join();
}
