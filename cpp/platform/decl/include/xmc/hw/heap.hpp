/**
 * @file heap.hpp
 * @brief RAM management interface
 */

#ifndef XMC_HW_HEAP_HPP
#define XMC_HW_HEAP_HPP

#include <stdbool.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

/** RAM capabilities */
typedef enum {
  /** No special capabilities */
  XMC_HEAP_CAP_NONE = 0,
  /** Supports DMA operations */
  XMC_HEAP_CAP_DMA = 1 << 0,
  /** Supports SPIRAM */
  XMC_HEAP_CAP_SPIRAM = 1 << 1,
} XmcHeapCap;

/**
 * Allocate memory with specific RAM capabilities.
 * @param size The size of the memory block to allocate.
 * @param caps The RAM capabilities required for the allocation.
 * @return A pointer to the allocated memory, or NULL if the allocation fails.
 */
void *xmcMalloc(size_t size, XmcHeapCap caps);

/**
 * Free memory allocated with xmcMalloc.
 * @param ptr A pointer to the memory block to free.
 */
void xmcFree(void *ptr);

#if defined(__cplusplus)
}
#endif

#endif
