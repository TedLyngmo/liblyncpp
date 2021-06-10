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
    ev.wait([]{
        std::cout << "second: " << ++state << '\n';
    });

    ev.wait([]{
        std::cout << "fourth: " << ++state << '\n';
    });

    // unguarded work

}

int main() {
    auto th = std::thread(a_thread);

    // runs the lambda
    // sets ev to signaled
    ev.set([]{
        std::cout << "first " << ++state << '\n';
    });

    ev.wait_for_reset([]{
        std::cout << "reset\n";
    });

    ev.set([]{
        std::cout << "third " << ++state << '\n';
    });

    th.join();
}
