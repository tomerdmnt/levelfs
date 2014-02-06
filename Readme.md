
Leveldb FUSE file system

Uses sublevels (0xff denotes a directory) as file paths.

All basic operations are implemented (mknod, mkdir, rmdir, read, write, etc...), 
however this project still requires testing.

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
- docs

