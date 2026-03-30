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

  inline vec2 operator+(const vec2 &other) const {
    return vec2(x + other.x, y + other.y);
  }

  inline vec2 operator-(const vec2 &other) const {
    return vec2(x - other.x, y - other.y);
  }

  inline vec2 operator*(float scalar) const {
    return vec2(x * scalar, y * scalar);
  }

  inline vec2 operator/(float scalar) const {
    float inv = 1.0f / scalar;
    return vec2(x * inv, y * inv);
  }

  inline vec2 operator-() const { return vec2(-x, -y); }

  inline vec2 &operator+=(const vec2 &other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  inline vec2 &operator-=(const vec2 &other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  inline vec2 &operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  inline vec2 &operator/=(float scalar) {
    float inv = 1.0f / scalar;
    x *= inv;
    y *= inv;
    return *this;
  }

  /**
   * @brief Calculate the dot product of this vector with another vector
   * @param other The other vector to dot with
   * @return The dot product (a scalar value)
   */
  inline float dot(const vec2 &other) const {
    return x * other.x + y * other.y;
  }

  /**
   * @brief Calculate the cross product of this vector with another vector
   * @param other The other vector to cross with
   * @return The cross product (a scalar value in 2D)
   */
  inline float cross(const vec2 &other) const {
    return x * other.y - y * other.x;
  }

  /**
   * @brief Calculate the length (magnitude) of the vector
   * @return The length of the vector
   */
  inline float length() const { return sqrtf(x * x + y * y); }

  /**
   * @brief Normalize the vector (make it unit length)
   * @return A normalized version of the vector
   */
  inline vec2 normalized() const {
    float len = length();
    if (len < 1e-8f) return vec2(0, 0);
    float inv = 1.0f / len;
    return vec2(x * inv, y * inv);
  }
};

}  // namespace xmc

#endif
