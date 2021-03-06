// SPDX-License-Identifier: GPL-2.0-or-later
/** \file
 * SVG <feComposite> implementation.
 *
 */
/*
 * Authors:
 *   hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "composite.h"

#include "attributes.h"
#include "helper-fns.h"

#include "display/nr-filter.h"
#include "display/nr-filter-composite.h"

#include "object/sp-filter.h"

#include "svg/svg.h"

#include "xml/repr.h"

SPFeComposite::SPFeComposite()
    : SPFilterPrimitive(), composite_operator(COMPOSITE_DEFAULT),
      k1(0), k2(0), k3(0), k4(0), in2(Inkscape::Filters::NR_FILTER_SLOT_NOT_SET)
{
}

SPFeComposite::~SPFeComposite() = default;

/**
 * Reads the Inkscape::XML::Node, and initializes SPFeComposite variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void SPFeComposite::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPFilterPrimitive::build(document, repr);

	this->readAttr(SPAttr::OPERATOR);

	if (this->composite_operator == COMPOSITE_ARITHMETIC) {
		this->readAttr(SPAttr::K1);
		this->readAttr(SPAttr::K2);
		this->readAttr(SPAttr::K3);
		this->readAttr(SPAttr::K4);
	}

	this->readAttr(SPAttr::IN2);

	/* Unlike normal in, in2 is required attribute. Make sure, we can call
	 * it by some name. */
	if (this->in2 == Inkscape::Filters::NR_FILTER_SLOT_NOT_SET ||
		this->in2 == Inkscape::Filters::NR_FILTER_UNNAMED_SLOT)
	{
		SPFilter *parent = SP_FILTER(this->parent);
		this->in2 = this->name_previous_out();
		repr->setAttribute("in2", parent->name_for_image(this->in2));
	}
}

/**
 * Drops any allocated memory.
 */
void SPFeComposite::release() {
	SPFilterPrimitive::release();
}

static FeCompositeOperator
sp_feComposite_read_operator(gchar const *value) {
    if (!value) {
    	return COMPOSITE_DEFAULT;
    }

    if (strcmp(value, "over") == 0) {
    	return COMPOSITE_OVER;
    } else if (strcmp(value, "in") == 0) {
    	return COMPOSITE_IN;
    } else if (strcmp(value, "out") == 0) {
    	return COMPOSITE_OUT;
    } else if (strcmp(value, "atop") == 0) {
    	return COMPOSITE_ATOP;
    } else if (strcmp(value, "xor") == 0) {
    	return COMPOSITE_XOR;
    } else if (strcmp(value, "arithmetic") == 0) {
    	return COMPOSITE_ARITHMETIC;
    } else if (strcmp(value, "lighter") == 0) {
    	return COMPOSITE_LIGHTER;
    }
    std::cout << "Inkscape::Filters::FilterCompositeOperator: Unimplemented operator: " << value << std::endl;

    return COMPOSITE_DEFAULT;
}

/**
 * Sets a specific value in the SPFeComposite.
 */
