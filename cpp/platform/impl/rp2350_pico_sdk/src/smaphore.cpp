#include "xmc/semaphore.hpp"

#include <pico/sem.h>
#include <stdlib.h>

namespace xmc {

Semaphore::Semaphore() {
  semaphore_t *semHandle = (semaphore_t *)malloc(sizeof(semaphore_t));
  if (!semHandle) return;
  handle = semHandle;
  sem_init(semHandle, 1, 1);
}

Semaphore::~Semaphore() {
  semaphore_t *semHandle = (semaphore_t *)handle;
  if (!semHandle) return;
  sem_release(semHandle);
  free(semHandle);
  handle = nullptr;
}

void Semaphore::take() {
  if (!handle) return;
  semaphore_t *semHandle = (semaphore_t *)handle;
  sem_acquire_blocking(semHandle);
}

bool Semaphore::tryTake() {
  if (!handle) return false;
  semaphore_t *semHandle = (semaphore_t *)handle;
  return sem_try_acquire(semHandle);
}

void Semaphore::give() {
  if (!handle) return;
  semaphore_t *semHandle = (semaphore_t *)handle;
  sem_release(semHandle);
}

}  // namespace xmc