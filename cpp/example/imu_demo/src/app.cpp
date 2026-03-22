#include "xmc/app.hpp"
#include "xmc/audio.hpp"
#include "xmc/display.hpp"
#include "xmc/geo.hpp"
#include "xmc/gfx.hpp"
#include "xmc/hw/timer.hpp"
#include "xmc/input.hpp"
#include "xmc/speaker.hpp"

#include <stdio.h>

#include "bmp_earth.hpp"
#include "lsm6dsv16x.hpp"
#include "xmc/font/ShapoSansP_s08c07.h"

namespace xmc {

static lsm6dsv16x::SensorI2C imu;
static quat imu_pos;

static Sprite screen = create_sprite565(display::WIDTH, display::HEIGHT);
static Mesh cube = createColoredCube();
static Mesh sphere = createSphere(1.0f, 18, 9);
static Sprite earth_texture =
    createSprite4444(256, 128, 0, (void *)bmp_earth_data);
static Rasterizer rasterizer =
    createRasterizer(display::WIDTH, display::HEIGHT);
static uint32_t frame_count = 0;
static uint64_t last_imu_update_us = 0;
static XmcStatus last_error = XMC_OK;

static void update_imu_position(quat *p, float *imu_values, float dt);
static void create_projection_matrix(mat4 *M, const quat *Q, const vec3 *E,
                                     float rw, float rh, int sw, int sh);

float yaw = 0, pitch = 0;
float vyaw = 0, vpitch = 0;

AppConfig appGetConfig() {
  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = display::InterfaceFormat::RGB565;
  return cfg;
}

void appSetup() {
  screen->clear(0);
  imu.init();
  imu_pos = {1, 0, 0, 0};
  sphere->material = createMaterial();
  sphere->material->colorTexture = earth_texture;
}

void appLoop() {
  float values[7] = {0};
  uint64_t now_us = getTimeUs();
  float dt = 0.0f;
  if (last_imu_update_us != 0) {
    dt = (float)(now_us - last_imu_update_us) * 1e-6f;
  }
  last_imu_update_us = now_us;

  XmcStatus sts = imu.read_sensor(values);
  if (sts != XMC_OK) {
    last_error = sts;
  } else if (dt > 0) {
    for (int i = 4; i <= 6; i++) {
      values[i] = -values[i];
    }
    update_imu_position(&imu_pos, values, dt);
  }

  if (input::isPressed(input::Button::LEFT)) {
    vyaw += dt;
  } else if (input::isPressed(input::Button::RIGHT)) {
    vyaw -= dt;
  }
  if (input::isPressed(input::Button::UP)) {
    vpitch -= dt;
  } else if (input::isPressed(input::Button::DOWN)) {
    vpitch += dt;
  }
  vyaw *= 0.98f;
  vpitch *= 0.98f;
  yaw += vyaw * dt;
  pitch += vpitch * dt;

  // complete the previous frame's transfer if it's still in progress, then
  // fill the frame buffer with the new frame's content. In this case, we just
  // draw a moving box, but you can draw anything you want here.
  screen->completeTransfer();

  screen->clear(0);

  screen->setTextColor(0xFFF);
  screen->setFont(&ShapoSansP_s08c07, 1);

  if (true) {
    char buf[64];
    screen->setCursor(10, 20);
    snprintf(buf, sizeof(buf), "Frame: %lu\n", frame_count);
    screen->drawString(buf);
    snprintf(buf, sizeof(buf), "Last error: 0x%08lx\n", (uint32_t)last_error);
    screen->drawString(buf);
    snprintf(buf, sizeof(buf), "Temp: %.2f C\n", values[0]);
    screen->drawString(buf);
    snprintf(buf, sizeof(buf), "Gyro: %.2f, %.2f, %.2f dps\n", values[1],
             values[2], values[3]);
    screen->drawString(buf);
    snprintf(buf, sizeof(buf), "Accel: %.2f, %.2f, %.2f G\n", values[4],
             values[5], values[6]);
    screen->drawString(buf);
    float roll, pitch, yaw;
    imu_pos.toEuler(&pitch, &roll, &yaw);
    roll *= 180.0f / M_PI;
    pitch *= 180.0f / M_PI;
    yaw *= 180.0f / M_PI;
    snprintf(buf, sizeof(buf), "Rot: %.2f, %.2f, %.2f deg\n", pitch, roll, yaw);
    screen->drawString(buf);
  }

  if (false) {
    char buf[32];
    screen->setCursor(0, 20);
    snprintf(buf, sizeof(buf), "Frame: %lu\n", frame_count);
    screen->drawString(buf);
    for (int i = 0; i < 16; i++) {
      screen->setCursor(12 + i * 14, 40);
      snprintf(buf, sizeof(buf), "%x\n", i);
      screen->drawString(buf);
      screen->setCursor(0, 55 + i * 10);
      snprintf(buf, sizeof(buf), "%x\n", i);
      screen->drawString(buf);
    }
    for (int reg = 0; reg < 256; reg++) {
      uint8_t value;
      screen->setCursor(12 + (reg % 16) * 14, 55 + (reg / 16) * 10);
      if (imu.read_reg((lsm6dsv16x::reg_t)reg, &value) == XMC_OK) {
        snprintf(buf, sizeof(buf), "%02X\n", value);
      } else {
        snprintf(buf, sizeof(buf), "--\n");
      }
      screen->drawString(buf);
    }
  }

  if (false) {
    int h = 8;
    int y = display::HEIGHT - h * 6;
    screen->fillRect(120, y, values[1] * 120 / 250, h, 0xF00);
    y += h;
    screen->fillRect(120, y, values[2] * 120 / 250, h, 0x0F0);
    y += h;
    screen->fillRect(120, y, values[3] * 120 / 250, h, 0x00F);
    y += h;
    screen->fillRect(120, y, values[4] * 120 / 4, h, 0xF00);
    y += h;
    screen->fillRect(120, y, values[5] * 120 / 4, h, 0x0F0);
    y += h;
    screen->fillRect(120, y, values[6] * 120 / 4, h, 0x00F);
  }

  if (false) {
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        if ((i + j) % 2 == 0) {
          // screen->fill_rect(i * 30, j * 30, 40, 40, 0x444);
          screen->fillRect(i * 30, j * 30, 40, 40, rgb565(8, 16, 8));
        } else {
          // screen->fill_rect(i * 30, j * 30, 40, 40, 0x888);
          screen->fillRect(i * 30, j * 30, 40, 40, rgb565(16, 32, 16));
        }
      }
    }

