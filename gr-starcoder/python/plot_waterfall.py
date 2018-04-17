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
import matplotlib
matplotlib.use('agg')

import io
import matplotlib.pyplot as plt
import matplotlib.cm as cm

def plot_waterfall(arr):
    plt.figure(dpi=400)
    fig = plt.imshow(arr, cmap=cm.nipy_spectral,
                    interpolation='none', vmin=-120, vmax=-70, alpha=0.8,
                    aspect='auto')

    fig.axes.get_xaxis().set_visible(False)
    fig.axes.get_yaxis().set_visible(False)
    plt.axis('off')

    buf = io.BytesIO()
    plt.savefig(buf, bbox_inches='tight', pad_inches=0)
    plt.savefig("/home/rei/yoyooyoyoy.png", bbox_inches='tight', pad_inches=0)
    return buf.getvalue()
