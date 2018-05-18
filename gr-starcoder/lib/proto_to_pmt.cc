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

#include "proto_to_pmt.h"

pmt::pmt_t convert_pmt_proto(const starcoder::PMT &grpc_msg) {
  starcoder::PMT::PmtOneofCase type = grpc_msg.pmt_oneof_case();
  switch (type) {
    case starcoder::PMT::PmtOneofCase::kBooleanValue:
      return pmt::from_bool(grpc_msg.boolean_value());
    case starcoder::PMT::PmtOneofCase::kSymbolValue:
      return pmt::string_to_symbol(grpc_msg.symbol_value());
    case starcoder::PMT::PmtOneofCase::kIntegerValue:
      return pmt::from_long(grpc_msg.integer_value());
    case starcoder::PMT::PmtOneofCase::kDoubleValue:
      return pmt::from_double(grpc_msg.double_value());
    case starcoder::PMT::PmtOneofCase::kComplexValue:
      return pmt::from_complex(grpc_msg.complex_value().real_value(),
                               grpc_msg.complex_value().imaginary_value());
    case starcoder::PMT::PmtOneofCase::kPmtPairValue:
      return pmt::cons(convert_pmt_proto(grpc_msg.pmt_pair_value().car()),
                       convert_pmt_proto(grpc_msg.pmt_pair_value().cdr()));
    case starcoder::PMT::PmtOneofCase::kPmtTupleValue:
      return convert_pmt_tuple(grpc_msg.pmt_tuple_value());
    case starcoder::PMT::PmtOneofCase::kPmtVectorValue:
      return convert_pmt_vector(grpc_msg.pmt_vector_value());
    case starcoder::PMT::PmtOneofCase::kPmtUniformVectorValue:
      return convert_pmt_uniform_vector(grpc_msg.pmt_uniform_vector_value());
    case starcoder::PMT::PmtOneofCase::kPmtDictValue:
      return convert_pmt_dict(grpc_msg.pmt_dict_value());
  }
  return pmt::get_PMT_NIL();
}

pmt::pmt_t convert_pmt_tuple(const starcoder::PMTTuple &grpc_pmt_tuple) {
  int size = grpc_pmt_tuple.pmt_size();
  switch (size) {
    case 0:
      return pmt::make_tuple();
    case 1:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)));
    case 2:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(1)));
    case 3:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(2)));
    case 4:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(3)));
    case 5:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(4)));
    case 6:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(4)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(5)));
    case 7:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(4)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(5)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(6)));
    case 8:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(4)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(5)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(6)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(7)));
    case 9:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(4)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(5)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(6)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(7)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(8)));
    case 10:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(4)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(5)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(6)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(7)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(8)),
                             convert_pmt_proto(grpc_pmt_tuple.pmt(9)));
    default:
      throw("PMT tuple sizes >10 not supported");
  }
}

pmt::pmt_t convert_pmt_vector(const starcoder::PMTVector &grpc_pmt_vector) {
  pmt::pmt_t vec =
      pmt::make_vector(grpc_pmt_vector.pmt_size(), pmt::get_PMT_NIL());
  for (int i = 0; i < grpc_pmt_vector.pmt_size(); i++) {
    pmt::vector_set(vec, i, convert_pmt_proto(grpc_pmt_vector.pmt(i)));
  }
  return vec;
}

