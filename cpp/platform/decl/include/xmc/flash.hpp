#ifndef XMC_FLASH_HPP
#define XMC_FLASH_HPP

#include "xmc/hw_common.hpp"

namespace xmc::flash {

// Initialize the flash memory interface.
XmcStatus init();

// Deinitialize the flash memory interface.
void deinit();

// Get the size of a flash sector.
size_t getSectorSize();

// Get the base offset and size of the flash memory.
void getRange(size_t *base, size_t *size);

// Write data to flash memory.
XmcStatus write(uint32_t offset, const void *data, size_t length);

// Map a region of flash memory.
XmcStatus mmap(uint32_t offset, size_t size, void **handle,
               const uint8_t **outPtr);

// Unmap a previously mapped region of flash memory.
void munmap(void *handle);

}  // namespace xmc::flash

#endif
