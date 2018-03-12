INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_GRGRPC grgrpc)

FIND_PATH(
    GRGRPC_INCLUDE_DIRS
    NAMES grgrpc/api.h
    HINTS $ENV{GRGRPC_DIR}/include
        ${PC_GRGRPC_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GRGRPC_LIBRARIES
    NAMES gnuradio-grgrpc
    HINTS $ENV{GRGRPC_DIR}/lib
        ${PC_GRGRPC_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GRGRPC DEFAULT_MSG GRGRPC_LIBRARIES GRGRPC_INCLUDE_DIRS)
MARK_AS_ADVANCED(GRGRPC_LIBRARIES GRGRPC_INCLUDE_DIRS)

