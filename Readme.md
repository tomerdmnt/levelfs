
Leveldb FUSE file system

uses sublevels (0xff denotes a directory) as file paths.

this project still requires testing.

## Install

on osx, install osxfuse
```
$ brew install osxfus
```

init submodules
```
$ git submodule init
$ git submodule update
```

build
```
$ make
```

## Usage

```
usage: ./levelfs dbpath mountpoint [options]

general options:
    -o opt,[opt...]        mount options
    -h   --help            print help
    -V   --version         print version

FUSE options:
    -d   -o debug          enable debug output (implies -f)
    -f                     foreground operation
    -s                     disable multi-threaded operation
```

## TODO

- support sublevel seperators other than 0xff
- support arbitrary binary in keys
- refactor some of the operations

