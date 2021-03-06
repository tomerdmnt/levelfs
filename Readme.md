
Leveldb FUSE filesystem

Uses [sublevels](https://github.com/dominictarr/level-sublevel) as file paths.

Experimental, backup your db before mounting.

## Examples

```
$ levelfs /path/to/db /mnt
$ ls /mnt
```

mount your local npmd

```
$ levelfs ~/.npmd /mnt/npmd
```

## Install

on osx, install osxfuse
```
$ brew install osxfus
```

using [clib](https://github.com/clibs/clib)
```
$ clib install tomerdmnt/levelfs
```

or build
```
$ git clone github.com/tomerdmnt/levelfs
$ cd levelfs
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
- For the same reason, a directory disappears when all files under it are deleted which causes various issues when running rm -rf

## TODO

- accept sublevel seperator as parameter
  - meanwhile the seperator can be changed [here](src/path.c#L8-L9)
- support arbitrary binary in keys
- tests

