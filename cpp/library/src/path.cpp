#include "xmc/path.hpp"

#include <stdio.h>
#include <string.h>

namespace xmc::path {

XmcStatus removeTailingSeparators(char* path, size_t pathSize) {
  size_t len = strlen(path);
  while (len > 0 && path[len - 1] == SEPARATOR) {
    path[len - 1] = '\0';
    len--;
  }
  return XMC_OK;
}

XmcStatus join(char* buff, const char* child, size_t buffSize) {
  size_t parentLen = strlen(buff);
  size_t childLen = strlen(child);

  if (parentLen + 1 + childLen >= buffSize) {
    XMC_ERR_RET(XMC_ERR_PATH_TOO_LONG);
  }

  if (parentLen > 0 && buff[parentLen - 1] != SEPARATOR) {
    buff[parentLen] = SEPARATOR;
    buff[parentLen + 1] = '\0';
    parentLen++;
  }

  strncpy(buff + parentLen, child, childLen + 1);
  return XMC_OK;
}

XmcStatus join(const char* parent, const char* child, char* out,
               size_t outSize) {
  if (outSize < strlen(parent) + 1 + strlen(child) + 1) {
    XMC_ERR_RET(XMC_ERR_PATH_TOO_LONG);
  }
  strncpy(out, parent, outSize);
  return join(out, child, outSize);
}

XmcStatus basename(const char* path, const char** out) {
  const char* lastSlash = strrchr(path, SEPARATOR);
  *out = lastSlash ? lastSlash + 1 : path;
  return XMC_OK;
}

XmcStatus basename(const char* path, char* out, size_t outSize) {
  const char* baseName;
  XMC_ERR_RET(basename(path, &baseName));
  size_t baseNameLen = strlen(baseName);
  if (baseNameLen >= outSize) {
    XMC_ERR_RET(XMC_ERR_FILENAME_TOO_LONG);
  }
  strncpy(out, baseName, baseNameLen + 1);
  return XMC_OK;
}

XmcStatus dirname(char* buff, size_t buffSize) {
  removeTailingSeparators(buff, buffSize);
  const char* lastSlash = strrchr(buff, SEPARATOR);
  if (!lastSlash) {
    // No slash found, so the directory name is "."
    if (buffSize < 2) {
      XMC_ERR_RET(XMC_ERR_PATH_TOO_LONG);
    }
    buff[0] = '.';
    buff[1] = '\0';
    return XMC_OK;
  }
  size_t dirNameLen = lastSlash - buff;
  if (dirNameLen == 0) {
    // The directory name is the root "/"
    if (buffSize < 2) {
      XMC_ERR_RET(XMC_ERR_PATH_TOO_LONG);
    }
    buff[0] = SEPARATOR;
    buff[1] = '\0';
    return XMC_OK;
  }
  if (dirNameLen >= buffSize) {
    XMC_ERR_RET(XMC_ERR_PATH_TOO_LONG);
  }
  buff[dirNameLen] = '\0';
  return XMC_OK;
}

XmcStatus dirname(const char* path, char* out, size_t outSize) {
  if (outSize < strlen(path) + 1) {
    XMC_ERR_RET(XMC_ERR_PATH_TOO_LONG);
  }
  strncpy(out, path, outSize);
  return dirname(out, outSize);
}

}  // namespace xmc::path
