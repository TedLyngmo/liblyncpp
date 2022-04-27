# lyn::mq::timer\_queue

Defined in header [`lyn/timer_queue.hpp`](../include/lyn/timer_queue.hpp).

---
```
template<
    class EventType = std::function<void()>,
    class Clock = std::chrono::steady_clock,
    class TimePoint = std::chrono::time_point<Clock>
> class timer_queue;
```

---
A timer queue provides constant time lookup of the first event to timeout, at the expense of logarithmic insertion and extraction.

#### Template parameters

|Parameter|Description|
|-:|:-|
| **EventType** | The type of events to store in the queue. `std::function<void()>` by default. |
|     **Clock** | The clock type used to keep time. `std::chrono::steady_clock` by default.     |
| **TimePoint** | `std::chrono::time_point<Clock>>`                                             |

|Member types| Definitions |
|-:|:-|
| `event_type` | EventType                         |
| `clock_type` | Clock                             |
| `duration`   | Clock::duration                   |
| `time_point` | TimePoint                         |
| `queue_type` | `std::priority_queue<TimedEvent>` |

---

|Member functions | |
|-|-|
|`timer_queue()` | constructs the `timer_queue` with zero delay for events added using `emplace_do` |
|`explicit timer_queue(duration now_delay)` | constructs the `timer_queue` with `now_delay` delay for events added using<br>`emplace_do` |
|`~timer_queue()` | destroys the `timer_queue` |
|`template<class... Args>`<br>`void emplace_do(Args&&... args)`| create an event in-place that is due after `now_delay` and sorts the underlying<br>container |
|`template<class... Args>`<br>`void emplace_do_urgently(Args&&... args)`| create an event in-place and sorts the underlying container, placing the event<br>last among those to be extracted immediately |
|`template<class TP, class... Args>`<br>`void emplace_do_at(TP&& tp, Args&&... args)` | create an event in-place that is due at at the specified `time_point` and sorts<br>the underlying container, placing the event in `time_point` order |
|`template<class... Args>`<br>`void emplace_do_in(duration dur, Args&&... args)` | create an event in-place that is due after the duration `dur` and sorts the<br>underlying container, placing the event in `time_point` order |
|`bool wait_pop(event_type& ev)` | wait until an event is due and populates `ev`. Returns `true` if an event was<br>successfully extracted or `false` if the queue was shutdown. |
| `void shutdown()` | shutdown queue |
| `bool operator!() const` | returns `true` if the `shutdown()` has been called, `false` otherwise |
| `bool is_open() const` | returns `true` if `shutdown()` has _not_ been called, `false`otherwise |
| `explicit operator bool() const` | returns the same as `is_open()` |
