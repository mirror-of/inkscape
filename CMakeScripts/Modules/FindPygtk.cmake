# - Try to find pygtk
# Once done this will define
#
#  PYGTK_FOUND - system has pygtk
#  PYGTK_INCLUDE_DIRS - the pygtk include directory
#  PYGTK_LIBRARIES - Link these to use pygtk
#  PYGTK_DEFINITIONS - Compiler switches required for using pygtk
#
#  Copyright (c) 2008 Joshua L. Blocher <verbalshadow@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

include(${CMAKE_CURRENT_LIST_DIR}/../HelperMacros.cmake)

if (PYGTK_INCLUDE_DIRS)
	# in cache already
	set(PYGTK_FOUND TRUE)
else (PYGTK_INCLUDE_DIRS)
	# use pkg-config to get the directories and then use these values
	# in the FIND_PATH() and FIND_LIBRARY() calls
	find_package(PkgConfig)
	if (PKG_CONFIG_FOUND)
		INKSCAPE_PKG_CONFIG_FIND_INCLUDE(PYGTK pygtk-2.0 0 pygobject.h pygtk-2.0)
	endif (PKG_CONFIG_FOUND)
endif (PYGTK_INCLUDE_DIRS)

