#include "xmc/app.h"
#include "xmc/audio.hpp"
#include "xmc/display.h"
#include "xmc/geo.hpp"
#include "xmc/gfx.hpp"
#include "xmc/hw/timer.h"
#include "xmc/input.h"
#include "xmc/speaker.h"

#include <stdio.h>

#include "lsm6dsv16x.hpp"
#include "xmc/font/ShapoSansP_s08c07.h"

static lsm6dsv16x::SensorI2C imu;
static xmc::quat imu_pos;

static xmc::Sprite444 frame_buffer(XMC_DISPLAY_WIDTH, XMC_DISPLAY_HEIGHT);
static uint32_t frame_count = 0;
static uint64_t last_imu_update_us = 0;
static xmc_status_t last_error = XMC_OK;

static void update_imu_position(xmc::quat *p, float *imu_values, float dt);
static void create_projection_matrix(xmc::mat4 *M, const xmc::quat *Q,
                                     const xmc::vec3 *E, float rw, float rh,
                                     int sw, int sh);

xmc_app_config_t xmc_app_get_config() { return xmc_get_default_app_config(); }

void xmc_app_setup() {
  frame_buffer.clear(0);
  imu.init();

  imu_pos = {1, 0, 0, 0};
}

void xmc_app_loop() {
  float values[7] = {0};
  uint64_t now_us = xmc_get_time_us();
  float dt = 0.0f;
  if (last_imu_update_us != 0) {
    dt = (float)(now_us - last_imu_update_us) * 1e-6f;
  }
  last_imu_update_us = now_us;

  xmc_status_t sts = imu.read_sensor(values);
  if (sts != XMC_OK) {
    last_error = sts;
  } else if (dt > 0) {
    for (int i = 4; i <= 6; i++) {
      values[i] = -values[i];
    }
    update_imu_position(&imu_pos, values, dt);
  }

  // complete the previous frame's transfer if it's still in progress, then
  // fill the frame buffer with the new frame's content. In this case, we just
  // draw a moving box, but you can draw anything you want here.
  frame_buffer.complete_transfer();

  frame_buffer.clear(0);

  frame_buffer.set_text_color(0xFFF);
  frame_buffer.set_font(&ShapoSansP_s08c07, 1);

  if (true) {
    char buf[64];
    frame_buffer.set_cursor(10, 20);
    snprintf(buf, sizeof(buf), "Frame: %lu\n", frame_count);
    frame_buffer.draw_string(buf);
    snprintf(buf, sizeof(buf), "Last error: 0x%08lx\n", (uint32_t)last_error);
    frame_buffer.draw_string(buf);
    snprintf(buf, sizeof(buf), "Temp: %.2f C\n", values[0]);
    frame_buffer.draw_string(buf);
    snprintf(buf, sizeof(buf), "Gyro: %.2f, %.2f, %.2f dps\n", values[1],
             values[2], values[3]);
    frame_buffer.draw_string(buf);
    snprintf(buf, sizeof(buf), "Accel: %.2f, %.2f, %.2f G\n", values[4],
             values[5], values[6]);
    frame_buffer.draw_string(buf);
    float roll, pitch, yaw;
    imu_pos.to_euler(&pitch, &roll, &yaw);
    roll *= 180.0f / M_PI;
    pitch *= 180.0f / M_PI;
    yaw *= 180.0f / M_PI;
    snprintf(buf, sizeof(buf), "Rot: %.2f, %.2f, %.2f deg\n", pitch, roll, yaw);
    frame_buffer.draw_string(buf);
  }

  if (false) {
    char buf[32];
    frame_buffer.set_cursor(0, 20);
    snprintf(buf, sizeof(buf), "Frame: %lu\n", frame_count);
    frame_buffer.draw_string(buf);
    for (int i = 0; i < 16; i++) {
      frame_buffer.set_cursor(12 + i * 14, 40);
      snprintf(buf, sizeof(buf), "%x\n", i);
      frame_buffer.draw_string(buf);
      frame_buffer.set_cursor(0, 55 + i * 10);
      snprintf(buf, sizeof(buf), "%x\n", i);
      frame_buffer.draw_string(buf);
    }
    for (int reg = 0; reg < 256; reg++) {
      uint8_t value;
      frame_buffer.set_cursor(12 + (reg % 16) * 14, 55 + (reg / 16) * 10);
      if (imu.read_reg((lsm6dsv16x::reg_t)reg, &value) == XMC_OK) {
        snprintf(buf, sizeof(buf), "%02X\n", value);
      } else {
        snprintf(buf, sizeof(buf), "--\n");
      }
      frame_buffer.draw_string(buf);
    }
  }

  if (false) {
    int h = 8;
    int y = XMC_DISPLAY_HEIGHT - h * 6;
    frame_buffer.fill_rect(120, y, values[1] * 120 / 250, h, 0xF00);
    y += h;
    frame_buffer.fill_rect(120, y, values[2] * 120 / 250, h, 0x0F0);
    y += h;
    frame_buffer.fill_rect(120, y, values[3] * 120 / 250, h, 0x00F);
    y += h;
    frame_buffer.fill_rect(120, y, values[4] * 120 / 4, h, 0xF00);
    y += h;
    frame_buffer.fill_rect(120, y, values[5] * 120 / 4, h, 0x0F0);
    y += h;
    frame_buffer.fill_rect(120, y, values[6] * 120 / 4, h, 0x00F);
  }

  if (true) {
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if ((i + j) % 2 == 0) {
          frame_buffer.fill_rect(i * 30, j * 30, 40, 40, 0x444);
        } else {
          frame_buffer.fill_rect(i * 30, j * 30, 40, 40, 0x888);
        }
      }
    }

    xmc::mat4 proj;
    xmc::vec3 eye_pos = {0, 0.1f, 0.2f};
    create_projection_matrix(&proj, &imu_pos, &eye_pos, 0.03f, 0.03f,
                             XMC_DISPLAY_WIDTH, XMC_DISPLAY_HEIGHT);

    float s = 0.005f;
    xmc::vec3 verts[8] = {
        {-s, -s, -s}, {s, -s, -s}, {s, s, -s}, {-s, s, -s},
        {-s, -s, s},  {s, -s, s},  {s, s, s},  {-s, s, s},
    };
    for (int i = 0; i < 8; i++) {
      verts[i].z += s;
    }
    xmc::vec3 projected[8];

    int faces[4 * 6] = {
        0, 1, 2, 3,  // back
        4, 5, 6, 7,  // front
        0, 1, 5, 4,  // bottom
        2, 3, 7, 6,  // top
        1, 2, 6, 5,  // right
        4, 7, 3, 0,  // left
    };

    for (int i = 0; i < 8; i++) {
      projected[i] = proj.transform_point(verts[i]);
    }

    int faces_order[6] = {0, 1, 2, 3, 4, 5};
    // Sort faces_order in drawing order (Painter's algorithm)
    for (int i = 0; i < 6; i++) {
      for (int j = i + 1; j < 6; j++) {
        float z_i = 0;
        float z_j = 0;
        for (int k = 0; k < 4; k++) {
          z_i += projected[faces[faces_order[i] * 4 + k]].z;
          z_j += projected[faces[faces_order[j] * 4 + k]].z;
        }
        if (z_i > z_j) {
          int t = faces_order[i];
          faces_order[i] = faces_order[j];
          faces_order[j] = t;
        }
      }
    }

    uint16_t colors[6] = {0xF00, 0xF00, 0x0F0, 0x0F0, 0x00F, 0x00F};
    for (int i = 0; i < 6; i++) {
      int j = faces_order[i];
      int idxs[6] = {
          faces[j * 4 + 0], faces[j * 4 + 1], faces[j * 4 + 2],
          faces[j * 4 + 0], faces[j * 4 + 2], faces[j * 4 + 3],
      };
      frame_buffer.fill_triangle(projected, idxs, 0, colors[j]);
      frame_buffer.fill_triangle(projected, idxs, 3, colors[j]);
    }
  }

  // start transferring the current frame to the display. This will return
  // immediately and the transfer will happen in the background.
  frame_buffer.start_transfer_to_display(0, 0);

  frame_count++;
}

