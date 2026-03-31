#ifndef XMC_GFX_COLOR4P12_HPP
#define XMC_GFX_COLOR4P12_HPP

#include "xmc/ap_fixed.hpp"
#include "xmc/gfx/colorf.hpp"
#include "xmc/gfx/gfx_common.hpp"

namespace xmc {

struct color4p12 {
  static constexpr int PRECISION = 12;
  fixed4p12 b;
  fixed4p12 g;
  fixed4p12 r;
  fixed4p12 a;

  color4p12() : b(0), g(0), r(0), a(0) {}
  color4p12(fixed4p12 r, fixed4p12 g, fixed4p12 b, fixed4p12 a)
      : b(b), g(g), r(r), a(a) {}
  color4p12(colorf c)
      : b(fixed4p12::fromFloat(c.b)),
        g(fixed4p12::fromFloat(c.g)),
        r(fixed4p12::fromFloat(c.r)),
        a(fixed4p12::fromFloat(c.a)) {}

  XMC_INLINE uint16_t to565() const {
    uint_fast16_t result = 0;
    if (r.raw >= (1 << PRECISION)) {
      result = 0xF800;
    } else if (r.raw >= 0) {
      result = (r.raw << (16 - PRECISION)) & 0xF800;
    }
    if (g.raw >= (1 << PRECISION)) {
      result |= 0x07E0;
    } else if (g.raw >= 0) {
      result |= (g.raw >> (PRECISION - 11)) & 0x07E0;
    }
    if (b.raw >= (1 << PRECISION)) {
      result |= 0x001F;
    } else if (b.raw >= 0) {
      result |= (b.raw >> (PRECISION - 5)) & 0x001F;
    }
    return ((result << 8) & 0xFF00) | ((result >> 8) & 0x00FF);
  }

  XMC_INLINE uint16_t to444() const {
    uint_fast16_t result = 0;
    if (r.raw >= (1 << PRECISION)) {
      result = 0x0F00;
    } else if (r.raw >= 0) {
      result = r.raw & 0x0F00;
    }
    if (g.raw >= (1 << PRECISION)) {
      result |= 0x00F0;
    } else if (g.raw >= 0) {
      result |= (g.raw >> (PRECISION - 8)) & 0x00F0;
    }
    if (b.raw >= (1 << PRECISION)) {
      result |= 0x000F;
    } else if (b.raw >= 0) {
      result |= (b.raw >> (PRECISION - 4)) & 0x000F;
    }
    return result;
  }

  XMC_INLINE color4p12 operator+(const color4p12 &other) const {
    return color4p12{b + other.b, g + other.g, r + other.r, a + other.a};
  }

  XMC_INLINE color4p12 operator-(const color4p12 &other) const {
    return color4p12{b - other.b, g - other.g, r - other.r, a - other.a};
  }

  XMC_INLINE color4p12 operator*(float scalar) const {
    return color4p12{b * scalar, g * scalar, r * scalar, a * scalar};
  }

  XMC_INLINE color4p12 operator*(const color4p12 &other) const {
    return color4p12{b * other.b, g * other.g, r * other.r, a * other.a};
  }

  XMC_INLINE color4p12 operator/(float scalar) const {
    return color4p12{b / scalar, g / scalar, r / scalar, a / scalar};
  }

  XMC_INLINE color4p12 operator/(int32_t scalar) const {
    return color4p12{b / scalar, g / scalar, r / scalar, a / scalar};
  }

  XMC_INLINE color4p12 &operator+=(const color4p12 &other) {
    b += other.b;
    g += other.g;
    r += other.r;
    a += other.a;
    return *this;
  }

  XMC_INLINE color4p12 &operator-=(const color4p12 &other) {
    b -= other.b;
    g -= other.g;
    r -= other.r;
    a -= other.a;
    return *this;
  }

  XMC_INLINE color4p12 &operator*=(float scalar) {
    b *= scalar;
    g *= scalar;
    r *= scalar;
    a *= scalar;
    return *this;
  }

  XMC_INLINE color4p12 &operator*=(int32_t scalar) {
    b *= scalar;
    g *= scalar;
    r *= scalar;
    a *= scalar;
    return *this;
  }

  XMC_INLINE color4p12 &operator*=(const color4p12 &other) {
    b *= other.b;
    g *= other.g;
    r *= other.r;
    a *= other.a;
    return *this;
  }

  XMC_INLINE color4p12 &operator/=(float scalar) {
    b /= scalar;
    g /= scalar;
    r /= scalar;
    a /= scalar;
    return *this;
  }

  XMC_INLINE color4p12 &operator/=(int32_t scalar) {
    b /= scalar;
    g /= scalar;
    r /= scalar;
    a /= scalar;
    return *this;
  }

  XMC_INLINE color4p12 &operator*=(const color4444 &other) {
    b = (int32_t)b.raw * other.b / 15;
    g = (int32_t)g.raw * other.g / 15;
    r = (int32_t)r.raw * other.r / 15;
    a = (int32_t)a.raw * other.a / 15;
    return *this;
  }
};

}  // namespace xmc

#endif
