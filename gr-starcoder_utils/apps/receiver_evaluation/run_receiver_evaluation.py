#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2018 Infostellar.
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

import argparse
from collections import OrderedDict
import json

import yaml
try:
    from yaml import CDumper as Dumper  # try to use LibYAML bindings if possible
except ImportError:
    from yaml import Dumper
_mapping_tag = yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG

from generator import generator
from evaluator import evaluator


parser = argparse.ArgumentParser(description="Scripts to run the receiver evaluation tool")
group = parser.add_mutually_exclusive_group(required=True)
group.add_argument("-G", "--generator", action="store_true", help="Run the signal generator")
group.add_argument("-E", "--evaluator", action="store_true", help="Run the offline evaluation "
                                                                  "of received packets")
parser.add_argument("-C", "--config", help="Location of configuration file", required=True)


# Code to make yaml work well with OrderedDict
def dict_representer(dumper, data):
    return dumper.represent_dict(data.iteritems())


Dumper.add_representer(OrderedDict, dict_representer)


def generate_report(filename, config, statistics):
    """Generates a report about the evaluator statistics"""
    with open(filename, 'w') as f:
        f.write(yaml.dump({
            "configuration": config,
            "statistics": statistics
        }, Dumper=Dumper, default_flow_style=False))


if __name__ == "__main__":
    args = parser.parse_args()

    print("Using configuration file: {}".format(args.config))
    with open(args.config) as f:
        config = yaml.load(f)

    print(json.dumps(config, indent=2))

    if args.generator:
        print("Running signal generator flowgraph")
        generator_params = config['generator_params'].copy()
        generator_params.update(config['shared_params'])
        flowgraph = generator(**generator_params)
        flowgraph.start()
        try:
            raw_input('\nPress Enter after the USRP has finished processing '
                      '(red light goes out). Unfortunately, we currently have '
                      'no way to have the flowgraph automatically finish after all '
                      'bits have been transmitted (see https://lists.gnu.org/'
                      'archive/html/discuss-gnuradio/2016-07/msg00319.html):\n ')
        except EOFError:
            pass
        flowgraph.stop()
        flowgraph.wait()
    elif args.evaluator:
        print("Running evaluation flowgraph")
        evaluator_params = config['evaluator_params'].copy()
        evaluator_params.update(config['shared_params'])
        flowgraph = evaluator(**evaluator_params)
        flowgraph.start()
        try:
            raw_input('Press Enter after all files have been read by the folder source.\n')
        except EOFError:
            pass
        flowgraph.stop()
        flowgraph.wait()
        if 'report_output_file' in config and config['report_output_file']:
            print("Saving statistics to {}".format(config['report_output_file']))
            generate_report(config['report_output_file'], config, flowgraph.starcoder_utils_prbs_sink_pdu_0.statistics)