void SPFeComposite::set(SPAttr key, gchar const *value) {
    int input;
    FeCompositeOperator op;
    double k_n;
    
    switch(key) {
	/*DEAL WITH SETTING ATTRIBUTES HERE*/
        case SPAttr::OPERATOR:
            op = sp_feComposite_read_operator(value);
            if (op != this->composite_operator) {
                this->composite_operator = op;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SPAttr::K1:
            k_n = value ? helperfns_read_number(value) : 0;
            if (k_n != this->k1) {
                this->k1 = k_n;
                if (this->composite_operator == COMPOSITE_ARITHMETIC)
                    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SPAttr::K2:
            k_n = value ? helperfns_read_number(value) : 0;
            if (k_n != this->k2) {
                this->k2 = k_n;
                if (this->composite_operator == COMPOSITE_ARITHMETIC)
                    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SPAttr::K3:
            k_n = value ? helperfns_read_number(value) : 0;
            if (k_n != this->k3) {
                this->k3 = k_n;
                if (this->composite_operator == COMPOSITE_ARITHMETIC)
                    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SPAttr::K4:
            k_n = value ? helperfns_read_number(value) : 0;
            if (k_n != this->k4) {
                this->k4 = k_n;
                if (this->composite_operator == COMPOSITE_ARITHMETIC)
                    this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        case SPAttr::IN2:
            input = this->read_in(value);
            if (input != this->in2) {
                this->in2 = input;
                this->parent->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
            break;

        default:
        	SPFilterPrimitive::set(key, value);
            break;
    }
}

/**
 * Receives update notifications.
 */
void SPFeComposite::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }

    /* Unlike normal in, in2 is required attribute. Make sure, we can call
     * it by some name. */
    /* This may not be true.... see issue at 
     * http://www.w3.org/TR/filter-effects/#feBlendElement (but it doesn't hurt). */
    if (this->in2 == Inkscape::Filters::NR_FILTER_SLOT_NOT_SET ||
        this->in2 == Inkscape::Filters::NR_FILTER_UNNAMED_SLOT)
    {
        SPFilter *parent = SP_FILTER(this->parent);
        this->in2 = this->name_previous_out();

		//XML Tree being used directly here while it shouldn't be.
        this->setAttribute("in2", parent->name_for_image(this->in2));
    }

    SPFilterPrimitive::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node* SPFeComposite::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags) {
    SPFilter *parent = SP_FILTER(this->parent);

    if (!repr) {
        repr = doc->createElement("svg:feComposite");
    }

    gchar const *in2_name = parent->name_for_image(this->in2);

    if( !in2_name ) {

        // This code is very similar to name_previous_out()
        SPObject *i = parent->firstChild();

        // Find previous filter primitive
        while (i && i->getNext() != this) {
        	i = i->getNext();
        }

        if( i ) {
            SPFilterPrimitive *i_prim = SP_FILTER_PRIMITIVE(i);
            in2_name = parent->name_for_image(i_prim->image_out);
        }
    }

    if (in2_name) {
        repr->setAttribute("in2", in2_name);
    } else {
        g_warning("Unable to set in2 for feComposite");
    }

    char const *comp_op;

    switch (this->composite_operator) {
        case COMPOSITE_OVER:
            comp_op = "over"; break;
        case COMPOSITE_IN:
            comp_op = "in"; break;
        case COMPOSITE_OUT:
            comp_op = "out"; break;
        case COMPOSITE_ATOP:
            comp_op = "atop"; break;
        case COMPOSITE_XOR:
            comp_op = "xor"; break;
        case COMPOSITE_ARITHMETIC:
            comp_op = "arithmetic"; break;
        case COMPOSITE_LIGHTER:
            comp_op = "lighter"; break;
        default:
            comp_op = nullptr;
    }

    repr->setAttribute("operator", comp_op);

    if (this->composite_operator == COMPOSITE_ARITHMETIC) {
        repr->setAttributeSvgDouble("k1", this->k1);
        repr->setAttributeSvgDouble("k2", this->k2);
        repr->setAttributeSvgDouble("k3", this->k3);
        repr->setAttributeSvgDouble("k4", this->k4);
    } else {
        repr->removeAttribute("k1");
        repr->removeAttribute("k2");
        repr->removeAttribute("k3");
        repr->removeAttribute("k4");
    }

    SPFilterPrimitive::write(doc, repr, flags);

    return repr;
}

void SPFeComposite::build_renderer(Inkscape::Filters::Filter* filter) {
    g_assert(filter != nullptr);

    int primitive_n = filter->add_primitive(Inkscape::Filters::NR_FILTER_COMPOSITE);
    Inkscape::Filters::FilterPrimitive *nr_primitive = filter->get_primitive(primitive_n);
    Inkscape::Filters::FilterComposite *nr_composite = dynamic_cast<Inkscape::Filters::FilterComposite*>(nr_primitive);
    g_assert(nr_composite != nullptr);

    this->renderer_common(nr_primitive);

    nr_composite->set_operator(this->composite_operator);
    nr_composite->set_input(1, this->in2);

    if (this->composite_operator == COMPOSITE_ARITHMETIC) {
        nr_composite->set_arithmetic(this->k1, this->k2,
                                     this->k3, this->k4);
    }
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
