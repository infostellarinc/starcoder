INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_GRPC grpc)

FIND_PATH(
    GRPC_INCLUDE_DIRS
    NAMES grpc/api.h
    HINTS $ENV{GRPC_DIR}/include
        ${PC_GRPC_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GRPC_LIBRARIES
    NAMES gnuradio-grpc
    HINTS $ENV{GRPC_DIR}/lib
        ${PC_GRPC_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GRPC DEFAULT_MSG GRPC_LIBRARIES GRPC_INCLUDE_DIRS)
MARK_AS_ADVANCED(GRPC_LIBRARIES GRPC_INCLUDE_DIRS)

