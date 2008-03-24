#!/bin/bash

if [ ! -d ./patches ]; then
    echo "No ./patches/ directory found"
    exit 1
fi

if [ ! -e ./patches/series ]; then
    echo "No ./patches/series file found"
    exit 1
fi

errs="
### RESULTS ###"
# Maybe should use quilt for this?
for patch in $(egrep -v '^ *#' ./patches/series | egrep '[a-z]'); do
    echo
    echo "Patching $patch"

    patch -f -d ../.. -p0 < ./patches/$patch
    if [ $? != 0 ]; then
        echo "ERROR:  Failed to apply $patch"
        errs="$errs
ERROR:   $patch failed to apply"
    else
        errs="$errs
OK:      $patch"
    fi
done

echo "$errs"