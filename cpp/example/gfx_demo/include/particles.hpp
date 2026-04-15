#ifndef XMC_APP_PARTICLES_HPP
#define XMC_APP_PARTICLES_HPP

#include <xiamocon.hpp>

void setupParticles();
void updateParticles(float dt);
void renderParticles(xmc::Graphics3D &g3d);

#endif
