#include "xmc/app.hpp"

__attribute__((weak)) xmc::AppConfig xmcAppGetConfig(void) {
  return xmc::getDefaultAppConfig();
}

__attribute__((weak)) void xmcAppSetup(void) {}

__attribute__((weak)) void xmcAppLoop(void) {}

__attribute__((weak)) XmcStatus
xmcAppTerminate(xmc::system::ShutdownReason reason) {
  return XMC_OK;
}
