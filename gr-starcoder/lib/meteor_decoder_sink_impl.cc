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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fstream>

#include <gnuradio/io_signature.h>
#include "meteor_decoder_sink_impl.h"

#include "meteor/meteor_decoder.h"
#include "meteor/meteor_packet.h"

#include "pmt_to_proto.h"

namespace gr {
namespace starcoder {

meteor_decoder_sink::sptr meteor_decoder_sink::make(
    const std::string &filename_png) {
  return gnuradio::get_initial_sptr(new meteor_decoder_sink_impl(filename_png));
}

/*
 * The private constructor
 */
meteor_decoder_sink_impl::meteor_decoder_sink_impl(
    const std::string &filename_png)
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

  uint8_t *buffer = new uint8_t[noutput_items * block_size];
  memcpy(buffer, in, noutput_items * block_size);
  item a = {.size = noutput_items * block_size, .partial_stream = buffer };
  items_.push_back(a);

  // Tell runtime system how many output items we produced.
  return noutput_items;
}

bool meteor_decoder_sink_impl::stop() {
  if (items_.empty()) {
    return true;
  }

  meteor::decoder decoder;
  meteor::packeter packeter;

  std::unique_ptr<uint8_t[]> raw(new uint8_t[total_size_]());
  int copied_so_far = 0;
  for (auto it = items_.cbegin(); it != items_.cend(); it++) {
    std::copy((*it).partial_stream, (*it).partial_stream + (*it).size,
              raw.get() + copied_so_far);
    copied_so_far += (*it).size;
    delete[](*it).partial_stream;
  }

  std::unique_ptr<uint8_t[]> error_corrected_data(
      new uint8_t[meteor::HARD_FRAME_LEN]());

  int total = 0;
  int ok = 0;
  while (decoder.pos() < total_size_ - meteor::SOFT_FRAME_LEN) {
    total++;
    bool res = decoder.decode_one_frame(raw.get(), total_size_,
                                        error_corrected_data.get());
    if (res) {
      ok++;
      std::cout << std::dec << 100. * decoder.pos() / total_size_ << "% "
                << decoder.prev_pos() << " " << std::hex << decoder.last_sync()
                << std::endl;
      packeter.parse_cvcdu(error_corrected_data.get(),
                           meteor::HARD_FRAME_LEN - 4 - 128);
    }
  }

  std::cout << std::dec << "packets: " << ok << " out of " << total
            << std::endl;

  std::string png_img = packeter.dump_image();
  std::string png_r = packeter.dump_gray_image(meteor::RED_APID);
  std::string png_g = packeter.dump_gray_image(meteor::GREEN_APID);
  std::string png_b = packeter.dump_gray_image(meteor::BLUE_APID);

  if (string_queue_ != NULL) {
    ::starcoder::BlockMessage grpc_pmt;
    if (!png_img.empty()) {
      grpc_pmt.set_blob_value(png_img);
      string_queue_->push(grpc_pmt.SerializeAsString());
    }
    if (!png_r.empty()) {
      grpc_pmt.set_blob_value(png_r);
      string_queue_->push(grpc_pmt.SerializeAsString());
    }
    if (!png_g.empty()) {
      grpc_pmt.set_blob_value(png_g);
      string_queue_->push(grpc_pmt.SerializeAsString());
    }
    if (!png_b.empty()) {
      grpc_pmt.set_blob_value(png_b);
      string_queue_->push(grpc_pmt.SerializeAsString());
    }
  }

  if (!filename_.empty()) {
    std::ofstream out;

    if (!png_img.empty()) {
      out = std::ofstream(filename_);
      out << png_img;
      out.close();
    }

    if (!png_r.empty()) {
      out = std::ofstream(construct_filename(filename_, meteor::RED_APID));
      out << png_r;
      out.close();
    }

    if (!png_g.empty()) {
      out = std::ofstream(construct_filename(filename_, meteor::GREEN_APID));
      out << png_g;
      out.close();
    }

    if (!png_b.empty()) {
      out = std::ofstream(construct_filename(filename_, meteor::BLUE_APID));
      out << png_b;
      out.close();
    }
  }
}

std::string meteor_decoder_sink_impl::construct_filename(
    const std::string &original, int apid) {
  boost::filesystem::path p(original);
  boost::filesystem::path modified_filename(p.stem().native() + "_apid_" +
                                            std::to_string(apid) + ".png");
  p = p.parent_path() / modified_filename;
  return p.native();
}

void meteor_decoder_sink_impl::register_starcoder_queue(uint64_t ptr) {
  string_queue_ = reinterpret_cast<string_queue *>(ptr);
}

} /* namespace starcoder */
} /* namespace gr */
