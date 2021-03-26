// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SP_FONT_H_SEEN
#define SP_FONT_H_SEEN

/*
 * SVG <font> element implementation
 *
 * Authors:
 *    Felipe C. da S. Sanches <juca@members.fsf.org>
 *
 * Copyright (C) 2008 Felipe C. da S. Sanches
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "sp-object.h"

class SPFont : public SPObject {
public:
	SPFont();
	~SPFont() override;

    double horiz_origin_x;
    double horiz_origin_y;
    double horiz_adv_x;
    double vert_origin_x;
    double vert_origin_y;
    double vert_adv_y;

protected:
	void build(SPDocument* doc, Inkscape::XML::Node* repr) override;
	void release() override;

	void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref) override;
	void remove_child(Inkscape::XML::Node* child) override;

	void set(SPAttr key, char const* value) override;

	void update(SPCtx* ctx, unsigned int flags) override;

	Inkscape::XML::Node* write(Inkscape::XML::Document* doc, Inkscape::XML::Node* repr, unsigned int flags) override;
};

MAKE_SP_OBJECT_DOWNCAST_FUNCTIONS(SP_FONT, SPFont)
MAKE_SP_OBJECT_TYPECHECK_FUNCTIONS(SP_IS_FONT, SPFont)

#endif //#ifndef SP_FONT_H_SEEN