    mat4 proj;
    vec3 eye_pos = {0, 0.1f, 0.2f};
    create_projection_matrix(&proj, &imu_pos, &eye_pos, 0.03f, 0.03f,
                             display::WIDTH, display::HEIGHT);

    float s = 0.005f;
    vec3 verts[8] = {
        {-s, -s, -s}, {s, -s, -s}, {s, s, -s}, {-s, s, -s},
        {-s, -s, s},  {s, -s, s},  {s, s, s},  {-s, s, s},
    };
    for (int i = 0; i < 8; i++) {
      verts[i].z += s;
    }
    vec3 projected[8];

    int faces[4 * 6] = {
        0, 1, 2, 3,  // back
        4, 5, 6, 7,  // front
        0, 1, 5, 4,  // bottom
        2, 3, 7, 6,  // top
        1, 2, 6, 5,  // right
        4, 7, 3, 0,  // left
    };

    for (int i = 0; i < 8; i++) {
      projected[i] = proj.transform(verts[i]);
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

    // uint16_t colors[6] = {0xF00, 0xF00, 0x0F0, 0x0F0, 0x00F, 0x00F};
    uint16_t colors[6] = {rgb565(31, 0, 0), rgb565(31, 0, 0), rgb565(0, 63, 0),
                          rgb565(0, 63, 0), rgb565(0, 0, 31), rgb565(0, 0, 31)};
    for (int i = 0; i < 6; i++) {
      int j = faces_order[i];
      int idxs[6] = {
          faces[j * 4 + 0], faces[j * 4 + 1], faces[j * 4 + 2],
          faces[j * 4 + 0], faces[j * 4 + 2], faces[j * 4 + 3],
      };
      screen->fill_triangle(projected, idxs, 0, colors[j]);
      screen->fill_triangle(projected, idxs, 3, colors[j]);
    }
  }

