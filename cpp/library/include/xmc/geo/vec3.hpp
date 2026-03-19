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

  vec3() : x(0), y(0), z(0) {}
  vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  inline vec3 operator+(const vec3 &other) const {
    return vec3(x + other.x, y + other.y, z + other.z);
  }

  inline vec3 operator-(const vec3 &other) const {
    return vec3(x - other.x, y - other.y, z - other.z);
  }

  inline vec3 operator*(float scalar) const {
    return vec3(x * scalar, y * scalar, z * scalar);
  }

  /**
   * @brief Calculate the dot product of this vector with another vector
   * @param other The other vector to dot with
   * @return The dot product (a scalar value)
   */
  inline float dot(const vec3 &other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  /**
   * @brief Calculate the cross product of this vector with another vector
   * @param other The other vector to cross with
   * @return The cross product (a vector perpendicular to both)
   */
  inline vec3 cross(const vec3 &other) const {
    return vec3(y * other.z - z * other.y, z * other.x - x * other.z,
                x * other.y - y * other.x);
  }

  /**
   * @brief Calculate the length (magnitude) of the vector
   * @return The length of the vector
   */
  inline float length() const { return sqrtf(x * x + y * y + z * z); }

  /**
   * @brief Normalize the vector (make it unit length)
   * @return A normalized version of the vector
   */
  inline vec3 normalized() const {
    float len = length();
    if (len < 1e-8f) return vec3(0, 0, 0);
    float inv = 1.0f / len;
    return vec3(x * inv, y * inv, z * inv);
  }
};

}  // namespace xmc

#endif
