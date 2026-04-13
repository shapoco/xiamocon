/**
 * @file mat4.hpp
 * @brief 4x4 matrix for 3D transformations
 */

#ifndef XMC_GEO_MAT4_HPP
#define XMC_GEO_MAT4_HPP

#include "xmc/geo/geo_common.hpp"
#include "xmc/geo/quat.hpp"
#include "xmc/geo/vec3.hpp"

#include <math.h>
#include <string.h>

namespace xmc {

struct mat4 {
  float m[16] = {0};

  /**
   * @return an identity matrix
   */
  static XMC_INLINE mat4 identity() {
    mat4 result;
    result.m[0] = 1;
    result.m[5] = 1;
    result.m[10] = 1;
    result.m[15] = 1;
    return result;
  }

  XMC_INLINE mat4 operator*(const mat4 &other) const {
    mat4 result = identity();
    for (int col = 0; col < 4; col++) {
      for (int row = 0; row < 4; row++) {
        result.m[col * 4 + row] = m[0 * 4 + row] * other.m[col * 4 + 0] +
                                  m[1 * 4 + row] * other.m[col * 4 + 1] +
                                  m[2 * 4 + row] * other.m[col * 4 + 2] +
                                  m[3 * 4 + row] * other.m[col * 4 + 3];
      }
    }
    return result;
  }

  XMC_INLINE mat4 &operator*=(const mat4 &other) {
    *this = *this * other;
    return *this;
  }

  /**
   * @brief Create an orthographic projection matrix
   * @param left The left clipping plane
   * @param right The right clipping plane
   * @param bottom The bottom clipping plane
   * @param top The top clipping plane
   * @param near The near clipping plane
   * @param far The far clipping plane
   * @return A 4x4 orthographic projection matrix
   */
  static mat4 ortho(float left, float right, float bottom, float top,
                    float near = -1.0f, float far = 1.0f) {
    mat4 result = identity();
    result.m[0] = 2.0f / (right - left);
    result.m[5] = 2.0f / (top - bottom);
    result.m[10] = -2.0f / (far - near);
    result.m[12] = -(right + left) / (right - left);
    result.m[13] = -(top + bottom) / (top - bottom);
    result.m[14] = -(far + near) / (far - near);
    return result;
  }

  /**
   * @brief Create a perspective projection matrix
   * @param fovY The vertical field of view in radians
   * @param aspect The aspect ratio (width / height) of the viewport
   * @param near The distance to the near clipping plane (must be positive)
   * @param far The distance to the far clipping plane (must be positive)
   * @return A 4x4 perspective projection matrix
   */
  static mat4 perspective(float fovY, float aspect, float near, float far) {
    mat4 result = identity();
    float f = 1.0f / tanf(fovY * 0.5f);
    result.m[0] = f / aspect;
    result.m[5] = f;
    result.m[10] = (far + near) / (near - far);
    result.m[11] = -1.0f;
    result.m[14] = (2.0f * far * near) / (near - far);
    return result;
  }

  static mat4 lookAt(const vec3 &eye, const vec3 &focus, const vec3 &up) {
    vec3 f = (focus - eye).normalized();
    vec3 s = f.cross(up).normalized();
    vec3 u = s.cross(f);
    mat4 result = identity();
    result.m[0] = s.x;
    result.m[1] = u.x;
    result.m[2] = -f.x;
    result.m[4] = s.y;
    result.m[5] = u.y;
    result.m[6] = -f.y;
    result.m[8] = s.z;
    result.m[9] = u.z;
    result.m[10] = -f.z;
    result.m[12] = -s.dot(eye);
    result.m[13] = -u.dot(eye);
    result.m[14] = f.dot(eye);
    return result;
  }

  /**
   * @brief Load the identity matrix into this matrix
   * After calling this function, this matrix will be an identity matrix
   */
  XMC_INLINE void loadIdentity() { *this = identity(); }

  /**
   * @brief Create a rotation matrix from a quaternion
   * @param q The quaternion representing the rotation
   * @return A 4x4 rotation matrix
   */
  static mat4 fromQuat(const quat &q) {
    mat4 result = identity();
    float xx = q.x * q.x;
    float yy = q.y * q.y;
    float zz = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;
    result.m[0] = 1.0f - (yy + zz) * 2;
    result.m[1] = (xy + wz) * 2;
    result.m[2] = (xz - wy) * 2;
    result.m[4] = (xy - wz) * 2;
    result.m[5] = 1.0f - (xx + zz) * 2;
    result.m[6] = (yz + wx) * 2;
    result.m[8] = (xz + wy) * 2;
    result.m[9] = (yz - wx) * 2;
    result.m[10] = 1.0f - (xx + yy) * 2;
    return result;
  }

  /**
   * @brief Transform a point by this matrix, applying perspective division
   * @param v The point to transform
   * @return The transformed point
   * @note If the w component after transformation is close to zero, returns
   * (0, 0, 0) to avoid division by zero
   */
  XMC_INLINE vec3 transform(const vec3 &v) const {
    float x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
    float y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
    float z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
    float w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15];
    float abs_w = fabsf(w);
    if (abs_w < 1e-8f) {
      return vec3(0, 0, 0);
    }
    float inv_w = 1.0f / abs_w;
    return vec3(x * inv_w, y * inv_w, z * inv_w);
  }

  XMC_INLINE vec3 transformNormal(const vec3 &v) const {
    float nx = m[0] * v.x + m[4] * v.y + m[8] * v.z;
    float ny = m[1] * v.x + m[5] * v.y + m[9] * v.z;
    float nz = m[2] * v.x + m[6] * v.y + m[10] * v.z;
    return vec3(nx, ny, nz).normalized();
  }

  XMC_INLINE void translate(const vec3 &t) {
    m[12] += t.x;
    m[13] += t.y;
    m[14] += t.z;
  }

  XMC_INLINE void translate(float x, float y, float z) {
    m[12] += x;
    m[13] += y;
    m[14] += z;
  }

  void rotate(const quat &q) { *this = fromQuat(q) * (*this); }

  void rotate(float pitch, float yaw, float roll) {
    rotate(quat::fromEuler(pitch, yaw, roll));
  }

  void rotate(const vec3 &axis, float angle) {
    rotate(quat::fromAxisAngle(axis, angle));
  }

  XMC_INLINE void scale(const vec3 &s) {
    m[0] *= s.x;
    m[5] *= s.y;
    m[10] *= s.z;
  }

  XMC_INLINE void scale(float s) {
    m[0] *= s;
    m[5] *= s;
    m[10] *= s;
  }

  void transpose() {
    float tmp;
    for (int i = 0; i < 4; i++) {
      for (int j = i + 1; j < 4; j++) {
        tmp = m[i * 4 + j];
        m[i * 4 + j] = m[j * 4 + i];
        m[j * 4 + i] = tmp;
      }
    }
  }
};

}  // namespace xmc

#endif
