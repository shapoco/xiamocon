#ifndef XMC_GFX_COLORF_HPP
#define XMC_GFX_COLORF_HPP

#include "xmc/gfx2d/gfx_common.hpp"

namespace xmc {

struct colorf {
  float r, g, b, a;

  XMC_INLINE colorf() : r(0), g(0), b(0), a(0) {}
  XMC_INLINE colorf(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

  XMC_INLINE static colorf from4444(color4444 c) {
    return colorf(c.r * (1.0f / 15.0f), c.g * (1.0f / 15.0f),
                  c.b * (1.0f / 15.0f), c.a * (1.0f / 15.0f));
  }

  XMC_INLINE uint16_t to565() const {
    return clipPack565(this->r * 31.0f, this->g * 63.0f, this->b * 31.0f);
  }

  XMC_INLINE colorf operator+(const colorf &other) const {
    return colorf(r + other.r, g + other.g, b + other.b, a + other.a);
  }

  XMC_INLINE colorf operator-(const colorf &other) const {
    return colorf(r - other.r, g - other.g, b - other.b, a - other.a);
  }

  XMC_INLINE colorf operator*(float scalar) const {
    return colorf(r * scalar, g * scalar, b * scalar, a * scalar);
  }

  XMC_INLINE colorf operator*(const colorf &other) const {
    return colorf(r * other.r, g * other.g, b * other.b, a * other.a);
  }

  XMC_INLINE colorf operator/(float scalar) const {
    return colorf(r / scalar, g / scalar, b / scalar, a / scalar);
  }

  XMC_INLINE colorf &operator+=(const colorf &other) {
    r += other.r;
    g += other.g;
    b += other.b;
    a += other.a;
    return *this;
  }

  XMC_INLINE colorf &operator-=(const colorf &other) {
    r -= other.r;
    g -= other.g;
    b -= other.b;
    a -= other.a;
    return *this;
  }

  XMC_INLINE colorf &operator*=(float scalar) {
    r *= scalar;
    g *= scalar;
    b *= scalar;
    a *= scalar;
    return *this;
  }

  XMC_INLINE colorf &operator*=(const colorf &other) {
    r *= other.r;
    g *= other.g;
    b *= other.b;
    a *= other.a;
    return *this;
  }

  XMC_INLINE colorf &operator/=(float scalar) {
    r /= scalar;
    g /= scalar;
    b /= scalar;
    a /= scalar;
    return *this;
  }
};

}  // namespace xmc

#endif
