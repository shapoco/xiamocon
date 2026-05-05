/**
 * @file vec2i.hpp
 * @brief 2D vector with integer components
 */

#ifndef XMC_GEO_VEC2I_HPP
#define XMC_GEO_VEC2I_HPP

#include "xmc/geo/geo_common.hpp"

#include <math.h>

namespace xmc {

struct vec2i {
  int x;
  int y;

  XMC_INLINE vec2i() : x(0), y(0) {}
  XMC_INLINE vec2i(int x, int y) : x(x), y(y) {}

  XMC_INLINE vec2i operator+(const vec2i &other) const {
    return vec2i(x + other.x, y + other.y);
  }

  XMC_INLINE vec2i operator-(const vec2i &other) const {
    return vec2i(x - other.x, y - other.y);
  }

  XMC_INLINE vec2i operator*(int scalar) const {
    return vec2i(x * scalar, y * scalar);
  }

  XMC_INLINE vec2i operator/(int scalar) const {
    return vec2i(x / scalar, y / scalar);
  }

  XMC_INLINE vec2i operator-() const { return vec2i(-x, -y); }

  XMC_INLINE vec2i &operator+=(const vec2i &other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  XMC_INLINE vec2i &operator-=(const vec2i &other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  XMC_INLINE vec2i &operator*=(int scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  XMC_INLINE vec2i &operator/=(int scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  /**
   * @brief Calculate the dot product of this vector with another vector
   * @param other The other vector to dot with
   * @return The dot product (a scalar value)
   */
  XMC_INLINE int dot(const vec2i &other) const {
    return x * other.x + y * other.y;
  }

  /**
   * @brief Calculate the cross product of this vector with another vector
   * @param other The other vector to cross with
   * @return The cross product (a scalar value in 2D)
   */
  XMC_INLINE int cross(const vec2i &other) const {
    return x * other.y - y * other.x;
  }

  /**
   * @brief Calculate the squared length (magnitude) of the vector
   * @return The squared length of the vector
   */
  XMC_INLINE int squaredLength() const { return x * x + y * y; }

  /**
   * @brief Calculate the length (magnitude) of the vector
   * @return The length of the vector
   */
  inline float length() const { return sqrtf(squaredLength()); }

  /**
   * @brief Normalize the vector (make it unit length)
   * @return A normalized version of the vector
   */
  inline vec2i normalized() const {
    float len = length();
    if (len < 1e-8f) return vec2i(0, 0);
    float inv = 1.0f / len;
    return vec2i(x * inv, y * inv);
  }
};

}  // namespace xmc

#endif
