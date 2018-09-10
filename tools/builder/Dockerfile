FROM openjdk:9-slim

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

COPY --from=0 /root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio /root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio

ADD ./tools/builder/run.sh /usr/bin/run-starcoder

ENV GR_CONF_PERFCOUNTERS_ON True
ENV STARCODER_SILENCED_COMMAND_BLOCKS doppler_command_source,doppler_command_source_transmit

ENTRYPOINT [ "run-starcoder" ]
CMD [ "serve" ]
