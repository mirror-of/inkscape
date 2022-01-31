// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * Utility functions for previewing icon representation.
 */
/* Authors:
 *   Jon A. Cruz
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *   Abhishek Sharma
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 2004 Bob Jamison
 * Copyright (C) 2005,2010 Jon A. Cruz
 * Copyright (C) 2021 Anshudhar Kumar Singh
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SP_PREVIEW_UTIL_H
#define SP_PREVIEW_UTIL_H

#include <glibmm/main.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "display/drawing.h"

class SPDocument;
class SPItem;

namespace Inkscape {
namespace UI {
namespace PREVIEW {

// takes doc, drawing, icon, and icon name to produce pixels
GdkPixbuf *render_preview(SPDocument *doc, Inkscape::Drawing &drawing, SPItem *item,
                          unsigned width_in, unsigned height_in, Geom::OptRect *dboxIn = nullptr);

} // namespace PREVIEW
} // namespace UI
} // namespace Inkscape

#endif
