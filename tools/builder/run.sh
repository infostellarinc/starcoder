#!/bin/bash

. /root/.gradle/curiostack/gnuradio/4.4.10/etc/profile.d/conda.sh
conda activate > /dev/null
/root/.gradle/curiostack/gnuradio/4.4.10/bin/starcoder $@
