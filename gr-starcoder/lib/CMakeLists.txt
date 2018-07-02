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
  ${CMAKE_CURRENT_SOURCE_DIR}/../../cqueue/
  ${PNG_INCLUDE_DIR}
)

########################################################################
# Generate Protobuf sources
########################################################################
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

get_filename_component(hw_proto "${CMAKE_CURRENT_SOURCE_DIR}/../../api/starcoder.proto" ABSOLUTE)

protobuf_generate_cpp(hw_proto_srcs hw_proto_hdrs "${hw_proto}")

list(APPEND starcoder_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cqueue/string_queue.cc
    ${hw_proto_srcs}
    proto_to_pmt.cc
    init.c
    driver.c
    firmware.c
    blocking_spsc_queue.cc
    ar2300_receiver.cc
    ar2300_source_impl.cc
    complex_to_msg_c_impl.cc
    waterfall_heatmap_impl.cc
    waterfall_plotter_impl.cc
    enqueue_message_sink_impl.cc
    ax25_decoder_bm_impl.cc
    command_source_impl.cc
    ax25_encoder_mb_impl.cc
    noaa_apt_sink_impl.cc )

set(starcoder_sources "${starcoder_sources}" PARENT_SCOPE)
if(NOT starcoder_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT starcoder_sources)

add_library(gnuradio-starcoder SHARED ${starcoder_sources})
target_link_libraries(gnuradio-starcoder
    ${Boost_LIBRARIES}
    ${GNURADIO_ALL_LIBRARIES}
    ${LOG4CPP_LIBRARY}
    ${_PROTOBUF_LIBPROTOBUF}
    ${PNG_LIBRARIES}
    usb-1.0
)
set_target_properties(gnuradio-starcoder PROPERTIES DEFINE_SYMBOL "gnuradio_starcoder_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-starcoder PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
include(GrMiscUtils)
GR_LIBRARY_FOO(gnuradio-starcoder RUNTIME_COMPONENT "starcoder_runtime" DEVEL_COMPONENT "starcoder_devel")

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

configure_file(
${CMAKE_CURRENT_SOURCE_DIR}/test_meteor_stream.s
${CMAKE_CURRENT_BINARY_DIR}/test_meteor_stream.s
COPYONLY
)

configure_file(
${CMAKE_CURRENT_SOURCE_DIR}/test_meteor_image.png
${CMAKE_CURRENT_BINARY_DIR}/test_meteor_image.png
COPYONLY
)

configure_file(
${CMAKE_CURRENT_SOURCE_DIR}/test_meteor_image_68.png
${CMAKE_CURRENT_BINARY_DIR}/test_meteor_image_68.png
COPYONLY
)

configure_file(
${CMAKE_CURRENT_SOURCE_DIR}/test_meteor_image_65.png
${CMAKE_CURRENT_BINARY_DIR}/test_meteor_image_65.png
COPYONLY
)

configure_file(
${CMAKE_CURRENT_SOURCE_DIR}/test_meteor_image_64.png
${CMAKE_CURRENT_BINARY_DIR}/test_meteor_image_64.png
COPYONLY
)

include_directories( ${CPPUNIT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../../cqueue/ )

list(APPEND test_starcoder_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_starcoder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_starcoder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_enqueue_message_sink.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_meteor_decoder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cqueue/string_queue.cc
    meteor_correlator.cc
    meteor_decoder.cc
    meteor_viterbi.cc
    meteor_bit_io.cc
    meteor_ecc.cc
    meteor_packet.cc
    meteor_image.cc
    gil_util.cc
)

add_executable(test-starcoder ${test_starcoder_sources})

target_link_libraries(
  test-starcoder
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-blocks
  gnuradio-starcoder
  ${PYTHON_LIBRARIES}
)

GR_ADD_TEST(test_starcoder test-starcoder)

########################################################################
# Print summary
########################################################################
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Building for version: ${VERSION} / ${LIBVER}")

