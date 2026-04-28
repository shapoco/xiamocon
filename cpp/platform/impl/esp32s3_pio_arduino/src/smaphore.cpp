#include "xmc/semaphore.hpp"

#include <Arduino.h>

namespace xmc {

Semaphore::Semaphore() {
  SemaphoreHandle_t semHandle = xSemaphoreCreateBinary();
  if (!semHandle) return;
  handle = semHandle;
  xSemaphoreGive(semHandle);
}

Semaphore::~Semaphore() {
  SemaphoreHandle_t semHandle = (SemaphoreHandle_t)handle;
  if (!semHandle) return;
  vSemaphoreDelete(semHandle);
  handle = nullptr;
}

void Semaphore::take() {
  if (!handle) return;
  SemaphoreHandle_t semHandle = (SemaphoreHandle_t)handle;
  xSemaphoreTake(semHandle, portMAX_DELAY);
}

bool Semaphore::tryTake() {
  if (!handle) return false;
  SemaphoreHandle_t semHandle = (SemaphoreHandle_t)handle;
  return xSemaphoreTake(semHandle, 0) == pdTRUE;
}

void Semaphore::give() {
  if (!handle) return;
  SemaphoreHandle_t semHandle = (SemaphoreHandle_t)handle;
  xSemaphoreGive(semHandle);
}

}  // namespace xmc