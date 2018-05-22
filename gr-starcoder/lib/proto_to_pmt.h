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

#ifndef INCLUDED_PROTO_TO_PMT_H
#define INCLUDED_PROTO_TO_PMT_H

#include <pmt/pmt.h>
#include "starcoder.pb.h"

pmt::pmt_t convert_pmt_proto(const starcoder::BlockMessage &proto_msg);
pmt::pmt_t convert_pmt_list(const starcoder::List &proto_pmt_list);
pmt::pmt_t convert_pmt_uniform_vector(
    const starcoder::UniformVector &proto_pmt_uniform_vector);
pmt::pmt_t convert_pmt_dict(const starcoder::Dict &proto_pmt_dict);

#endif /* INCLUDED_PROTO_TO_PMT_H */
