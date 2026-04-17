#include "grass.hpp"

using namespace xmc;

static constexpr int NUM_LEAFS = 500;

Vec3Buffer leafVerts = createVec3Buffer(NUM_LEAFS * 3);
ColorBuffer leafColors = createColorBuffer(NUM_LEAFS * 3);
Material3D leafMat = createMaterial3D();
Primitive3D leafPrim =
    createPrimitive3D(PrimitiveMode::TRIANGLES, leafVerts, nullptr, leafColors,
                      nullptr, nullptr, leafMat);

void setupGrass() {
  leafMat->flags |= MaterialFlags3D::DOUBLE_SIDED;
  for (int i = 0; i < NUM_LEAFS; i++) {
    vec3 pos;
    // distribute within a circle of radius R
    const float R = 8.0f;
    do {
      pos.x = R * (randomF32() * 2 - 1);
      pos.y = 0;
      pos.z = R * (randomF32() * 2 - 1);
    } while (pos.length() > R);

    // Random rotation
    float a = randomF32() * 2 * M_PI;
    vec3 rot = vec3(sinf(a) * 0.2f, 0, cosf(a) * 0.2f);

    // Random direction
    vec3 dir = {
        randomF32() * 2 - 1,
        1.0f + powf(randomF32(), 2),
        randomF32() * 2 - 1,
    };

    leafVerts->data[i * 3 + 0] = pos - rot;
    leafVerts->data[i * 3 + 1] = pos + rot;
    leafVerts->data[i * 3 + 2] = pos + dir;
    leafColors->data[i * 3 + 0] = {0, 0, 0, 1};
    leafColors->data[i * 3 + 1] = {0, 0, 0, 1};
    leafColors->data[i * 3 + 2] = {0, 1, 0, 1};
  }
}

void renderGrass(Graphics3D &g3d) {
  g3d->pushState();
  g3d->render(leafPrim);
  g3d->popState();
}