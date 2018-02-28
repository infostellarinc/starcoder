// Copyright (c) 2018 InfoStellar, Inc. All Rights Reserved.

#include "ar2300_receiver.h"
#include <stdio.h>
#include <stdexcept>

using namespace std;

#define PIPE_READ 0
#define PIPE_WRITE 1
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
ar2300_receiver::ar2300_receiver() {
  context = NULL;
  ar2300 = NULL;
  read_pipe[PIPE_READ] = -1;
  read_pipe[PIPE_WRITE] = -1;
  init_flag = false;
}

/*
 * Destructor
 */
ar2300_receiver::~ar2300_receiver() { stop(); }

/*
 * Initializer
 */
void ar2300_receiver::initialize() {
  err_code = ERROR_CODE_NA;
  init_flag = false;

  // Initialize libusb context
  int ret = libusb_init(&context);
  if (ret < 0) {
    fprintf(stderr, "ar2300_receiver::initialize: failed to initialize libusb. ret=%d\n", ret);
    throw std::runtime_error("ar2300_receiver::initialize");
  }

  // Open AR2300
  ar2300 = ar2300_open(context);
  if (ar2300 == NULL) {
    throw std::runtime_error("ar2300_receiver::initialize: couldn't open AR2300.");
  }

  // Prepare a pipe for reading
  ret = pipe(read_pipe);
  if (ret < 0) {
     fprintf(stderr, "ar2300_receiver::initialize: failed to create the pipe for reading data. ret=%d\n", ret);
     throw std::runtime_error("ar2300_receiver::initialize");
  }

  fcntl(read_pipe[PIPE_WRITE], F_SETFL, O_NONBLOCK);

  // Set the pipe for writing received data
  ar2300_set_fd(ar2300, read_pipe[PIPE_WRITE]);

  // Set the callback for error handling
  ar2300_set_err_handler(ar2300, err_callback);

  // Start the thread for receiving data
  ar2300_start_thread(ar2300);

  // Start transfer
  ret = ar2300_start_transfer(ar2300);
  if (ret != 0) {
    fprintf(stderr, "ar2300_receiver::initialize: failed to start transfer. ret=%d\n", ret);
    throw std::runtime_error("ar2300_receiver::initialize");
  }

  init_flag = true;
}

/*
 * Stop receiving
 */
void ar2300_receiver::stop() {
  if (init_flag) {
    return;
  }

  if (ar2300 != NULL) {
    ar2300_stop_transfer(ar2300);
    ar2300_close(ar2300);
    ar2300 = NULL;
  }
  if (read_pipe[PIPE_READ] >= 0) {
    close(read_pipe[PIPE_READ]);
    close(read_pipe[PIPE_WRITE]);
    read_pipe[PIPE_READ] = -1;
    read_pipe[PIPE_WRITE] = -1;
  }

  if (context != NULL) {
    libusb_exit(context);
    context = NULL;
  }

  init_flag = false;
}

/*
 * Read IQ data from device
 */
int ar2300_receiver::read(char* buf, int size, int timeout) {
  if (err_code != ERROR_CODE_NA) {
    stop();
    fprintf(stderr, "ar2300_receiver::read: something error occurred while reading data. err_code=%d\n", err_code);
    throw std::runtime_error("ar2300_receiver::read");
  }

  int stat = select_for_read(timeout);
  if (stat < 0) {
    stop();
    fprintf(stderr, "ar2300_receiver::read: invalid status was returned. stat=%d\n", stat);
    throw std::runtime_error("ar2300_receiver::read");
  } else if (stat == 0) {
    // timeout
    return 0;
  }

  int ret = ::read(read_pipe[PIPE_READ], buf, size);
  if (ret < 0) {
    stop();
    fprintf(stderr, "ar2300_receiver::read: failed to read data from the device. ret=%d\n", ret);
    throw std::runtime_error("ar2300_receiver::read");
  }

  return ret;
}

/*
 * Select data
 */
int ar2300_receiver::select_for_read(int timeout) {
  // Set timeout
  struct timeval tv;
  tv.tv_sec = (int)(timeout / 1000.0);
  tv.tv_usec = (timeout % 1000) * 1000;

  // Initialize file descriptor
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(read_pipe[PIPE_READ], &rfds);

  int ret = ::select(FD_SETSIZE, &rfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
  return (ret);
}