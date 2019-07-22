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

import threading
import time

import numpy as np
from gnuradio import gr
import pmt

import grpc
from google.auth import jwt as google_auth_jwt
from google.auth.transport import grpc as google_auth_transport_grpc
from google.protobuf.timestamp_pb2 import Timestamp
from stellarstation.api.v1.groundstation import groundstation_pb2
from stellarstation.api.v1.groundstation import groundstation_pb2_grpc

SPEED_OF_LIGHT = 299792458.0

class groundstation_api_doppler(gr.sync_block):
    """
    This block communicates with the Ground Station API server, finds the specified plan ID, and sends out the
    Doppler shift of the downlink/uplink frequencies of the satellite at the specified number of corrections per second.

    downlink_frequency (float): Center downlink frequency for satellite.
    uplink_frequency (float): Center uplink frequency for satellite.
    api_key (str): Path to API key file for authentication. If empty string, use insecure channel.
    api_url (str): URL to gRPC API.
    root_cert_path (str): Root certificate path for TLS. If empty string, use insecure channel.
    groundstation_id (str): Groundstation ID
    plan_id (str): Plan ID
    corrections_per_second (int): Number of times the Doppler shift is sent out per second.
    verbose (bool): Enables verbose logging.
    test_channel (grpc.Channel): Test channel used for unit testing purposes.
    """
    def __init__(self, downlink_frequency, uplink_frequency, api_key, api_url, root_cert_path, groundstation_id,
                 plan_id, corrections_per_second, verbose,
                 test_channel=None):  # This last argument test_channel is used only for unit testing purposes.
        gr.sync_block.__init__(self,
                               name="groundstation_api_doppler",
                               in_sig=None,
                               out_sig=None)
        self.downlink_frequency = downlink_frequency
        self.uplink_frequency = uplink_frequency
        self.api_key = api_key
        self.api_url = api_url
        self.root_cert_path = root_cert_path
        self.groundstation_id = groundstation_id
        self.plan_id = plan_id
        self.corrections_per_second = corrections_per_second
        self.verbose = verbose
        self.test_channel = test_channel

        self.stopped = False

        self.message_port_register_out(pmt.intern("downlink_shift"))
        self.message_port_register_out(pmt.intern("uplink_shift"))

        self.log = gr.logger("log")

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

    def work(self, input_items, output_items):
        return len(output_items[0])

    def processing_loop(self):
        """
        This processing loop runs in a separate thread and sends out Doppler shifted frequencies at a fixed interval.
        """
        listPlansResponse = self.api_client.ListPlans(groundstation_pb2.ListPlansRequest(
            ground_station_id=self.groundstation_id,
            aos_after=Timestamp(seconds=int(time.time()) - 120, nanos=0),
            aos_before=Timestamp(seconds=int(time.time() + 3600), nanos=0),
        ))
        self.log.debug("Fetched {} plans".format(len(listPlansResponse.plan)))

        plan = [x for x in listPlansResponse.plan if x.plan_id == self.plan_id]
        if not plan:
            self.log.fatal("No plans matching plan ID {} found.".format(self.plan_id))
            raise Exception('No plans matching plan ID {} found'.format(self.plan_id))
        else:
            plan = plan[0]

        interpolated_satellite_coordinates = interpolate_coordinates(
            plan.satellite_coordinates, self.corrections_per_second)
        self.log.debug("{} satellite coordinates interpolated".format(
            len(interpolated_satellite_coordinates)))
        for coord in interpolated_satellite_coordinates:
            if self.stopped:
                return
            coord_time = coord.time.seconds + coord.time.nanos / 10.**9
            if coord_time < time.time():
                continue
            while coord_time - time.time() > 1.1:
                time.sleep(1)
                if self.stopped:
                    return
            time.sleep(coord_time - time.time())
            if self.verbose:
                self.log.debug("Publishing to ports at {}. Scheduled time {}".format(time.time(), coord_time))
            self.publish_to_ports(coord.range_rate)

    def publish_to_ports(self, range_rate):
        shifted_downlink = self.downlink_frequency * (1.0 - range_rate / SPEED_OF_LIGHT)
        shifted_uplink = self.uplink_frequency * (1.0 + range_rate / SPEED_OF_LIGHT)
        self.message_port_pub(
            pmt.intern("downlink_shift"), pmt.to_pmt(self.downlink_frequency - shifted_downlink))
        self.message_port_pub(
            pmt.intern("uplink_shift"), pmt.to_pmt(self.uplink_frequency - shifted_uplink))

    def start(self):
        self.api_client = self.setup_api_client()

        self.stream_thread = threading.Thread(target=self.processing_loop)
        self.stream_thread.start()
        return True

    def stop(self):
        self.stopped = True
        self.stream_thread.join()
        return True


def interpolate_coordinates(satellite_coordinates, corrections_per_second):
    if not satellite_coordinates:
        return []
    interpolated_satellite_coordinates = []
    for i, coord in enumerate(satellite_coordinates):
        if i == len(satellite_coordinates) - 1:
            c = groundstation_pb2.SatelliteCoordinates()
            c.CopyFrom(coord)
            interpolated_satellite_coordinates.append(c)
            return interpolated_satellite_coordinates

        range_rate_diff = satellite_coordinates[i+1].range_rate - coord.range_rate
        range_rate_interval = range_rate_diff/corrections_per_second
        time_interval = 1./corrections_per_second
        for j in range(corrections_per_second):
            time_to_add = j * time_interval
            base_time = coord.time.seconds + coord.time.nanos / 10.**9
            new_time = base_time + time_to_add
            new_range_rate = coord.range_rate + j * range_rate_interval
            new_coord = groundstation_pb2.SatelliteCoordinates(
                range_rate = new_range_rate,
                time = Timestamp(seconds=int(new_time),
                                 nanos=int((new_time - int(new_time)) * 10**9))
            )
            interpolated_satellite_coordinates.append(new_coord)
