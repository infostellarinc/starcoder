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

#ifndef AR2300_RECEIVER_H
#define AR2300_RECEIVER_H

#include <fcntl.h>
#include <unistd.h>
#include "blocking_spsc_queue.h"

extern "C" {
#include <libusb-1.0/libusb.h>
#include "ar2300_driver.h"
}

class ar2300_receiver {
 public:
  ar2300_receiver(int buffer_size);
  ~ar2300_receiver();

  void start();
  void stop();
  bool check();
  int read(char* buf, int size, int timeout_ms);
  static void set_error_code(int code) { err_code = code; }

 private:
  int select_for_read(int timeout);

  // libusb context
  libusb_context* context;

  // AR2300 handler
  AR2300_HANDLE* ar2300;

  // Blocking queue
  blocking_spsc_queue queue_;

  // Initialization flag
  bool started;

  //! Error code
  static int err_code;

};

#endif /* AR2300_RECEIVER_H */
