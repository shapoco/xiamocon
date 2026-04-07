/**
 * @file quat.hpp
 * @brief Quaternion for 3D rotations
 */

#ifndef XMC_GEO_QUAT_HPP
#define XMC_GEO_QUAT_HPP

#include "xmc/geo/geo_common.hpp"
#include "xmc/geo/vec3.hpp"

#include <math.h>

namespace xmc {

struct quat {
  float w, x, y, z;

  quat() : w(1), x(0), y(0), z(0) {}
  quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

  quat operator*(const quat &other) const {
    return quat(w * other.w - x * other.x - y * other.y - z * other.z,
                w * other.x + x * other.w + y * other.z - z * other.y,
                w * other.y - x * other.z + y * other.w + z * other.x,
                w * other.z + x * other.y - y * other.x + z * other.w);
  }

  /**
   * @brief Create a quaternion from Euler angles (pitch, yaw, roll)
   * @param pitch Rotation around the x-axis in radians
   * @param yaw Rotation around the y-axis in radians
   * @param roll Rotation around the z-axis in radians
   * @return A quaternion representing the combined rotation
   */
  static quat fromEuler(float pitch, float yaw, float roll) {
    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);
    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);
    return quat(cr * cy * cp + sr * sy * sp, cr * cy * sp - sr * sy * cp,
                cr * sy * cp + sr * cy * sp, sr * cy * cp - cr * sy * sp);
  }

  /**
   * @brief Convert the quaternion to Euler angles (pitch, yaw, roll)
   * @param pitch Output parameter for rotation around the x-axis in radians
   * @param yaw Output parameter for rotation around the y-axis in radians
   * @param roll Output parameter for rotation around the z-axis in radians
   */
  void toEuler(float *pitch, float *yaw, float *roll) const {
    // pitch (x-axis rotation)
    float sinp = 2.0f * (w * x + y * z);
    if (fabsf(sinp) >= 1.0f) {
      *pitch = copysignf(M_PI / 2.0f, sinp);  // use 90 degrees if out of range
    } else {
      *pitch = asinf(sinp);
    }

    // yaw (y-axis rotation)
    float sinyCosp = 2.0f * (w * y - z * x);
    float cosyCosp = 1.0f - 2.0f * (x * x + y * y);
    *yaw = atan2f(sinyCosp, cosyCosp);

    // roll (z-axis rotation)
    float sinrCosp = 2.0f * (w * z - x * y);
    float cosrCosp = 1.0f - 2.0f * (y * y + z * z);
    *roll = atan2f(sinrCosp, cosrCosp);
  }

  /**
   * @brief Create a quaternion from an axis-angle representation
   * @param axis The axis of rotation (should be normalized)
   * @param angle The angle of rotation in radians
   * @return A quaternion representing the rotation around the given axis by the
   * specified angle
   */
  static quat fromAxisAngle(const vec3 &axis, float angle) {
    float half = angle * 0.5f;
    float s = sinf(half);
    return quat(cosf(half), axis.x * s, axis.y * s, axis.z * s);
  }

  /**
   * @brief Rotate a vector by this quaternion
   * @param v The vector to rotate
   * @return The rotated vector
   */
  vec3 rotate(const vec3 &v) const {
    quat v_quat(0, v.x, v.y, v.z);
    quat result = (*this) * v_quat * conjugate();
    return vec3(result.x, result.y, result.z);
  }

  /**
   * @return the conjugate of the quaternion (inverse for unit quaternions)
   */
  quat conjugate() const { return quat(w, -x, -y, -z); }

  /**
   * @return a normalized version of the quaternion (unit quaternion
   * representing the same rotation)
   */
  quat normalized() const {
    float len = sqrtf(w * w + x * x + y * y + z * z);
    if (len < 1e-8f) return quat(1, 0, 0, 0);
    float inv = 1.0f / len;
    return quat(w * inv, x * inv, y * inv, z * inv);
  }
};

}  // namespace xmc

#endif
