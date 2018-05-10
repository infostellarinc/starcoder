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

#include "ar2300_receiver.h"
#include <stdio.h>
#include <stdexcept>

using namespace std;

#define ERROR_CODE_NA -1  // No assigned error code

int ar2300_receiver::err_code = ERROR_CODE_NA;

/*
 * Error handler
 */
void err_callback(struct libusb_transfer* transfer, int code) {
  switch (code) {
    case AR2300_ERR_INCOMPLETE_WRITE:
    case AR2300_ERR_USBISO_TRANSFER:
    case AR2300_ERR_ISO_STATUS:
      ar2300_receiver::set_error_code(code);
      break;
    case AR2300_ERR_DATA_WRITE:
      break;
  }
}

/*
 * Constructor
 */
ar2300_receiver::ar2300_receiver(int buffer_size) : queue_(buffer_size) {
  context = NULL;
  ar2300 = NULL;
  started = false;
}

/*
 * Destructor
 */
ar2300_receiver::~ar2300_receiver() { stop(); }

/*
 * Initializer
 */
void ar2300_receiver::start() {
  err_code = ERROR_CODE_NA;
  started = false;

  // Initialize libusb context
  int ret = libusb_init(&context);
  if (ret < 0) {
    fprintf(
        stderr,
        "ar2300_receiver::initialize: failed to initialize libusb. ret=%d\n",
        ret);
    throw std::runtime_error("ar2300_receiver::initialize");
  }

  // Open AR2300
  ar2300 = ar2300_open(context);
  if (ar2300 == NULL) {
    throw std::runtime_error(
        "ar2300_receiver::initialize: couldn't open AR2300.");
  }

  // Set the blocking queue for received data
  ar2300_set_queue(ar2300, &queue_);

  // Set the callback for error handling
  ar2300_set_err_handler(ar2300, err_callback);

  // Start the thread for receiving data
  ar2300_start_thread(ar2300);

  // Start transfer
  ret = ar2300_start_transfer(ar2300);
  if (ret != 0) {
    fprintf(stderr,
            "ar2300_receiver::initialize: failed to start transfer. ret=%d\n",
            ret);
    throw std::runtime_error("ar2300_receiver::initialize");
  }

  started = true;
}

/*
 * Stop receiving
 */
void ar2300_receiver::stop() {
  if (!started) {
    return;
  }

  if (ar2300 != NULL) {
    ar2300_stop_transfer(ar2300);
    ar2300_close(ar2300);
    ar2300 = NULL;
  }

  if (context != NULL) {
    libusb_exit(context);
    context = NULL;
  }

  started = false;
}

/*
 * Read IQ data from device
 * @return: Number of bytes read
 */
int ar2300_receiver::read(char* buf, int size, int timeout_ms) {

  if (err_code != ERROR_CODE_NA) {
    stop();
    fprintf(stderr, "ar2300_receiver::read: something error occurred while "
                    "reading data. err_code=%d\n",
            err_code);
    throw std::runtime_error("ar2300_receiver::read");
  }

  int ret = queue_.pop(buf, size, timeout_ms);

  return ret;
}
