#!/bin/sh

TEMPFILENAME=/tmp/tmpepsifile.pdf

ps2pdf "$1" "${TEMPFILENAME}" &> /dev/null
cat ${TEMPFILENAME}
rm ${TEMPFILENAME}
