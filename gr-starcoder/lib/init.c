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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include <sched.h>
#endif

libusb_device_handle *write_firmware(libusb_context *ctx,
                                     libusb_device_handle *handle);
int is_required_device(libusb_device *device);
int allocate_buffers(AR2300_HANDLE *ar2300);
void deallocate_buffers(AR2300_HANDLE *ar2300);

/*
 * this variable is set to 0 to end the event handler thread
 */
volatile int event_thread_run = 0;
int request_thread_creation = 0;
void *thread_context = NULL;
#if AR2300_USE_PTHREAD
pthread_t event_thread;
#endif

/* determine if usb device is the AR2300
 *
 * returns 1 if the device is the required type
 *
 */
int is_required_device(libusb_device *device) {
  struct libusb_device_descriptor descriptor;

  if (0 != libusb_get_device_descriptor(device, &descriptor)) {
    /* some error occurs, should not happen */
    return 0;
  }
  if (descriptor.idVendor != AR2300_VENDOR_ID ||
      descriptor.idProduct != AR2300_PRODUCT_ID) {
    return 0;
  }
  return 1;
}

/* allocate the buffers required to operate the device
 */
int allocate_buffers(AR2300_HANDLE *ar2300) {
  int idx;

  if (!ar2300) {
    return -1;
  }

  for (idx = 0; idx < MAX_AR2300_ALLOCATED_PACKETS; ++idx) {
    AR2300_PACKET_INFO *info = &ar2300->packets[idx];

    info->iso_buffer = (unsigned char *)malloc(AR2300_ISO_PACKET_SIZE *
                                               MAX_AR2300_ISO_PACKETS);
    if (!info->iso_buffer) {
      /* error */
      return -2;
    }

    info->usb_transfer = libusb_alloc_transfer(MAX_AR2300_ISO_PACKETS);
    if (!info->usb_transfer) {
      /* usb transfer structure allocation error */
      return -3;
    }
    libusb_fill_iso_transfer(info->usb_transfer, ar2300->device_handle,
                             AR2300_ISO_EP, info->iso_buffer,
                             AR2300_ISO_PACKET_SIZE * MAX_AR2300_ISO_PACKETS,
                             MAX_AR2300_ISO_PACKETS, NULL, ar2300, 5000);
    libusb_set_iso_packet_lengths(info->usb_transfer, AR2300_ISO_PACKET_SIZE);
  }
  ar2300->iso_status = AR2300_ISO_READY;

  /* allocate the bulk transfer buffer */
  ar2300->bulk_transfer = libusb_alloc_transfer(0);
  if (!ar2300->bulk_transfer) {
    /* bulk transfer structure allocation error */
    return -4;
  }

  ar2300->bulk_buffer = malloc(AR2300_BULK_PACKET_SIZE);
  if (!ar2300->bulk_buffer) {
    return -5;
  }
  memset(ar2300->bulk_buffer, 0, AR2300_BULK_PACKET_SIZE);

  return 0;
}

/* deallocate previously acquired buffers
 */
void deallocate_buffers(AR2300_HANDLE *ar2300) {
  if (!ar2300) {
    return;
  }
  for (int idx = 0; idx < MAX_AR2300_ALLOCATED_PACKETS; ++idx) {
    AR2300_PACKET_INFO *info = &ar2300->packets[idx];

    if (info->iso_buffer) {
      free(info->iso_buffer);
    }
    info->iso_buffer = NULL;

    if (info->usb_transfer) {
      libusb_free_transfer(info->usb_transfer);
    }
    info->usb_transfer = NULL;
  }

  if (ar2300->bulk_buffer) {
    free(ar2300->bulk_buffer);
  }
  ar2300->bulk_buffer = NULL;

  if (ar2300->bulk_transfer) {
    libusb_free_transfer(ar2300->bulk_transfer);
  }
  ar2300->bulk_transfer = NULL;
}

void ar2300_close(AR2300_HANDLE *ar2300) {
  int thread_status = event_thread_run;

  if (!ar2300) {
    return;
  }
  if (ar2300->packets_in_orbit) {
    LOGGER(LOG_WARNING,
           "usb packets in use - stop transfers before calling ar2300_close");
  }

#if AR2300_USE_PTHREAD
  /* attempt to shutdown thread within the specified time
   */
  event_thread_run = 0;
  if (thread_status != 0) {
    int result;
    // result = pthread_timedjoin_np(event_thread, NULL, &tv);
    result = pthread_join(event_thread, NULL);
    if (result != 0 && result != ESRCH) {
      LOGGER(LOG_CRIT,
             "failed to shutdown thread within the allotted time! (%d)",
             result);
    }
  }
#endif

  if (ar2300->device_handle) {
    libusb_release_interface(ar2300->device_handle, AR2300_IF_NO);
  }
  libusb_close(ar2300->device_handle);
  ar2300->device_handle = NULL;

  thread_context = NULL;
  request_thread_creation = 0;

  deallocate_buffers(ar2300);

  free(ar2300);
}

