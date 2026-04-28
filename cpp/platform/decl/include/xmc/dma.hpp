/**
 * @file dma.hpp
 * @brief DMA transfer configuration structure
 */

#ifndef XMC_HW_DMA_HPP
#define XMC_HW_DMA_HPP

namespace xmc::dma {

/**
 * DMA transfer configuration.
 */
struct Config {
  /** Pointer to the data buffer. */
  void *ptr;
  /** Size of each element in bytes. */
  int elementSize;
  /** Number of elements to transfer. */
  int length;
};

}  // namespace xmc::dma

#endif
