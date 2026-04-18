#include "cubes.hpp"

#include "cave_env_texture.hpp"

using namespace xmc;

Sprite envTexture = createSprite565(256, 128, (void *)caveEnvTextureData);
Material3D envMat = createMaterial3D();
Mesh3D cube = createCube(0.5f);

void setupCubes() {
  // Set up environment mapping material for the cubes
  envMat->flags |= MaterialFlags3D::ENVIRONMENT_MAPPED;
  envMat->colorTexture = envTexture;
  cube->setMaterial(envMat);
}

void renderCubes(Graphics3D &g3d) {
  uint64_t nowMs = getTimeMs();
  g3d->pushState();
  g3d->disableFlags(RenderFlags3D::GOURAUD_SHADING);

  float largeRot = (float)nowMs * 0.0002f;
  g3d->rotate(largeRot * 1.1f, largeRot * 1.2f, largeRot * 1.3f);
  g3d->translate(0, 3.3f, 0);
  for (int iz = 0; iz < 2; iz++) {
    for (int iy = 0; iy < 2; iy++) {
      for (int ix = 0; ix < 2; ix++) {
        g3d->pushState();

        // rotation animation based on time and cube index
        int i = ix + iy * 2 + iz * 4;
        float t = ((float)nowMs * 0.0002f);
        int tInt = (int)floorf(t);
        int axis = tInt % 3;
        float p = (t - tInt) * 2;
        if (tInt % 8 == i && p < 1.0f) {
          if (p < 0.5f) {
            p *= 2;
            p *= p;
            p /= 2;
          } else {
            p = (1 - p) * 2;
            p *= p;
            p = 1 - p / 2;
          }
          float rot = p * (M_PI / 2);
          switch (axis) {
            case 0: g3d->rotate(rot, 0, 0); break;
            case 1: g3d->rotate(0, rot, 0); break;
            case 2: g3d->rotate(0, 0, rot); break;
            default: break;
          }
        }

        // position cubes in a 2x2x2 grid
        g3d->translate((ix * 2 - 1) * 0.6f, (iy * 2 - 1) * 0.6f,
                       (iz * 2 - 1) * 0.6f);

        g3d->render(cube);
        g3d->popState();
      }
    }
  }
  g3d->popState();
}
