
if (DEFINED ENV{XMC_REPO_PATH} AND (NOT XMC_REPO_PATH))
    set(XMC_REPO_PATH $ENV{XMC_REPO_PATH})
    message("Using XMC_REPO_PATH from environment ('${XMC_REPO_PATH}')")
endif ()

set(XMC_PFM_DIR ${XMC_REPO_PATH}/cpp/platform/impl/${XMC_PLATFORM})
set(XMC_LIB_DIR ${XMC_REPO_PATH}/cpp/library)

add_subdirectory(${XMC_PFM_DIR} xmc_pfm)
add_subdirectory(${XMC_LIB_DIR} xmc_lib)
