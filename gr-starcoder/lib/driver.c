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

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

#include <libusb-1.0/libusb.h>
#include "ar2300_driver.h"

#if AR2300_USE_SYSLOG
#include <syslog.h>
#define LOGGER(l, ...) syslog(l, ##__VA_ARGS__)
#else
#define LOGGER(l, ...)
#endif

#if AR2300_USE_PTHREAD
#include <pthread.h>
#endif

extern int create_thread(void);
extern volatile int event_thread_run;

/* write isochronous data to file
 *
 * Internal function to write out the received isochronous data
 * out to disk.
 *
 * Returns the number of bytes unwritten or -1
 */
int iq_packet_write(AR2300_HANDLE *ar2300, const unsigned char *buffer, int length) {
  int left;
  size_t written;

  left = length;
  written = blocking_queue_push(ar2300->queue_, buffer, left);

  left -= written;
  return left;
}

/* isochronous transfer completion callback
 *
 * This is an internal callback function that is called
 * whenever a usb transfer completes execution.
 *
 * It will extract the I/Q data from the buffer and write it out
 * to the predefined file descriptor
 *
 */
void callback_libusb_iso_done(struct libusb_transfer *transfer) {
  int result;
  AR2300_HANDLE *ar2300 = transfer->user_data;

  switch (transfer->status) {
    case LIBUSB_TRANSFER_COMPLETED: {
      /* transfer in general succeeded */
      int idx;
      int packets = transfer->num_iso_packets;
      struct libusb_iso_packet_descriptor *hdr = &transfer->iso_packet_desc[0];
      unsigned char *buffer = transfer->buffer;

      for (idx = 0; idx < packets; ++idx) {
        if (hdr->status == LIBUSB_TRANSFER_COMPLETED) {
          int bytes_left = 0;

          // uint16_t *p = (uint16_t *)buffer;
          // printf("$%x \n", p[1]);
          bytes_left = iq_packet_write(ar2300, buffer, hdr->actual_length);
          if (bytes_left > 0) {
            /* some error occurred during write */
            ar2300->err_func(transfer, AR2300_ERR_INCOMPLETE_WRITE);
          }
          if (bytes_left < 0) {
            ar2300->err_func(transfer, AR2300_ERR_DATA_WRITE);
          }
        } else {
          /* some error occurred for this packet */
          ar2300->err_func(transfer, AR2300_ERR_ISO_PACKET);
        }

        hdr++;
        buffer += AR2300_ISO_PACKET_SIZE;
      }
    } break;
    case LIBUSB_TRANSFER_CANCELLED:
      ar2300->packets_in_orbit--;
      return;
      break;
    default: {
      /* some serious error occurred during transfer */
      ar2300->err_func(transfer, AR2300_ERR_ISO_STATUS);
    }
  }

  /* reuse this transfer */
  if (ar2300->iso_status == AR2300_ISO_READY) {
    result = libusb_submit_transfer(transfer);
    if (result != 0) {
      /* error reusing the transfer request */
      ar2300->err_func(transfer, AR2300_ERR_USBISO_TRANSFER);
    }
  } else if (ar2300->iso_status == AR2300_ISO_CANCELLING) {
    ar2300->packets_in_orbit--;
  }
}

void callback_libusb_bulk_done(struct libusb_transfer *transfer) {
  AR2300_HANDLE *ar2300 = transfer->user_data;

  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    /* some error occurred during bulk transfer */
    ar2300->err_func(transfer, AR2300_ERR_BULK_STATUS);
  }
  ar2300->bulk_status = AR2300_BULK_IDLE;
}

