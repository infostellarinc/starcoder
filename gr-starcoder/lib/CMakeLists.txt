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

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
list(APPEND starcoder_sources
    init.c
    driver.c
    firmware.c
    blocking_queue.cc
    ar2300_receiver.cc
    ar2300_source_impl.cc
    complex_to_msg_c_impl.cc
    waterfall_heatmap_impl.cc
    waterfall_plotter_impl.cc )

set(starcoder_sources "${starcoder_sources}" PARENT_SCOPE)
if(NOT starcoder_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT starcoder_sources)

add_library(gnuradio-starcoder SHARED ${starcoder_sources})
target_link_libraries(gnuradio-starcoder ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES} ${LOG4CPP_LIBRARY} usb-1.0)
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

include_directories( ${CPPUNIT_INCLUDE_DIRS} )

list(APPEND test_starcoder_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_starcoder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_starcoder.cc
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

