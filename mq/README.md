# lyn::mq::timer\_queue

Defined in header [`lyn/timer_queue.hpp`](../include/lyn/timer_queue.hpp).

---
```
template<
    class EventType = std::function<void()>,
    class Clock = std::chrono::steady_clock,
    class TimePoint = std::chrono::time_point<Clock>>
class timer_queue;
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
| `time_point` | TimePoint                         |
| `queue_type` | `std::priority_queue<TimedEvent>` |

---

|Member functions | |
|-|-|
|`(constructor)`| constructs the `timer_queue` |
|`(destructor)` | destroys the `timer_queue` |
|`template<class... Args>`<br>`void emplace_do_now(Args&&... args)`| create an event in-place and sorts the underlying<br>container, placing the event last among those to<br>be extracted immediately |
|`template<class TP, class... Args>`<br>`void emplace_do_at(TP&& tp, Args&&... args)` | create an event in-place and sorts the underlying<br>container, placing the event in `time_point` order |
|`bool wait_pop(event_type& ev)` | wait until an event is due and populates `ev`. Returns<br>`true` if an event was successfully extracted or<br>`false` if the queue was shutdown. |
| `void shutdown()` | shutdown queue |
