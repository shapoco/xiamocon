#include "xmc/hw/spinlock.hpp"
#include "xmc/hw/heap.hpp"

#include <Arduino.h>

namespace xmc {

SpinLock::SpinLock() {
  spinlock_t *spinlock = (spinlock_t *)xmcMalloc(sizeof(spinlock_t), XMC_HEAP_CAP_DMA);
  if (!spinlock) return;
  handle = spinlock;
  spinlock_initialize(spinlock);
}

SpinLock::~SpinLock() {
  spinlock_t *spinlock = (spinlock_t *)handle;
  if (!spinlock) return;
  xmcFree(spinlock);
  handle = nullptr;
}

void SpinLock::get() {
  if (!handle) return;
  spinlock_t *spinlock = (spinlock_t *)handle;
  taskENTER_CRITICAL(spinlock);
}

void SpinLock::release() {
  if (!handle) return;
  spinlock_t *spinlock = (spinlock_t *)handle;
  taskEXIT_CRITICAL(spinlock);
}

}  // namespace xmc