# Starcoder - a server to read/write data from/to the stars, written in Go.

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