pmt::pmt_t convert_pmt_uniform_vector(
    const starcoder::PMTUniformVector &grpc_pmt_uniform_vector) {
  starcoder::PMTUniformVector::PmtUniformVectorOneofCase type =
      grpc_pmt_uniform_vector.pmt_uniform_vector_oneof_case();
  switch (type) {
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kU8Value: {
      std::string str = grpc_pmt_uniform_vector.u8_value().payload();
      std::vector<uint8_t> vec(str.begin(), str.end());
      return pmt::init_u8vector(str.size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kI8Value: {
      std::vector<int8_t> vec(
          grpc_pmt_uniform_vector.i8_value().payload().begin(),
          grpc_pmt_uniform_vector.i8_value().payload().end());
      return pmt::init_s8vector(
          grpc_pmt_uniform_vector.i8_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kU16Value: {
      std::vector<uint16_t> vec(
          grpc_pmt_uniform_vector.u16_value().payload().begin(),
          grpc_pmt_uniform_vector.u16_value().payload().end());
      return pmt::init_u16vector(
          grpc_pmt_uniform_vector.u16_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kI16Value: {
      std::vector<int16_t> vec(
          grpc_pmt_uniform_vector.i16_value().payload().begin(),
          grpc_pmt_uniform_vector.i16_value().payload().end());
      return pmt::init_s16vector(
          grpc_pmt_uniform_vector.i16_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kU32Value: {
      std::vector<uint32_t> vec(
          grpc_pmt_uniform_vector.u32_value().payload().begin(),
          grpc_pmt_uniform_vector.u32_value().payload().end());
      return pmt::init_u32vector(
          grpc_pmt_uniform_vector.u32_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kI32Value: {
      std::vector<int32_t> vec(
          grpc_pmt_uniform_vector.i32_value().payload().begin(),
          grpc_pmt_uniform_vector.i32_value().payload().end());
      return pmt::init_s32vector(
          grpc_pmt_uniform_vector.i32_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kU64Value: {
      std::vector<uint64_t> vec(
          grpc_pmt_uniform_vector.u64_value().payload().begin(),
          grpc_pmt_uniform_vector.u64_value().payload().end());
      return pmt::init_u64vector(
          grpc_pmt_uniform_vector.u64_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kI64Value: {
      std::vector<int64_t> vec(
          grpc_pmt_uniform_vector.i64_value().payload().begin(),
          grpc_pmt_uniform_vector.i64_value().payload().end());
      return pmt::init_s64vector(
          grpc_pmt_uniform_vector.i64_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kF32Value: {
      std::vector<float> vec(
          grpc_pmt_uniform_vector.f32_value().payload().begin(),
          grpc_pmt_uniform_vector.f32_value().payload().end());
      return pmt::init_f32vector(
          grpc_pmt_uniform_vector.f32_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kF64Value: {
      std::vector<double> vec(
          grpc_pmt_uniform_vector.f64_value().payload().begin(),
          grpc_pmt_uniform_vector.f64_value().payload().end());
      return pmt::init_f64vector(
          grpc_pmt_uniform_vector.f64_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kC32Value: {
      std::vector<std::complex<float>> vec;
      std::transform(grpc_pmt_uniform_vector.c32_value().payload().begin(),
                     grpc_pmt_uniform_vector.c32_value().payload().end(),
                     std::back_inserter(vec),
                     [](starcoder::Complex32 c)->std::complex<float> {
        return std::complex<float>(c.real_value(), c.imaginary_value());
      });
      return pmt::init_c32vector(
          grpc_pmt_uniform_vector.c32_value().payload().size(), vec);
    }
    case starcoder::PMTUniformVector::PmtUniformVectorOneofCase::kC64Value: {
      std::vector<std::complex<double>> vec;
      std::transform(grpc_pmt_uniform_vector.c64_value().payload().begin(),
                     grpc_pmt_uniform_vector.c64_value().payload().end(),
                     std::back_inserter(vec),
                     [](starcoder::Complex c)->std::complex<double> {
        return std::complex<double>(c.real_value(), c.imaginary_value());
      });
      return pmt::init_c64vector(
          grpc_pmt_uniform_vector.c64_value().payload().size(), vec);
    }
    default:
      throw("Type of uniform vector not set");
  }
}

pmt::pmt_t convert_pmt_dict(const starcoder::PMTDict &grpc_pmt_dict) {
  pmt::pmt_t dict = pmt::make_dict();
  for (int i = 0; i < grpc_pmt_dict.entries_size(); i++) {
    dict =
        pmt::dict_add(dict, convert_pmt_proto(grpc_pmt_dict.entries(i).key()),
                      convert_pmt_proto(grpc_pmt_dict.entries(i).value()));
  }
  return dict;
}
