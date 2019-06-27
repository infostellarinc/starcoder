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
import os
import sys
import importlib
import yaml
try:
    from yaml import CDumper as Dumper  # try to use LibYAML bindings if possible
except ImportError:
    from yaml import Dumper
_mapping_tag = yaml.resolver.BaseResolver.DEFAULT_MAPPING_TAG

from gnuradio import gr

parser = argparse.ArgumentParser(description="Script to compile and run a flowgraph")

parser.add_argument("-F", "--flowgraph", help="Location of flowgraph file", required=True)
parser.add_argument("-C", "--config", help="Location of configuration file", required=True)
parser.add_argument('-X','--collect', nargs='*', help='List of files to collect and send to the ground station API '
                                                      'at the end of the pass. e.g. Waterfall plots')

# Code to make yaml work well with OrderedDict
def dict_representer(dumper, data):
    return dumper.represent_dict(data.iteritems())


Dumper.add_representer(OrderedDict, dict_representer)


if __name__ == "__main__":
    args = parser.parse_args()

    print("Using flowgraph file: {}".format(args.flowgraph))
    print("Using configuration file: {}".format(args.config))
    print("Files to collect: {}".format(args.collect))
    with open(args.config) as f:
        config = yaml.load(f, Loader=yaml.FullLoader)

    print(json.dumps(config, indent=2))

    _, ext = os.path.splitext(args.flowgraph)
    if ext == '.py':
        flowgraph_py_path = args.flowgraph
    elif ext == '.grc':
        # TODO: Compile grc into python file
        flowgraph_py_path = 'something.py'
    else:
        raise Exception('Flowgraph uses invalid extension {}'.format(ext))

    py_folder = os.path.dirname(flowgraph_py_path)
    sys.path.append(py_folder)
    module_name, _ = os.path.splitext(os.path.basename(flowgraph_py_path))
    print("Using module name {}".format(module_name))
    module = importlib.import_module(module_name)
    flowgraph_class = module.__dict__[module_name]
    assert issubclass(flowgraph_class, gr.top_block)

    flowgraph = flowgraph_class(**config)
    print("Running flowgraph. CTRL + C to stop.")
    flowgraph.run()

    # TODO: Collect files before exiting
    if args.collect:
        print("Collecting files: {}".format(args.collect))
