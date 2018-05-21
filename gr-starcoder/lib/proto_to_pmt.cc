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

pmt::pmt_t convert_pmt_proto(const starcoder::BlockMessage &proto_msg) {
  starcoder::BlockMessage::MessageOneofCase type =
      proto_msg.message_oneof_case();
  switch (type) {
    case starcoder::BlockMessage::MessageOneofCase::kBooleanValue:
      return pmt::from_bool(proto_msg.boolean_value());
    case starcoder::BlockMessage::MessageOneofCase::kSymbolValue:
      return pmt::string_to_symbol(proto_msg.symbol_value());
    case starcoder::BlockMessage::MessageOneofCase::kIntegerValue:
      return pmt::from_long(proto_msg.integer_value());
    case starcoder::BlockMessage::MessageOneofCase::kDoubleValue:
      return pmt::from_double(proto_msg.double_value());
    case starcoder::BlockMessage::MessageOneofCase::kComplexValue:
      return pmt::from_complex(proto_msg.complex_value().real_value(),
                               proto_msg.complex_value().imaginary_value());
    case starcoder::BlockMessage::MessageOneofCase::kPairValue:
      return pmt::cons(convert_pmt_proto(proto_msg.pair_value().car()),
                       convert_pmt_proto(proto_msg.pair_value().cdr()));
    case starcoder::BlockMessage::MessageOneofCase::kListValue:
      return convert_pmt_list(proto_msg.list_value());
    case starcoder::BlockMessage::MessageOneofCase::kUniformVectorValue:
      return convert_pmt_uniform_vector(proto_msg.uniform_vector_value());
    case starcoder::BlockMessage::MessageOneofCase::kDictValue:
      return convert_pmt_dict(proto_msg.dict_value());
  }
  return pmt::get_PMT_NIL();
}

pmt::pmt_t convert_pmt_list(const starcoder::List &proto_pmt_list) {
  int size = proto_pmt_list.value_size();
  if (proto_pmt_list.type() == starcoder::List::TUPLE) {
    switch (size) {
      case 0:
        return pmt::make_tuple();
      case 1:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)));
      case 2:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)),
                               convert_pmt_proto(proto_pmt_list.value(1)));
      case 3:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)),
                               convert_pmt_proto(proto_pmt_list.value(1)),
                               convert_pmt_proto(proto_pmt_list.value(2)));
      case 4:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)),
                               convert_pmt_proto(proto_pmt_list.value(1)),
                               convert_pmt_proto(proto_pmt_list.value(2)),
                               convert_pmt_proto(proto_pmt_list.value(3)));
      case 5:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)),
                               convert_pmt_proto(proto_pmt_list.value(1)),
                               convert_pmt_proto(proto_pmt_list.value(2)),
                               convert_pmt_proto(proto_pmt_list.value(3)),
                               convert_pmt_proto(proto_pmt_list.value(4)));
      case 6:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)),
                               convert_pmt_proto(proto_pmt_list.value(1)),
                               convert_pmt_proto(proto_pmt_list.value(2)),
                               convert_pmt_proto(proto_pmt_list.value(3)),
                               convert_pmt_proto(proto_pmt_list.value(4)),
                               convert_pmt_proto(proto_pmt_list.value(5)));
      case 7:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)),
                               convert_pmt_proto(proto_pmt_list.value(1)),
                               convert_pmt_proto(proto_pmt_list.value(2)),
                               convert_pmt_proto(proto_pmt_list.value(3)),
                               convert_pmt_proto(proto_pmt_list.value(4)),
                               convert_pmt_proto(proto_pmt_list.value(5)),
                               convert_pmt_proto(proto_pmt_list.value(6)));
      case 8:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)),
                               convert_pmt_proto(proto_pmt_list.value(1)),
                               convert_pmt_proto(proto_pmt_list.value(2)),
                               convert_pmt_proto(proto_pmt_list.value(3)),
                               convert_pmt_proto(proto_pmt_list.value(4)),
                               convert_pmt_proto(proto_pmt_list.value(5)),
                               convert_pmt_proto(proto_pmt_list.value(6)),
                               convert_pmt_proto(proto_pmt_list.value(7)));
      case 9:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)),
                               convert_pmt_proto(proto_pmt_list.value(1)),
                               convert_pmt_proto(proto_pmt_list.value(2)),
                               convert_pmt_proto(proto_pmt_list.value(3)),
                               convert_pmt_proto(proto_pmt_list.value(4)),
                               convert_pmt_proto(proto_pmt_list.value(5)),
                               convert_pmt_proto(proto_pmt_list.value(6)),
                               convert_pmt_proto(proto_pmt_list.value(7)),
                               convert_pmt_proto(proto_pmt_list.value(8)));
      case 10:
        return pmt::make_tuple(convert_pmt_proto(proto_pmt_list.value(0)),
                               convert_pmt_proto(proto_pmt_list.value(1)),
                               convert_pmt_proto(proto_pmt_list.value(2)),
                               convert_pmt_proto(proto_pmt_list.value(3)),
                               convert_pmt_proto(proto_pmt_list.value(4)),
                               convert_pmt_proto(proto_pmt_list.value(5)),
                               convert_pmt_proto(proto_pmt_list.value(6)),
                               convert_pmt_proto(proto_pmt_list.value(7)),
                               convert_pmt_proto(proto_pmt_list.value(8)),
                               convert_pmt_proto(proto_pmt_list.value(9)));
      default:
        throw("PMT tuple sizes >10 not supported");
    }
  } else if (proto_pmt_list.type() == starcoder::List::VECTOR) {
    pmt::pmt_t vec =
        pmt::make_vector(proto_pmt_list.value_size(), pmt::get_PMT_NIL());
    for (int i = 0; i < proto_pmt_list.value_size(); i++) {
      pmt::vector_set(vec, i, convert_pmt_proto(proto_pmt_list.value(i)));
    }
    return vec;
  } else
    throw("Invalid List type");
}

