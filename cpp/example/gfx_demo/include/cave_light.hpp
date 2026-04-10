#pragma once

#include "xmc/gfx3d/scene3d.hpp"

namespace {

xmc::Material3D cave_light_mat0_create() {
  xmc::Material3DClass mat;
  mat.baseColor = {1.0f, 1.0f, 1.0f, 1.0f};
  return std::make_shared<xmc::Material3DClass>(mat);
}

const xmc::Material3D cave_light_mat0 = cave_light_mat0_create();

const float cave_light_mesh0_prim0_posData[] = {
    -0.820008f, 4.457969f, 2.304334f,
    1.003577f, 4.461825f, 2.608716f,
    -0.848779f, 4.458907f, 0.593039f,
    1.318302f, 4.455406f, 1.421453f,
    0.485025f, 4.455437f, 2.80691f,
    -1.102487f, 4.46383f, 1.550012f,
    -0.178087f, 4.465105f, 0.388258f,
    1.192912f, 4.458323f, 0.774633f,
    -0.128283f, -0.026678f, -2.51189f,
    -1.843293f, -0.046072f, -1.771754f,
    -2.254833f, -0.044121f, -0.035497f,
    -1.443934f, -0.078818f, 1.818137f,
    0.354643f, -0.064153f, 2.164531f,
    2.045674f, -0.055926f, 1.269632f,
    2.307772f, -0.065913f, -0.320727f,
    1.516693f, -0.021628f, -1.978327f,
    -0.153185f, 2.219214f, -1.061816f,
    -1.346036f, 2.206417f, -0.589357f,
    -1.67866f, 2.209855f, 0.757258f,
    -1.131971f, 2.189575f, 2.061236f,
    0.419834f, 2.195642f, 2.48572f,
    1.524625f, 2.20295f, 1.939174f,
    1.813037f, 2.194746f, 0.550363f,
    1.354802f, 2.218348f, -0.601847f,
    -0.165636f, 3.342159f, -0.336779f,
    -1.097408f, 3.332662f, 0.001841f,
    -1.390573f, 3.336843f, 1.153635f,
    -0.97599f, 3.323772f, 2.182785f,
    0.452429f, 3.325539f, 2.646315f,
    1.264101f, 3.332387f, 2.273945f,
    1.565669f, 3.325076f, 0.985908f,
    1.273857f, 3.338336f, 0.086393f,
    -0.140734f, 1.096268f, -1.786853f,
    -1.594665f, 1.080173f, -1.180556f,
    -1.966746f, 1.082867f, 0.360881f,
    -1.287952f, 1.055379f, 1.939687f,
    0.387238f, 1.065744f, 2.325126f,
    1.78515f, 1.073512f, 1.604403f,
    2.060405f, 1.064417f, 0.114818f,
    1.435748f, 1.09836f, -1.290087f,
};

const xmc::Vec3Buffer cave_light_mesh0_prim0_pos = xmc::createVec3Buffer(
    (xmc::vec3 *)cave_light_mesh0_prim0_posData, 40, false
);

const float cave_light_mesh0_prim0_colData[] = {
    1.0f, 1.0f, 1.0f, 0.501961f,
    1.0f, 1.0f, 1.0f, 0.501961f,
    1.0f, 1.0f, 1.0f, 0.501961f,
    1.0f, 1.0f, 1.0f, 0.501961f,
    1.0f, 1.0f, 1.0f, 0.501961f,
    1.0f, 1.0f, 1.0f, 0.501961f,
    1.0f, 1.0f, 1.0f, 0.501961f,
    1.0f, 1.0f, 1.0f, 0.501961f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.25098f,
    1.0f, 1.0f, 1.0f, 0.25098f,
    1.0f, 1.0f, 1.0f, 0.25098f,
    1.0f, 1.0f, 1.0f, 0.25098f,
    1.0f, 1.0f, 1.0f, 0.25098f,
    1.0f, 1.0f, 1.0f, 0.25098f,
    1.0f, 1.0f, 1.0f, 0.25098f,
    1.0f, 1.0f, 1.0f, 0.25098f,
    1.0f, 1.0f, 1.0f, 0.376471f,
    1.0f, 1.0f, 1.0f, 0.376471f,
    1.0f, 1.0f, 1.0f, 0.376471f,
    1.0f, 1.0f, 1.0f, 0.376471f,
    1.0f, 1.0f, 1.0f, 0.376471f,
    1.0f, 1.0f, 1.0f, 0.376471f,
    1.0f, 1.0f, 1.0f, 0.376471f,
    1.0f, 1.0f, 1.0f, 0.376471f,
    1.0f, 1.0f, 1.0f, 0.12549f,
    1.0f, 1.0f, 1.0f, 0.12549f,
    1.0f, 1.0f, 1.0f, 0.12549f,
    1.0f, 1.0f, 1.0f, 0.12549f,
    1.0f, 1.0f, 1.0f, 0.12549f,
    1.0f, 1.0f, 1.0f, 0.12549f,
    1.0f, 1.0f, 1.0f, 0.12549f,
    1.0f, 1.0f, 1.0f, 0.12549f,
};

const xmc::ColorBuffer cave_light_mesh0_prim0_col = xmc::createColorBuffer(
    (xmc::colorf *)cave_light_mesh0_prim0_colData, 40, false
);

const uint16_t cave_light_mesh0_prim0_idxData[] = {
    32, 15, 39, 9, 32, 33, 33, 10, 9, 11, 34, 35, 35, 12, 11, 37,
    12, 36, 38, 13, 37, 30, 7, 31, 38, 23, 39, 30, 21, 29, 29, 20,
    28, 27, 20, 19, 19, 26, 27, 25, 18, 17, 17, 24, 25, 24, 23, 31,
    6, 31, 7, 25, 6, 2, 2, 26, 25, 27, 5, 0, 0, 28, 27, 1,
    28, 4, 3, 29, 1, 22, 31, 23, 14, 39, 15, 22, 37, 21, 21, 36,
    20, 19, 36, 35, 35, 18, 19, 17, 34, 33, 33, 16, 17, 16, 39, 23,
    32, 8, 15, 9, 8, 32, 33, 34, 10, 11, 10, 34, 35, 36, 12, 37,
    13, 12, 38, 14, 13, 30, 3, 7, 38, 22, 23, 30, 22, 21, 29, 21,
    20, 27, 28, 20, 19, 18, 26, 25, 26, 18, 17, 16, 24, 24, 16, 23,
    6, 24, 31, 25, 24, 6, 2, 5, 26, 27, 26, 5, 0, 4, 28, 1,
    29, 28, 3, 30, 29, 22, 30, 31, 14, 38, 39, 22, 38, 37, 21, 37,
    36, 19, 20, 36, 35, 34, 18, 17, 18, 34, 33, 32, 16, 16, 32, 39,
};

const xmc::IndexBuffer cave_light_mesh0_prim0_idx = xmc::createIndexBuffer(
    (uint16_t *)cave_light_mesh0_prim0_idxData, 192, false
);

xmc::Primitive3D cave_light_mesh0_prim0_create() {
  return xmc::createPrimitive3D(
      xmc::PrimitiveMode::TRIANGLES,
      (xmc::Vec3Buffer)cave_light_mesh0_prim0_pos,
      nullptr,
      (xmc::ColorBuffer)cave_light_mesh0_prim0_col,
      nullptr,
      (xmc::IndexBuffer)cave_light_mesh0_prim0_idx,
      (xmc::Material3D)cave_light_mat0
  );
}

const xmc::Primitive3D cave_light_mesh0_prim0 = cave_light_mesh0_prim0_create();

xmc::Mesh3D cave_light_mesh0_create() {
  std::vector<xmc::Primitive3D> prims = {cave_light_mesh0_prim0};
  return xmc::createMesh3D(std::move(prims));
}

const xmc::Mesh3D cave_light_mesh0 = cave_light_mesh0_create();

xmc::Node3D cave_light_node0_create() {
  xmc::Node3D node = xmc::createNode3D(cave_light_mesh0);
  return node;
}

const xmc::Node3D cave_light_node0 = cave_light_node0_create();

xmc::Scene3D cave_light_scene0_create() {
  xmc::Scene3D scene = xmc::createScene3D();
  scene->addNode(cave_light_node0);
  return scene;
}

const xmc::Scene3D cave_light_scene0 = cave_light_scene0_create();

const xmc::Scene3D cave_light = cave_light_scene0;

}  // namespace
