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

#include "pmt_to_proto.h"

void convert_proto_uniform_vector(const pmt::pmt_t &pmt_msg,
                                  starcoder::UniformVector *uni_vector) {
  if (pmt::is_u8vector(pmt_msg)) {
    starcoder::UVector *u_vector = uni_vector->mutable_u_value();
    u_vector->set_size(starcoder::IntSize::Size8);
    const std::vector<uint8_t> vector_elements =
        pmt::u8vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<uint32_t>(
            u_vector->mutable_value()));
  } else if (pmt::is_s8vector(pmt_msg)) {
    starcoder::IVector *i_vector = uni_vector->mutable_i_value();
    i_vector->set_size(starcoder::IntSize::Size8);
    const std::vector<int8_t> vector_elements = pmt::s8vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<int32_t>(
            i_vector->mutable_value()));
  } else if (pmt::is_u16vector(pmt_msg)) {
    starcoder::UVector *u_vector = uni_vector->mutable_u_value();
    u_vector->set_size(starcoder::IntSize::Size16);
    const std::vector<uint16_t> vector_elements =
        pmt::u16vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<uint32_t>(
            u_vector->mutable_value()));
  } else if (pmt::is_s16vector(pmt_msg)) {
    starcoder::IVector *i_vector = uni_vector->mutable_i_value();
    i_vector->set_size(starcoder::IntSize::Size16);
    const std::vector<int16_t> vector_elements =
        pmt::s16vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<int32_t>(
            i_vector->mutable_value()));
  } else if (pmt::is_u32vector(pmt_msg)) {
    starcoder::UVector *u_vector = uni_vector->mutable_u_value();
    u_vector->set_size(starcoder::IntSize::Size32);
    const std::vector<uint32_t> vector_elements =
        pmt::u32vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<uint32_t>(
            u_vector->mutable_value()));
  } else if (pmt::is_s32vector(pmt_msg)) {
    starcoder::IVector *i_vector = uni_vector->mutable_i_value();
    i_vector->set_size(starcoder::IntSize::Size32);
    const std::vector<int32_t> vector_elements =
        pmt::s32vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<int32_t>(
            i_vector->mutable_value()));
  } else if (pmt::is_u64vector(pmt_msg)) {
    starcoder::U64Vector *u64_vector = uni_vector->mutable_u64_value();
    const std::vector<uint64_t> vector_elements =
        pmt::u64vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<uint64_t>(
            u64_vector->mutable_value()));
  } else if (pmt::is_s64vector(pmt_msg)) {
    starcoder::I64Vector *i64_vector = uni_vector->mutable_i64_value();
    const std::vector<int64_t> vector_elements =
        pmt::s64vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<int64_t>(
            i64_vector->mutable_value()));
  } else if (pmt::is_f32vector(pmt_msg)) {
    starcoder::F32Vector *f32_vector = uni_vector->mutable_f32_value();
    const std::vector<float> vector_elements = pmt::f32vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<float>(
            f32_vector->mutable_value()));
  } else if (pmt::is_f64vector(pmt_msg)) {
    starcoder::F64Vector *f64_vector = uni_vector->mutable_f64_value();
    const std::vector<double> vector_elements =
        pmt::f64vector_elements(pmt_msg);
    std::copy(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedFieldBackInsertIterator<double>(
            f64_vector->mutable_value()));
  } else if (pmt::is_c32vector(pmt_msg)) {
    starcoder::C32Vector *c32_vector = uni_vector->mutable_c32_value();
    const std::vector<std::complex<float>> vector_elements =
        pmt::c32vector_elements(pmt_msg);
    std::transform(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedPtrFieldBackInsertIterator<
            starcoder::Complex32>(c32_vector->mutable_value()),
        [](std::complex<float> c)->starcoder::Complex32 {
      starcoder::Complex32 new_val;
      new_val.set_real_value(c.real());
      new_val.set_imaginary_value(c.imag());
      return new_val;
    });
  } else if (pmt::is_c64vector(pmt_msg)) {
    starcoder::C64Vector *c64_vector = uni_vector->mutable_c64_value();
    const std::vector<std::complex<double>> vector_elements =
        pmt::c64vector_elements(pmt_msg);
    std::transform(
        vector_elements.begin(), vector_elements.end(),
        google::protobuf::internal::RepeatedPtrFieldBackInsertIterator<
            starcoder::Complex>(c64_vector->mutable_value()),
        [](std::complex<double> c)->starcoder::Complex {
      starcoder::Complex new_val;
      new_val.set_real_value(c.real());
      new_val.set_imaginary_value(c.imag());
      return new_val;
    });
  }
}

