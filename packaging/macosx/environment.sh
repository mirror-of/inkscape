#!/bin/sh

export VERSIONER_PYTHON_VERSION=2.7
ARCH=`arch`
PYTHON_VERS=`python -V 2>&1 | cut -c 8-10`
export PYTHONPATH="$bundle_res/python/site-package"
export INKSCAPE_LOCALEDIR="$bundle_res/locale"
export GNOME_VFS_MODULE_CONFIG_PATH="$bundle_etc/gnome-vfs-2.0/modules"
export GNOME_VFS_MODULE_PATH="$bundle_lib/gnome-vfs-2.0/modules"
