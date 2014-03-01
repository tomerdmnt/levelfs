
Leveldb FUSE file system

Uses [sublevels](https://github.com/dominictarr/level-sublevel) as file paths.

Experimental, backup your db before mounting.

```
$ levelfs /path/to/db /mnt
$ ls /mnt
```

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
$ make install
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

## Issues
- Empty directories won't persist between mounts
- For the same reason, a directory disappears when all files under it are delted which causes various issues when running rm -rf

## TODO

- accept sublevel seperator as parameter
- support arbitrary binary in keys
- tests

