#!/bin/bash

. /root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio/etc/profile.d/conda.sh
conda activate > /dev/null
/root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio/bin/starcoder $@
