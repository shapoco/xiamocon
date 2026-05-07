#ifndef XMC_XMC_COMMON_H
#define XMC_XMC_COMMON_H

#include "xmc/xmc_status.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define XMC_INLINE inline __attribute__((always_inline))

#define XMC_ENUM_FLAGS(TEnum, TBase)                                           \
  XMC_INLINE constexpr TEnum operator|(TEnum a, TEnum b) {                     \
    return (TEnum)((TBase)a | (TBase)b);                                       \
  }                                                                            \
  XMC_INLINE constexpr TEnum operator&(TEnum a, TEnum b) {                     \
    return (TEnum)((TBase)a & (TBase)b);                                       \
  }                                                                            \
  XMC_INLINE constexpr TEnum operator~(TEnum a) { return (TEnum)(~(TBase)a); } \
  XMC_INLINE constexpr bool operator!(TEnum a) { return !(TBase)a; }           \
  XMC_INLINE TEnum &operator|=(TEnum &a, TEnum b) {                            \
    a = (TEnum)((TBase)a | (TBase)b);                                          \
    return a;                                                                  \
  }                                                                            \
  XMC_INLINE TEnum &operator&=(TEnum &a, TEnum b) {                            \
    a = (TEnum)((TBase)a & (TBase)b);                                          \
    return a;                                                                  \
  }                                                                            \
  XMC_INLINE constexpr bool hasFlag(TEnum value, TEnum flag) {                 \
    return ((TBase)value & (TBase)flag) != 0;                                  \
  }

#define XMC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define XMC_MAX(a, b) ((a) > (b) ? (a) : (b))
#define XMC_CLIP(min, max, value) \
  ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

namespace xmc {

/** Pixel formats for sprites and display interfaces. */
enum class PixelFormat {
  /** 1-bit grayscale format */
  GRAY1,
  /** 12-bit RGB format (4 bits per channel) */
  RGB444,
  /** 16-bit ARGB format (4 bits per channel) */
  ARGB4444,
  /** 16-bit RGB format (5 bits for red and blue, 6 bits for green) */
  RGB565,
};

/**
 * Contents of a tight loop.
 */
void tightLoopContents();

}  // namespace xmc

#endif
