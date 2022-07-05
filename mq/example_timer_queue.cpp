#include "lyn/timer_queue.hpp"

#include <atomic>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

using queue_1_t = lyn::mq::timer_queue<void()>;

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

    q.emplace_do_at(queue_1_t::clock_type::now() + std::chrono::seconds(10), [&q] {
        std::cout << "shutdown\n";
        q.shutdown();
    });

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
using queue_2_t = lyn::mq::timer_queue<void(int, double)>;

void sync2(queue_2_t& q) {
    auto res = q.synchronize<double>([](int i, double d) -> double { return i + d; });
    q.shutdown();
    std::cout << "got " << res << " via synchronize\n";
}

void test2() {
    queue_2_t q;
    auto syncth = std::thread(&sync2, std::ref(q));
    queue_2_t::event_type ev;
    while(q.wait_pop(ev)) {
        ev(1, 3.14159);
    }
    syncth.join();
}
// -----
class tester : public lyn::mq::timer_queue<void()> {
public:
    using lyn::mq::timer_queue<void()>::wait_pop_future;
    using lyn::mq::timer_queue<void()>::wait_pop_all_future;
};

void test3() {
    std::cout << "Placing future events in queue and extract them one by one:\n";
    tester t;
    t.emplace_do_in(13s, [&t] { t.shutdown(); });
    t.emplace_do_in(12s, [] { std::cout << "12s passed\n"; });
    t.emplace_do_in(11s, [] { std::cout << "11s passed\n"; });
    t.emplace_do_in(10s, [] { std::cout << "10s passed\n"; });

    for(tester::event_type e; t.wait_pop_future(e);) {
        e();
    }
}

void test4() {
    std::cout << "Placing future events in queue and extract them all:\n";
    tester t;
    t.emplace_do_in(12s, [] { std::cout << "12s passed\n"; });
    t.emplace_do_in(11s, [] { std::cout << "11s passed\n"; });
    t.emplace_do_in(10s, [] { std::cout << "10s passed\n"; });

    tester::queue_type q;
    t.wait_pop_all_future(q);

    for(tester::event_type e; q.pop(e);) {
        e();
    }
}

using queue_5_t = lyn::mq::timer_queue<bool(int)>;

void sync5(queue_5_t& q) {
    auto res = q.synchronize<int>([](int val) { return val*val; }, true);
    q.shutdown();
    std::cout << "test5 got " << res << " via synchronize\n";
}

void test5() {
    queue_5_t q;
    auto syncth = std::thread(&sync5, std::ref(q));
    queue_5_t::event_type ev;
    while(q.wait_pop(ev)) {
        if(ev(10)) std::cout << "test5 event true\n";
        else std::cout << "test5 event false\n";
    }
    syncth.join();
}
// -----
int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
}
