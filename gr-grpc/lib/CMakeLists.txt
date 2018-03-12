# Copyright 2011,2012,2016 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(
    ${Boost_INCLUDE_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

# {Protobuf,PROTOBUF}_FOUND is defined based on find_package type ("MODULE" vs "CONFIG").
# For "MODULE", the case has also changed between cmake 3.5 and 3.6.
# We use the legacy uppercase version for *_LIBRARIES AND *_INCLUDE_DIRS variables
# as newer cmake versions provide them too for backward compatibility.
if(Protobuf_FOUND OR PROTOBUF_FOUND)
  if(TARGET protobuf::libprotobuf)
    set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
  else()
    set(_PROTOBUF_LIBPROTOBUF ${PROTOBUF_LIBRARIES})
    include_directories(${PROTOBUF_INCLUDE_DIRS})
  endif()
  if(TARGET protobuf::protoc)
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
  else()
    set(_PROTOBUF_PROTOC ${PROTOBUF_PROTOC_EXECUTABLE})
  endif()
else()
  message(WARNING "Failed to locate libprotobuf and protoc!")
endif()

# gRPC C++ plugin
get_target_property(gRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin
    IMPORTED_LOCATION_RELEASE)

get_filename_component(hw_proto "../api/grpcapi.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

protobuf_generate_cpp(hw_proto_srcs hw_proto_hdrs "${hw_proto}")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/grpcapi.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/grpcapi.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}" -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${gRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

link_directories(${Boost_LIBRARY_DIRS})
list(APPEND grpcall_sources
    msg_sink_impl.cc
    ${hw_proto_srcs}
    ${hw_grpc_srcs}
    )

set(grpcall_sources "${grpcall_sources}" PARENT_SCOPE)
if(NOT grpcall_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT grpcall_sources)

add_library(gnuradio-grpcall SHARED ${grpcall_sources})
target_link_libraries(gnuradio-grpcall
    ${Boost_LIBRARIES}
    ${GNURADIO_ALL_LIBRARIES}
    ${_PROTOBUF_LIBPROTOBUF}
    gRPC::grpc++_unsecure
)
set_target_properties(gnuradio-grpcall PROPERTIES DEFINE_SYMBOL "gnuradio_grpcall_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-grpcall PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
include(GrMiscUtils)
GR_LIBRARY_FOO(gnuradio-grpcall RUNTIME_COMPONENT "grpcall_runtime" DEVEL_COMPONENT "grpcall_devel")

########################################################################
# Print summary
########################################################################
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Building for version: ${VERSION} / ${LIBVER}")
