/*
 * Utility functions for switching tools (= contexts)
 *
 * Authors:
 *   bulia byak <bulia@dr.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

enum {
  TOOLS_INVALID,
  TOOLS_SELECT,
  TOOLS_NODES,
  TOOLS_SHAPES_RECT,
  TOOLS_SHAPES_ARC,
  TOOLS_SHAPES_STAR,
  TOOLS_SHAPES_SPIRAL,
  TOOLS_FREEHAND_PENCIL,
  TOOLS_FREEHAND_PEN,
  TOOLS_CALLIGRAPHIC,
  TOOLS_TEXT,
  TOOLS_ZOOM,
  TOOLS_DROPPER
};

int tools_isactive (SPDesktop *dt, unsigned num);
int tools_active (SPDesktop *dt);
void tools_switch (SPDesktop *dt, int num);
void tools_switch_current (int num);
