FROM openjdk:10-slim

ADD build.gradle /workspace/gnuradio/build.gradle
ADD gradle.properties /workspace/gnuradio/gradle.properties
ADD gradle /workspace/gnuradio/gradle
ADD gradlew /workspace/gnuradio/gradlew
ADD gr-recipes /workspace/gnuradio/gr-recipes
ADD mockpygtk /workspace/gnuradio/mockpygtk

RUN cd /workspace/gnuradio && ./gradlew setupPrefix

ADD . /workspace/starcoder

RUN cd /workspace/starcoder && ./gradlew install

FROM debian:9-slim

COPY --from=0 /root/.gradle/curiostack/gnuradio/4.4.10/etc /root/.gradle/curiostack/gnuradio/4.4.10/etc
COPY --from=0 /root/.gradle/curiostack/gnuradio/4.4.10/lib /root/.gradle/curiostack/gnuradio/4.4.10/lib
COPY --from=0 /root/.gradle/curiostack/gnuradio/4.4.10/lib64 /root/.gradle/curiostack/gnuradio/4.4.10/lib64
COPY --from=0 /root/.gradle/curiostack/gnuradio/4.4.10/share/gnuradio /root/.gradle/curiostack/gnuradio/4.4.10/share/gnuradio
COPY --from=0 /root/.gradle/curiostack/gnuradio/4.4.10/share/uhd /root/.gradle/curiostack/gnuradio/4.4.10/share/uhd
COPY --from=0 /root/.gradle/curiostack/gnuradio/4.4.10/ssl /root/.gradle/curiostack/gnuradio/4.4.10/ssl
COPY --from=0 /root/.gradle/curiostack/gnuradio/4.4.10/x86_64-conda_cos6-linux-gnu /root/.gradle/curiostack/gnuradio/4.4.10/x86_64-conda_cos6-linux-gnu
COPY --from=0 /root/.grc_gnuradio /root/.grc_gnuradio


COPY --from=0 /root/.gradle/curiostack/gnuradio/4.4.10/bin/conda \
  /root/.gradle/curiostack/gnuradio/4.4.10/bin/python* \
  /root/.gradle/curiostack/gnuradio/4.4.10/bin/grcc \
  /root/.gradle/curiostack/gnuradio/4.4.10/bin/starcoder \
  /root/.gradle/curiostack/gnuradio/4.4.10/bin/

ADD ./tools/builder/run.sh /usr/bin/run-starcoder

ENV GR_CONF_PERFCOUNTERS_ON True
ENV STARCODER_SILENCED_COMMAND_BLOCKS doppler_command_source,doppler_command_source_transmit

ENTRYPOINT [ "run-starcoder" ]
CMD [ "serve" ]
