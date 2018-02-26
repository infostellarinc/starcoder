INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_STARCODER starcoder)

FIND_PATH(
    STARCODER_INCLUDE_DIRS
    NAMES starcoder/api.h
    HINTS $ENV{STARCODER_DIR}/include
        ${PC_STARCODER_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    STARCODER_LIBRARIES
    NAMES gnuradio-starcoder
    HINTS $ENV{STARCODER_DIR}/lib
        ${PC_STARCODER_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(STARCODER DEFAULT_MSG STARCODER_LIBRARIES STARCODER_INCLUDE_DIRS)
MARK_AS_ADVANCED(STARCODER_LIBRARIES STARCODER_INCLUDE_DIRS)