int ar2300_stop_transfer(AR2300_HANDLE *ar2300) {
  int result;
  int idx;

  if (!ar2300 || !ar2300->bulk_transfer) {
    /* required information unavailable */
    return AR2300_ERR_INPUT_PARAMETERS;
  }

  /* since we only allocate one bulk buffer, make sure
   * that the buffer is not in use.
   *
   * function should handle libusb_events here or
   * wait while the event handler processes the bulk event
   */
  while (ar2300->bulk_status != AR2300_BULK_IDLE) {
    if (event_thread_run) {
      usleep(10);
    } else {
      libusb_handle_events(ar2300->context);
    }
  }

  ar2300->bulk_buffer[0] = 0x5a;
  ar2300->bulk_buffer[1] = 0xa5;
  ar2300->bulk_buffer[2] = 0x00;
  ar2300->bulk_buffer[3] = 0x02;
  ar2300->bulk_buffer[4] = 0x41;
  ar2300->bulk_buffer[5] = 0x45;

  libusb_fill_bulk_transfer(ar2300->bulk_transfer, ar2300->device_handle,
                            AR2300_BULK_EP, ar2300->bulk_buffer, 6,
                            callback_libusb_bulk_done, ar2300, 5000);
  ar2300->bulk_status = AR2300_BULK_SENT;

  result = libusb_submit_transfer(ar2300->bulk_transfer);
  if (result != 0) {
    /* error during stop */
    result = AR2300_ERR_USBBULK_TRANSFER;
  }

  ar2300->iso_status = AR2300_ISO_CANCELLING;

  /* wait for transfer cancels to complete */
  while (1) {
    int isStopped = 1;

    if (ar2300->bulk_status != AR2300_BULK_IDLE) {
      isStopped = 0;
    }
    if (ar2300->packets_in_orbit != 0) {
      isStopped = 0;
    }
    if (isStopped) {
      break;
    }

    for (idx = 0; idx < MAX_AR2300_ALLOCATED_PACKETS; ++idx) {
      libusb_cancel_transfer(ar2300->packets[idx].usb_transfer);
    }

    if (event_thread_run) {
      sched_yield();
    } else {
      libusb_handle_events(ar2300->context);
    }
  }

  return result;
}

int ar2300_start_transfer(AR2300_HANDLE *ar2300) {
  int idx;
  int result;

  if (!ar2300 || !ar2300->bulk_transfer) {
    /* required information unavailable */
    return AR2300_ERR_INPUT_PARAMETERS;
  }

  /* prepare the isochronous buffers */
  for (idx = 0; idx < MAX_AR2300_ALLOCATED_PACKETS; ++idx) {
    AR2300_PACKET_INFO *info = &ar2300->packets[idx];

    info->usb_transfer->callback = callback_libusb_iso_done;
    result = libusb_submit_transfer(info->usb_transfer);
    if (result < 0) {
      LOGGER(LOG_ERR, "usb submit error %d %d", idx, result);
      return AR2300_ERR_USBISO_TRANSFER;
    }
    ar2300->packets_in_orbit++;
  }

  result = create_thread();
  if (result != 0) {
    return result;
  }

  /* since we only allocate one bulk buffer, make sure
   * that the buffer is not in use.
   */
  while (ar2300->bulk_status != AR2300_BULK_IDLE) {
    if (event_thread_run) {
      usleep(10);
    } else {
      libusb_handle_events(ar2300->context);
    }
  }

  ar2300->bulk_buffer[0] = 0x5a;
  ar2300->bulk_buffer[1] = 0xa5;
  ar2300->bulk_buffer[2] = 0x00;
  ar2300->bulk_buffer[3] = 0x02;
  ar2300->bulk_buffer[4] = 0x41;
  ar2300->bulk_buffer[5] = 0x53;

  libusb_fill_bulk_transfer(ar2300->bulk_transfer, ar2300->device_handle,
                            AR2300_BULK_EP, ar2300->bulk_buffer, 6,
                            callback_libusb_bulk_done, ar2300, 5000);
  ar2300->bulk_status = AR2300_BULK_SENT;

  result = libusb_submit_transfer(ar2300->bulk_transfer);
  if (result != 0) {
    /* error during start */
    return AR2300_ERR_USBBULK_TRANSFER;
  }
  return 0;
}

void *ar2300_libusb_event_thread(void *ctx) {
  struct timeval tv;
  AR2300_HANDLE *ar2300 = (AR2300_HANDLE *)ctx;

  tv.tv_sec = 1;
  tv.tv_usec = 0;
  while (event_thread_run) {
    libusb_handle_events_timeout_completed(ar2300->context, &tv, NULL);
  }
  return NULL;
}
