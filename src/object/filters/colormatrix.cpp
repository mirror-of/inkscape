// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * SVG <feColorMatrix> implementation.
 *
 */
/*
 * Authors:
 *   Felipe Sanches <juca@members.fsf.org>
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2007 Felipe C. da S. Sanches
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cstring>

#include "attributes.h"
#include "svg/svg.h"
#include "colormatrix.h"
#include "xml/repr.h"
#include "helper-fns.h"

#include "display/nr-filter.h"

SPFeColorMatrix::SPFeColorMatrix() 
    : SPFilterPrimitive()
{
}

SPFeColorMatrix::~SPFeColorMatrix() = default;

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeColorMatrix variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeColorMatrix::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPFilterPrimitive::build(document, repr);

	/*LOAD ATTRIBUTES FROM REPR HERE*/
	this->readAttr(SPAttr::TYPE);
	this->readAttr(SPAttr::VALUES);
}

/**
 * Drops any allocated memory.
 */
void SPFeColorMatrix::release() {
	SPFilterPrimitive::release();
}

static Inkscape::Filters::FilterColorMatrixType sp_feColorMatrix_read_type(gchar const *value){
    if (!value) {
    	return Inkscape::Filters::COLORMATRIX_MATRIX; //matrix is default
    }

    switch(value[0]){
        case 'm':
            if (strcmp(value, "matrix") == 0) return Inkscape::Filters::COLORMATRIX_MATRIX;
            break;
        case 's':
            if (strcmp(value, "saturate") == 0) return Inkscape::Filters::COLORMATRIX_SATURATE;
            break;
        case 'h':
            if (strcmp(value, "hueRotate") == 0) return Inkscape::Filters::COLORMATRIX_HUEROTATE;
            break;
        case 'l':
            if (strcmp(value, "luminanceToAlpha") == 0) return Inkscape::Filters::COLORMATRIX_LUMINANCETOALPHA;
            break;
    }

    return Inkscape::Filters::COLORMATRIX_MATRIX; //matrix is default
}

/**
 * Sets a specific value in the SPFeColorMatrix.
 */
void SPFeColorMatrix::set(SPAttr key, gchar const *str) {
    Inkscape::Filters::FilterColorMatrixType read_type;

	/*DEAL WITH SETTING ATTRIBUTES HERE*/
    switch(key) {
        case SPAttr::TYPE:
            read_type = sp_feColorMatrix_read_type(str);

            if (this->type != read_type){
                this->type = read_type;

                // Set the default value of "value" (this may happen if the attribute "Type" is changed interactively).
                if (!value_set) {
                    value = 0;
                    if (type == Inkscape::Filters::COLORMATRIX_SATURATE) {
                        value = 1;
                    }
                }
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SPAttr::VALUES:
            if (str) {
                this->values = helperfns_read_vector(str);
                this->value = helperfns_read_number(str, HELPERFNS_NO_WARNING);
                value_set = true;
            } else {
                // Set defaults
                switch (type) {
                    case Inkscape::Filters::COLORMATRIX_MATRIX:
                        values = {1, 0, 0, 0, 0,  0, 1, 0, 0, 0,  0, 0, 1, 0, 0,  0, 0, 0, 1, 0 };
                        break;
                    case Inkscape::Filters::COLORMATRIX_SATURATE:
                        // Default value for saturate is 1.0 ("values" not used).
                        value = 1.0;
                        break;
                    case Inkscape::Filters::COLORMATRIX_HUEROTATE:
                        value = 0.0;
                        break;
                    case Inkscape::Filters::COLORMATRIX_LUMINANCETOALPHA:
                        // value, values not used.
                        break;
                }
                value_set = false;
            }
            this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        default:
        	SPFilterPrimitive::set(key, str);
            break;
    }
}

/**
 * Receives update notifications.
 */
void SPFeColorMatrix::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

	SPFilterPrimitive::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeColorMatrix::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    /* TODO: Don't just clone, but create a new repr node and write all
     * relevant values into it */
    if (!repr) {
        repr = this->getRepr()->duplicate(doc);
    }

    SPFilterPrimitive::write(doc, repr, flags);

    return repr;
}

void SPFeColorMatrix::build_renderer(Inkscape::Filters::Filter* filter) {
    g_assert(filter != nullptr);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_COLORMATRIX);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterColorMatrix *nr_colormatrix = dynamic_cast<Inkscape::Filters::FilterColorMatrix*>(nr_primitive);
    g_assert(nr_colormatrix != nullptr);

    this->renderer_common(nr_primitive);
    nr_colormatrix->set_type(this->type);
    nr_colormatrix->set_value(this->value);
    nr_colormatrix->set_values(this->values);
}

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