  if (true) {
    mat4 proj;
    vec3 eye_pos = {0, 0.1f, 0.15f};
    create_projection_matrix(&proj, &imu_pos, &eye_pos, 0.03f, 0.03f,
                             display::WIDTH, display::HEIGHT);
    // proj.loadIdentity();
    // proj.scale(10);
    // proj.translate(120, 120, 0);
    float a = (float)now_us * 0.0000005f;
    float t = 0.004f;

    rasterizer->setTarget(screen);
    rasterizer->clearDepth();
    rasterizer->setDepthRange(-1.0f, 1.0f);

    vec3 light_dir = {0, 0.5f, 1.0f};
    light_dir = imu_pos.conjugate().rotate(light_dir);
    rasterizer->setParallelLight(light_dir, colorf(1.5f, 1.5f, 1.5f, 1));

    rasterizer->setProjection(proj);
    rasterizer->loadIdentity();

    // int n = 5;
    // for (int i = 0; i < n; i++) {
    //   float tx = -t + i * t * 2 / (n - 1);
    //   for (int j = 0; j < n; j++) {
    //     float ty = -t + j * t * 2 / (n - 1);
    //     float tz = sinf(sqrt(tx * tx + ty * ty) * 256 + now_us * 0.000005f) *
    //     0.003f; rasterizer->pushMatrix(); rasterizer->rotate(i * M_PI / 2, j
    //     * M_PI / 2, 0); rasterizer->translate(tx, ty, tz);
    //     rasterizer->renderMesh(cube);
    //     rasterizer->popMatrix();
    //   }
    // }

    // int n = 2;
    // for (int i = 0; i < n; i++) {
    //   float tx = -t + i * t * 2 / (n - 1);
    //   for (int j = 0; j < n; j++) {
    //     float ty = -t + j * t * 2 / (n - 1);
    //     for (int k = 0; k < n; k++) {
    //       float tz = -t + k * t * 2 / (n - 1);
    //       rasterizer->pushMatrix();
    //       rasterizer->scale(t / n * 1.5f);
    //       rasterizer->translate(tx, ty, tz);
    //       rasterizer->rotate(pitch, 0, yaw);
    //       rasterizer->renderMesh(cube);
    //       rasterizer->popMatrix();
    //     }
    //   }
    // }

    rasterizer->pushMatrix();
    rasterizer->scale(0.009f);
    rasterizer->rotate(0, M_PI / 2, 0);
    rasterizer->rotate(pitch, 0, yaw);
    rasterizer->renderMesh(sphere);
    rasterizer->popMatrix();

    // rasterizer->pushMatrix();
    // rasterizer->rotate(a, 0, 0);
    // rasterizer->translate(-t, -t, 0);
    // rasterizer->renderMesh(cube);
    // rasterizer->popMatrix();
    //
    // rasterizer->pushMatrix();
    // rasterizer->rotate(0, a, 0);
    // rasterizer->translate(t, -t, 0);
    // rasterizer->renderMesh(cube);
    // rasterizer->popMatrix();
    //
    // rasterizer->pushMatrix();
    // rasterizer->rotate(0, 0, a);
    // rasterizer->translate(-t, t, 0);
    // rasterizer->renderMesh(cube);
    // rasterizer->popMatrix();
    //
    // rasterizer->pushMatrix();
    // rasterizer->rotate(a, a, a);
    // rasterizer->translate(t, t, 0);
    // rasterizer->renderMesh(cube);
    // rasterizer->popMatrix();
  }

  // render status bar
  appDrawStatusBar(screen);
  appDrawDebugInfo(screen);

  // start transferring the current frame to the display. This will return
  // immediately and the transfer will happen in the background.
  screen->startTransferToDisplay(0, 0);

  frame_count++;
}

static void update_imu_position(quat *p, float *imu_values, float dt) {
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
  quat q_gyro(1.0f, gx * half_dt, gy * half_dt, gz * half_dt);
  *p = (*p * q_gyro).normalized();

  // Tilt correction using accelerometer (correct pitch/roll based on gravity
  // direction)
  float a_len = sqrtf(ax * ax + ay * ay + az * az);
  if (a_len > 0.9f && a_len < 1.1f) {
    // Estimate gravity direction from acceleration (body frame)
    vec3 a_meas(ax / a_len, ay / a_len, az / a_len);

    // Expected gravity direction from current attitude (body frame)
    vec3 g_world(0, 0, 1);
    vec3 g_body = p->conjugate().rotate(g_world);

    // Calculate correction rotation from deviation between measured and
    // expected values
    vec3 cross = a_meas.cross(g_body);
    float cross_len = cross.length();
    float dot = a_meas.dot(g_body);

    if (cross_len > 1e-6f) {
      float err_angle = atan2f(cross_len, dot);
      constexpr float ALPHA = 0.02f;
      quat q_corr =
          quat::fromAxisAngle(cross * (1.0f / cross_len), err_angle * ALPHA);
      *p = (q_corr * (*p)).normalized();
    }
  }
}

static void create_projection_matrix(mat4 *M, const quat *Q, const vec3 *E,
                                     float rw, float rh, int sw, int sh) {
  // Transform viewpoint from world space to virtual space (display local
  // coordinate system)
  vec3 e = Q->conjugate().rotate(*E);

  float px_per_m_x = (float)sw / rw;  // pixels per meter in x direction
  float px_per_m_y = (float)sh / rh;  // pixels per meter in y direction
  float hw = (float)sw * 0.5f;        // screen half-width (px)
  float hh = (float)sh * 0.5f;        // screen half-height (px)

  // Build a matrix to transform virtual space vertex v to screen coordinates
  // with perspective division Utilizing that transform returns (rx/rw,
  // ry/rw, rz/rw), realize perspective projection with rw = e.z - v.z
  mat4 result;

  // Column 0 (coefficient of vx)
  result.m[0] = e.z * px_per_m_x;

  // Column 1 (coefficient of vy)
  result.m[5] = e.z * px_per_m_y;

  // Column 2 (coefficient of vz)
  result.m[8] = -e.x * px_per_m_x - hw;
  result.m[9] = -e.y * px_per_m_y - hh;
  result.m[10] = -1.0f;
  result.m[11] = -1.0f;

  // Column 3 (constant term)
  result.m[12] = e.z * hw;
  result.m[13] = e.z * hh;
  result.m[15] = e.z;

  *M = result;
}

}  // namespace xmc
