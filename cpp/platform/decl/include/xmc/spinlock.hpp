#ifndef XMC_HW_SPINLOCK_HPP
#define XMC_HW_SPINLOCK_HPP

#include "xmc/hw/hw_common.hpp"

namespace xmc {

class SpinLock {
 public:
  void *handle = nullptr;
  SpinLock();
  ~SpinLock();
  void get();
  void release();
};

}  // namespace xmc

#endif
