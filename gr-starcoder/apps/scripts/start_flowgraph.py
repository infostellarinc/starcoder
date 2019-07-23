#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2019 Infostellar.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#
"""
Sample command to use this script
$ python tools/builder/start_flowgraph.py \
    -F flowgraphs/test.grc \
    -C /tmp/flowgraph_params.yaml \
    -X /tmp/waterfall.png WATERFALL \
    -U localhost:50051 \
    -G 2 \
    -P 3
"""

import argparse
import json
import os
import sys
import subprocess
import importlib
import tempfile
import yaml
_mapping_tag = yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG
from itertools import izip
import time

from gnuradio import gr

import grpc
from google.auth import jwt as google_auth_jwt
from google.auth.transport import grpc as google_auth_transport_grpc
from google.protobuf.timestamp_pb2 import Timestamp

from stellarstation.api.v1.groundstation import groundstation_pb2
from stellarstation.api.v1.groundstation import groundstation_pb2_grpc
from stellarstation.api.v1 import transport_pb2

parser = argparse.ArgumentParser(description="Script to compile and run a flowgraph")

parser.add_argument("-F", "--flowgraph", help="Location of flowgraph file", required=True)
parser.add_argument("-C", "--flowgraph-config", help="Location of flowgraph configuration file in YAML format", required=True)
parser.add_argument('-X','--collect', nargs='*', help='List of files to collect and their respective framing type to '
                                                      'send to the ground station API at the end of the pass. The '
                                                      'filepath and corresponding frame type must be sent alternately'
                                                      ' e.g. waterfall.png WATERFALL weather.jpg IMAGE_JPEG')

# The below arguments are only used when there are files to collect.
parser.add_argument('-K', '--api-key', help="Location of API key for connecting to the ground station API.")
parser.add_argument('-R', '--root-cert-path', help="Location of root certificate for connecting to the ground station API.")
parser.add_argument('-U', '--api-url', help="URL for ground station API")
parser.add_argument('-G', '--ground-station-id', help="Ground Station ID", default='')
parser.add_argument('-S', '--stream-tag', help="Stream tag for this plan", default='')
parser.add_argument('-P', '--plan-id', help="Plan ID", default='')


def pairwise(iterable):
    """s -> (s0, s1), (s2, s3), (s4, s5), ..."""
    a = iter(iterable)
    return izip(a, a)


def compile_flowgraph(path):
    print('Compiling flowgraph {}'.format(path))

    # Create temporary directory to store compiled .py file. We need this because we can't control the output filename
    # of the compiled file
    temp_dir = tempfile.mkdtemp("", "starcoder")
    print('temporary directory {}'.format(temp_dir))

    subprocess.check_call(['grcc', '-d', temp_dir, path])

    return os.path.join(temp_dir, os.listdir(temp_dir)[0])


def setup_api_client(root_cert_path, api_key, api_url):
    print('Setting up client to communicate with Ground Station API')
    if not root_cert_path or not api_key:
        print('Root certificate path or API key not set. Using insecure channel')
        channel = grpc.insecure_channel(api_url)
    else:
        credentials = google_auth_jwt.Credentials.from_service_account_file(
            api_key,
            audience='https://api.stellarstation.com'
        )

        jwt_creds = google_auth_jwt.OnDemandCredentials.from_signing_credentials(credentials)
        channel_credential = grpc.ssl_channel_credentials(
            open(root_cert_path, 'br').read()
        )
        channel = google_auth_transport_grpc.secure_authorized_channel(
            jwt_creds, None, api_url, channel_credential
        )
    return groundstation_pb2_grpc.GroundStationServiceStub(channel)


def generate_request(files_to_collect, ground_station_id, stream_tag, plan_id):
    # Send the first request to activate the stream.
    yield groundstation_pb2.GroundStationStreamRequest(ground_station_id=ground_station_id,
                                                       stream_tag=stream_tag)
    for path, frame_type in pairwise(files_to_collect):
        print('Processing {} of framing type {}'.format(path, frame_type))
        if os.path.isfile(path):
            with open(path) as f:
                curr_time = time.time()
                stream_request = groundstation_pb2.GroundStationStreamRequest(
                    ground_station_id=ground_station_id,
                    stream_tag=stream_tag,
                    satellite_telemetry=groundstation_pb2.SatelliteTelemetry(
                        plan_id=plan_id,
                        telemetry=transport_pb2.Telemetry(
                            framing=transport_pb2.Framing.Value(frame_type.upper()),
                            data=f.read(),
                            time_first_byte_received=Timestamp(seconds=int(curr_time)),
                            time_last_byte_received=Timestamp(seconds=int(curr_time)),
                        )
                    )
                )
            yield stream_request
        else:
            print("WARNING: {} is not a file".format(path))


if __name__ == "__main__":
    args = parser.parse_args()

    print("Using flowgraph file: {}".format(args.flowgraph))
    print("Using flowgraph configuration file: {}".format(args.flowgraph_config))
    print("Files to collect: {}".format(args.collect))
    if len(args.collect) % 2 != 0:
        raise Exception('files_to_collect does not have an even number of elements. Do all files have a frame type?')
    print("Reading flowgraph configuration..")
    with open(args.flowgraph_config) as f:
        flowgraph_config = yaml.load(f, Loader=yaml.FullLoader)
        if flowgraph_config is None:
            flowgraph_config = dict()
    print('Flowgraph configuration', json.dumps(flowgraph_config, indent=2))

    _, ext = os.path.splitext(args.flowgraph)
    if ext == '.py':
        flowgraph_py_path = args.flowgraph
    elif ext == '.grc':
        flowgraph_py_path = compile_flowgraph(args.flowgraph)
    else:
        raise Exception('Flowgraph uses invalid extension {}'.format(ext))

    py_folder = os.path.dirname(flowgraph_py_path)
    sys.path.append(py_folder)
    module_name, _ = os.path.splitext(os.path.basename(flowgraph_py_path))
    print("Using module name {}".format(module_name))
    module = importlib.import_module(module_name)
    flowgraph_class = module.__dict__[module_name]
    assert issubclass(flowgraph_class, gr.top_block)

    flowgraph = flowgraph_class(**flowgraph_config)
    print("Running flowgraph. CTRL + C to stop.")
    flowgraph.run()

    if args.collect:
        print("Collecting files: {}".format(args.collect))
        api_client = setup_api_client(args.root_cert_path, args.api_key, args.api_url)
        for _ in api_client.OpenGroundStationStream(generate_request(
                args.collect, args.ground_station_id, args.stream_tag, args.plan_id)):
            # No need to do anything with responses from the server.
            continue
