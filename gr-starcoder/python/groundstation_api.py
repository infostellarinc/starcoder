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

from Queue import Queue, Empty
import threading
import time

import numpy as np
from gnuradio import gr
import pmt

import grpc
from google.auth import jwt as google_auth_jwt
from google.auth.transport import grpc as google_auth_transport_grpc
from stellarstation.api.v1.groundstation import groundstation_pb2
from stellarstation.api.v1.groundstation import groundstation_pb2_grpc
from stellarstation.api.v1 import transport_pb2
from google.protobuf.timestamp_pb2 import Timestamp

class groundstation_api(gr.sync_block):
    """
    This block communicates with the Ground Station API server and performs two functions: retrieving commands from the
    groundstation and sending telemetry to the groundstation.
    Input PMTs are expected to be uint8 PDUs containing the data bytes inside a transport.Telemetry message.
    https://github.com/infostellarinc/stellarstation-api/blob/0.3.0/api/src/main/proto/stellarstation/api/v1/transport.proto#L63
    Commands received from the groundstation are sent to the rest of the flowgraph as uint8 PDUs.

    api_key (str): Path to API key file for authentication. If empty string, use insecure channel.
    api_url (str): URL to gRPC API.
    root_cert_path (str): Root certificate path for TLS. If empty string, use insecure channel.
    groundstation_id (str): Groundstation ID
    plan_id (str): Plan ID
    stream_tag (str): Stream tag
    verbose (bool): Controls verbosity
    test_channel (grpc.Channel): Test channel used for unit testing purposes.
    """
    def __init__(self, api_key, api_url, root_cert_path, groundstation_id, plan_id, stream_tag, verbose,
                 test_channel=None):  # This last argument test_channel is used only for unit testing purposes.
        gr.sync_block.__init__(self,
            name="groundstation_api",
            in_sig=None,
            out_sig=None)
        self.api_key = api_key
        self.api_url = api_url
        self.root_cert_path = root_cert_path
        self.groundstation_id = groundstation_id
        self.plan_id = plan_id
        self.stream_tag = stream_tag
        self.verbose = verbose
        self.test_channel = test_channel

        self.message_port_register_out(pmt.intern("command"))

        self.register_port_and_message_handler("bitstream")
        self.register_port_and_message_handler("ax25")
        self.register_port_and_message_handler("iq")

        self.queue = Queue()
        self.lock = threading.Lock()
        self.stopped = False

        self.log = gr.logger("log")

    def register_port_and_message_handler(self, port):
        self.message_port_register_in(pmt.intern(port))
        self.set_msg_handler(pmt.intern(port), self.msg_handler_factory(port))

    def msg_handler_factory(self, port):
        if port == 'bitstream':
            framing = transport_pb2.BITSTREAM
        elif port == 'ax25':
            framing = transport_pb2.AX25
        elif port == 'iq':
            framing = transport_pb2.IQ
        else:
            raise RuntimeError("Unknown framing {}".format(port))

        def handler(msg):
            """
            This method is called for every input PMT and places a message on the queue to be sent to the groundstation
            Input PMTs are expected to be uint8 PDUs containing the actual data inside Telemetry
            """
            # TODO: 59 is a placeholder so we can test the time. Replace with an actual mock clock?
            curr_time = time.time() if self.test_channel is None else 59
            data = pmt.to_python(pmt.cdr(msg)).tostring()
            received = transport_pb2.Telemetry(
                framing=framing,
                data=data,
                time_first_byte_received=Timestamp(seconds=int(curr_time)),
                time_last_byte_received=Timestamp(seconds=int(curr_time)),
            )
            self.queue.put(received)
        return handler

    def get_stopped(self):
        with self.lock:
            return self.stopped

    def set_stopped(self, x):
        with self.lock:
            self.stopped = x

    def setup_api_client(self):
        if self.test_channel is None:
            if self.root_cert_path == '' or self.api_key == '':
                channel = grpc.insecure_channel(self.api_url)
            else:
                credentials = google_auth_jwt.Credentials.from_service_account_file(
                    self.api_key,
                    audience='https://api.stellarstation.com'
                )

                jwt_creds = google_auth_jwt.OnDemandCredentials.from_signing_credentials(
                    credentials)
                channel_credential = grpc.ssl_channel_credentials(
                    open(self.root_cert_path, 'br').read())
                channel = google_auth_transport_grpc.secure_authorized_channel(
                    jwt_creds, None, self.api_url, channel_credential)
        else:
            channel = self.test_channel
        return groundstation_pb2_grpc.GroundStationServiceStub(channel)

    def _telemetry_to_stream_request(self, t):
        stream_request = groundstation_pb2.GroundStationStreamRequest(
            ground_station_id=self.groundstation_id,
            stream_tag=self.stream_tag,
            satellite_telemetry=groundstation_pb2.SatelliteTelemetry(
                plan_id=self.plan_id,
                telemetry=t
            )
        )
        return stream_request

    # This generator continuously reads requests from the queue and sends them to the groundstation.
    # It stops and closes the stream once get_stopped() returns True (when the block's stop() function is
    # called at close).
    def generate_request(self):
        # Send the first request to activate the stream. Commands will start
        # to be received at this point.
        yield groundstation_pb2.GroundStationStreamRequest(ground_station_id=self.groundstation_id,
                                                           stream_tag=self.stream_tag)

        while not self.get_stopped():
            try:
                telemetry = self.queue.get(timeout=1)
                yield self._telemetry_to_stream_request(telemetry)
                self.queue.task_done()
            except Empty:
                continue

        while not self.queue.empty():
            telemetry = self.queue.get()
            yield self._telemetry_to_stream_request(telemetry)
            self.queue.task_done()

    # This function runs in another thread and starts the bidirectional stream.
    # All commands/messages from the groundstation are sent to the appropriate PMT ports.
    def handle_stream(self):
        for response in self.api_client.OpenGroundStationStream(self.generate_request()):
            if response.HasField("satellite_commands"):
                for command in response.satellite_commands.command:
                    if self.verbose:
                        self.log.info("Sending command: {}".format(command[:20]))
                    send_pmt = pmt.to_pmt(np.fromstring(command, dtype=np.uint8))
                    # TODO: Send plan_id and response_id as metadata
                    self.message_port_pub(pmt.intern("command"), pmt.cons(pmt.PMT_NIL, send_pmt))

    def work(self, input_items, output_items):
        return len(output_items[0])

    def start(self):
        self.api_client = self.setup_api_client()

        self.stream_thread = threading.Thread(target=self.handle_stream)
        self.stream_thread.start()
        return True

    def stop(self):
        self.set_stopped(True)
        self.queue.join()
        self.stream_thread.join()
        return True
