/* -*- c++ -*- */
/*
 * Copyright 2018 Infostellar, Inc.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_GIL_UTIL_H
#define INCLUDED_GIL_UTIL_H

#define png_infopp_NULL (png_infopp) NULL
#define int_p_NULL (int *)NULL

#include <boost/filesystem.hpp>
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>

#include <cstring>

namespace gr {
namespace starcoder {

std::string store_rgb_to_png_string(
    boost::gil::rgb8_image_t::view_t image_view);

std::string store_gray_to_png_string(
    boost::gil::gray8_image_t::view_t image_view);

}  // namespace starcoder
}  // namespace gr

#endif /* INCLUDED_GIL_UTIL_H */
