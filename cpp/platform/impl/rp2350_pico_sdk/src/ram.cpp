#include "xmc/hw/ram.hpp"

#include <pico/stdlib.h>

void *xmcMalloc(size_t size, XmcRamCap caps) { return malloc(size); }
void xmcFree(void *ptr) { free(ptr); }
