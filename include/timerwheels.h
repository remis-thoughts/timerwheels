#ifndef TIMERWHEELS_H
#define TIMERWHEELS_H

#include <array>
#include <atomic>
#include <chrono>

namespace timerwheels {
	/**
	 * \brief A timer using the "Scheme 4 - Basic Scheme for Timer Intervals within a Specified Range"
	 */
	template<
		typename DURATION = std::chrono::milliseconds,
		std::size_t RANGE = 1024,
		std::size_t BUCKET = 4,
		typename CLOCK = std::chrono::high_resolution_clock>
	class FixedRangeTimerWheel final {
    	static_assert(BUCKET <= RANGE,"The fixed time range must be bigger than a bucket");
    	static_assert(RANGE % BUCKET == 0,"The fixed time range must an integer multiple of the bucket size");

		struct Timer final {
			std::atomic<struct Timer*> next;

			/**
			 * @brief Function to call when the timer elapses.
			 */
			std::function<void()> fire;

			/**
			 * @brief if non-`DURATION::zero()`, how long to repeat the timer after.
			 */
			DURATION repeat;

			/**
			 * @brief When to schedule.
			 * @description Stored here as we schedule work separately
			 * 
			 */
			DURATION when;

			Timer(std::function<void()>&& fire, DURATION&& repeat, DURATION&& when)
				: fire(fire), repeat(repeat), next(nullptr), when(when) {}
		};

		std::array<std::atomic<struct Timer*>, RANGE / BUCKET> wheel;

		/**
		 * @brief Used by the `tick()` thread only; the index in the wheel which is "now".
		 */
		std::size_t index;

		/**
		 * @brief Used by the `tick()` thread only; stores the last time `tick()` was called
		 */
		std::chrono::time_point<CLOCK> last_tick;

		/**
		 * @brief Shared between threads; stores any timers that haven't been added to the wheel.
		 */
		std::atomic<struct Timer*> unscheduled;

		inline void queue(struct Timer* timer, std::atomic<struct Timer*>& onto) {
			struct Timer* head = nullptr;
			do {
				head = onto.load(std::memory_order::memory_order_acq_rel);
				timer->next.store(head, std::memory_order::memory_order_acq_rel);
			} while (!onto.compare_exchange_weak(head, timer, std::memory_order::memory_order_acq_rel));
		}

		/**
		 * @brief insert a timer into the wheel.
		 * @param when how much time remaining before the timer elapses.
		 */
		inline void schedule(struct Timer* timer, DURATION when) {
			// round up time remaining to the next multiple of BUCKET size
			std::size_t ticks = (when + DURATION(BUCKET) - DURATION(1)) / DURATION(BUCKET);

			// fixed time range
			ticks = std::min<std::size_t>(ticks, RANGE / BUCKET);

			std::size_t pos = (index + ticks) % (RANGE / BUCKET);

			queue(timer, wheel[pos]);
		}

	public:
		FixedRangeTimerWheel() : last_tick(CLOCK::now()), index(0), unscheduled(nullptr), wheel {} {}

		/**
		 * @brief Add a function to be called after a specified delay.
		 * 
		 * @param fire The function to call; always executed on the thread calling `tick()`.
		 * @param when The initial delay before calling the function.
		 * @param repeat If non-zero, the function will be rescheduled immediately after firing after this delay.
		 */
		void schedule(std::function<void()> fire, DURATION when, DURATION repeat = DURATION::zero()) {
			queue(
				new Timer(std::move(fire), std::move(repeat), std::move(when)),
				unscheduled);
		}

		/**
		 * @brief Do any outstanding work
		 * @details Fire any timers that have elapsed, and schedule any unscheduled timers.
		 */
		void tick() {
			auto now = CLOCK::now();

			// advance wheel and fire any timers
			std::size_t ticks_to_advance = std::min<std::size_t>((now - last_tick) / DURATION(BUCKET), RANGE / BUCKET);
			for (std::size_t i = 1; i <= ticks_to_advance; ++i) {
				index = ++index % (RANGE / BUCKET);
				for (
					struct Timer * t = wheel[index].exchange(nullptr, std::memory_order::memory_order_acq_rel), * next = nullptr;
					t != nullptr;
					t = next) {
					t->fire();

					// clean-up or reschedule
					next = t->next.load(std::memory_order::memory_order_acq_rel);
					if (t->repeat == DURATION::zero()) {
						delete t;
					} else {
						schedule(t, t->repeat);
					}
				}
			}

			last_tick += ticks_to_advance * DURATION(BUCKET);

			// schedule any timers added by other threads
			for (
				struct Timer * t = unscheduled.exchange(nullptr, std::memory_order::memory_order_acq_rel), * next = nullptr;
				t != nullptr;
				t = next) {
				next = t->next.load(std::memory_order::memory_order_acq_rel);
				schedule(t, t->when);
			}
		}
	};
}

#endif