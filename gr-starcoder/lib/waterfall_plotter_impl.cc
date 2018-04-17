/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
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

#include <Python.h>
#include <gnuradio/io_signature.h>
#include "waterfall_plotter_impl.h"
#include <iostream>

namespace gr {
  namespace starcoder {

    waterfall_plotter::sptr
    waterfall_plotter::make(double samp_rate, double center_freq, double rps, size_t fft_size, char* filename)
    {
      return gnuradio::get_initial_sptr
        (new waterfall_plotter_impl(samp_rate, center_freq, rps, fft_size, filename));
    }

    /*
     * The private constructor
     */
    waterfall_plotter_impl::waterfall_plotter_impl(double samp_rate, double center_freq, double rps, size_t fft_size, char* filename)
      : gr::sync_block("waterfall_plotter",
              gr::io_signature::make(1, 1, fft_size * sizeof(int8_t)),
              gr::io_signature::make(0, 0, 0)),
        totalSize(0),
        blob(new char[0])
    {}

    /*
     * Our virtual destructor.
     */
    waterfall_plotter_impl::~waterfall_plotter_impl()
    {
      delete[] blob;
    }

    int
    waterfall_plotter_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      size_t block_size = input_signature()->sizeof_stream_item (0);

      const char *in = (const char *) input_items[0];

      totalSize += noutput_items * block_size;

      item a;
      char *buffer = new char[noutput_items * block_size];
      memcpy (buffer, in, noutput_items * block_size);
      a.size = noutput_items * block_size;
      a.arr = buffer;
      listOfArrays.push_back(a);

      std::cout << block_size << " block size" << std::endl;
      std::cout << noutput_items << " output items" << std::endl;
      for (int i = 0; i < a.size; i ++) {
        std::cout << static_cast<unsigned>(buffer[i]) << " buffer" << std::endl;
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

    bool waterfall_plotter_impl::stop() {

      if (listOfArrays.begin() == listOfArrays.end()) {
      	std::cout << "list empty." << std::endl;
      	return true;
      }

      char *numpyArrayBuffer = new char[totalSize];
      int copiedSoFar = 0;
      for (auto it = listOfArrays.cbegin(); it != listOfArrays.cend(); it++) {
        std::cout << (*it).size << " size buffer" << std::endl;
        std::copy((*it).arr, (*it).arr+(*it).size, numpyArrayBuffer + copiedSoFar);
        copiedSoFar += (*it).size;
        delete[] (*it).arr;
      }

      for (int i = 0; i < totalSize; i ++) {
        std::cout << static_cast<unsigned>(numpyArrayBuffer[i]) << " numpyarraybuffer" << std::endl;
      }

      PyGILState_STATE gstate;
      gstate = PyGILState_Ensure();

      PyObject* myModuleString = PyString_FromString((char *)"starcoder");
      if (!myModuleString) {
        std::cerr << "null starcoder string" << std::endl;
        PyGILState_Release(gstate);
        return false;
      }
      PyObject* myModule = PyImport_Import(myModuleString);
      if (!myModule) {
        std::cerr << "failed to import module" << std::endl;
        PyGILState_Release(gstate);
        return false;
      }
      PyObject* myFunction = PyObject_GetAttrString(myModule,(char *)"plot_waterfall");
      if (!myModule) {
        std::cerr << "failed to retrieve plot_waterfall" << std::endl;
        PyGILState_Release(gstate);
        return false;
      }
      PyObject* myResult = PyObject_CallObject(myFunction, NULL);
      if (!myResult) {
        std::cerr << "failed to call function" << std::endl;
        PyGILState_Release(gstate);
        return false;
      }
      Py_ssize_t imageSize = PyString_Size(myResult);
      std::cout << imageSize << " = Retrieved image size" << std::endl;
      char *imageBuffer = PyString_AsString(myResult);
      if (!imageBuffer) {
        std::cerr << "failed to retrieve image string" << std::endl;
        PyGILState_Release(gstate);
        return false;
      }

      blob = new char[imageSize];
      memcpy(blob, imageBuffer, imageSize);

      Py_DECREF(myModuleString);
      Py_DECREF(myModule);
      Py_DECREF(myFunction);
      Py_DECREF(myResult);

      PyGILState_Release(gstate);
      return true;
    }

  } /* namespace starcoder */
} /* namespace gr */
