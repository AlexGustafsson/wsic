# A Web Server written in C
### An efficient web server written in C for the course DV1457 at BTH, Sweden
***

### Setting up

##### Installing from prebuilt binaries

TODO.

##### Running using docker

TODO.

##### Installing from source

TODO.

##### Quickstart

TODO.

### Documentation

The documentation is currently a bit sparse. For more information, refer to the source, tests and issues.

### Contributing

Any contribution is welcome. If you're not able to code it yourself, perhaps someone else is - so post an issue if there's anything on your mind.

##### Development

Clone the repository:
```Bash
git clone https://github.com/AlexGustafsson/wsic.git && cd wsic
```

```Bash
# Lint
make lint
# Perform static analysis
make analyze
# Format the code
make format
# Build and run a debugging build (memory analyzer and GDB debugging enabled)
make debug && ASAN_OPTIONS=detect_leaks=1 ./build/wsic.debug
# Build and run a release build
make build && ./build
```

##### Git branching conventions

The branches `master` and `development` are locked for pushing. Code is merged in to `development` by feature branches named `feature/my-feature`, `fix/my-fix` or the like. Try to keep the names consise and descriptive.

The feature branches should when applicable be up to speed to `development` via `git rebase development`. Feature branches are merged to `development` with `git merge --no-ff branch`. Feature branches may be squashed, but prefer to keep the history clean so that all commits can be kept.

When `development` is stable enough and provides meaningful value, it is merged into the `master` branch.

### Disclaimer

_Although the project is very capable, it is not built with production in mind. Therefore there might be complications when trying to use the server for large-scale projects meant for the public. The server was written by students as part of a course at BTH, Sweden and as such it might not promote best practices nor be performant._
