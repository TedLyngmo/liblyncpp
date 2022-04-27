#include "lyn/timer_queue.hpp"

#include <atomic>
#include <iostream>
#include <thread>
using queue_t = lyn::mq::timer_queue<>;

void one(queue_t& q) {
    for(int i = 0; q; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        q.emplace_do([i] { std::cout << "one: " << i << '\n'; });
    }
}

void two(queue_t& q) {
    for(int i = 0; q; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        q.emplace_do([i] { std::cout << "two: " << i << '\n'; });
    }
}

int main() {
    queue_t q(std::chrono::seconds(1));
    auto twoth = std::thread(&two, std::ref(q));
    auto oneth = std::thread(&one, std::ref(q));

    q.emplace_do_at(queue_t::clock_type::now() + std::chrono::seconds(10),
            [&q] { std::cout << "shutdown\n"; q.shutdown(); });

    q.emplace_do_at(queue_t::clock_type::now() + std::chrono::milliseconds(4500),
            [] { std::cout << "Hello world\n"; });


    std::function<void()> ev;
    while(q.wait_pop(ev)) {
        ev();
    }
    oneth.join();
    twoth.join();
}
