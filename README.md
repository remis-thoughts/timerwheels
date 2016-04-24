# TimerWheels

A C++14 header-only collection of thread-safe, atomic timer wheel implementations, based on the ["Hashed and Hierarchical Timing Wheels: Data Structures for the Efficient Implementation of a Timer Facility"](http://www.cs.columbia.edu/~nahum/w6998/papers/sosp87-timing-wheels.pdf) paper.

## API

Each timerwheel implementation has the following methods, where `DURATION` is a `std::chrono` duration (like `std::chrono::milliseconds`). Each implementation also takes a `CLOCK` template parameter, specifying a class that implements the `TrivialClock` concept, like `std::chrono::high_resolution_clock`.

- `void tick()`. Call this regularly; this performs housekeeping like firing timers.
- `void schedule(std::function<void()> fire, DURATION when, DURATION repeat = DURATION::zero())` to schedule the function after a delay of `when`. If the `repeat` parameter is non-zero then the timer is rescheduled immediately after firing with a delay of `repeat`. The `fire` function is always executed by the thread that calls `tick()`, which is not necessarily the thread that called `schedule`.

## Implementations

- "Scheme 4 - Basic Scheme for Timer Intervals within a Specified Range" implemented as `timerwheels::FixedRangeTimerWheel`. The template parameter `RANGE` is the "specifed range" - the furthest away any timer can be scheduled. If `schedule` is called with a greater duration, it is rounded down to this `RANGE`. The `BUCKET` template parameter is another duration; it is the accuracy of the wheel, as defined as the longest delay between the time requested and the actual time the function executed, assuming `tick()` is called with a frequency of at least once every `BUCKET` duration.

## Other Libraries

If this library doesn't fit your needs, try one of these:

- [timeout.c](https://github.com/wahern/timeout)
- [simple-timing-wheel](https://github.com/hoterran/simple-timing-wheel)
