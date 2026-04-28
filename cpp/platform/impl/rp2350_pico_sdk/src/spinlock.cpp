#include "xmc/spinlock.hpp"

#include <pico/sync.h>
#include <stdlib.h>

namespace xmc {

static const int NUM_SPINLOCKS = 16;
static uint32_t usedSpinlocks = 0;

struct SpinLockHw {
  spin_lock_t *hwLock;
  uint32_t irqs;
  int id;
};

SpinLock::SpinLock() {
  int id = __builtin_ffs(~usedSpinlocks) - 1;
  if (id < 0 || NUM_SPINLOCKS <= id) {
    return;
  }

  SpinLockHw *hw = (SpinLockHw *)malloc(sizeof(SpinLockHw));
  if (!hw) return;
  hw->hwLock = spin_lock_init(id);
  hw->id = id;
  handle = hw;
  spin_lock_claim(id);
  usedSpinlocks |= (1U << id);
}

SpinLock::~SpinLock() {
  SpinLockHw *hw = (SpinLockHw *)handle;
  if (!hw) return;
  spin_lock_unclaim(hw->id);
  usedSpinlocks &= ~(1U << hw->id);
  free(hw);
  handle = nullptr;
}

void SpinLock::get() {
  if (!handle) return;
  SpinLockHw *hw = (SpinLockHw *)handle;
  hw->irqs = spin_lock_blocking(hw->hwLock);
}

void SpinLock::release() {
  if (!handle) return;
  SpinLockHw *hw = (SpinLockHw *)handle;
  spin_unlock(hw->hwLock, hw->irqs);
}

}  // namespace xmc