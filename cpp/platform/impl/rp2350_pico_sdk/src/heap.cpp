#include "xmc/hw/heap.hpp"

#include <pico/stdlib.h>

void *xmcMalloc(size_t size, XmcHeapCap caps) { return malloc(size); }
void xmcFree(void *ptr) { free(ptr); }
