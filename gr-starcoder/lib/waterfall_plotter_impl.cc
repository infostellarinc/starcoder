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

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <Python.h>
#include "numpy/arrayobject.h"
#include <gnuradio/io_signature.h>
#include "waterfall_plotter_impl.h"
#include <iostream>

namespace gr {
  namespace starcoder {

    waterfall_plotter::sptr
    waterfall_plotter::make(size_t fft_size, char* filename)
    {
      return gnuradio::get_initial_sptr
        (new waterfall_plotter_impl(fft_size, filename));
    }

    /*
     * The private constructor
     */
    waterfall_plotter_impl::waterfall_plotter_impl(size_t fft_size, char* filename)
      : gr::sync_block("waterfall_plotter",
              gr::io_signature::make(1, 1, fft_size * sizeof(int8_t)),
              gr::io_signature::make(0, 0, 0)),
        total_size_(0),
        fft_size_(fft_size),
        filename_(filename)
    {}

    /*
     * Our virtual destructor.
     */
    waterfall_plotter_impl::~waterfall_plotter_impl()
    {}

    int
    waterfall_plotter_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      size_t block_size = input_signature()->sizeof_stream_item (0);

      const char *in = (const char *) input_items[0];

      total_size_ += noutput_items * block_size;

      item a;
      char *buffer = new char[noutput_items * block_size];
      memcpy (buffer, in, noutput_items * block_size);
      a.size = noutput_items * block_size;
      a.arr = buffer;
      list_of_arrays_.push_back(a);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

    void waterfall_plotter_impl::init_numpy_array() {
      import_array();
    }

    bool waterfall_plotter_impl::stop() {

      if (list_of_arrays_.begin() == list_of_arrays_.end()) {
      	return true;
      }

      char *numpy_array_buffer = new char[total_size_];
      int copied_so_far = 0;
      for (auto it = list_of_arrays_.cbegin(); it != list_of_arrays_.cend(); it++) {
        std::copy((*it).arr, (*it).arr+(*it).size, numpy_array_buffer + copied_so_far);
        copied_so_far += (*it).size;
        delete[] (*it).arr;
      }

      PyGILState_STATE gstate;
      gstate = PyGILState_Ensure();
      init_numpy_array();

      npy_intp dims[2]{
        static_cast<long int>(total_size_ / (fft_size_ * sizeof(int8_t))),
        static_cast<long int>(fft_size_)};
      const int ND = 2;

      PyObject *numpy_array = NULL, *module_string = NULL, *module = NULL,
        *plot_waterfall_func = NULL, *pyFilename = NULL, *result = NULL;

      numpy_array = PyArray_SimpleNewFromData(
        ND, dims, NPY_INT8, reinterpret_cast<void*>(numpy_array_buffer));
      if (numpy_array == NULL)
        goto error;

      module_string = PyString_FromString((char *)"starcoder");
      if (module_string == NULL)
        goto error;

      module = PyImport_Import(module_string);
      if (module == NULL)
        goto error;

      plot_waterfall_func = PyObject_GetAttrString(module,(char *)"plot_waterfall");
      if (module == NULL)
        goto error;

      pyFilename = PyString_FromString(filename_);
      if (pyFilename == NULL)
        goto error;

      if (filename_[0] != '\0') {
        result = PyObject_CallFunctionObjArgs(plot_waterfall_func, numpy_array, pyFilename, NULL);
      } else {
        result = PyObject_CallFunctionObjArgs(plot_waterfall_func, numpy_array, NULL);
      }
      if (result == NULL)
        goto error;

      Py_ssize_t image_size;
      image_size = PyString_Size(result);
      char *image_buffer;
      image_buffer = PyString_AsString(result);
      if (image_buffer == NULL)
        goto error;

error:
      /* Cleanup code, shared by success and failure path */
      Py_XDECREF(numpy_array);
      Py_XDECREF(module_string);
      Py_XDECREF(module);
      Py_XDECREF(plot_waterfall_func);
      Py_XDECREF(pyFilename);
      Py_XDECREF(result);

      if (PyErr_Occurred()) {
        PyErr_Print();
      }

      PyGILState_Release(gstate);
      delete[] numpy_array_buffer;
      return true;
    }

  } /* namespace starcoder */
} /* namespace gr */
