#! /bin/sh
# Requires gnu find, gnu xargs.
set -e
mydir=`dirname "$0"`
cd "$mydir"
for d in `find -name .cvsignore -printf '%h '`; do
	(cd "$d" && rm -rf .deps && grep -v / .cvsignore | tr \\n \\0 | xargs -0r rm -f)
done
