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

#ifndef INCLUDED_STARCODER_ENQUEUE_MESSAGE_SINK_IMPL_H
#define INCLUDED_STARCODER_ENQUEUE_MESSAGE_SINK_IMPL_H

#include <starcoder/enqueue_message_sink.h>
#include <queue>
#include <mutex>
#include <string_queue.h>

namespace gr {
  namespace starcoder {

    class enqueue_message_sink_impl : public enqueue_message_sink
    {
     private:
      std::mutex mutex_;
      string_queue *string_queue_;

     public:
      enqueue_message_sink_impl();
      ~enqueue_message_sink_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);

      void handler(pmt::pmt_t msg);

      void register_starcoder_queue(uint64_t ptr);
    };

  } // namespace starcoder
} // namespace gr

#endif /* INCLUDED_STARCODER_ENQUEUE_MESSAGE_SINK_IMPL_H */

