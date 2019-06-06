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

from gnuradio import gr
import pmt

from Queue import Queue, Empty
import threading

import grpc

from google.auth import jwt as google_auth_jwt
from google.auth.transport import grpc as google_auth_transport_grpc
from stellarstation.api.v1.groundstation import groundstation_pb2
from stellarstation.api.v1.groundstation import groundstation_pb2_grpc

class starpass_api(gr.sync_block):
    """
    docstring for block starpass_api
    """
    def __init__(self, api_key, api_url, root_cert_path, groundstation_id, plan_id, stream_tag, files_to_collect):
        gr.sync_block.__init__(self,
            name="starpass_api",
            in_sig=None,
            out_sig=None)
        self.api_key = api_key
        self.api_url = api_url
        self.root_cert_path = root_cert_path
        self.groundstation_id = groundstation_id
        self.plan_id = plan_id
        self.stream_tag = stream_tag
        self.files_to_collect = files_to_collect
        print(
            self.api_key,
            self.api_url,
            self.root_cert_path,
            self.groundstation_id,
            self.plan_id,
            self.stream_tag,
            self.files_to_collect,
        )

        self.message_port_register_out(pmt.intern("command"))
        self.message_port_register_out(pmt.intern("receive_frequency_shift"))
        self.message_port_register_out(pmt.intern("transmit_frequency_shift"))

        self.message_port_register_in(pmt.intern("in"))
        self.set_msg_handler(pmt.intern("in"), self.msg_handler)

        self.queue = Queue()
        self.lock = threading.Lock()
        self.stopped = False

        self.api_client = self.setup_api_client()

        self.stream_thread = threading.Thread(target=self.handle_stream)
        self.stream_thread.start()

    def get_stopped(self):
        with self.lock:
            return self.stopped

    def set_stopped(self, x):
        with self.lock:
            self.stopped = x

    def setup_api_client(self):
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
        return groundstation_pb2_grpc.GroundStationServiceStub(channel)

    def msg_handler(self, msg):
        pass

    def work(self, input_items, output_items):
        return len(output_items[0])

    def stop(self):
        # TODO: self.collect_files()
        self.set_stopped(True)
        self.queue.join()
        self.stream_thread.join()
        return True

    def generate_request(self):
        # Send the first request to activate the stream. Commands will start
        # to be received at this point.
        yield groundstation_pb2.GroundStationStreamRequest(ground_station_id=self.groundstation_id,
                                                           stream_tag=self.stream_tag)

        def _process_queue_entry(t):
            stream_request = groundstation_pb2.GroundStationStreamRequest(
                ground_station_id=self.groundstation_id,
                stream_tag=self.stream_tag,
                satellite_telemetry=groundstation_pb2.SatelliteTelemetry(
                    plan_id=self.plan_id,
                    telemetry=t
                )
            )
            yield stream_request
            self.queue.task_done()

        while not self.get_stopped():
            try:
                telemetry = self.queue.get(timeout=1)
                _process_queue_entry(telemetry)
            except Empty:
                continue

        while not self.queue.empty():
            telemetry = self.queue.get()
            _process_queue_entry(telemetry)

    def handle_stream(self):
        for response in self.api_client.OpenGroundStationStream(self.generate_request()):
            # TODO: Handle commands received from server
            pass
