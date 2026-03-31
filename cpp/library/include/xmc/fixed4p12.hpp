#ifndef XMC_FIXED4P12_HPP
#define XMC_FIXED4P12_HPP

#include "xmc/xmc_common.hpp"

namespace xmc {

struct fixed4p12 {
  int16_t raw;
  fixed4p12(int16_t raw) : raw(raw) {}
  fixed4p12() : raw(0) {}

  XMC_INLINE static fixed4p12 fromFloat(float f) {
    return fixed4p12((int16_t)(f * (1 << 12)));
  }

  XMC_INLINE float toFloat() const { return (float)raw / (1 << 12); }

  XMC_INLINE fixed4p12 operator+(const fixed4p12 &other) const {
    return fixed4p12(raw + other.raw);
  }

  XMC_INLINE fixed4p12 operator-(const fixed4p12 &other) const {
    return fixed4p12(raw - other.raw);
  }

  XMC_INLINE fixed4p12 operator*(const fixed4p12 &other) const {
    return fixed4p12((int32_t)raw * (int32_t)other.raw / (1 << 12));
  }

  XMC_INLINE fixed4p12 operator*(int32_t scalar) const {
    return fixed4p12((int32_t)raw * scalar);
  }

  XMC_INLINE fixed4p12 operator/(const fixed4p12 &other) const {
    return fixed4p12((int32_t)raw * (1 << 12) / (int32_t)other.raw);
  }

  XMC_INLINE fixed4p12 operator/(int32_t &other) const {
    return fixed4p12((int32_t)raw / other);
  }

  XMC_INLINE fixed4p12 &operator+=(const fixed4p12 &other) {
    raw += other.raw;
    return *this;
  }

  XMC_INLINE fixed4p12 &operator-=(const fixed4p12 &other) {
    raw -= other.raw;
    return *this;
  }

  XMC_INLINE fixed4p12 &operator*=(const fixed4p12 &other) {
    raw = (int32_t)raw * (int32_t)other.raw / (1 << 12);
    return *this;
  }

  XMC_INLINE fixed4p12 &operator*=(int32_t scalar) {
    raw = (int32_t)raw * scalar;
    return *this;
  }

  XMC_INLINE fixed4p12 &operator/=(const fixed4p12 &other) {
    raw = (int32_t)raw * (1 << 12) / (int32_t)other.raw;
    return *this;
  }

  XMC_INLINE fixed4p12 &operator/=(int32_t scalar) {
    raw = (int32_t)raw / scalar;
    return *this;
  }
};

}  // namespace xmc

#endif
