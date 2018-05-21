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

pmt::pmt_t convert_pmt_proto(const starcoder::BlockMessage &grpc_msg) {
  starcoder::BlockMessage::MessageOneofCase type =
      grpc_msg.message_oneof_case();
  switch (type) {
    case starcoder::BlockMessage::MessageOneofCase::kBooleanValue:
      return pmt::from_bool(grpc_msg.boolean_value());
    case starcoder::BlockMessage::MessageOneofCase::kSymbolValue:
      return pmt::string_to_symbol(grpc_msg.symbol_value());
    case starcoder::BlockMessage::MessageOneofCase::kIntegerValue:
      return pmt::from_long(grpc_msg.integer_value());
    case starcoder::BlockMessage::MessageOneofCase::kDoubleValue:
      return pmt::from_double(grpc_msg.double_value());
    case starcoder::BlockMessage::MessageOneofCase::kComplexValue:
      return pmt::from_complex(grpc_msg.complex_value().real_value(),
                               grpc_msg.complex_value().imaginary_value());
    case starcoder::BlockMessage::MessageOneofCase::kPairValue:
      return pmt::cons(convert_pmt_proto(grpc_msg.pair_value().car()),
                       convert_pmt_proto(grpc_msg.pair_value().cdr()));
    case starcoder::BlockMessage::MessageOneofCase::kListValue:
      if (grpc_msg.list_value().type() == starcoder::List::TUPLE)
        return convert_pmt_tuple(grpc_msg.list_value());
      else
        return convert_pmt_vector(grpc_msg.list_value());
    case starcoder::BlockMessage::MessageOneofCase::kUniformVectorValue:
      return convert_pmt_uniform_vector(grpc_msg.uniform_vector_value());
    case starcoder::BlockMessage::MessageOneofCase::kDictValue:
      return convert_pmt_dict(grpc_msg.dict_value());
  }
  return pmt::get_PMT_NIL();
}

pmt::pmt_t convert_pmt_tuple(const starcoder::List &grpc_pmt_tuple) {
  int size = grpc_pmt_tuple.value_size();
  switch (size) {
    case 0:
      return pmt::make_tuple();
    case 1:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)));
    case 2:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)),
                             convert_pmt_proto(grpc_pmt_tuple.value(1)));
    case 3:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)),
                             convert_pmt_proto(grpc_pmt_tuple.value(1)),
                             convert_pmt_proto(grpc_pmt_tuple.value(2)));
    case 4:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)),
                             convert_pmt_proto(grpc_pmt_tuple.value(1)),
                             convert_pmt_proto(grpc_pmt_tuple.value(2)),
                             convert_pmt_proto(grpc_pmt_tuple.value(3)));
    case 5:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)),
                             convert_pmt_proto(grpc_pmt_tuple.value(1)),
                             convert_pmt_proto(grpc_pmt_tuple.value(2)),
                             convert_pmt_proto(grpc_pmt_tuple.value(3)),
                             convert_pmt_proto(grpc_pmt_tuple.value(4)));
    case 6:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)),
                             convert_pmt_proto(grpc_pmt_tuple.value(1)),
                             convert_pmt_proto(grpc_pmt_tuple.value(2)),
                             convert_pmt_proto(grpc_pmt_tuple.value(3)),
                             convert_pmt_proto(grpc_pmt_tuple.value(4)),
                             convert_pmt_proto(grpc_pmt_tuple.value(5)));
    case 7:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)),
                             convert_pmt_proto(grpc_pmt_tuple.value(1)),
                             convert_pmt_proto(grpc_pmt_tuple.value(2)),
                             convert_pmt_proto(grpc_pmt_tuple.value(3)),
                             convert_pmt_proto(grpc_pmt_tuple.value(4)),
                             convert_pmt_proto(grpc_pmt_tuple.value(5)),
                             convert_pmt_proto(grpc_pmt_tuple.value(6)));
    case 8:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)),
                             convert_pmt_proto(grpc_pmt_tuple.value(1)),
                             convert_pmt_proto(grpc_pmt_tuple.value(2)),
                             convert_pmt_proto(grpc_pmt_tuple.value(3)),
                             convert_pmt_proto(grpc_pmt_tuple.value(4)),
                             convert_pmt_proto(grpc_pmt_tuple.value(5)),
                             convert_pmt_proto(grpc_pmt_tuple.value(6)),
                             convert_pmt_proto(grpc_pmt_tuple.value(7)));
    case 9:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)),
                             convert_pmt_proto(grpc_pmt_tuple.value(1)),
                             convert_pmt_proto(grpc_pmt_tuple.value(2)),
                             convert_pmt_proto(grpc_pmt_tuple.value(3)),
                             convert_pmt_proto(grpc_pmt_tuple.value(4)),
                             convert_pmt_proto(grpc_pmt_tuple.value(5)),
                             convert_pmt_proto(grpc_pmt_tuple.value(6)),
                             convert_pmt_proto(grpc_pmt_tuple.value(7)),
                             convert_pmt_proto(grpc_pmt_tuple.value(8)));
    case 10:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.value(0)),
                             convert_pmt_proto(grpc_pmt_tuple.value(1)),
                             convert_pmt_proto(grpc_pmt_tuple.value(2)),
                             convert_pmt_proto(grpc_pmt_tuple.value(3)),
                             convert_pmt_proto(grpc_pmt_tuple.value(4)),
                             convert_pmt_proto(grpc_pmt_tuple.value(5)),
                             convert_pmt_proto(grpc_pmt_tuple.value(6)),
                             convert_pmt_proto(grpc_pmt_tuple.value(7)),
                             convert_pmt_proto(grpc_pmt_tuple.value(8)),
                             convert_pmt_proto(grpc_pmt_tuple.value(9)));
    default:
      throw("PMT tuple sizes >10 not supported");
  }
}

