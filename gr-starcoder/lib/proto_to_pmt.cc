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
  switch(type) {
    case starcoder::PMT::PmtOneofCase::kBooleanValue:
      return pmt::from_bool(grpc_msg.boolean_value());
    case starcoder::PMT::PmtOneofCase::kSymbolValue:
      return pmt::string_to_symbol(grpc_msg.symbol_value());
    case starcoder::PMT::PmtOneofCase::kIntegerValue:
      return pmt::from_long(grpc_msg.integer_value());
    case starcoder::PMT::PmtOneofCase::kDoubleValue:
      return pmt::from_double(grpc_msg.double_value());
    case starcoder::PMT::PmtOneofCase::kComplexValue:
      return pmt::from_complex(grpc_msg.complex_value().real_value(), grpc_msg.complex_value().imaginary_value());
    case starcoder::PMT::PmtOneofCase::kPmtPairValue:
      return pmt::cons(
        convert_pmt_proto(grpc_msg.pmt_pair_value().car()),
        convert_pmt_proto(grpc_msg.pmt_pair_value().cdr()));
    case starcoder::PMT::PmtOneofCase::kPmtTupleValue:
      return convert_pmt_tuple(grpc_msg.pmt_tuple_value());
    case starcoder::PMT::PmtOneofCase::kPmtVectorValue:
      return convert_pmt_vector(grpc_msg.pmt_vector_value());
  }
  return pmt::get_PMT_NIL();
}

pmt::pmt_t convert_pmt_tuple(const starcoder::PMTTuple &grpc_pmt_tuple) {
  int size = grpc_pmt_tuple.pmt_size();
  switch(size) {
    case 0:
      return pmt::make_tuple();
    case 1:
      return pmt::make_tuple(convert_pmt_proto(grpc_pmt_tuple.pmt(0)));
    case 2:
      return pmt::make_tuple(
        convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(1)));
    case 3:
      return pmt::make_tuple(
        convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(2)));
    case 4:
      return pmt::make_tuple(
        convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(3)));
    case 5:
      return pmt::make_tuple(
        convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(4)));
    case 6:
      return pmt::make_tuple(
        convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(4)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(5)));
    case 7:
      return pmt::make_tuple(
        convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(4)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(5)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(6)));
    case 8:
      return pmt::make_tuple(
        convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(4)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(5)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(6)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(7)));
    case 9:
      return pmt::make_tuple(
        convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(1)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(2)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(3)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(4)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(5)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(6)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(7)),
        convert_pmt_proto(grpc_pmt_tuple.pmt(8)));
    case 10:
      return pmt::make_tuple(
        convert_pmt_proto(grpc_pmt_tuple.pmt(0)),
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
  pmt::pmt_t vec = pmt::make_vector(grpc_pmt_vector.pmt_size(), pmt::get_PMT_NIL());
  for (int i = 0; i < grpc_pmt_vector.pmt_size(); i++) {
    pmt::vector_set(vec, i, convert_pmt_proto(grpc_pmt_vector.pmt(i)));
  }
  return vec;
}
