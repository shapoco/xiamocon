#include "xmc/hw/ram.hpp"

#include <Arduino.h>
#include <esp_heap_caps.h>

void *xmcMalloc(size_t size, xmc_ram_cap_t caps) {
  int heapCaps = 0;
  if (caps & XMC_RAM_CAP_DMA) {
    heapCaps |= MALLOC_CAP_DMA;
  }
  return heap_caps_malloc(size, heapCaps);
}
void xmcFree(void *ptr) { heap_caps_free(ptr); }
