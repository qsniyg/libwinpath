## About

libwinpath is both a software library, and an injection library, that
allows you to use Windows-like case-insensitive paths on *NIX systems.

This is useful for tools written for *NIX that modify data for Windows apps,
running under WINE.
In fact, the path resolving part of the code comes from WINE itself.

## Using

For users, compile, install, then just prepend `winpath` like you would
with `sudo`, or any other wrapper:

    winpath my_app --some-argument 1 2 3

Currently there's no documentation for the developer library,
but see `libwinpath.h` for a list of available functions, and `wrapper.c` for how to use them.

## Compiling

Standard CMake build process, no special dependencies required.

    mkdir build && cd build
    cmake ..
    make
    # optionally: sudo make install

## Notes

Right now it's still in early development, which means there are still a few issues to sort out:

 * For now, the injection library only supports glibc, which means it only runs under standard Linux installations
 * The list of wrappers for the developer library is very incomplete
