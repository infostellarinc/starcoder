/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2017,2018 Libre Space Foundation
 * <http://librespacefoundation.org/>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define png_infopp_NULL (png_infopp) NULL
#define int_p_NULL (int *)NULL

#include <gnuradio/io_signature.h>
#include "noaa_apt_sink_impl.h"

#include <cmath>

#include <boost/filesystem.hpp>
#include <boost/gil/extension/io/png_io.hpp>

namespace gr {
namespace starcoder {

// Noaa apt sync pattern A
// (see https://sourceforge.isae.fr/attachments/download/1813/apt_synch.gif)
const bool noaa_apt_sink_impl::synca_seq[] = {
  false, false, false, false, true, true, false, false,  // Pulse 1
  true, true, false, false,                              // Pulse 2
  true, true, false, false,                              // Pulse 3
  true, true, false, false,                              // Pulse 4
  true, true, false, false,                              // Pulse 5
  true, true, false, false,                              // Pulse 6
  true, true, false, false,                              // Pulse 7
  false, false, false, false, false, false, false, false
};

// Noaa apt sync pattern B
// (see https://sourceforge.isae.fr/attachments/download/1813/apt_synch.gif)
const bool noaa_apt_sink_impl::syncb_seq[] = {
  false, false, false, false, true, true, true, false, false, true, true, true,
  false, false, true, true, true, false, false, true, true, true, false, false,
  true, true, true, false, false, true, true, true, false, false, true, true,
  true, false, false, false
};

noaa_apt_sink::sptr noaa_apt_sink::make(const char *filename_png, size_t width,
                                        size_t height, bool sync, bool flip) {
  return gnuradio::get_initial_sptr(
      new noaa_apt_sink_impl(filename_png, width, height, sync, flip));
}

/*
 * The private constructor
 */
noaa_apt_sink_impl::noaa_apt_sink_impl(const char *filename_png, size_t width,
                                       size_t height, bool sync, bool flip)
    : gr::sync_block("noaa_apt_sink",
                     gr::io_signature::make(1, 1, sizeof(float)),
                     gr::io_signature::make(0, 0, 0)),
      f_average_alpha(0.25),
      d_filename_png(filename_png),
      d_width(width),
      d_height(height),
      d_synchronize_opt(sync),
      d_flip(flip),
      d_history_length(40),
      d_has_sync(false),
      d_image_received(false),
      d_current_x(0),
      d_current_y(0),
      d_num_images(0),
      f_max_level(0.0),
      f_min_level(1.0),
      f_average(0.0),
      image_received_(width, height),
      string_queue_(NULL) {
  set_history(d_history_length);
  image_received_view_ = view(image_received_);
}

/*
 * Our virtual destructor.
 */
noaa_apt_sink_impl::~noaa_apt_sink_impl() {}

void noaa_apt_sink_impl::write_image(std::string filename) {
  if (d_filename_png != "") {
    boost::gil::detail::png_writer writer(filename.c_str());
    if (!d_flip)
      writer.apply(image_received_view_);
    else
      writer.apply(flipped_up_down_view(image_received_view_));
  }

  if (string_queue_ != NULL) {
    // TODO: Writes out to /tmp since Boost GIL doesn't support writing to streams.
    // This should be fixed moving forward
    boost::filesystem::path temp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();

    boost::gil::detail::png_writer writer(temp.native().c_str());

    if (!d_flip)
      writer.apply(image_received_view_);
    else
      writer.apply(flipped_up_down_view(image_received_view_));

    std::ifstream t(temp.native());
    // Using std::string directly as the buffer breaks the generated .png.
    std::stringstream buffer;
    buffer << t.rdbuf();

    string_queue_->push(buffer.str());
    boost::filesystem::remove(temp);
  }
}

bool noaa_apt_sink_impl::stop() {
  if (!d_image_received) {
    write_image(d_filename_png);
  }
  return true;
}

void noaa_apt_sink_impl::set_pixel(size_t x, size_t y, float sample) {
  // Adjust dynamic range, using minimum and maximum values
  sample = (sample - f_min_level) / (f_max_level - f_min_level) * 255;
  // Set the pixel in the full image
  image_received_view_(x, y) = boost::gil::gray8_pixel_t(sample);
}

void noaa_apt_sink_impl::skip_to(size_t new_x, size_t pos,
                                 const float *samples) {
  // Check if the skip is forward or backward
  if (new_x > d_current_x) {
    // In case it is forward there will be a new_x - d_current_x sized hole
    // in the image. Holes up 39 pixels can be filled from the modules
    // history
    size_t dist = std::min(size_t(39), new_x - d_current_x);
    // Fill the hole using the previous samples of pos
    for (size_t i = 0; i < dist; i++) {
      set_pixel(new_x - dist + i, d_current_y, samples[pos - dist + i]);
    }
  }
  // Jump to new location
  d_current_x = new_x;
}

noaa_apt_sync_marker noaa_apt_sink_impl::is_marker(size_t pos,
                                                   const float *samples) {
  // Initialize counters for 'hacky' correlation
  size_t count_a = 0;
  size_t count_b = 0;

  for (size_t i = 0; i < 40; i++) {
    // history of previous 39 samples + current one
    // -> start 39 samples in the past
    float sample = samples[pos - 39 + i];
    // Remove DC-offset (aka. the average value of the sync pattern)
    sample = sample - f_average;

    // Very basic 1/0 correlation between pattern constan and history
    if ((sample > 0 && synca_seq[i]) || (sample < 0 && !syncb_seq[i])) {
      count_a += 1;
    }
    if ((sample > 0 && syncb_seq[i]) || (sample < 0 && !syncb_seq[i])) {
      count_b += 1;
    }
  }

  // Prefer sync pattern a as it is detected more reliable
  if (count_a > 35) {
    return noaa_apt_sync_marker::SYNC_A;
  } else if (count_b > 35) {
    return noaa_apt_sync_marker::SYNC_B;
  } else {
    return noaa_apt_sync_marker::NONE;
  }
}

int noaa_apt_sink_impl::work(int noutput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items) {
  const float *in = (const float *)input_items[0];
  /* If we have already produced one image, ignore the remaining observation*/
  if (d_image_received) {
    return noutput_items;
  }

  // Structure of in[]:
  // - d_history_length many historical samples
  // - noutput_items many samples to process
  for (size_t i = d_history_length - 1;
       i < noutput_items + d_history_length - 1; i++) {

    // Get the current sample
    float sample = in[i];

    // Update min and max level to adjust dynamic range in set pixel
    f_max_level = std::fmax(f_max_level, sample);
    f_min_level = std::fmin(f_min_level, sample);

    // Update exponential smoothing average used in sync pattern detection
    f_average = f_average_alpha * sample + (1.0 - f_average_alpha) * f_average;

    // If line sync is enabled
    if (d_synchronize_opt) {
      // Check if the history for the current sample is a sync pattern
      noaa_apt_sync_marker marker = is_marker(i, in);

      // For pattern a
      if (marker == noaa_apt_sync_marker::SYNC_A) {
        // Skip to right location, pattern starts 40 samples in the past
        skip_to(39, i, in);
        // If this is the first sync, reset min and max
        if (!d_has_sync) {
          f_max_level = 0.0;
          f_min_level = 1.0;
          d_has_sync = true;
        }
      }
          // For pattern b
          else if (marker == noaa_apt_sync_marker::SYNC_B) {
        // Skip to right location, pattern starts 40 samples in the past
        skip_to(d_width / 2 + 39, i, in);
        // If this is the first sync, reset min and max
        if (!d_has_sync) {
          f_max_level = 0.0;
          f_min_level = 1.0;
          d_has_sync = true;
        }
      }
    }

    // Set the the pixel at the current position
    set_pixel(d_current_x, d_current_y, sample);

    // Increment x position
    d_current_x += 1;
    // If we are beyond the end of line
    if (d_current_x >= d_width) {
      // Increment y position
      d_current_y += 1;
      // Reset x position to line start
      d_current_x = 0;

      // Split the image if there are enough lines decoded
      if (d_current_y >= d_height) {
        d_current_y = 0;
        d_num_images += 1;
        // Write out the full image
        write_image(d_filename_png);
        d_image_received = true;
      }
    }
  }

  // Tell gnu radio how many samples were consumed
  return noutput_items;
}

void noaa_apt_sink_impl::register_starcoder_queue(uint64_t ptr) {
  string_queue_ = reinterpret_cast<string_queue *>(ptr);
}

} /* namespace starcoder */
} /* namespace gr */
