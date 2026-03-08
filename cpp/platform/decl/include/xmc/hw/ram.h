/**
 * @file ram.h
 * @brief RAM management interface
 */

#ifndef XMC_HW_RAM_H
#define XMC_HW_RAM_H

#include <stdbool.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

/** RAM capabilities */
typedef enum {
  /** No special capabilities */
  XMC_RAM_CAP_NONE = 0,
  /** Supports DMA operations */
  XMC_RAM_CAP_DMA = 1 << 0,
} xmc_ram_cap_t;

/**
 * Allocate memory with specific RAM capabilities.
 * @param size The size of the memory block to allocate.
 * @param caps The RAM capabilities required for the allocation.
 * @return A pointer to the allocated memory, or NULL if the allocation fails.
 */
void *xmc_malloc(size_t size, xmc_ram_cap_t caps);

/**
 * Free memory allocated with xmc_malloc.
 * @param ptr A pointer to the memory block to free.
 */
void xmc_free(void *ptr);

#if defined(__cplusplus)
}
#endif

#endif
