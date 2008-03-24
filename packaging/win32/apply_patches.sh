#!/bin/bash

if [ ! -d ./patches ]; then
    echo "No ./patches/ directory found"
    exit 1
fi

if [ ! -e ./patches/series ]; then
    echo "No ./patches/series file found"
    exit 1
fi

# Maybe should use quilt for this?
for patch in $(egrep -v '^ *#' ./patches/series | egrep '[a-z]'); do
    echo "Patching $patch"

    patch -d ../.. -p0 < $patch
    if [ $? != 0 ]; then
        echo "ERROR:  Failed to apply $patch"
        exit 2
    fi
done

