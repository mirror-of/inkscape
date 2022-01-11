// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SP_EXPORT_PREVIEW_H
#define SP_EXPORT_PREVIEW_H

#include <gtkmm.h>

#include "desktop.h"
#include "document.h"

class SPObject;
namespace Glib {
class Timer;
}

namespace Inkscape {
class Drawing;
namespace UI {
namespace Dialog {

class ExportPreview : public Gtk::Box
{
public:
    ExportPreview();
    ~ExportPreview() override;

private:
    bool isLastHide = false;

private:
    SPItem *_item = nullptr;

    Geom::OptRect _dbox;

    SPDocument *_document = nullptr;

    Drawing *drawing;
    unsigned int visionkey;
    Glib::Timer *timer;
    Glib::Timer *renderTimer;
    bool pending;
    gdouble minDelay;

    int size; // size of preview image

    guchar *pixMem;    // Our Itme pixels
    Gtk::Image *image; // Our Item Image

public:
    void setDocument(SPDocument *document);
    void refreshHide(const std::vector<SPItem *> *list);
    void hide_other_items_recursively(SPObject *o, const std::vector<SPItem *> &list);
    void setItem(SPItem *item);
    void setDbox(double x0, double x1, double y0, double y1);
    void queueRefresh();
    void resetPixels();

    void setSize(int newSize)
    {
        size = newSize;
        resetPixels();
    }

private:
    void refreshPreview();
    void renderPreview();
    bool refreshCB();
};
} // namespace Dialog
} // namespace UI
} // namespace Inkscape
#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :