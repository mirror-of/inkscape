#!/bin/bash
# SPDX-License-Identifier: GPL-2.0-or-later

if [ "$#" -lt 2 ]; then
    echo "pass the path of the inkscape executable as parameter then the name of the test" $#
    exit 1
fi

INKSCAPE_EXE=$1
exit_status=0
test=$2
EXPECTED=$(dirname $test)"/expected/"$(basename $test)
testname=$(basename $test)
    INKTESTENV=true ${INKSCAPE_EXE} --export-filename=${testname}_ORIG.svg ${test}.svg #2>/dev/null >/dev/null
    INKTESTENV=true ${INKSCAPE_EXE} --export-filename=${testname}_ESPECT.svg ${EXPECTED}.svg #2>/dev/null >/dev/null
    cat ${testname}_ORIG.svg | sed 's| />||' | sed -En 's| (d=".*?")|\1|p' > ${testname}A.diff
    cat ${testname}_ESPECT.svg | sed 's| />||' | sed -En 's| (d=".*?")|\1|p' > ${testname}B.diff
    if cmp --silent -- "${testname}A.diff" "${testname}B.diff"; then
        echo ${testname} "PASSED"
        rm ${testname}_ORIG.svg ${testname}_ESPECT.svg ${testname}A.diff ${testname}B.diff
    else
	echo ${testname} "FAILED"
        exit_status=1     
    fi
exit $exit_status
