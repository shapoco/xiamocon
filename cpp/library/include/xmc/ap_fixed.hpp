#ifndef XMC_AP_FIXED_HPP
#define XMC_AP_FIXED_HPP

#include "xmc/xmc_common.hpp"

#include <limits>
#include <type_traits>

namespace xmc {

template <typename TRaw = int32_t, int PREC = 12>
struct apFixed {
  static constexpr int PRECISION = PREC;
  static constexpr TRaw ONE = (TRaw)1 << PREC;
  using TDouble = typename std::conditional<(sizeof(TRaw) <= sizeof(int16_t)),
                                            int32_t, int64_t>::type;
  static constexpr TRaw RAW_MAX = std::numeric_limits<TRaw>::max();
  static constexpr TRaw RAW_MIN = std::numeric_limits<TRaw>::min();
  TRaw raw;
  XMC_INLINE constexpr apFixed(TRaw raw) : raw(raw) {}
  XMC_INLINE constexpr apFixed() : raw(0) {}

  XMC_INLINE static constexpr apFixed fromFloat(float f) {
    TDouble value = (TDouble)(f * ONE);
    return apFixed(XMC_CLIP(RAW_MIN, RAW_MAX, value));
  }

  XMC_INLINE static constexpr apFixed subDiv(const apFixed &a, const apFixed &b,
                                             int32_t span) {
    return apFixed(((TDouble)a.raw - (TDouble)b.raw) / span);
  }

  XMC_INLINE constexpr float toFloat() const { return (float)raw / ONE; }
  XMC_INLINE constexpr TRaw floorToInt() const { return raw >> PREC; }
  XMC_INLINE constexpr TRaw roundToInt() const {
    return (raw + ONE / 2) >> PREC;
  }
  XMC_INLINE constexpr TRaw ceilToInt() const {
    return (raw + ONE - 1) >> PREC;
  }

  XMC_INLINE constexpr apFixed operator+(const apFixed &other) const {
    return apFixed(raw + other.raw);
  }

  XMC_INLINE constexpr apFixed operator-(const apFixed &other) const {
    return apFixed(raw - other.raw);
  }

  XMC_INLINE constexpr apFixed operator*(const apFixed &other) const {
    return apFixed(((TDouble)raw * (TDouble)other.raw) >> PREC);
  }

  XMC_INLINE constexpr apFixed operator*(int scalar) const {
    return apFixed(raw * scalar);
  }

  XMC_INLINE constexpr apFixed operator*(float scalar) const {
    return apFixed(raw * scalar);
  }

  XMC_INLINE constexpr apFixed operator/(const apFixed &other) const {
    return apFixed((((TDouble)raw) << PREC) / (TDouble)other.raw);
  }

  XMC_INLINE constexpr apFixed operator/(int scalar) const {
    return apFixed(raw / scalar);
  }

  XMC_INLINE constexpr apFixed &operator+=(const apFixed &other) {
    raw += other.raw;
    return *this;
  }

  XMC_INLINE constexpr apFixed &operator-=(const apFixed &other) {
    raw -= other.raw;
    return *this;
  }

  XMC_INLINE constexpr apFixed &operator*=(const apFixed &other) {
    raw = ((TDouble)raw * (TDouble)other.raw) >> PREC;
    return *this;
  }

  XMC_INLINE constexpr apFixed &operator*=(int32_t scalar) {
    raw *= scalar;
    return *this;
  }

  XMC_INLINE constexpr apFixed &operator/=(const apFixed &other) {
    raw = (((TDouble)raw) << PREC) / (TDouble)other.raw;
    return *this;
  }

  XMC_INLINE constexpr apFixed &operator/=(int32_t scalar) {
    raw /= scalar;
    return *this;
  }

  XMC_INLINE constexpr bool operator<(const apFixed &other) const {
    return raw < other.raw;
  }
  XMC_INLINE constexpr bool operator<=(const apFixed &other) const {
    return raw <= other.raw;
  }
  XMC_INLINE constexpr bool operator>(const apFixed &other) const {
    return raw > other.raw;
  }
  XMC_INLINE constexpr bool operator>=(const apFixed &other) const {
    return raw >= other.raw;
  }
  XMC_INLINE constexpr bool operator==(const apFixed &other) const {
    return raw == other.raw;
  }
  XMC_INLINE constexpr bool operator!=(const apFixed &other) const {
    return raw != other.raw;
  }
};

using fixed4p12 = apFixed<int16_t, 12>;
using fixed20p12 = apFixed<int32_t, 12>;
using fixed16p16 = apFixed<int32_t, 16>;
using fixed12p20 = apFixed<int32_t, 20>;
using fixed8p24 = apFixed<int32_t, 24>;

}  // namespace xmc

#endif