static void update_imu_position(xmc::quat *p, float *imu_values, float dt) {
  constexpr float DEG_TO_RAD = (float)(M_PI / 180.0);
  float gx = imu_values[1] * DEG_TO_RAD;
  float gy = imu_values[2] * DEG_TO_RAD;
  float gz = imu_values[3] * DEG_TO_RAD;
  float ax = imu_values[4];
  float ay = imu_values[5];
  float az = imu_values[6];

  // --- IMU position update (complementary filter) ---

  // Gyroscope integration: convert dps to rad/s
  // and generate rotation quaternion using small angle approximation

  float half_dt = dt * 0.5f;
  xmc::quat q_gyro(1.0f, gx * half_dt, gy * half_dt, gz * half_dt);
  *p = (*p * q_gyro).normalized();

  // Tilt correction using accelerometer (correct pitch/roll based on gravity
  // direction)
  float a_len = sqrtf(ax * ax + ay * ay + az * az);
  if (a_len > 0.9f && a_len < 1.1f) {
    // Estimate gravity direction from acceleration (body frame)
    xmc::vec3 a_meas(ax / a_len, ay / a_len, az / a_len);

    // Expected gravity direction from current attitude (body frame)
    xmc::vec3 g_world(0, 0, 1);
    xmc::vec3 g_body = p->conjugate().rotate(g_world);

    // Calculate correction rotation from deviation between measured and
    // expected values
    xmc::vec3 cross = a_meas.cross(g_body);
    float cross_len = cross.length();
    float dot = a_meas.dot(g_body);

    if (cross_len > 1e-6f) {
      float err_angle = atan2f(cross_len, dot);
      constexpr float ALPHA = 0.02f;
      xmc::quat q_corr = xmc::quat::from_axis_angle(cross * (1.0f / cross_len),
                                                    err_angle * ALPHA);
      *p = (q_corr * (*p)).normalized();
    }
  }
}

