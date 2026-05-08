#include "xmc/heap.hpp"

#include <pico/stdlib.h>

bool xmcHasSpiRam() { return false; }
void *xmcMalloc(size_t size, XmcHeapCap caps) { return malloc(size); }
void xmcFree(void *ptr) { free(ptr); }
