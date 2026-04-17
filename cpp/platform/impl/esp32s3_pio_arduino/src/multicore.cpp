#include "xmc/multicore.hpp"
#include "xmc/hw/timer.hpp"

#include <Arduino.h>
#include <soc/timer_group_reg.h>
#include <soc/timer_group_struct.h>
#include <soc/wdt_periph.h>

namespace xmc {

static TaskHandle_t core1TaskHandle = nullptr;
static volatile Core1TaskFunc core1Func = nullptr;
static volatile bool stopRequested = false;

static void core1Loop(void *arg);
static void feedWatchdog();

XmcStatus startCore1(Core1TaskFunc task) {
  if (core1Func) {
    XMC_ERR_RET(XMC_ERR_MULTICORE_ALREADY_RUNNING);
  }
  stopRequested = false;
  core1Func = task;
  xTaskCreatePinnedToCore(core1Loop, "Core1Loop", 8192, NULL, 10,
                          &core1TaskHandle, PRO_CPU_NUM);
  return XMC_OK;
}

XmcStatus stopCore1(uint32_t timeoutMs) {
  stopRequested = true;
  uint64_t expireMs = getTimeMs() + timeoutMs;
  while (core1Func) {
    if (getTimeMs() > expireMs) {
      vTaskDelete(core1TaskHandle);
      core1TaskHandle = nullptr;
      XMC_ERR_RET(XMC_ERR_MULTICORE_STOP_FAILED);
    }
  }
  return XMC_OK;
}

void core1Loop(void *arg) {
  Core1TaskFunc task = core1Func;
  uint64_t nextWdtResetMs = 0;
  while (!stopRequested) {
    // prevent watchdog reset by this core
    uint64_t nowMs = millis();
    if (nowMs > nextWdtResetMs) {
      nextWdtResetMs = nowMs + 1000;
      feedWatchdog();
    }

    if (!task()) break;
  }
  vTaskDelete(core1TaskHandle);
  core1TaskHandle = nullptr;
  core1Func = nullptr;
}

static void feedWatchdog() {
  TIMERG0.wdtwprotect.val = TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdtfeed.val = 1;
  TIMERG0.wdtwprotect.val = 0;
  TIMERG1.wdtwprotect.val = TIMG_WDT_WKEY_VALUE;
  TIMERG1.wdtfeed.val = 1;
  TIMERG1.wdtwprotect.val = 0;
}

}  // namespace xmc
