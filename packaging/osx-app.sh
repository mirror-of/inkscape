#!/bin/bash
# Copyright 2005, Kees Cook <kees@outflux.net>
# Licensed under GNU General Public License
#
# Usage: osx-app Inkscape /path/to/bin/inkscape
#
# This attempts to build an Inkscape.app package for OSX, resolving
# Dynamic libraries, etc.
#
# Thanks to GNUnet's "build_app" script for help with library dep resolution.
# https://gnunet.org/svn/GNUnet/contrib/OSX/build_app

SW=/sw

pkg=$1
if [ -z $pkg ]; then
        echo "What do you want to call the package?" >&2
        exit 1
fi
shift

package=$pkg.app

binary=$1
if [ ! -x $binary ]; then
        echo "Not executable: $binary" >&2
        exit 1
fi


# Fix a given executable or library to be relocatable
fixlib () {
if [ ! -d $1 ]; then
  libs="`otool -L $1 | fgrep compatibility | cut -d\( -f1`"
  for lib in $libs; do
    base=`echo $lib | awk -F/ '{print $NF}'`
    first=`echo $lib | cut -d/ -f1-3`
    to=@executable_path/../lib/$base
    if [ $first != /usr/lib -a $first != /usr/X11R6 ]; then
      /usr/bin/install_name_tool -change $lib $to $1
      if [ "`echo $lib | fgrep libcrypto`" = "" ]; then
        /usr/bin/install_name_tool -id $to ../lib/$base
        for ll in $libs; do
          base=`echo $ll | awk -F/ '{print $NF}'`
          first=`echo $ll | cut -d/ -f1-3`
          to=@executable_path/../lib/$base
          if [ $first != /usr/lib -a $first != /usr/X11R6 -a "`echo $ll | fgrep libcrypto`" = "" ]; then
            /usr/bin/install_name_tool -change $ll $to ../lib/$base
          fi
        done
      fi
    fi
  done
fi
}



mkdir -p $package/Contents/{Frameworks,Resources}
mkdir -p $package/Contents/MacOS/{bin,lib}

binname=`basename $binary`
binpath=$package/Contents/MacOS/bin/$binname

cp $binary $binpath

# Find out libs we need from fink (e.g. $SW) - loop until no changes
a=1
nfiles=0
endl=true
while $endl; do
  echo "Looking for dependencies. Round " $a
  libs="`otool -L $package/Contents/MacOS/{bin,lib}/* 2>/dev/null | fgrep compatibility | cut -d\( -f1 | grep $SW | sort | uniq`"
  cp -f $libs $package/Contents/MacOS/lib
  let "a+=1"  
  nnfiles=`ls $package/Contents/MacOS/lib | wc -l`
  if [ $nnfiles = $nfiles ]; then
    endl=false
  else
    nfiles=$nnfiles
  fi
done

# Pull down all the share files
rsync -av `dirname $binary`/../share/$binname/* $package/Contents/Resources/

# Make an image
/usr/bin/hdiutil create -srcfolder $pkg.app $pkg.dmg
