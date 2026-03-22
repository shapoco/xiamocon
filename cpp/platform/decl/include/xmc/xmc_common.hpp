#ifndef XMC_XMC_COMMON_H
#define XMC_XMC_COMMON_H

#include "xmc/xmc_status.h"

#include <stdbool.h>
#include <stdint.h>

#define XMC_INLINE inline __attribute__((always_inline))

#define XMC_ENUM_FLAGS(TEnum, TBase)                             \
  inline TEnum operator|(TEnum a, TEnum b) {                     \
    return (TEnum)((TBase)a | (TBase)b);                         \
  }                                                              \
  inline TEnum operator&(TEnum a, TEnum b) {                     \
    return (TEnum)((TBase)a & (TBase)b);                         \
  }                                                              \
  inline TEnum operator~(TEnum a) { return (TEnum)(~(TBase)a); } \
  inline bool operator!(TEnum a) { return !(TBase)a; }           \
  inline TEnum &operator|=(TEnum &a, TEnum b) {                  \
    a = (TEnum)((TBase)a | (TBase)b);                            \
    return a;                                                    \
  }                                                              \
  inline TEnum &operator&=(TEnum &a, TEnum b) {                  \
    a = (TEnum)((TBase)a & (TBase)b);                            \
    return a;                                                    \
  }                                                              \
  inline bool hasFlag(TEnum value, TEnum flag) {                 \
    return ((TBase)value & (TBase)flag) != 0;                    \
  }

namespace xmc {

/**
 * Contents of a tight loop.
 */
void tightLoopContents();

}  // namespace xmc

#endif
