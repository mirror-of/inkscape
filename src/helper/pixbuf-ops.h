// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef __SP_PIXBUF_OPS_H__
#define __SP_PIXBUF_OPS_H__

/*
 * Helpers for SPItem -> gdk_pixbuf related stuff
 *
 * Authors:
 *   John Cliff <simarilius@yahoo.com>
 *
 * Copyright (C) 2008 John Cliff
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <glib.h>

class SPDocument;
class SPItem;
namespace Inkscape { class Pixbuf; }

Inkscape::Pixbuf *sp_generate_internal_bitmap(SPDocument *document,
                                              Geom::Rect const &area,
                                              double dpi,
                                              std::vector<SPItem *> items = std::vector<SPItem*>(),
                                              bool set_opaque = false);
#endif
