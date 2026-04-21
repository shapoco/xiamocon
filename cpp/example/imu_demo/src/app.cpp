#include "xiamocon.hpp"

#include <stdio.h>

#include "earth_cloud.hpp"
#include "earth_surface.hpp"
#include "lsm6dsv16x.hpp"
#include "xmc/font/ShapoSansP_s08c07.h"

namespace xmc {

static constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB565;

static lsm6dsv16x::SensorI2C imu;
static quat imuPos;

FrameBuffer frameBuffer = createFrameBuffer(DISPLAY_FORMAT, true);

static Mesh3D earth = createSphere(1.0f, 18, 9);
static Sprite surfaceTexture =
    createSprite565(256, 128, (void *)earthSurfaceData);
static Sprite cloudTexture = createSprite4444(256, 128, (void *)earthCloudData);
static Graphics3D g3d = createGraphics3D(display::WIDTH, display::HEIGHT);

static uint64_t lastImuUpdateUs = 0;

static void updateImuPosition(quat *p, float *imu_values, float dt);
static void createProjectionMatrix(mat4 *M, const quat *Q, const vec3 *E,
                                   float rw, float rh, int sw, int sh);
static void updateScene();
static void renderScene();

float yaw = 0, pitch = 0;
float vyaw = 0, vpitch = 0;

AppConfig appGetConfig() {
  auto cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  return cfg;
}

void appSetup() {
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);
  imu.init();
  imuPos = {1, 0, 0, 0};
  Material3D earthMaterial = createMaterial3D();
  earthMaterial->colorTexture = surfaceTexture;
  earth->setMaterial(earthMaterial);
}

void appLoop() {
  updateScene();

  frameBuffer->beginRender();
  renderScene();
  frameBuffer->endRender();
}

static void updateScene() {
  float values[7] = {0};
  uint64_t now_us = getTimeUs();
  float dt = 0.0f;
  if (lastImuUpdateUs != 0) {
    dt = (float)(now_us - lastImuUpdateUs) * 1e-6f;
  }
  lastImuUpdateUs = now_us;

  XmcStatus sts = imu.read_sensor(values);
  if (sts == XMC_OK && dt > 0) {
    for (int i = 4; i <= 6; i++) {
      values[i] = -values[i];
    }
    updateImuPosition(&imuPos, values, dt);
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
}

static void renderScene() {
  Graphics2D gfx = frameBuffer->createGraphics();

  gfx->clear(0);

  mat4 proj;
  vec3 eye_pos = {0, 0.1f, 0.15f};
  createProjectionMatrix(&proj, &imuPos, &eye_pos, 0.03f, 0.03f, display::WIDTH,
                         display::HEIGHT);
  g3d->setTarget(frameBuffer->getBackBuffer());
  g3d->beginRender();

  g3d->setDepthRange(-1.0f, 1.0f);

  vec3 light_dir = {1, 0, 1};
  light_dir = imuPos.conjugate().rotate(light_dir);
  g3d->setParallelLight(light_dir, colorf(2.0f, 2.0f, 2.0f, 1));
  g3d->setEnvironmentLight(colorf(0.1f, 0.1f, 0.1f, 1));

  g3d->setScreenMatrix(mat4::identity());
  g3d->setProjection(proj);
  g3d->setBlendMode(BlendMode::OVERWRITE);

  earth->primitives[0]->material->colorTexture = surfaceTexture;
  earth->primitives[0]->material->baseColor.a = 1;
  g3d->pushState();
  g3d->scale(0.01f);
  g3d->rotate(pitch, yaw, 0);
  g3d->render(earth);
  g3d->popState();

#if 0
  earth->primitives[0]->material->colorTexture = cloudTexture;
  earth->primitives[0]->material->baseColor.a = 0.75f;
  g3d->pushState();
  g3d->scale(0.0105f);
  g3d->rotate(0, getTimeMs() * 0.0001f, 0);
  g3d->rotate(pitch, yaw, 0);
  g3d->setBlendMode(BlendMode::ALPHA_BLEND);
  g3d->render(earth);
  g3d->setBlendMode(BlendMode::OVERWRITE);
  g3d->popState();
#endif

  g3d->endRender();
}

static void updateImuPosition(quat *p, float *imu_values, float dt) {
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

  float halfDT = dt * 0.5f;
  quat qGyro(1.0f, gx * halfDT, gy * halfDT, gz * halfDT);
  *p = (*p * qGyro).normalized();

  // Tilt correction using accelerometer (correct pitch/roll based on gravity
  // direction)
  float aLen = sqrtf(ax * ax + ay * ay + az * az);
  if (aLen > 0.9f && aLen < 1.1f) {
    // Estimate gravity direction from acceleration (body frame)
    vec3 aMeas(ax / aLen, ay / aLen, az / aLen);

    // Expected gravity direction from current attitude (body frame)
    vec3 gWorld(0, 0, 1);
    vec3 gBody = p->conjugate().rotate(gWorld);

    // Calculate correction rotation from deviation between measured and
    // expected values
    vec3 cross = aMeas.cross(gBody);
    float crossLen = cross.length();
    float dot = aMeas.dot(gBody);

    if (crossLen > 1e-6f) {
      float errAngle = atan2f(crossLen, dot);
      constexpr float ALPHA = 0.02f;
      quat qCorr =
          quat::fromAxisAngle(cross * (1.0f / crossLen), errAngle * ALPHA);
      *p = (qCorr * (*p)).normalized();
    }
  }
}

static void createProjectionMatrix(mat4 *M, const quat *Q, const vec3 *E,
                                   float rw, float rh, int sw, int sh) {
  // Transform viewpoint from world space to virtual space (display local
  // coordinate system)
  vec3 e = Q->conjugate().rotate(*E);

  float pxPerMX = (float)sw / rw;  // pixels per meter in x direction
  float pxPerMY = (float)sh / rh;  // pixels per meter in y direction
  float hw = (float)sw * 0.5f;     // screen half-width (px)
  float hh = (float)sh * 0.5f;     // screen half-height (px)

  // Build a matrix to transform virtual space vertex v to screen coordinates
  // with perspective division Utilizing that transform returns (rx/rw,
  // ry/rw, rz/rw), realize perspective projection with rw = e.z - v.z
  mat4 result;

  // Column 0 (coefficient of vx)
  result.m[0] = e.z * pxPerMX;

  // Column 1 (coefficient of vy)
  result.m[5] = -e.z * pxPerMY;

  // Column 2 (coefficient of vz)
  result.m[8] = -e.x * pxPerMX - hw;
  result.m[9] = -e.y * pxPerMY - hh;
  result.m[10] = -1.0f;
  result.m[11] = -1.0f;

  // Column 3 (constant term)
  result.m[12] = e.z * hw;
  result.m[13] = e.z * hh;
  result.m[15] = e.z;

  *M = result;
}

}  // namespace xmc
