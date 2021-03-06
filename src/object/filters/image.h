// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * @brief SVG image filter effect
 *//*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SP_FEIMAGE_H_SEEN
#define SP_FEIMAGE_H_SEEN

#include "sp-filter-primitive.h"

#define SP_FEIMAGE(obj) (dynamic_cast<SPFeImage*>((SPObject*)obj))
#define SP_IS_FEIMAGE(obj) (dynamic_cast<const SPFeImage*>((SPObject*)obj) != NULL)

class SPItem;

namespace Inkscape {
class URIReference;
}

class SPFeImage : public SPFilterPrimitive {
public:
	SPFeImage();
	~SPFeImage() override;

    gchar *href;

    /* preserveAspectRatio */
    unsigned int aspect_align : 4;
    unsigned int aspect_clip : 1;

    bool from_element;
    SPItem* SVGElem;
    Inkscape::URIReference* SVGElemRef;
    sigc::connection _image_modified_connection;
    sigc::connection _href_modified_connection;

    bool valid_for(SPObject const *obj) const override;

protected:
	void build(SPDocument* doc, Inkscape::XML::Node* repr) override;
	void release() override;

	void set(SPAttr key, const gchar* value) override;

	void update(SPCtx* ctx, unsigned int flags) override;

	Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, guint flags) override;

	void build_renderer(Inkscape::Filters::Filter* filter) override;
};

#endif /* !SP_FEIMAGE_H_SEEN */

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
