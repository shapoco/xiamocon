#include "xmc/hw/timer.hpp"
#include "xmc/hw/ram.hpp"

#include <Arduino.h>

namespace xmc {

struct RepeatingTimerHwEsp {
  hw_timer_t *timer;
  TimerCallback cb;
  void *context;
};

static void ARDUINO_ISR_ATTR repeatingTimerCallback(void *hw);

uint64_t getTimeMs() { return millis(); }
uint64_t getTimeUs() { return micros(); }
void sleepMs(uint32_t ms) { delay(ms); }
void sleepUs(uint32_t us) { delayMicroseconds(us); }

RepeatingTimer::RepeatingTimer() {
  RepeatingTimerHwEsp *hw = (RepeatingTimerHwEsp *)xmcMalloc(
      sizeof(RepeatingTimerHwEsp), XMC_RAM_CAP_DMA);
  if (!hw) return;
  handle = hw;
  hw->timer = nullptr;
}

RepeatingTimer::~RepeatingTimer() {
  if (handle) {
    RepeatingTimerHwEsp *hw = (RepeatingTimerHwEsp *)handle;
    if (hw->timer) {
      timerStop(hw->timer);
      timerEnd(hw->timer);
      hw->timer = nullptr;
    }
    xmcFree(handle);
    handle = nullptr;
  }
}

XmcStatus RepeatingTimer ::startMs(uint32_t intervalMs, TimerCallback cb,
                                   void *context) {
  if (!handle) XMC_ERR_RET(XMC_ERR_NOT_INITIALIZED);
  RepeatingTimerHwEsp *hw = (RepeatingTimerHwEsp *)handle;
  hw->cb = cb;
  hw->context = context;
  cancel();
  hw->timer = timerBegin(1000);
  if (!hw->timer) {
    return XMC_ERR_TIMER_REPEATING_TIMER_INIT_FAILED;
  }
  timerAttachInterruptArg(hw->timer, repeatingTimerCallback, hw);
  timerAlarm(hw->timer, intervalMs, true, 0);
  timerStart(hw->timer);
  return XMC_OK;
}

void RepeatingTimer::cancel() {
  if (!handle) return;
  RepeatingTimerHwEsp *hw = (RepeatingTimerHwEsp *)handle;
  if (!hw->timer) return;
  timerStop(hw->timer);
  timerEnd(hw->timer);
  hw->timer = nullptr;
}

static void ARDUINO_ISR_ATTR repeatingTimerCallback(void *hw) {
  RepeatingTimerHwEsp *timerHw = (RepeatingTimerHwEsp *)hw;
  if (!timerHw || !timerHw->cb || !timerHw->timer) return;
  if (!timerHw->cb(timerHw->context)) {
    timerStop(timerHw->timer);
  }
}

}  // namespace xmc