starcoder::BlockMessage convert_pmt_to_proto(const pmt::pmt_t &pmt_msg) {
  starcoder::BlockMessage proto_msg;
  if (pmt::is_blob(pmt_msg)) {
    proto_msg.set_blob_value(pmt::blob_data(pmt_msg),
                             pmt::blob_length(pmt_msg));
  } else if (pmt::is_uniform_vector(pmt_msg)) {
    convert_proto_uniform_vector(pmt_msg,
                                 proto_msg.mutable_uniform_vector_value());
  } else if (pmt::is_bool(pmt_msg)) {
    proto_msg.set_boolean_value(pmt::to_bool(pmt_msg));
  } else if (pmt::is_symbol(pmt_msg)) {
    proto_msg.set_symbol_value(pmt::symbol_to_string(pmt_msg));
  } else if (pmt::is_integer(pmt_msg)) {
    proto_msg.set_integer_value(pmt::to_long(pmt_msg));
  } else if (pmt::is_uint64(pmt_msg)) {
    proto_msg.set_integer_value(pmt::to_uint64(pmt_msg));
  } else if (pmt::is_real(pmt_msg)) {
    proto_msg.set_double_value(pmt::to_double(pmt_msg));
  } else if (pmt::is_complex(pmt_msg)) {
    std::complex<double> val = pmt::to_complex(pmt_msg);
    starcoder::Complex *complex = proto_msg.mutable_complex_value();
    complex->set_real_value(val.real());
    complex->set_imaginary_value(val.imag());
  } else if (pmt::is_pair(pmt_msg)) {
    starcoder::Pair *pair = proto_msg.mutable_pair_value();
    starcoder::BlockMessage car = convert_pmt_to_proto(pmt::car(pmt_msg));
    starcoder::BlockMessage cdr = convert_pmt_to_proto(pmt::cdr(pmt_msg));
    pair->mutable_car()->Swap(&car);
    pair->mutable_cdr()->Swap(&cdr);
  } else if (pmt::is_tuple(pmt_msg)) {
    starcoder::List *list = proto_msg.mutable_list_value();
    list->set_type(starcoder::List::TUPLE);
    for (int i = 0; i < pmt::length(pmt_msg); i++) {
      starcoder::BlockMessage element =
          convert_pmt_to_proto(pmt::tuple_ref(pmt_msg, i));
      list->add_value()->Swap(&element);
    }
  } else if (pmt::is_vector(pmt_msg)) {
    starcoder::List *list = proto_msg.mutable_list_value();
    list->set_type(starcoder::List::VECTOR);
    for (int i = 0; i < pmt::length(pmt_msg); i++) {
      starcoder::BlockMessage element =
          convert_pmt_to_proto(pmt::vector_ref(pmt_msg, i));
      list->add_value()->Swap(&element);
    }
  } else if (pmt::is_dict(pmt_msg)) {
    starcoder::Dict *dict = proto_msg.mutable_dict_value();
    pmt::pmt_t key_value_pairs_list = pmt::dict_items(pmt_msg);
    for (int i = 0; i < pmt::length(key_value_pairs_list); i++) {
      starcoder::Dict_Entry *entry = dict->add_entry();

      starcoder::BlockMessage key =
          convert_pmt_to_proto(pmt::car(pmt::nth(i, key_value_pairs_list)));
      starcoder::BlockMessage value =
          convert_pmt_to_proto(pmt::cdr(pmt::nth(i, key_value_pairs_list)));

      entry->mutable_key()->Swap(&key);
      entry->mutable_value()->Swap(&value);
    }
  }
  return proto_msg;
}
