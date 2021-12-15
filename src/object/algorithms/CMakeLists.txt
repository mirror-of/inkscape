# SPDX-License-Identifier: GPL-2.0-or-later

set(object_algorithms_SRC
  graphlayout.cpp
  removeoverlap.cpp
  unclump.cpp

  # -------
  # Headers
  bboxsort.h
  graphlayout.h
  removeoverlap.h
  unclump.h
)

add_inkscape_source("${object_algorithms_SRC}")
