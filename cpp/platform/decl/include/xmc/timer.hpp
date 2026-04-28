/**
 * @file timer.hpp
 * @brief Timer hardware interface
 */

#ifndef XMC_HW_TIMER_HPP
#define XMC_HW_TIMER_HPP

#include "xmc/xmc_common.hpp"

#include <stdint.h>

namespace xmc {

/**
 * Timer tick callback function type. This is the type of the callback function
 * that will be called when a timer tick occurs. The callback function will
 * receive a context pointer that can be used to pass user-defined data to the
 * callback function.
 * @param context A user-defined context pointer that was provided when the
 * timer was initialized.
 * @return A boolean value indicating whether the timer should continue running.
 * If the callback returns true, the timer will continue to run and call
 * the callback again after the next interval. If the callback returns
 * false, the timer will be canceled and will not call the callback again.
 */
using TimerCallback = bool (*)(void *context);

class RepeatingTimer {
 public:
  /** Hardware-specific handle for the timer instance */
  void *handle = nullptr;

  RepeatingTimer();
  ~RepeatingTimer();

  /**
   * Start a repeating timer that calls the specified callback function at the
   * specified interval in milliseconds. The callback function will be called
   * with the provided context pointer. The timer will continue to run and call
   * the callback function at each interval until the callback function returns
   * false or the timer is canceled.
   * @param intervalMs The interval in milliseconds at which to call the
   * callback function.
   * @param cb The callback function to call at each timer tick.
   * @param context A user-defined context pointer that will be passed to the
   * callback function.
   * @return XmcStatus indicating success or failure of the operation.
   */
  XmcStatus startMs(uint32_t intervalMs, TimerCallback cb, void *context);

  /**
   * Stop the repeating timer. This will cancel the timer and prevent it from
   * calling the callback function again. If the timer is not currently running,
   * this function will have no effect.
   */
  void cancel();
};

/**
 * Get the current time in milliseconds since an arbitrary epoch. The epoch and
 * resolution are implementation-defined, but the value will always be
 * increasing and will wrap around on overflow.
 */
uint64_t getTimeMs();

/**
 * Get the current time in microseconds since an arbitrary epoch. The epoch and
 * resolution are implementation-defined, but the value will always be
 * increasing and will wrap around on overflow.
 */
uint64_t getTimeUs();

/**
 * Sleep for the specified number of milliseconds.
 * @param ms The number of milliseconds to sleep.
 */
void sleepMs(uint32_t ms);

/**
 * Sleep for the specified number of microseconds.
 * @param us The number of microseconds to sleep.
 */
void sleepUs(uint32_t us);

}  // namespace xmc

#endif
