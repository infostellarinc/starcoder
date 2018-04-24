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

/** @file */
#ifndef AR2300_DRIVER_H
#define AR2300_DRIVER_H

#define AR2300_USE_PTHREAD 1 /**< include threading code for event handler */
#define AR2300_USE_SYSLOG 1  /**< output debug messages to syslog */
#include "blocking_queue.h"
/*
 * constants related to receiver spec
 */
#define AR2300_VENDOR_ID 0x08d0  /**< AOR-2300 USB vendor ID */
#define AR2300_PRODUCT_ID 0xa001 /**< AOR-2300 product ID */
#define AR2300_BULK_EP 0x02      /**< endpoint of bulk command */
#define AR2300_ISO_EP 0x86       /**< endpoint of isochronous data */
#define AR2300_IF_NO (0)
#define AR2300_ALT_IF_NO (0)

/*
 * library constant parameters
 */

/**
 * how many usb requests to use
 */
#define MAX_AR2300_ALLOCATED_PACKETS (4)
/**
 * how many isochronous packets to fill in a single request
 */
#define MAX_AR2300_ISO_PACKETS (32)
/**
 * maximum size of received data in the isochronous transfer
 */
#define AR2300_ISO_PACKET_SIZE (3 * 512)
/**
 * maximum amount of time to wait for the event handler to exit
 */
#define AR2300_THREAD_SHUTDOWN_WAIT_SECS (10)

struct libusb_transfer;
struct ar2300_handle;

/**
 * isochronous buffer and associated usb transfer
 *
 */
typedef struct ar2300_packet_info {
  struct libusb_transfer *usb_transfer; /**< the isochronous transfer */
                                        /** the size of the buffer is
                                         *  MAX_AR2300_ISO_PACKETS * AR2300_ISO_PACKET_SIZE */
  unsigned char *iso_buffer;
} AR2300_PACKET_INFO;

/**
 * bulk transfer usage status
 */
typedef enum {
  AR2300_BULK_IDLE = 0, /**< transfer is idle */
  AR2300_BULK_SENT      /**< transfer has been sent */
} ar2300_bulk_status;

/**
 * iso transfer usage status
 */
typedef enum {
  AR2300_ISO_NOTREADY = 0, /**< iso packets are not ready */
  AR2300_ISO_READY,        /**< iso packets are ready to be used */
  AR2300_ISO_CANCELLING    /**< iso packets are being cancelled */
} ar2300_iso_status;

/**
 * prototype for error callback function
 *
 * this function will be called if an error occurs
 * during data handling
 *
 * @param t the transfer structure that caused the error
 * @param e the error type
 */
typedef void (*transfer_error_callback_func)(struct libusb_transfer *t, int e);

/**
 * Handle for AR2300 device
 *
 */
typedef struct ar2300_handle {
  libusb_context *context; /**< libusb context */

  /** this is the transfer used for sending start/stop commands */
  struct libusb_transfer *bulk_transfer;

  /** the usb device handle to the ar2300 */
  libusb_device_handle *device_handle;

  /** buffer initialized with the AR2300 start/stop protocol */
  unsigned char *bulk_buffer;

  ar2300_bulk_status bulk_status; /**< status of bulk transfer */

  blocking_queue* queue_; // Blocking queue for storing IQ data

  transfer_error_callback_func err_func; /**< error callback */

  /** prepare multiple iso transfers so that
   *  data will be filled while one is being processed */
  AR2300_PACKET_INFO packets[MAX_AR2300_ALLOCATED_PACKETS];

  ar2300_iso_status iso_status; /**< status of iso transfers */

  volatile int packets_in_orbit; /**< number of packets submitted to libusb */
} AR2300_HANDLE;

/**
 * maximum size of bulk transfers
 */
#define AR2300_BULK_PACKET_SIZE 64

/**
 * Find and open the AR2300
 *
 * Open the AR2300 device according to the vendor-id and product-id
 * found in the usb device descriptor.
 *
 * The Cypress USB controller has no firmware ROM on the receiver
 * and will come up as a blank device ready to receive a ROM image.
 * If the manufacturer field of the usb device descriptor is
 * undefined, then the function will proceed to download the firmware
 * to the receiver and perform a reset.
 *
 * If the device is successfully opened, then allocate the required
 * data structures to obtain the I/Q data.
 *
 * Assumes that only one device is connected.
 *
 * @param ctx the libusb library context
 * @returns a handle to the AR2300 device
 */
AR2300_HANDLE *ar2300_open(libusb_context *ctx);

/**
 * close the AR2300 device
 *
 * Close the AR2300 device and clean up resources.
 * This will also shut down the event handler thread
 * if it is running.
 *
 * @param ar2300 handle to the ar2300 device
 */
void ar2300_close(AR2300_HANDLE *ar2300);

/**
 * start the I/Q data transfer
 *
 * Send the command to start to the I/Q data capture
 *
 * @param ar2300 handle to the ar2300 device
 * @returns 0 on success
 */
int ar2300_start_transfer(AR2300_HANDLE *ar2300);

/**
 * stop the I/Q data transfer
 *
 * Send the command to stop to the I/Q data capture
 *
 * @param ar2300 handle to the ar2300 device
 * @returns 0 on success
 */
int ar2300_stop_transfer(AR2300_HANDLE *ar2300);

/**
 * set the queue to write IQ packets to
 */
void ar2300_set_queue(AR2300_HANDLE *ar2300, blocking_queue *q);

/**
 * start an event handling thread
 *
 * The event handling thread will be terminated when
 * ar2300_close is called.
 *
 * @param ctx the libusb library context
 * @returns 0 on success
 */
int ar2300_start_thread(void *ctx);

/**
 * thread entry point to handle libusb events
 *
 * This function loops over libusb_handle_event()
 * to handle asynchronous libusb I/O in the main thread.
 * The function receives the libusb context passed to it
 * as the argument to the ar2300_start_thread() function.
 *
 * @param ctx the argument passed to ar2300_start_thread()
 */
void *ar2300_libusb_event_thread(void *ctx) __attribute__((weak));

/**
 * set the transfer error callback
 *
 * This function is called with an error code if the
 * USB bulk or isochronous transfer fails.pi
 *
 * @param ar2300  handle to the ar2300 device
 * @param f       the function to be called on error
 */
void ar2300_set_err_handler(AR2300_HANDLE *ar2300,
                            transfer_error_callback_func f);

/** input parameters are wrong or missing */
#define AR2300_ERR_INPUT_PARAMETERS (1)

/** external thread has already been created */
#define AR2300_ERR_THREAD_EXISTS (2)

/** problem when spawning new thread */
#define AR2300_ERR_THREAD_CREATION (3)

/** problem transferring data to device */
#define AR2300_ERR_USBBULK_TRANSFER (4)

/** problem transferring data to device */
#define AR2300_ERR_USBISO_TRANSFER (5)

/** problem with bulk transfer */
#define AR2300_ERR_BULK_STATUS (6)

/** problem with isochronous transfer */
#define AR2300_ERR_ISO_STATUS (7)

/** could not write complete data to output */
#define AR2300_ERR_INCOMPLETE_WRITE (8)

/** error during data write */
#define AR2300_ERR_DATA_WRITE (9)

/** transfer error for this iso packet */
#define AR2300_ERR_ISO_PACKET (10)

#endif
