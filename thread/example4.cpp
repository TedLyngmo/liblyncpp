#include "lyn/thread.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

// event example

lyn::thread::event<false> ev; // false = manual reset
int state = 0;

void a_thread() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    bool executed = ev.wait_for(std::chrono::milliseconds(100), []{
        std::cout << "second true " << ++state << '\n';
    });

    if(not executed) {
        ev.wait([]{
            std::cout << "second false\n";
        });
    }
}

int main() {
    std::vector<std::thread> threads(128);
    for(auto& th : threads) th = std::thread(a_thread);

    ev.set([]{ std::cout << "RUN\n"; });

    std::this_thread::sleep_for(std::chrono::milliseconds(9));

    ev.reset([]{ std::cout << "STOP\n"; });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ev.set([]{ std::cout << "RESTORE\n"; });

    for(auto& th : threads) th.join();
    std::cout << "DONE " << state << '\n';
}
