#ifndef XMC_GFX_COLORF_HPP
#define XMC_GFX_COLORF_HPP

#include "xmc/gfx2d/gfx_common.hpp"

namespace xmc {

struct colorf {
  float r, g, b, a;

  colorf() : r(0), g(0), b(0), a(0) {}
  colorf(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

  static colorf from4444(color4444 c) {
    return colorf(c.r * (1.0f / 15.0f), c.g * (1.0f / 15.0f),
                  c.b * (1.0f / 15.0f), c.a * (1.0f / 15.0f));
  }

  inline uint16_t to565() const {
    int r = (int)(this->r * 31.0f);
    if (r < 0)
      r = 0;
    else if (r > 31)
      r = 31;
    int g = (int)(this->g * 63.0f);
    if (g < 0)
      g = 0;
    else if (g > 63)
      g = 63;
    int b = (int)(this->b * 31.0f);
    if (b < 0)
      b = 0;
    else if (b > 31)
      b = 31;
    return pack565(r, g, b);
  }

  inline colorf operator+(const colorf &other) const {
    return colorf(r + other.r, g + other.g, b + other.b, a + other.a);
  }

  inline colorf operator-(const colorf &other) const {
    return colorf(r - other.r, g - other.g, b - other.b, a - other.a);
  }

  inline colorf operator*(float scalar) const {
    return colorf(r * scalar, g * scalar, b * scalar, a * scalar);
  }

  inline colorf operator*(const colorf &other) const {
    return colorf(r * other.r, g * other.g, b * other.b, a * other.a);
  }

  inline colorf operator/(float scalar) const {
    return colorf(r / scalar, g / scalar, b / scalar, a / scalar);
  }

  inline colorf &operator+=(const colorf &other) {
    r += other.r;
    g += other.g;
    b += other.b;
    a += other.a;
    return *this;
  }

  inline colorf &operator-=(const colorf &other) {
    r -= other.r;
    g -= other.g;
    b -= other.b;
    a -= other.a;
    return *this;
  }

  inline colorf &operator*=(float scalar) {
    r *= scalar;
    g *= scalar;
    b *= scalar;
    a *= scalar;
    return *this;
  }

  inline colorf &operator*=(const colorf &other) {
    r *= other.r;
    g *= other.g;
    b *= other.b;
    a *= other.a;
    return *this;
  }

  inline colorf &operator/=(float scalar) {
    r /= scalar;
    g /= scalar;
    b /= scalar;
    a /= scalar;
    return *this;
  }
};

}  // namespace xmc

#endif
