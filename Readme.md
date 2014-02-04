
Leveldb FUSE file system

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

- support table delimeters other than '.'
- support arbitrary binary in keys
- write
- unlink
- mkdir
- create
