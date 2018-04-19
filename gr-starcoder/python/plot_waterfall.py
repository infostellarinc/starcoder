#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2018 Infostellar. Inc,
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
#  Copyright (C) 2017, Libre Space Foundation <http://librespacefoundation.org/>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
#


import matplotlib
matplotlib.use('agg')

import io
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm
from matplotlib.ticker import IndexLocator, AutoMinorLocator


def float_to_str(f):
    r = f / 1e9
    if(r > 1 or r < -1 or np.isclose(r, 1.0, rtol=1e-03, atol=1e-05)
        or np.isclose(r, -1.0, rtol=1e-03, atol=1e-05)):
        return '{:.1f}'.format(r) + "G"
    r = f / 1e6
    if(r > 1 or r < -1 or np.isclose(r, 1.0, rtol=1e-03, atol=1e-05) or
        np.isclose(r, -1.0, rtol=1e-03, atol=1e-05)):
        return '{:.1f}'.format(r) + "M"
    r = f / 1e3
    if(r > 1 or r < -1 or np.isclose(r, 1.0, rtol=1e-03, atol=1e-05)
        or np.isclose(r, -1.0, rtol=1e-03, atol=1e-05)):
        return '{:.1f}'.format(r) + "k"
    return '{:.1}'.format(f)


def plot_waterfall(arr, samp_rate, center_freq, rps, fft, filename):

    plt.figure(figsize=(12.8, 20.8), dpi=200)
    plt.imshow(arr, cmap=cm.nipy_spectral,
               interpolation='none', vmin=int(np.mean(arr)), vmax=arr.max(),
               alpha=1, aspect='auto')
    cb = plt.colorbar(aspect=50)
    cb.set_label('Power (dB)')

    cell_freq = samp_rate / 10.0
    center_tick = fft/2
    carriers_per_tick = int(cell_freq * fft / samp_rate)
    nticks = 9

    x_ticks = [center_tick]
    x_tick_labels = [float_to_str(center_freq)]
    for i in range(nticks/2):
        x_ticks.append((i + 1) * carriers_per_tick + center_tick)
        x_ticks.insert(0, center_tick - (i + 1) * carriers_per_tick)
        x_tick_labels.append(float_to_str(center_freq + (i + 1) * cell_freq))
        x_tick_labels.insert(0, float_to_str(center_freq - (i + 1) * cell_freq))

    n_rows_per_tick = 60 * rps
    y_ticks = []
    y_tick_labels = []
    j = 0
    for i in range(len(arr), 0, -n_rows_per_tick):
        y_ticks.append(i)
        y_tick_labels.append(j * 60)
        j = j + 1

    plt.xticks(x_ticks, x_tick_labels, size='x-large')
    if len(y_ticks) > 0:
        plt.yticks(y_ticks, y_tick_labels, size='x-large')
    plt.grid(b='on', linestyle='dashed', linewidth=1,
             color='#000000', alpha=0.3)

    minor_locator_y = IndexLocator(10 * rps, len(arr) % (10 * rps))
    plt.axes().yaxis.set_minor_locator(minor_locator_y)
    plt.axes().yaxis.grid(which='minor', color='black', linestyle='-', linewidth=0.05, alpha=0.7)
    plt.axes().xaxis.grid(which='minor', color='black', linestyle='-', linewidth=0.1, alpha=0.6)
    plt.axes().xaxis.set_minor_locator(AutoMinorLocator(10))
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Time (seconds)')

    if filename != '':
        plt.savefig(filename, bbox_inches='tight', pad_inches=0.2)

    buf = io.BytesIO()
    plt.savefig(buf, bbox_inches='tight', pad_inches=0, format='png')
    return buf.getvalue()
