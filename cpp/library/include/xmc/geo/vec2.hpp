/**
 * @file vec2.hpp
 * @brief 2D vector with float components
 */

#ifndef XMC_GEO_VEC2_HPP
#define XMC_GEO_VEC2_HPP

#include "xmc/geo/geo_common.hpp"

#include <math.h>

namespace xmc {

struct vec2 {
  float x;
  float y;

  vec2() : x(0), y(0) {}
  vec2(float x, float y) : x(x), y(y) {}
  vec2 operator+(const vec2 &other) const {
    return vec2(x + other.x, y + other.y);
  }
  vec2 operator-(const vec2 &other) const {
    return vec2(x - other.x, y - other.y);
  }
  vec2 operator*(float scalar) const { return vec2(x * scalar, y * scalar); }

  /**
   * @brief Calculate the dot product of this vector with another vector
   * @param other The other vector to dot with
   * @return The dot product (a scalar value)
   */
  float dot(const vec2 &other) const { return x * other.x + y * other.y; }

  /**
   * @brief Calculate the length (magnitude) of the vector
   * @return The length of the vector
   */
  float length() const { return sqrtf(x * x + y * y); }

  /**
   * @brief Normalize the vector (make it unit length)
   * @return A normalized version of the vector
   */
  vec2 normalized() const {
    float len = length();
    if (len < 1e-8f) return vec2(0, 0);
    float inv = 1.0f / len;
    return vec2(x * inv, y * inv);
  }
};

}  // namespace xmc

#endif