pmt::pmt_t convert_pmt_vector(const starcoder::List &grpc_pmt_vector) {
  pmt::pmt_t vec =
      pmt::make_vector(grpc_pmt_vector.value_size(), pmt::get_PMT_NIL());
  for (int i = 0; i < grpc_pmt_vector.value_size(); i++) {
    pmt::vector_set(vec, i, convert_pmt_proto(grpc_pmt_vector.value(i)));
  }
  return vec;
}

pmt::pmt_t convert_pmt_uniform_vector(
    const starcoder::UniformVector &grpc_pmt_uniform_vector) {
  starcoder::UniformVector::UniformVectorOneofCase type =
      grpc_pmt_uniform_vector.uniform_vector_oneof_case();
  switch (type) {
    case starcoder::UniformVector::UniformVectorOneofCase::kUValue: {
      switch (grpc_pmt_uniform_vector.u_value().size()) {
        case starcoder::IntSize::Size8: {
          std::vector<uint8_t> vec(
              grpc_pmt_uniform_vector.u_value().value().begin(),
              grpc_pmt_uniform_vector.u_value().value().end());
          return pmt::init_u8vector(
              grpc_pmt_uniform_vector.u_value().value().size(), vec);
        }
        case starcoder::IntSize::Size16: {
          std::vector<uint16_t> vec(
              grpc_pmt_uniform_vector.u_value().value().begin(),
              grpc_pmt_uniform_vector.u_value().value().end());
          return pmt::init_u16vector(
              grpc_pmt_uniform_vector.u_value().value().size(), vec);
        }
        case starcoder::IntSize::Size32: {
          std::vector<uint32_t> vec(
              grpc_pmt_uniform_vector.u_value().value().begin(),
              grpc_pmt_uniform_vector.u_value().value().end());
          return pmt::init_u32vector(
              grpc_pmt_uniform_vector.u_value().value().size(), vec);
        }
      }
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kIValue: {
      switch (grpc_pmt_uniform_vector.i_value().size()) {
        case starcoder::IntSize::Size8: {
          std::vector<int8_t> vec(
              grpc_pmt_uniform_vector.i_value().value().begin(),
              grpc_pmt_uniform_vector.i_value().value().end());
          return pmt::init_s8vector(
              grpc_pmt_uniform_vector.i_value().value().size(), vec);
        }
        case starcoder::IntSize::Size16: {
          std::vector<int16_t> vec(
              grpc_pmt_uniform_vector.i_value().value().begin(),
              grpc_pmt_uniform_vector.i_value().value().end());
          return pmt::init_s16vector(
              grpc_pmt_uniform_vector.i_value().value().size(), vec);
        }
        case starcoder::IntSize::Size32: {
          std::vector<int32_t> vec(
              grpc_pmt_uniform_vector.i_value().value().begin(),
              grpc_pmt_uniform_vector.i_value().value().end());
          return pmt::init_s32vector(
              grpc_pmt_uniform_vector.i_value().value().size(), vec);
        }
      }
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kU64Value: {
      std::vector<uint64_t> vec(
          grpc_pmt_uniform_vector.u64_value().value().begin(),
          grpc_pmt_uniform_vector.u64_value().value().end());
      return pmt::init_u64vector(
          grpc_pmt_uniform_vector.u64_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kI64Value: {
      std::vector<int64_t> vec(
          grpc_pmt_uniform_vector.i64_value().value().begin(),
          grpc_pmt_uniform_vector.i64_value().value().end());
      return pmt::init_s64vector(
          grpc_pmt_uniform_vector.i64_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kF32Value: {
      std::vector<float> vec(
          grpc_pmt_uniform_vector.f32_value().value().begin(),
          grpc_pmt_uniform_vector.f32_value().value().end());
      return pmt::init_f32vector(
          grpc_pmt_uniform_vector.f32_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kF64Value: {
      std::vector<double> vec(
          grpc_pmt_uniform_vector.f64_value().value().begin(),
          grpc_pmt_uniform_vector.f64_value().value().end());
      return pmt::init_f64vector(
          grpc_pmt_uniform_vector.f64_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kC32Value: {
      std::vector<std::complex<float>> vec;
      std::transform(grpc_pmt_uniform_vector.c32_value().value().begin(),
                     grpc_pmt_uniform_vector.c32_value().value().end(),
                     std::back_inserter(vec),
                     [](starcoder::Complex32 c)->std::complex<float> {
        return std::complex<float>(c.real_value(), c.imaginary_value());
      });
      return pmt::init_c32vector(
          grpc_pmt_uniform_vector.c32_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kC64Value: {
      std::vector<std::complex<double>> vec;
      std::transform(grpc_pmt_uniform_vector.c64_value().value().begin(),
                     grpc_pmt_uniform_vector.c64_value().value().end(),
                     std::back_inserter(vec),
                     [](starcoder::Complex c)->std::complex<double> {
        return std::complex<double>(c.real_value(), c.imaginary_value());
      });
      return pmt::init_c64vector(
          grpc_pmt_uniform_vector.c64_value().value().size(), vec);
    }
    default:
      throw("Type of uniform vector not set");
  }
}

pmt::pmt_t convert_pmt_dict(const starcoder::Dict &grpc_pmt_dict) {
  pmt::pmt_t dict = pmt::make_dict();
  for (int i = 0; i < grpc_pmt_dict.entries_size(); i++) {
    dict =
        pmt::dict_add(dict, convert_pmt_proto(grpc_pmt_dict.entries(i).key()),
                      convert_pmt_proto(grpc_pmt_dict.entries(i).value()));
  }
  return dict;
}
