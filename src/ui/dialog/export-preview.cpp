// SPDX-License-Identifier: GPL-2.0-or-later
/* Authors:
 *   Anshudhar Kumar Singh <anshudhar2001@gmail.com>
 *
 * Copyright (C) 2021 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "export-preview.h"

#include <glibmm/i18n.h>
#include <glibmm/main.h>
#include <glibmm/timer.h>
#include <gtkmm.h>

#include "display/cairo-utils.h"
#include "inkscape.h"
#include "object/sp-defs.h"
#include "object/sp-item.h"
#include "object/sp-namedview.h"
#include "object/sp-root.h"
#include "preview-util.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

ExportPreview::ExportPreview()
    : drawing(nullptr)
    , visionkey(0)
    , timer(nullptr)
    , renderTimer(nullptr)
    , pending(false)
    , minDelay(0.1)
    , size(128)
{
    pixMem = nullptr;
    image = nullptr;
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, size);
    pixMem = new guchar[size * stride];
    memset(pixMem, 0x00, size * stride);

    auto pb = Gdk::Pixbuf::create_from_data(pixMem, Gdk::COLORSPACE_RGB, true, 8, size, size, stride);
    image = Gtk::manage(new Gtk::Image(pb));
    image->show();
    image->set_name("export_preview_image");
    // add this image to box here
    this->pack_start(*image, true, true, 0);
    show_all_children();
    this->set_name("export_preview_box");
    this->set_can_focus(false);
    image->set_can_focus(false);
}

void ExportPreview::resetPixels()
{
    if (pixMem) {
        delete pixMem;
        pixMem = nullptr;
    }
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, size);
    pixMem = new guchar[size * stride];
    auto pb = Gdk::Pixbuf::create_from_data(pixMem, Gdk::COLORSPACE_RGB, true, 8, size, size, stride);
    memset(pixMem, 0x00, size * stride);
    image->set(pb);
    image->show();
}

ExportPreview::~ExportPreview()
{
    if (drawing) {
        if (_document) {
            _document->getRoot()->invoke_hide(visionkey);
        }
        delete drawing;
        drawing = nullptr;
    }
    if (timer) {
        timer->stop();
        delete timer;
        timer = nullptr;
    }
    if (renderTimer) {
        renderTimer->stop();
        delete renderTimer;
        renderTimer = nullptr;
    }
    _item = nullptr;
    _document = nullptr;
}

void ExportPreview::setItem(SPItem *item)
{
    _item = item;
}
void ExportPreview::setDbox(double x0, double x1, double y0, double y1)
{
    if (!_document) {
        return;
    }
    if ((x1 - x0 == 0) || (y1 - y0) == 0) {
        return;
    }
    _dbox = Geom::Rect(Geom::Point(x0, y0), Geom::Point(x1, y1)) * _document->dt2doc();
}

void ExportPreview::setDocument(SPDocument *document)
{
    if (drawing) {
        if (_document) {
            _document->getRoot()->invoke_hide(visionkey);
        }
        delete drawing;
        drawing = nullptr;
    }
    _document = document;
    if (_document) {
        drawing = new Inkscape::Drawing();
        visionkey = SPItem::display_key_new(1);
        DrawingItem *ai = _document->getRoot()->invoke_show(*drawing, visionkey, SP_ITEM_SHOW_DISPLAY);
        if (ai) {
            drawing->setRoot(ai);
        }
    }
}

void ExportPreview::refreshHide(const std::vector<SPItem *> *list)
{
    if (_document) {
        if (isLastHide) {
            if (drawing) {
                if (_document) {
                    _document->getRoot()->invoke_hide(visionkey);
                }
                delete drawing;
                drawing = nullptr;
            }
            drawing = new Inkscape::Drawing();
            visionkey = SPItem::display_key_new(1);
            DrawingItem *ai = _document->getRoot()->invoke_show(*drawing, visionkey, SP_ITEM_SHOW_DISPLAY);
            if (ai) {
                drawing->setRoot(ai);
            }
            isLastHide = false;
        }
        if (list && !list->empty()) {
            hide_other_items_recursively(_document->getRoot(), *list);
            isLastHide = true;
        }
    }
}

void ExportPreview::hide_other_items_recursively(SPObject *o, const std::vector<SPItem *> &list)
{
    if (SP_IS_ITEM(o) && !SP_IS_DEFS(o) && !SP_IS_ROOT(o) && !SP_IS_GROUP(o) &&
        list.end() == find(list.begin(), list.end(), o)) {
        SP_ITEM(o)->invoke_hide(visionkey);
    }

    // recurse
    if (list.end() == find(list.begin(), list.end(), o)) {
        for (auto &child : o->children) {
            hide_other_items_recursively(&child, list);
        }
    }
}

void ExportPreview::queueRefresh()
{
    if (drawing == nullptr) {
        return;
    }
    if (!pending) {
        pending = true;
        if (!timer) {
            timer = new Glib::Timer();
        }
        Glib::signal_idle().connect(sigc::mem_fun(this, &ExportPreview::refreshCB), Glib::PRIORITY_DEFAULT_IDLE);
    }
}

bool ExportPreview::refreshCB()
{
    bool callAgain = true;
    if (!timer) {
        timer = new Glib::Timer();
    }
    if (timer->elapsed() > minDelay) {
        callAgain = false;
        refreshPreview();
        pending = false;
    }
    return callAgain;
}

void ExportPreview::refreshPreview()
{
    auto document = _document;
    if (!timer) {
        timer = new Glib::Timer();
    }
    if (timer->elapsed() < minDelay) {
        // Do not refresh too quickly
        queueRefresh();
    } else if (document) {
        renderPreview();
        timer->reset();
    }
}

/*
This is main function which finally render preview. Call this after setting document, item and dbox.
If dbox is given it will use it.
if item is given and not dbox then item is used
If both are not given then simply we do nothing.
*/
void ExportPreview::renderPreview()
{
    if (!renderTimer) {
        renderTimer = new Glib::Timer();
    }
    renderTimer->reset();
    if (drawing == nullptr) {
        return;
    }

    if (_document) {
        unsigned unused;
        int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, size);
        guchar *px = nullptr;

        if (_dbox) {
            px = Inkscape::UI::PREVIEW::sp_icon_doc_icon(_document, *drawing, nullptr, size, unused, &_dbox);
        } else if (_item) {
            gchar const *id = _item->getId();
            px = Inkscape::UI::PREVIEW::sp_icon_doc_icon(_document, *drawing, id, size, unused);
        }

        if (px) {
            memcpy(pixMem, px, size * stride);
            g_free(px);
            px = nullptr;
        } else {
            memset(pixMem, 0, size * stride);
        }
        image->set(image->get_pixbuf());
        image->show();
    }

    renderTimer->stop();
    minDelay = std::max(0.1, renderTimer->elapsed() * 3.0);
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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