pmt::pmt_t convert_pmt_uniform_vector(
    const starcoder::UniformVector &proto_pmt_uniform_vector) {
  starcoder::UniformVector::UniformVectorOneofCase type =
      proto_pmt_uniform_vector.uniform_vector_oneof_case();
  switch (type) {
    case starcoder::UniformVector::UniformVectorOneofCase::kUValue: {
      switch (proto_pmt_uniform_vector.u_value().size()) {
        case starcoder::IntSize::Size8: {
          std::vector<uint8_t> vec(
              proto_pmt_uniform_vector.u_value().value().begin(),
              proto_pmt_uniform_vector.u_value().value().end());
          return pmt::init_u8vector(
              proto_pmt_uniform_vector.u_value().value().size(), vec);
        }
        case starcoder::IntSize::Size16: {
          std::vector<uint16_t> vec(
              proto_pmt_uniform_vector.u_value().value().begin(),
              proto_pmt_uniform_vector.u_value().value().end());
          return pmt::init_u16vector(
              proto_pmt_uniform_vector.u_value().value().size(), vec);
        }
        case starcoder::IntSize::Size32: {
          std::vector<uint32_t> vec(
              proto_pmt_uniform_vector.u_value().value().begin(),
              proto_pmt_uniform_vector.u_value().value().end());
          return pmt::init_u32vector(
              proto_pmt_uniform_vector.u_value().value().size(), vec);
        }
      }
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kIValue: {
      switch (proto_pmt_uniform_vector.i_value().size()) {
        case starcoder::IntSize::Size8: {
          std::vector<int8_t> vec(
              proto_pmt_uniform_vector.i_value().value().begin(),
              proto_pmt_uniform_vector.i_value().value().end());
          return pmt::init_s8vector(
              proto_pmt_uniform_vector.i_value().value().size(), vec);
        }
        case starcoder::IntSize::Size16: {
          std::vector<int16_t> vec(
              proto_pmt_uniform_vector.i_value().value().begin(),
              proto_pmt_uniform_vector.i_value().value().end());
          return pmt::init_s16vector(
              proto_pmt_uniform_vector.i_value().value().size(), vec);
        }
        case starcoder::IntSize::Size32: {
          std::vector<int32_t> vec(
              proto_pmt_uniform_vector.i_value().value().begin(),
              proto_pmt_uniform_vector.i_value().value().end());
          return pmt::init_s32vector(
              proto_pmt_uniform_vector.i_value().value().size(), vec);
        }
      }
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kU64Value: {
      std::vector<uint64_t> vec(
          proto_pmt_uniform_vector.u64_value().value().begin(),
          proto_pmt_uniform_vector.u64_value().value().end());
      return pmt::init_u64vector(
          proto_pmt_uniform_vector.u64_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kI64Value: {
      std::vector<int64_t> vec(
          proto_pmt_uniform_vector.i64_value().value().begin(),
          proto_pmt_uniform_vector.i64_value().value().end());
      return pmt::init_s64vector(
          proto_pmt_uniform_vector.i64_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kF32Value: {
      std::vector<float> vec(
          proto_pmt_uniform_vector.f32_value().value().begin(),
          proto_pmt_uniform_vector.f32_value().value().end());
      return pmt::init_f32vector(
          proto_pmt_uniform_vector.f32_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kF64Value: {
      std::vector<double> vec(
          proto_pmt_uniform_vector.f64_value().value().begin(),
          proto_pmt_uniform_vector.f64_value().value().end());
      return pmt::init_f64vector(
          proto_pmt_uniform_vector.f64_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kC32Value: {
      std::vector<std::complex<float>> vec;
      std::transform(proto_pmt_uniform_vector.c32_value().value().begin(),
                     proto_pmt_uniform_vector.c32_value().value().end(),
                     std::back_inserter(vec),
                     [](starcoder::Complex32 c)->std::complex<float> {
        return std::complex<float>(c.real_value(), c.imaginary_value());
      });
      return pmt::init_c32vector(
          proto_pmt_uniform_vector.c32_value().value().size(), vec);
    }
    case starcoder::UniformVector::UniformVectorOneofCase::kC64Value: {
      std::vector<std::complex<double>> vec;
      std::transform(proto_pmt_uniform_vector.c64_value().value().begin(),
                     proto_pmt_uniform_vector.c64_value().value().end(),
                     std::back_inserter(vec),
                     [](starcoder::Complex c)->std::complex<double> {
        return std::complex<double>(c.real_value(), c.imaginary_value());
      });
      return pmt::init_c64vector(
          proto_pmt_uniform_vector.c64_value().value().size(), vec);
    }
    default:
      throw("Type of uniform vector not set");
  }
}

pmt::pmt_t convert_pmt_dict(const starcoder::Dict &proto_pmt_dict) {
  pmt::pmt_t dict = pmt::make_dict();
  for (const starcoder::Dict_Entry &entry : proto_pmt_dict.entry()) {
    dict = pmt::dict_add(dict, convert_pmt_proto(entry.key()),
                         convert_pmt_proto(entry.value()));
  }
  return dict;
}
