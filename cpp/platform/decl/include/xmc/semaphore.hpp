#ifndef XMC_HW_SEMAPHORE_HPP
#define XMC_HW_SEMAPHORE_HPP

#include "xmc/hw_common.hpp"

namespace xmc {

class Semaphore {
 public:
  void *handle = nullptr;
  Semaphore();
  ~Semaphore();
  inline bool isInitialized() const { return handle != nullptr; }
  void take();
  bool tryTake();
  void give();
};

}  // namespace xmc

#endif
