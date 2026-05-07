/**
 * @file random.h
 * @brief Random number generation for XMC library
 */

#ifndef XMC_RANDOM_H
#define XMC_RANDOM_H

#include "xmc/xmc_common.hpp"

namespace xmc {

/**
 * Get the next random 32-bit unsigned integer.
 * @return A random 32-bit unsigned integer.
 */
uint32_t randomU32();

/**
 * Get the next random 32-bit floating-point number in the range [0, 1).
 * @return A random 32-bit floating-point number in the range [0, 1).
 */
float randomF32();

/**
 * Seed the random number generator with a specific value.
 * @param seed The seed value to initialize the random number generator.
 */
XMC_INLINE void updateLfsr32(uint32_t *lfsr) {
  uint32_t bit =
      ((*lfsr >> 0) ^ (*lfsr >> 1) ^ (*lfsr >> 21) ^ (*lfsr >> 31)) & 1;
  *lfsr = (*lfsr >> 1) | (bit << 31);
}

}  // namespace xmc

#endif
