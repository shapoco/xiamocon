/**
 * @file vec3.hpp
 * @brief 3D vector with float components
 */

#ifndef XMC_GEO_VEC3_HPP
#define XMC_GEO_VEC3_HPP

#include "xmc/geo/geo_common.hpp"

#include <math.h>

namespace xmc {

struct vec3 {
  float x, y, z;

  XMC_INLINE vec3() : x(0), y(0), z(0) {}
  XMC_INLINE vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  XMC_INLINE vec3 operator+(const vec3 &other) const {
    return vec3(x + other.x, y + other.y, z + other.z);
  }

  XMC_INLINE vec3 operator-(const vec3 &other) const {
    return vec3(x - other.x, y - other.y, z - other.z);
  }

  XMC_INLINE vec3 operator*(float scalar) const {
    return vec3(x * scalar, y * scalar, z * scalar);
  }

  XMC_INLINE vec3 operator/(float scalar) const {
    float inv = 1.0f / scalar;
    return vec3(x * inv, y * inv, z * inv);
  }

  XMC_INLINE vec3 operator-() const { return vec3(-x, -y, -z); }

  XMC_INLINE vec3 &operator+=(const vec3 &other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  XMC_INLINE vec3 &operator-=(const vec3 &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  XMC_INLINE vec3 &operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  XMC_INLINE vec3 &operator/=(float scalar) {
    float inv = 1.0f / scalar;
    x *= inv;
    y *= inv;
    z *= inv;
    return *this;
  }

  /**
   * @brief Calculate the dot product of this vector with another vector
   * @param other The other vector to dot with
   * @return The dot product (a scalar value)
   */
  XMC_INLINE float dot(const vec3 &other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  /**
   * @brief Calculate the cross product of this vector with another vector
   * @param other The other vector to cross with
   * @return The cross product (a vector perpendicular to both)
   */
  XMC_INLINE vec3 cross(const vec3 &other) const {
    return vec3(y * other.z - z * other.y, z * other.x - x * other.z,
                x * other.y - y * other.x);
  }

  /**
   * @brief Calculate the squared length (magnitude) of the vector
   * @return The squared length of the vector
   */
  XMC_INLINE float squaredLength() const { return x * x + y * y + z * z; }

  /**
   * @brief Calculate the length (magnitude) of the vector
   * @return The length of the vector
   */
  inline float length() const { return sqrtf(squaredLength()); }

  /**
   * @brief Normalize the vector (make it unit length)
   * @return A normalized version of the vector
   */
  inline vec3 normalized() const {
    float len = length();
    if (len < 1e-8f) return vec3(0, 1, 0);
    float inv = 1.0f / len;
    return vec3(x * inv, y * inv, z * inv);
  }
};

}  // namespace xmc

#endif
