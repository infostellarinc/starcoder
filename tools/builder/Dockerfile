FROM openjdk:9-slim

ADD . /workspace

RUN cd /workspace && ./gradlew install

FROM debian:9-slim

COPY --from=0 /root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio /root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio

ADD ./tools/builder/run.sh /usr/bin/run-starcoder

ENTRYPOINT [ "run-starcoder" ]
CMD [ "serve" ]
