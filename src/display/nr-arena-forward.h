#ifndef __NR_ARENA_FORWARD_H__
#define __NR_ARENA_FORWARD_H__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

typedef struct _NRArena NRArena;
typedef struct _NRArenaClass NRArenaClass;

typedef struct _NRArenaItem NRArenaItem;
typedef struct _NRArenaItemClass NRArenaItemClass;

typedef struct _NRArenaGroup NRArenaGroup;
typedef struct _NRArenaGroupClass NRArenaGroupClass;

typedef struct _NRArenaShape NRArenaShape;
typedef struct _NRArenaShapeClass NRArenaShapeClass;

typedef struct _NRArenaShapeGroup NRArenaShapeGroup;
typedef struct _NRArenaShapeGroupClass NRArenaShapeGroupClass;

typedef struct _NRArenaImage NRArenaImage;
typedef struct _NRArenaImageClass NRArenaImageClass;

typedef struct _NRArenaGlyphs NRArenaGlyphs;
typedef struct _NRArenaGlyphsClass NRArenaGlyphsClass;

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
