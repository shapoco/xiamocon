set(LIB_NAME xmc_lib)

set(PFM_INC_DIR ../platform/decl/include)

set(LIB_SRC_DIR src)
set(LIB_INC_DIR include)
set(LIB_EXT_DIR ext)

project(${LIB_NAME} C CXX ASM)

file(GLOB CPP_FILES
    ${LIB_SRC_DIR}/*.cpp
    ${LIB_SRC_DIR}/*.c
    ${LIB_SRC_DIR}/**/*.cpp
    ${LIB_SRC_DIR}/**/*.c
    ${LIB_EXT_DIR}/**/*.cpp
    ${LIB_EXT_DIR}/**/*.c
)

add_library(${LIB_NAME} STATIC
    ${CPP_FILES}
)

target_compile_options(${LIB_NAME} PUBLIC -O2 -Wall)
target_compile_features(${LIB_NAME} PUBLIC cxx_std_17)

target_include_directories(${LIB_NAME} PUBLIC
    ${PFM_INC_DIR}
    ${LIB_INC_DIR}
    ${LIB_EXT_DIR}/fatfs
    ${LIB_EXT_DIR}/tjpgd
    ${LIB_EXT_DIR}/gfxfont
)
