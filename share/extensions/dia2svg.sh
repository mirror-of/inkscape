#!/bin/sh

TEMPFILENAME=/tmp/tmpdiafile.svg

dia -n --export=${TEMPFILENAME} --export-to-format=svg "$1" &> /dev/null
cat ${TEMPFILENAME}
rm ${TEMPFILENAME}
