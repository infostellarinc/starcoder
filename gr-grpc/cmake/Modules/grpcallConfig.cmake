INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_GRPCALL grpcall)

FIND_PATH(
    GRPCALL_INCLUDE_DIRS
    NAMES grpcall/api.h
    HINTS $ENV{GRPCALL_DIR}/include
        ${PC_GRPCALL_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GRPCALL_LIBRARIES
    NAMES gnuradio-grpcall
    HINTS $ENV{GRPCALL_DIR}/lib
        ${PC_GRPCALL_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GRPCALL DEFAULT_MSG GRPCALL_LIBRARIES GRPCALL_INCLUDE_DIRS)
MARK_AS_ADVANCED(GRPCALL_LIBRARIES GRPCALL_INCLUDE_DIRS)

