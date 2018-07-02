/* -*- c++ -*- */
/*
 * Copyright 2018 gr-starcoder author.
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

#include <gnuradio/io_signature.h>
#include "meteor_decoder_sink_impl.h"

#include "meteor_decoder.h"
#include "meteor_packet.h"

namespace gr {
namespace starcoder {

meteor_decoder_sink::sptr meteor_decoder_sink::make(std::string filename_png) {
  return gnuradio::get_initial_sptr(new meteor_decoder_sink_impl(filename_png));
}

/*
 * The private constructor
 */
meteor_decoder_sink_impl::meteor_decoder_sink_impl(std::string filename_png)
    : gr::sync_block("meteor_decoder_sink",
                     gr::io_signature::make(1, 1, sizeof(uint8_t)),
                     gr::io_signature::make(0, 0, 0)),
      total_size_(0),
      filename_(filename_png),
      string_queue_(NULL) {}

/*
 * Our virtual destructor.
 */
meteor_decoder_sink_impl::~meteor_decoder_sink_impl() {}

int meteor_decoder_sink_impl::work(int noutput_items,
                                   gr_vector_const_void_star &input_items,
                                   gr_vector_void_star &output_items) {
  size_t block_size = input_signature()->sizeof_stream_item(0);
  const uint8_t *in = (const uint8_t *)input_items[0];

  total_size_ += noutput_items * block_size;

  item a;
  uint8_t *buffer = new uint8_t[noutput_items * block_size];
  memcpy(buffer, in, noutput_items * block_size);
  a.size = noutput_items * block_size;
  a.arr = buffer;
  list_of_arrays_.push_back(a);

  // Tell runtime system how many output items we produced.
  return noutput_items;
}

bool meteor_decoder_sink_impl::stop() {
  if (list_of_arrays_.begin() == list_of_arrays_.end()) {
    return true;
  }

  gr::starcoder::meteor_decoder decoder;
  gr::starcoder::meteor_packet packeter;

  uint8_t *raw = new uint8_t[total_size_];
  int copied_so_far = 0;
  for (auto it = list_of_arrays_.cbegin(); it != list_of_arrays_.cend(); it++) {
    std::copy((*it).arr, (*it).arr + (*it).size, raw + copied_so_far);
    copied_so_far += (*it).size;
    delete[](*it).arr;
  }

  uint8_t *ecced_data = new uint8_t[HARD_FRAME_LEN];

  int total = 0;
  int ok = 0;
  while (decoder.pos_ < total_size_ - SOFT_FRAME_LEN) {
    total++;
    bool res = decoder.decode_one_frame(raw, ecced_data);
    if (res) {
      ok++;
      std::cout << std::dec << 100. * decoder.pos_ / total_size_ << "% "
                << decoder.prev_pos_ << " " << std::hex << decoder.last_sync_
                << std::endl;
      packeter.parse_cvcdu(ecced_data, gr::starcoder::HARD_FRAME_LEN - 4 - 128);
    }
  }

  std::cout << std::dec << "packets: " << ok << " out of " << total
            << std::endl;

  std::string png_img = packeter.dump_image();
  std::string png_r = packeter.dump_gray_image(RED_APID);
  std::string png_g = packeter.dump_gray_image(GREEN_APID);
  std::string png_b = packeter.dump_gray_image(BLUE_APID);

  delete[] raw;
  delete[] ecced_data;

  if (string_queue_ != NULL) {
    string_queue_->push(png_img);
    string_queue_->push(png_r);
    string_queue_->push(png_g);
    string_queue_->push(png_b);
  }

  if (filename_ != "") {
    std::ofstream out(filename_);
    out << png_img;
    out.close();

    out = std::ofstream(construct_filename(filename_, RED_APID));
    out << png_r;
    out.close();

    out = std::ofstream(construct_filename(filename_, GREEN_APID));
    out << png_g;
    out.close();

    out = std::ofstream(construct_filename(filename_, BLUE_APID));
    out << png_b;
    out.close();
  }
}

std::string meteor_decoder_sink_impl::construct_filename(std::string original,
                                                         int apid) {
  boost::filesystem::path p(original);
  boost::filesystem::path mod(p.stem().native() + "_apid_" +
                              std::to_string(apid) + p.extension().native());
  p = p.parent_path() / mod;
  return p.native();
}

void meteor_decoder_sink_impl::register_starcoder_queue(uint64_t ptr) {
  string_queue_ = reinterpret_cast<string_queue *>(ptr);
}

} /* namespace starcoder */
} /* namespace gr */
