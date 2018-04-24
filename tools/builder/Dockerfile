FROM openjdk:9-slim

ADD . /workspace

RUN cd /workspace && ./gradlew install

FROM debian:9-slim

COPY --from=0 /root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio /root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio

CMD [ "bash", "-c", ". /root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio/etc/profile.d/conda.sh && conda activate > /dev/null && /root/.gradle/curiostack/python/bootstrap/miniconda2-gnuradio/bin/starcoder" ]
