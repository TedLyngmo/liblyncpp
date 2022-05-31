#include "lyn/timer_queue.hpp"

#include <atomic>
#include <iostream>
#include <thread>

using queue_1_t = lyn::mq::timer_queue<>;

void one(queue_1_t& q) {
    for(int i = 0; q; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        q.emplace_do([i] { std::cout << "one: " << i << '\n'; });
    }
}

void two(queue_1_t& q) {
    for(int i = 0; q; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        q.emplace_do([i] { std::cout << "two: " << i << '\n'; });
    }
}

void three(queue_1_t& q) {
    for(int i = 0; q; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        q.emplace_do([i] { std::cout << "three: " << i << '\n'; });
    }
}

void test1() {
    queue_1_t q(std::chrono::seconds(1));

    auto twoth = std::thread(&two, std::ref(q));
    auto oneth = std::thread(&one, std::ref(q));

    q.emplace_do_at(queue_1_t::clock_type::now() + std::chrono::seconds(10),
            [&q] { std::cout << "shutdown\n"; q.shutdown(); });

    q.emplace_do_at(queue_1_t::clock_type::now() + std::chrono::milliseconds(4500),
            [] { std::cout << "Hello world\n"; });


    queue_1_t::event_type ev;
    while(q.wait_pop(ev)) {
        ev();
    }
    oneth.join();
    twoth.join();

    q.restart();
    auto threeth = std::thread(&three, std::ref(q));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    decltype(q)::queue_type iq;
    while(q.wait_pop_all(iq)) {
        q.shutdown();
        std::cout << iq.size() << '\n';
        while(iq.pop(ev)) {
            ev();
        }
    }
    threeth.join();
}
// -----
using queue_2_t = lyn::mq::timer_queue<std::function<void(int,double)>>;

void sync(queue_2_t& q) {
    auto res = q.synchronize<double>([](int i, double d) -> double {
        return i + d;
    });
    q.shutdown();
    std::cout << "got " << res << " via synchronize\n";
}

void test2() {
    queue_2_t q;
    auto syncth = std::thread(&sync, std::ref(q));
    queue_2_t::event_type ev;
    while(q.wait_pop(ev)) {
        ev(1, 3.14159);
    }
    syncth.join();
}
// -----
int main() {
    test1();
    test2();
}
