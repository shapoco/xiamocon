#ifndef XMC_APP_FLAME_HPP
#define XMC_APP_FLAME_HPP

#include <xiamocon.hpp>

extern float fireSpeed;
extern float fireBuoyancy;
extern float fireAttraction;
extern float fireRepulsion;

void setupFlame();
void updateFlame(float dt);
void renderFlame(xmc::Graphics3D &g3d);

#endif
