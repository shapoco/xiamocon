#include "grass.hpp"

using namespace xmc;

Vec3Buffer leafVerts = createVec3Buffer(3);
ColorBuffer leafColors = createColorBuffer(3);
Material3D leafMat = createMaterial3D();
Primitive3D leafPrim =
    createPrimitive3D(PrimitiveMode::TRIANGLES, leafVerts, nullptr, leafColors,
                      nullptr, nullptr, leafMat);

static constexpr int NUM_LEAFS = 500;
struct Leaf {
  vec3 pos;
  vec3 rot;
  vec3 dir;
};
Leaf leafArray[NUM_LEAFS];

void setupGrass() {
  leafColors->data[0] = {0, 0, 0, 1};
  leafColors->data[1] = {0, 0, 0, 1};
  leafColors->data[2] = {0, 1, 0, 1};
  leafMat->flags |= MaterialFlags3D::DOUBLE_SIDED;
  for (int i = 0; i < NUM_LEAFS; i++) {
    // distribute within a circle of radius R
    const float R = 7.0f;
    do {
      vec3 pos(randomF32() * 2 - 1, 0, randomF32() * 2 - 1);
      pos *= R * pow(pos.length(), 1.1f);
      leafArray[i].pos = pos;
    } while (leafArray[i].pos.length() > R);

    // Random rotation
    float a = randomF32() * 2 * M_PI;
    leafArray[i].rot = vec3(sinf(a) * 0.2f, 0, cosf(a) * 0.2f);

    // Random direction
    leafArray[i].dir = {
        randomF32() * 2 - 1,
        1.0f + powf(randomF32(), 2),
        randomF32() * 2 - 1,
    };
  }
}

void renderGrass(Graphics3D &g3d) {
  g3d->pushState();
  for (int i = 0; i < NUM_LEAFS; i++) {
    leafVerts->data[0] = leafArray[i].pos - leafArray[i].rot;
    leafVerts->data[1] = leafArray[i].pos + leafArray[i].rot;
    leafVerts->data[2] = leafArray[i].pos + leafArray[i].dir;
    g3d->render(leafPrim);
  }
  g3d->popState();
}