#!/bin/bash

. /root/.gradle/curiostack/gnuradio/etc/profile.d/conda.sh
conda activate > /dev/null
/root/.gradle/curiostack/gnuradio/bin/starcoder $@
