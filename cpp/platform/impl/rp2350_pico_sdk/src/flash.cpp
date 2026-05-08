#include "xmc/flash.hpp"
#include "xmc/multicore.hpp"

#include <hardware/flash.h>
#include <hardware/sync.h>
#include <pico/multicore.h>

namespace xmc::flash {

XmcStatus init() {
  multicore_lockout_victim_init();
  return XMC_OK;
}

void deinit() {}

size_t getSectorSize() { return FLASH_SECTOR_SIZE; }

void getRange(size_t *base, size_t *size) {
  if (base) *base = 0;
  if (size) *size = PICO_FLASH_SIZE_BYTES;
}

XmcStatus erase(uint32_t offset, size_t size) {
  bool core1Running = isCore1Running();
  if (core1Running) multicore_lockout_start_blocking();
  uint32_t irq_state = save_and_disable_interrupts();

  size_t numSectors = (size + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE;
  flash_range_erase(offset, numSectors * FLASH_SECTOR_SIZE);

  restore_interrupts(irq_state);
  if (core1Running) multicore_lockout_end_blocking();
  return XMC_OK;
}

XmcStatus write(uint32_t offset, const void *data, size_t size) {
  bool core1Running = isCore1Running();

  if (core1Running) multicore_lockout_start_blocking();
  uint32_t irq_state = save_and_disable_interrupts();

  size_t remaining = size;
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  while (remaining > 0) {
    size_t bytesToWrite = XMC_MIN(remaining, FLASH_SECTOR_SIZE);
    if (bytesToWrite > FLASH_SECTOR_SIZE) {
      bytesToWrite = FLASH_SECTOR_SIZE;
    }
    flash_range_program(offset + (size - remaining), ptr, bytesToWrite);
    ptr += bytesToWrite;
    remaining -= bytesToWrite;
  }

  restore_interrupts(irq_state);
  if (core1Running) multicore_lockout_end_blocking();

  return XMC_OK;
}

XmcStatus mmap(uint32_t offset, size_t length, void **handle,
               const uint8_t **outPtr) {
  *handle = new int[1];  // dummy handle
  *outPtr = (const uint8_t *)(XIP_BASE + offset);
  return XMC_OK;
}

void munmap(void *handle) {
  if (handle) {
    delete[] static_cast<int *>(handle);
  }
}

}  // namespace xmc::flash
