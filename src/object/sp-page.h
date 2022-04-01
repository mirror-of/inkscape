// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * SPPage -- a page object.
 *//*
 * Authors:
 *   Martin Owens 2021
 * 
 * Copyright (C) 2021 Martin Owens
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef SEEN_SP_PAGE_H
#define SEEN_SP_PAGE_H

#include <2geom/rect.h>
#include <vector>

#include "display/control/canvas-page.h"
#include "page-manager.h"
#include "sp-object.h"
#include "svg/svg-length.h"

class SPDesktop;
class SPItem;
namespace Inkscape {
    class ObjectSet;
}

class SPPage : public SPObject
{
public:
    SPPage();
    ~SPPage() override;

    void movePage(Geom::Affine translate, bool with_objects);
    void swapPage(SPPage *other, bool with_objects);
    static void moveItems(Geom::Affine translate, std::vector<SPItem *> const &objects);

    // Canvas visualisation
    void showPage(Inkscape::CanvasItemGroup *fg, Inkscape::CanvasItemGroup *bg);
    void hidePage(Inkscape::UI::Widget::Canvas *canvas) { _canvas_item->remove(canvas); }
    void showPage() { _canvas_item->show(); }
    void hidePage() { _canvas_item->hide(); }

    void setSelected(bool selected);
    bool setDefaultAttributes();
    int getPageIndex() const;
    int getPagePosition() const { return getPageIndex() + 1; }
    bool setPageIndex(int index, bool swap_page);
    bool setPagePosition(int position, bool swap_page) { return setPageIndex(position - 1, swap_page); }

    SPPage *getNextPage();
    SPPage *getPreviousPage();

    Geom::Rect getRect() const;
    Geom::Rect getDesktopRect() const;
    Geom::Rect getDocumentRect() const;
    Geom::Rect getSensitiveRect() const;
    void setRect(Geom::Rect rect);
    void setDesktopRect(Geom::Rect rect);
    void setDesktopSize(double width, double height);
    std::vector<SPItem *> getExclusiveItems(bool hidden = true) const;
    std::vector<SPItem *> getOverlappingItems(bool hidden = true) const;
    bool itemOnPage(SPItem *item, bool contains = false) const;
    bool isViewportPage() const;
    std::string getDefaultLabel() const;
    std::string getLabel() const;

protected:
    void build(SPDocument *doc, Inkscape::XML::Node *repr) override;
    void release() override;
    void update(SPCtx *ctx, unsigned int flags) override;
    void set(SPAttr key, const char *value) override;
    Inkscape::XML::Node *write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) override;

private:
    Inkscape::CanvasPage *_canvas_item = nullptr;

    SVGLength x;
    SVGLength y;
    SVGLength width;
    SVGLength height;
};

#endif // SEEN_SP_PAGE_H

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
