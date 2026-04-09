#ifndef XMC_PATH_HPP
#define XMC_PATH_HPP

#include "xmc/xmc_common.hpp"

#include <stddef.h>

namespace xmc::path {

/** Maximum length of a path, excluding the null terminator. */
static constexpr uint32_t MAX_PATH_LENGTH = 255;

/** Maximum length of a filename, excluding the null terminator. */
static constexpr uint32_t MAX_FILENAME_LENGTH = 255;

/** Path separator character. */
static constexpr char SEPARATOR = '/';

/**
 * @brief Remove trailing path separators from a path.
 * @param path The path to modify. The modified path will be stored back in this
 * buffer.
 * @param pathSize The size of the path buffer, including space for the null
 * terminator.
 * @return XMC_OK on success, or an error code on failure (e.g., if the
 * resulting path is too long).
 */
XmcStatus removeTailingSeparators(char* path, size_t pathSize);

/**
 * @brief Join a child path to a parent path.
 * @param buff The buffer containing the parent path. The joined path will be
 * stored back in this buffer.
 * @param child The child path to join to the parent path.
 * @param buffSize The size of the buffer, including space for the null
 * terminator.
 * @return XMC_OK on success, or an error code on failure (e.g., if the
 * resulting path is too long).
 */
XmcStatus join(char* buff, const char* child,
               size_t buffSize = MAX_PATH_LENGTH + 1);

/**
 * @brief Extract the base name from a path.
 * @param path The input path.
 * @param out The buffer to store the base name.
 * @param outSize The size of the output buffer, including space for the
 * null terminator.
 * @return XMC_OK on success, or an error code on failure (e.g., if the base
 * name is too long).
 */
XmcStatus join(const char* parent, const char* child, char* out,
               size_t outSize = MAX_PATH_LENGTH + 1);

/** @brief Extract the base name from a path.
 *  @param path The input path.
 *  @param out The pointer to the base name within the input path.
 *  @return XMC_OK on success, or an error code on failure.
 */
XmcStatus basename(const char* path, const char** out);

/**
 * @brief Extract the base name from a path.
 * @param path The input path.
 * @param out The buffer to store the base name.
 * @param outSize The size of the output buffer, including space for the
 * null terminator.
 * @return XMC_OK on success, or an error code on failure (e.g., if the base
 * name is too long).
 */
XmcStatus basename(const char* path, char* out,
                   size_t outSize = MAX_FILENAME_LENGTH + 1);

/**
 * @brief Extract the directory name from a path.
 * @param buff The buffer containing the path. The directory name will
 * be stored back in this buffer.
 * @param buffSize The size of the buffer, including space for the
 * null terminator.
 * @return XMC_OK on success, or an error code on failure (e.g., if the
 * directory name is too long).
 */
XmcStatus dirname(char* buff, size_t buffSize);

/**
 * @brief Extract the directory name from a path.
 * @param path The input path.
 * @param out The buffer to store the directory name.
 * @param outSize The size of the output buffer, including space for the
 * null terminator.
 * @return XMC_OK on success, or an error code on failure (e.g., if the
 * directory name is too long).
 */
XmcStatus dirname(const char* path, char* out,
                  size_t outSize = MAX_PATH_LENGTH + 1);

}  // namespace xmc::path

#endif