libusb_device_handle *open_device(libusb_context *ctx) {
  libusb_device_handle *handle = NULL;
  libusb_device **devarray = NULL;
  ssize_t devices;
  int idx;

  devices = libusb_get_device_list(ctx, &devarray);
  if (devices < 0) {
    /* some error occurs */
    goto ERR;
  }

  for (idx = 0; idx < devices; ++idx) {
    libusb_device *dev = devarray[idx];
    if (is_required_device(dev)) {
      /* found the device, attempt to open it */
      int result;

      result = libusb_open(dev, &handle);
      if (result == 0) {
        break;
      }
      if (result != 0) {
        /* some error during device open */
      }
    }
  }
ERR:
  libusb_free_device_list(devarray, 1);
  return handle;
}

void default_error_handler(struct libusb_transfer *transfer, int errcode) {}

void ar2300_set_err_handler(AR2300_HANDLE *handler,
                            transfer_error_callback_func f) {
  if (!handler) {
    return;
  }
  handler->err_func = f;
}

libusb_device_handle *download_firmware(libusb_context *ctx,
                                        libusb_device_handle *handle) {
  int result = -1;

  if (libusb_set_configuration(handle, 1) < 0) {
    goto ERR;
  }

  result = libusb_claim_interface(handle, AR2300_IF_NO);
  if (result < 0) {
    goto ERR;
  }
  result =
      libusb_set_interface_alt_setting(handle, AR2300_IF_NO, AR2300_ALT_IF_NO);
  if (result < 0) {
    goto ERR;
  }

  handle = write_firmware(ctx, handle);
  if (!handle) {
    goto ERR;
  }
  result = 0;

ERR:
  if (result != 0) {
    libusb_close(handle);
    handle = NULL;
  }
  return handle;
}

AR2300_HANDLE *ar2300_open(libusb_context *ctx) {
  libusb_device_handle *handle = NULL;
  AR2300_HANDLE *ar2300 = NULL;
  int result;
  struct libusb_device_descriptor desc;

  handle = open_device(ctx);
  if (handle == NULL) {
    goto ERR;
  }

  result = libusb_get_device_descriptor(libusb_get_device(handle), &desc);
  if (result < 0) {
    goto ERR;
  }
  if (desc.iManufacturer == 0) {
    /* must download firmware */
    handle = download_firmware(ctx, handle);
    if (handle == NULL) {
      goto ERR;
    }
  } else {
    result = libusb_set_configuration(handle, 1);
    if (result < 0) {
      libusb_close(handle);
      goto ERR;
    }
    result = libusb_claim_interface(handle, AR2300_IF_NO);
    if (result < 0) {
      libusb_close(handle);
      goto ERR;
    }
  }

  /* at this point the device is initialized */
  ar2300 = malloc(sizeof(AR2300_HANDLE));
  if (!ar2300) {
    /* memory error */
    libusb_close(handle);
    goto ERR;
  }
  memset(ar2300, 0, sizeof(AR2300_HANDLE));
  ar2300->context = ctx;
  ar2300->device_handle = handle;
  ar2300->err_func = default_error_handler;
  handle = NULL;

  result = allocate_buffers(ar2300);
  if (result != 0) {
    ar2300_close(ar2300);
    ar2300 = NULL;
    goto ERR;
  }

ERR:
  return ar2300;
}

int ar2300_start_thread(void *ctx) {
  thread_context = ctx;
  request_thread_creation = 1;
  return 0;
}

int create_thread() {
  if (!request_thread_creation) {
    return 0;
  }
#if AR2300_USE_PTHREAD
  if (event_thread_run != 0) {
    return AR2300_ERR_THREAD_EXISTS;
  }
  event_thread_run = 1;

  {
    struct sched_param params;
    int sched_min, sched_max;
    int result;
    result = pthread_create(&event_thread, NULL, &ar2300_libusb_event_thread,
                            thread_context);
    if (result != 0) {
      event_thread_run = 0;
      /* some error during thread creation */
      return AR2300_ERR_THREAD_CREATION;
    }

    /*
     * adjust the scheduler and priority if we can
     */
    sched_max = sched_get_priority_max(SCHED_RR);
    sched_min = sched_get_priority_min(SCHED_RR);

    memset(&params, 0, sizeof(struct sched_param));
    params.sched_priority =
        sched_min > sched_max - 1 ? sched_min : sched_max - 1;
    result = pthread_setschedparam(event_thread, SCHED_RR, &params);
    if (result != 0) {
      LOGGER(LOG_WARNING, "could not adjust thread priority\n");
    } else {
      LOGGER(LOG_INFO, "adjust thread to priority %d\n", params.sched_priority);
    }
  }
  return 0;
#else
  return AR2300_ERR_THREAD_CREATION;
#endif
}

void ar2300_set_queue(AR2300_HANDLE *ar2300, blocking_spsc_queue *q) {
  if (!ar2300) {
    return;
  }
  ar2300->queue_ = q;
}
