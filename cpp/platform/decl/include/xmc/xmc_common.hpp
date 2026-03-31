#ifndef XMC_XMC_COMMON_H
#define XMC_XMC_COMMON_H

#include "xmc/xmc_status.h"

#include <stdbool.h>
#include <stdint.h>

#define XMC_INLINE inline __attribute__((always_inline))

#define XMC_ENUM_FLAGS(TEnum, TBase)                                       \
  inline constexpr TEnum operator|(TEnum a, TEnum b) {                     \
    return (TEnum)((TBase)a | (TBase)b);                                   \
  }                                                                        \
  inline constexpr TEnum operator&(TEnum a, TEnum b) {                     \
    return (TEnum)((TBase)a & (TBase)b);                                   \
  }                                                                        \
  inline constexpr TEnum operator~(TEnum a) { return (TEnum)(~(TBase)a); } \
  inline constexpr bool operator!(TEnum a) { return !(TBase)a; }           \
  inline TEnum &operator|=(TEnum &a, TEnum b) {                            \
    a = (TEnum)((TBase)a | (TBase)b);                                      \
    return a;                                                              \
  }                                                                        \
  inline TEnum &operator&=(TEnum &a, TEnum b) {                            \
    a = (TEnum)((TBase)a & (TBase)b);                                      \
    return a;                                                              \
  }                                                                        \
  inline constexpr bool hasFlag(TEnum value, TEnum flag) {                 \
    return ((TBase)value & (TBase)flag) != 0;                              \
  }

namespace xmc {

/**
 * Contents of a tight loop.
 */
void tightLoopContents();

template <typename T>
static XMC_INLINE T xmcClip(T min, T max, T value) {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  } else {
    return value;
  }
}

}  // namespace xmc

#endif