static void create_projection_matrix(xmc::mat4 *M, const xmc::quat *Q,
                                     const xmc::vec3 *E, float rw, float rh,
                                     int sw, int sh) {
  // Transform viewpoint from world space to virtual space (display local
  // coordinate system)
  xmc::vec3 e = Q->conjugate().rotate(*E);

  float px_per_m_x = (float)sw / rw;  // pixels per meter in x direction
  float px_per_m_y = (float)sh / rh;  // pixels per meter in y direction
  float hw = (float)sw * 0.5f;        // screen half-width (px)
  float hh = (float)sh * 0.5f;        // screen half-height (px)

  // Build a matrix to transform virtual space vertex v to screen coordinates
  // with perspective division Utilizing that transform_point returns (rx/rw,
  // ry/rw, rz/rw), realize perspective projection with rw = e.z - v.z
  xmc::mat4 result;

  // Column 0 (coefficient of vx)
  result.m[0] = e.z * px_per_m_x;

  // Column 1 (coefficient of vy)
  result.m[5] = e.z * px_per_m_y;

  // Column 2 (coefficient of vz)
  result.m[8] = -e.x * px_per_m_x - hw;
  result.m[9] = -e.y * px_per_m_y - hh;
  result.m[10] = 1.0f;
  result.m[11] = -1.0f;

  // Column 3 (constant term)
  result.m[12] = e.z * hw;
  result.m[13] = e.z * hh;
  result.m[15] = e.z;

  *M = result;
}
