# Starcoder - a server to read/write data from/to the stars, written in Go.

[![CircleCI](https://circleci.com/gh/infostellarinc/starcoder/tree/master.svg?style=svg)](https://circleci.com/gh/infostellarinc/starcoder/tree/master)

[![Docker Repository on Quay](https://quay.io/repository/infostellarinc/starcoder/status "Docker Repository on Quay")](https://quay.io/repository/infostellarinc/starcoder)

## Status

Starcoder is currently released at an alpha level. It is used in production at Infostellar, but has not
been verified elsewhere and is currently still somewhat tailored to Infostellar's workflows.
Stay tuned for more generalization on the way to production

## Running

Currently, the simplest way to run Starcoder is using docker.

```bash
$ docker run -it --rm quay.io/infostellarinc/starcoder:0.1.0
```

## Developing

Starcoder uses Gradle for building. The only dependency for building Starcoder is Java, all other components
like Python, a build toolchain, and even GnuRadio will be automatically setup by the build.

```bash
$ ./gradlew install
```

will create a GnuRadio prefix at `~/.gradle/curiostack/gnuradio` with Starcoder installed.
