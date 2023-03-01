# lyn::mq::timer\_queue

Defined in header [`lyn/timer_queue.hpp`](../include/lyn/timer_queue.hpp).

---
```
template<
    class R, class... Args
    class Clock = std::chrono::steady_clock,
    class TimePoint = std::chrono::time_point<Clock>
> class timer_queue; /* undefined */

template<class R, class... Args, class Clock, class TimePoint>
class timer_queue<R(Args...), Clock, TimePoint>;
```

---
A timer queue provides constant time lookup of the first event to timeout, at the expense of logarithmic insertion and extraction.

#### Template parameters

|Parameter|Description|
|-:|:-|
|         **R** | The return type of events to store in the queue.                              |
|   **Args...** | The arguments to pass to events stored in the queue.                          |
|     **Clock** | The clock type used to keep time. `std::chrono::steady_clock` by default.     |
| **TimePoint** | `std::chrono::time_point<Clock>`                                              |

|Member types| Definitions |
|-:|:-|
| `event_type`       | The type you extract in the event loop |
| `clock_type`       | Clock                                  |
| `duration`         | Clock::duration                        |
| `time_point`       | TimePoint                              |
| `queue_type`       | _unspecified_ - Has a member function `bool pop(event_type& ev)` - see `wait_pop_all` |
| `schedule_at_type` | `std::pair<time_point, event_type>` |
| `schedule_in_type` | `std::pair<duration, event_type>` |


---

|Member functions | |
|-|-|
|`timer_queue()` | constructs the `timer_queue` with zero delay for events added using `emplace_do` |
|`explicit timer_queue(duration now_delay)` | constructs the `timer_queue` with `now_delay` delay for events added using<br>`emplace_do` |
|`~timer_queue()` | destroys the `timer_queue` |
|`void emplace_do(event_type ev)`| create an event in-place that is due after `now_delay` and sorts the underlying<br>container |
|`void emplace_do_urgently(event_type ev)`| create an event in-place and sorts the underlying container, placing the event<br>last among those to be extracted immediately |
|`void emplace_do_at(time_point tp, event_type ev)` | create an event in-place that is due at the specified `time_point` and sorts<br>the underlying container, placing the event in `time_point` order |
|`void emplace_do_in(duration dur, event_type)` | create an event in-place that is due after the duration `dur` and sorts the<br>underlying container, placing the event in `time_point` order |
|`template<class Iter>`<br>`void emplace_schedule(Iter first, Iter last)` | Place a number of events in queue. This overload only participates in overload resolution if `std::iterator_traits<Iter>::value_type` is `event_type`.|
|`template<class Iter>`<br>`void emplace_schedule(Iter first, Iter last)` | Place a number of events in queue. This overload only participates in overload resolution if `std::iterator_traits<Iter>::value_type` is `schedule_at_type`.|
|`template<class Iter>`<br>`void emplace_schedule(time_point T0, Iter first, Iter last)` | Place a number of events in queue in relation to `T0`. This overload only participates in overload resolution if `std::iterator_traits<Iter>::value_type` is `schedule_in_type`.|
|`template<class Re, class Func>`<br>`Re synchronize(Func&& func)`|Execute `func`, that should return `Re`, in the task queue and wait for the execution to complete. This overload only participates in overload resolution if `R` is `void`.|
|`template<class Re, class Func>`<br>`Re synchronize(Func&& func, R&& event_loop_return_value = R{})`|Execute `func`, that should return `Re`, in the task queue and wait for the execution to complete. The value returned by the event when executed in the event loop is stored in `event_loop_return_value`. This overload only participates in overload resolution if `R` is not `void`.|
|`bool wait_pop(event_type& ev)` | wait until an event is due and populates `ev`. Returns `true` if an event was<br>successfully extracted or `false` if the queue was shutdown. |
|`bool wait_pop_all(queue_type& in_out)` | wait until an event is due and extracts all events that are due.<br>Returns `true` unless the queue was shutdown in which case it returns `false`.|
| `void shutdown()` | shutdown the queue, leaving unprocessed events in the queue |
| `void clear()` | removes unprocessed events from the queue |
| `void restart()` | restarts the queue with unprocessed events intact |
| `bool operator!() const` | returns `true` if `shutdown()` has been called, `false` otherwise |
| `bool is_open() const` | returns `true` if `shutdown()` has _not_ been called, `false`otherwise |
| `explicit operator bool() const` | returns the same as `is_open()` |

[Compiler Explorer `emplace_schedule` demo](https://godbolt.org/z/ME1rq1EfY)
