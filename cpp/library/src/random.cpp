#include "xmc/random.hpp"

#include <stdlib.h>

namespace xmc {

uint32_t randomU32() { return (uint32_t)rand(); }
float randomF32() { return (float)(rand() & 0xFFFF) / 65536.0f; }

}  // namespace xmc
