
Leveldb FUSE file system

uses sublevels ('.' denotes a directory) as file paths.

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

- support table seperators other than '.'
- support arbitrary binary in keys
- create
- unlink
- mkdir
