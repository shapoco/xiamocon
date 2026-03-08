#ifndef XMC_HW_DMA_H
#define XMC_HW_DMA_H

/**
 * DMA transfer configuration.
 */
typedef struct {
  /** Pointer to the data buffer. */
  void *ptr;
  /** Size of each element in bytes. */
  int element_size;
  /** Number of elements to transfer. */
  int length;
  
} xmc_dma_config_t;

#endif
