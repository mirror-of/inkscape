# SPDX-License-Identifier: GPL-2.0-or-later

set(actions_SRC
  actions-canvas-transform.cpp
  actions-canvas-snapping.cpp

  # HEADERS
  actions-canvas-transform.h
  actions-canvas-snapping.h
  )

add_inkscape_source("${actions_SRC}")
