# Starcoder - a server to read/write data from/to the stars, written in Go.

[![CircleCI](https://circleci.com/gh/infostellarinc/starcoder/tree/master.svg?style=svg)](https://circleci.com/gh/infostellarinc/starcoder/tree/master)

[![Docker Repository on Quay](https://quay.io/repository/infostellarinc/starcoder/status "Docker Repository on Quay")](https://quay.io/repository/infostellarinc/starcoder)

## Adding a dependency

To add a golang dependency to a project, add an entry to `Gopkg.toml`, following the existing format.
Prefer to specify a version if the project is following semantic versioning (make sure to check
whether they actually do update their versions at a reasonable pace, many golang projects define
a version once and then stop), otherwise specify master branch. Then run `depUpdate`, e.g.,

```bash
$ ./gradlew depUpdate
```

Verify your change by running `depEnsure`, e.g.,

```bash
$ ./gradlew depEnsure
```
