#include "lyn/thread.hpp"

#include <chrono>
#include <iostream>
#include <thread>

// event example

lyn::thread::event<true> ev; // true = auto reset
int state = 0;

void a_thread() {
    // waits for ev to be signaled
    // runs the lambda
    // sets ev to non-signaled
    bool executed = ev.wait_for_then_if(std::chrono::milliseconds(5), []{
        ++state;
        std::cout << "second true\n";
    });

    if(not executed) {
        ev.wait_then([]{
            std::cout << "second false\n";
        });
    }

    ev.wait_then([]{
        std::cout << "fourth: " << ++state << '\n';
    });

    // unguarded work

}

int main() {

    for(int i=0; i < 1000; ++i) {
        auto th = std::thread(a_thread);
        // runs the lambda
        // sets ev to signaled
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

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
}
