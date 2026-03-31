#ifndef XMC_AP_FIXED_HPP
#define XMC_AP_FIXED_HPP

#include "xmc/xmc_common.hpp"

namespace xmc {

template <typename TRaw = int32_t, int PREC = 12>
struct apFixed {
  static constexpr int PRECISION = PREC;
  using TDouble =
      std::conditional_t<(sizeof(TRaw) <= sizeof(int16_t)), int32_t, int64_t>;
  TRaw raw;
  apFixed(TRaw raw) : raw(raw) {}
  apFixed() : raw(0) {}

  XMC_INLINE static apFixed fromFloat(float f) {
    return apFixed((TRaw)(f * (1 << PREC)));
  }

  XMC_INLINE float toFloat() const { return (float)raw / (1 << PREC); }

  XMC_INLINE apFixed operator+(const apFixed &other) const {
    return apFixed(raw + other.raw);
  }

  XMC_INLINE apFixed operator-(const apFixed &other) const {
    return apFixed(raw - other.raw);
  }

  XMC_INLINE apFixed operator*(const apFixed &other) const {
    return apFixed((TDouble)raw * (TDouble)other.raw / (1 << PREC));
  }

  XMC_INLINE apFixed operator*(int32_t scalar) const {
    return apFixed(raw * scalar);
  }

  XMC_INLINE apFixed operator/(const apFixed &other) const {
    return apFixed((TDouble)raw * (1 << PREC) / (TDouble)other.raw);
  }

  XMC_INLINE apFixed operator/(int32_t scalar) const {
    return apFixed(raw / scalar);
  }

  XMC_INLINE apFixed &operator+=(const apFixed &other) {
    raw += other.raw;
    return *this;
  }

  XMC_INLINE apFixed &operator-=(const apFixed &other) {
    raw -= other.raw;
    return *this;
  }

  XMC_INLINE apFixed &operator*=(const apFixed &other) {
    raw = (TDouble)raw * (TDouble)other.raw / (1 << PREC);
    return *this;
  }

  XMC_INLINE apFixed &operator*=(int32_t scalar) {
    raw *= scalar;
    return *this;
  }

  XMC_INLINE apFixed &operator/=(const apFixed &other) {
    raw = (TDouble)raw * (1 << PREC) / (TDouble)other.raw;
    return *this;
  }

  XMC_INLINE apFixed &operator/=(int32_t scalar) {
    raw /= scalar;
    return *this;
  }
};

using fixed4p12 = apFixed<int16_t, 12>;
using fixed20p12 = apFixed<int32_t, 12>;
using fixed16p16 = apFixed<int32_t, 16>;

}  // namespace xmc

#endif
