#include "xmc/heap.hpp"

#include <Arduino.h>
#include <esp_heap_caps.h>

void *xmcMalloc(size_t size, XmcHeapCap caps) {
  int heapCaps = 0;
  void *ptr = nullptr;
  if (caps & XMC_HEAP_CAP_DMA) {
    ptr = heap_caps_malloc(size, MALLOC_CAP_DMA);
    if (ptr) return ptr;
  }
  if (caps & XMC_HEAP_CAP_SPIRAM) {
    ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (ptr) return ptr;
  }
  return nullptr;
}
void xmcFree(void *ptr) { heap_caps_free(ptr); }
