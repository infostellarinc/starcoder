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

void convert_pmt_to_proto(const pmt::pmt_t &pmt_msg, starcoder::BlockMessage *proto_msg) {
  if (pmt::is_bool(pmt_msg)) {
    proto_msg->set_boolean_value(pmt::to_bool(pmt_msg));
  } else if (pmt::is_symbol(pmt_msg)) {
    proto_msg->set_symbol_value(pmt::symbol_to_string(pmt_msg));
  } else if (pmt::is_integer(pmt_msg)) {
    proto_msg->set_integer_value(pmt::to_long(pmt_msg));
  } else if (pmt::is_uint64(pmt_msg)) {
    proto_msg->set_integer_value(pmt::to_uint64(pmt_msg));
  } else if (pmt::is_real(pmt_msg)) {
    proto_msg->set_double_value(pmt::to_double(pmt_msg));
  } else if (pmt::is_complex(pmt_msg)) {
    std::complex<double> val = pmt::to_complex(pmt_msg);
    starcoder::Complex *complex = proto_msg->mutable_complex_value();
    complex->set_real_value(val.real());
    complex->set_imaginary_value(val.imag());
  } else if (pmt::is_pair(pmt_msg)) {
    starcoder::Pair *pair = proto_msg->mutable_pair_value();
    starcoder::BlockMessage *car = pair->mutable_car();
    starcoder::BlockMessage *cdr = pair->mutable_cdr();
    convert_pmt_to_proto(pmt::car(pmt_msg), car);
    convert_pmt_to_proto(pmt::cdr(pmt_msg), cdr);
  } else if (pmt::is_tuple(pmt_msg)) {
    starcoder::List *list = proto_msg->mutable_list_value();
    list->set_type(starcoder::List::TUPLE);
    for (int i=0; i<pmt::length(pmt_msg); i++) {
      starcoder::BlockMessage *element = list->add_value();
      convert_pmt_to_proto(pmt::tuple_ref(pmt_msg, i), element);
    }
  } else if (pmt::is_vector(pmt_msg)) {
    starcoder::List *list = proto_msg->mutable_list_value();
    list->set_type(starcoder::List::VECTOR);
    for (int i=0; i<pmt::length(pmt_msg); i++) {
      starcoder::BlockMessage *element = list->add_value();
      convert_pmt_to_proto(pmt::vector_ref(pmt_msg, i), element);
    }
  } else if (pmt::is_dict(pmt_msg)) {
    starcoder::Dict *dict = proto_msg->mutable_dict_value();
    pmt::pmt_t key_value_pairs_list = pmt::dict_items(pmt_msg);
    for (int i=0; i<pmt::length(key_value_pairs_list); i++) {
      starcoder::Dict_Entry *entry = dict->add_entry();
      starcoder::BlockMessage *key = entry->mutable_key();
      starcoder::BlockMessage *value = entry->mutable_value();
      convert_pmt_to_proto(pmt::car(pmt::nth(i, key_value_pairs_list)), key);
      convert_pmt_to_proto(pmt::cdr(pmt::nth(i, key_value_pairs_list)), value);
    }
  } else if (pmt::is_uniform_vector(pmt_msg)) {
    starcoder::UniformVector *uni_vector = proto_msg->mutable_uniform_vector_value();
    convert_proto_uniform_vector(pmt_msg, uni_vector);
  }
}

void convert_proto_uniform_vector(const pmt::pmt_t &pmt_msg, starcoder::UniformVector *uni_vector) {
  if (pmt::is_u8vector(pmt_msg)) {
    starcoder::UVector *u_vector = uni_vector->mutable_u_value();
    u_vector->set_size(starcoder::IntSize::Size8);
    const std::vector < uint8_t > vector_elements = pmt::u8vector_elements(pmt_msg);
    *u_vector->mutable_value() = {vector_elements.begin(), vector_elements.end()};
  } else if (pmt::is_s8vector(pmt_msg)) {
    starcoder::IVector *i_vector = uni_vector->mutable_i_value();
    i_vector->set_size(starcoder::IntSize::Size8);
    const std::vector<int8_t> vector_elements = pmt::s8vector_elements(pmt_msg);
    *i_vector->mutable_value() = {vector_elements.begin(), vector_elements.end()};
  } else if (pmt::is_u16vector(pmt_msg)) {
    starcoder::UVector *u_vector = uni_vector->mutable_u_value();
    u_vector->set_size(starcoder::IntSize::Size16);
    const std::vector < uint16_t > vector_elements = pmt::u16vector_elements(pmt_msg);
    *u_vector->mutable_value() = {vector_elements.begin(), vector_elements.end()};
  } else if (pmt::is_s16vector(pmt_msg)) {
    starcoder::IVector *i_vector = uni_vector->mutable_i_value();
    i_vector->set_size(starcoder::IntSize::Size16);
    const std::vector<int16_t> vector_elements = pmt::s16vector_elements(pmt_msg);
    *i_vector->mutable_value() = {vector_elements.begin(), vector_elements.end()};
  } else if (pmt::is_u32vector(pmt_msg)) {
    starcoder::UVector *u_vector = uni_vector->mutable_u_value();
    u_vector->set_size(starcoder::IntSize::Size32);
    const std::vector < uint32_t > vector_elements = pmt::u32vector_elements(pmt_msg);
    *u_vector->mutable_value() = {vector_elements.begin(), vector_elements.end()};
  } else if (pmt::is_s32vector(pmt_msg)) {
    starcoder::IVector *i_vector = uni_vector->mutable_i_value();
    i_vector->set_size(starcoder::IntSize::Size32);
    const std::vector<int32_t> vector_elements = pmt::s32vector_elements(pmt_msg);
    *i_vector->mutable_value() = {vector_elements.begin(), vector_elements.end()};
  } else if (pmt::is_u64vector(pmt_msg)) {
    starcoder::U64Vector *u64_vector = uni_vector->mutable_u64_value();
    const std::vector<uint64_t> vector_elements = pmt::u64vector_elements(pmt_msg);
    *u64_vector->mutable_value() = {vector_elements.begin(), vector_elements.end()};
  } else if (pmt::is_s64vector(pmt_msg)) {
    starcoder::I64Vector *i64_vector = uni_vector->mutable_i64_value();
    const std::vector<int64_t> vector_elements = pmt::s64vector_elements(pmt_msg);
    *i64_vector->mutable_value() = {vector_elements.begin(), vector_elements.end()};
  }
}
