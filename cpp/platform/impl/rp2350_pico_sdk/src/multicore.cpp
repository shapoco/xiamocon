#include "xmc/multicore.hpp"
#include "xmc/hw/timer.hpp"

#include <pico/multicore.h>

namespace xmc {

static volatile Core1TaskFunc core1Func = nullptr;
static volatile bool stopRequested = false;

static void core1Loop();

XmcStatus startCore1(Core1TaskFunc task) {
  if (core1Func) {
    XMC_ERR_RET(XMC_ERR_MULTICORE_ALREADY_RUNNING);
  }
  stopRequested = false;
  core1Func = task;
  multicore_launch_core1(core1Loop);
  return XMC_OK;
}

XmcStatus stopCore1(uint32_t timeoutMs) {
  if (!core1Func) {
    return XMC_OK;
  }

  stopRequested = true;
  uint64_t expireMs = getTimeMs() + timeoutMs;
  while (core1Func) {
    if (getTimeMs() > expireMs) {
      multicore_reset_core1();
      XMC_ERR_RET(XMC_ERR_MULTICORE_STOP_FAILED);
    }
  }
  multicore_reset_core1();
  return XMC_OK;
}

void core1Loop() {
  Core1TaskFunc task = core1Func;
  while (!stopRequested) {
    if (!task()) break;
  }
  core1Func = nullptr;
}

}  // namespace xmc
