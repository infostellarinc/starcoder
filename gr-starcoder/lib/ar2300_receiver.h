// Copyright (c) 2018 InfoStellar, Inc. All Rights Reserved.

#ifndef AR2300_RECEIVER_H
#define AR2300_RECEIVER_H

#include <fcntl.h>
#include <unistd.h>

extern "C" {
#include <libusb-1.0/libusb.h>
#include "ar2300_driver.h"
}

class ar2300_receiver {
 public:
  ar2300_receiver();
  ~ar2300_receiver();

  void initialize();
  void stop();
  bool check();
  int read(char* buf, int size, int timeout);
  static void set_error_code(int code) { err_code = code; }

 private:
  int select_for_read(int timeout);

  // libusb context
  libusb_context* context;

  // AR2300 handler
  AR2300_HANDLE* ar2300;

  // Pipe for reading data from AR2300
  int read_pipe[2];

  // Initialization flag
  bool init_flag;

  //! Error code
  static int err_code;

};


#endif /* AR2300_RECEIVER_H */