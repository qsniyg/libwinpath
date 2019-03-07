#!/bin/sh
DIRNAME0="`dirname -- "$0"`"
LIBRARY_PATH="`readlink -f -- "$DIRNAME0/../lib/libwinpath_inject.so"`"

export LD_PRELOAD="$LIBRARY_PATH"
exec "$@"
