#! /bin/sh

rc=0

TMPDIR="${TMPDIR-/tmp}"
TEMPFILENAME=`mktemp 2>/dev/null || echo "$TMPDIR/tmpskfile.svg"`
skconvert "$1" ${TEMPFILENAME}.svg > /dev/null 2>&1 || rc=1

cat < "${TEMPFILENAME}.svg" || rc=1
rm -f "${TEMPFILENAME}.svg"
rm -f "${TEMPFILENAME}"
exit $rc
