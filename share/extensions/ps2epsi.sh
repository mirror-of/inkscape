#!/bin/sh

TEMPFILENAME=/tmp/tmpepsifile.epsi

ps2epsi "$1" "${TEMPFILENAME}" &> /dev/null
cat ${TEMPFILENAME}
rm ${TEMPFILENAME}
