#define __SP_STROKE_STYLE_C__

/**
 * \brief  Stroke style dialog
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Bryce Harrington <brycehar@bryceharrington.com>
 *
 * Copyright (C) 2001-2002 authors
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2004 John Cliff
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_SS_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <glib.h>

#include <libnr/nr-values.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-matrix-fns.h>

#include <gtk/gtk.h>

#include "helper/sp-intl.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "widgets/sp-widget.h"
#include "widgets/spw-utilities.h"
#include "sp-gradient.h"
#include <sp-use.h>
#include <sp-pattern.h>
#include <widgets/paint-selector.h>
#include <widgets/dash-selector.h>
#include "enums.h"
#include "style.h"
#include "gradient-chemistry.h"
#include "document.h"
#include "desktop-handles.h"
#include "marker-status.h"
#include "selection.h"
#include "sp-item.h"
#include "inkscape.h"
#include "inkscape-stock.h"
#include "dialogs/dialog-events.h"
#include "sp-root.h"
#include "sp-defs.h"
#include "sp-text.h"
#include "document-private.h"
#include "xml/repr-private.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"
#include "path-prefix.h"
#include "widgets/icon.h"
#include "helper/stock-items.h"
#include <file.h>

#include "dialogs/stroke-style.h"

/* Paint */

static void sp_stroke_style_paint_construct(SPWidget *spw, SPPaintSelector *psel);
static void sp_stroke_style_paint_selection_modified (SPWidget *spw, SPSelection *selection, guint flags, SPPaintSelector *psel);
static void sp_stroke_style_paint_selection_changed (SPWidget *spw, SPSelection *selection, SPPaintSelector *psel);
static void sp_stroke_style_paint_attr_changed(SPWidget *spw, gchar const *key, gchar const *oldval, gchar const *newval);
static void sp_stroke_style_paint_update(SPWidget *spw, SPSelection *sel);
static void sp_stroke_style_paint_update_repr(SPWidget *spw, SPRepr *repr);

static void sp_stroke_style_paint_mode_changed(SPPaintSelector *psel, SPPaintSelectorMode mode, SPWidget *spw);
static void sp_stroke_style_paint_dragged(SPPaintSelector *psel, SPWidget *spw);
static void sp_stroke_style_paint_changed(SPPaintSelector *psel, SPWidget *spw);

static void sp_stroke_style_get_average_color_rgba(GSList const *objects, gfloat *c);
static void sp_stroke_style_get_average_color_cmyka(GSList const *objects, gfloat *c);
static SPPaintSelectorMode sp_stroke_style_determine_paint_selector_mode(SPStyle *style);



GtkWidget *
sp_stroke_style_paint_widget_new(void)
{
    GtkWidget *spw, *psel;

    spw = sp_widget_new_global(INKSCAPE);

    psel = sp_paint_selector_new();
    gtk_widget_show(psel);
    gtk_container_add(GTK_CONTAINER(spw), psel);
    gtk_object_set_data(GTK_OBJECT(spw), "paint-selector", psel);

    gtk_signal_connect(GTK_OBJECT(spw), "construct",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_construct),
                       psel);
    gtk_signal_connect(GTK_OBJECT(spw), "modify_selection",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_selection_modified),
                       psel);
    gtk_signal_connect(GTK_OBJECT(spw), "change_selection",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_selection_changed),
                       psel);
    gtk_signal_connect(GTK_OBJECT(spw), "attr_changed",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_attr_changed),
                       psel);

    gtk_signal_connect(GTK_OBJECT(psel), "mode_changed",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_mode_changed),
                       spw);
    gtk_signal_connect(GTK_OBJECT(psel), "dragged",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_dragged),
                       spw);
    gtk_signal_connect(GTK_OBJECT(psel), "changed",
                       GTK_SIGNAL_FUNC(sp_stroke_style_paint_changed),
                       spw);

    sp_stroke_style_paint_update(SP_WIDGET(spw),
                                 ( SP_ACTIVE_DESKTOP
                                   ? SP_DT_SELECTION(SP_ACTIVE_DESKTOP)
                                   : NULL ));

    return spw;

}


static void
sp_stroke_style_paint_construct(SPWidget *spw, SPPaintSelector *psel)
{
#ifdef SP_SS_VERBOSE
    g_print( "Stroke style widget constructed: inkscape %p repr %p\n",
             spw->inkscape, spw->repr );
#endif
    if (spw->inkscape) {
        sp_stroke_style_paint_update( spw,
                                      ( SP_ACTIVE_DESKTOP
                                        ? SP_DT_SELECTION(SP_ACTIVE_DESKTOP)
                                        : NULL ));
    } else if (spw->repr) {
        sp_stroke_style_paint_update_repr(spw, spw->repr);
    }
}



static void
sp_stroke_style_paint_selection_modified ( SPWidget *spw,
                                        SPSelection *selection,
                                        guint flags,
                                        SPPaintSelector *psel)
{
    if (flags & ( SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG |
                  SP_OBJECT_STYLE_MODIFIED_FLAG) ) {
        sp_stroke_style_paint_update(spw, selection);
    }
}


static void
sp_stroke_style_paint_selection_changed ( SPWidget *spw,
                                        SPSelection *selection,
                                        SPPaintSelector *psel )
{
    sp_stroke_style_paint_update (spw, selection);
}


static void
sp_stroke_style_paint_attr_changed( SPWidget *spw,
                                    gchar const *key,
                                    gchar const *oldval,
                                    gchar const *newval )
{
    if (!strcmp(key, "style")) {
        /* This sounds interesting */
        sp_stroke_style_paint_update_repr(spw, spw->repr);
    }
}


static void
sp_stroke_style_paint_update (SPWidget *spw, SPSelection *sel)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    SPPaintSelector *psel = SP_PAINT_SELECTOR(gtk_object_get_data(GTK_OBJECT(spw),
                                                                  "paint-selector"));

    if ( !sel || sel->isEmpty() ) {
        /* No objects, set empty */
        sp_paint_selector_set_mode(psel, SP_PAINT_SELECTOR_MODE_EMPTY);
        gtk_object_set_data( GTK_OBJECT(spw), "update",
                             GINT_TO_POINTER(FALSE) );
        return;
    }

    GSList const *objects = sel->itemList();
    SPObject *object = SP_OBJECT(objects->data);
    // prevent change of style on clones.
    for (GSList const *l = sel->itemList(); l != NULL; l = l->next) {
      if (SP_IS_USE(l->data)) {
            sp_paint_selector_set_mode(psel, SP_PAINT_SELECTOR_MODE_CLONE);
            gtk_object_set_data( GTK_OBJECT(spw), "update", GINT_TO_POINTER(FALSE) );
            return;
        }
    }
    // prevent trying to modify objects with multiple fill modes
    SPPaintSelectorMode pselmode =
        sp_stroke_style_determine_paint_selector_mode(SP_OBJECT_STYLE(object));

    for (GSList const *l = objects->next; l != NULL; l = l->next) {
        SPPaintSelectorMode nextmode
            = sp_stroke_style_determine_paint_selector_mode(SP_OBJECT_STYLE(l->data));
        if (nextmode != pselmode) {
            /* Multiple styles */
            sp_paint_selector_set_mode(psel, SP_PAINT_SELECTOR_MODE_MULTIPLE);
            gtk_object_set_data( GTK_OBJECT(spw),
                                 "update",
                                 GINT_TO_POINTER(FALSE) );
            return;
        }
    }
#ifdef SP_SS_VERBOSE
    g_print("StrokeStyleWidget: paint selector mode %d\n", pselmode);
#endif
    switch (pselmode) {
        case SP_PAINT_SELECTOR_MODE_NONE:
            /* No paint at all */
            sp_paint_selector_set_mode(psel, SP_PAINT_SELECTOR_MODE_NONE);
            break;

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        {
            gfloat c[4];
            sp_paint_selector_set_mode(psel, SP_PAINT_SELECTOR_MODE_COLOR_RGB);
            sp_stroke_style_get_average_color_rgba(objects, c);
            SPColor color;
            sp_color_set_rgb_float(&color, c[0], c[1], c[2]);
            sp_paint_selector_set_color_alpha(psel, &color, c[3]);
            break;
        }

        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            gfloat c[5];
            sp_paint_selector_set_mode( psel,
                                        SP_PAINT_SELECTOR_MODE_COLOR_CMYK);
            sp_stroke_style_get_average_color_cmyka(objects, c);
            SPColor color;
            sp_color_set_cmyk_float(&color, c[0], c[1], c[2], c[3]);
            sp_paint_selector_set_color_alpha(psel, &color, c[4]);
            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
        {
            object = SP_OBJECT(objects->data);
            /* We know that all objects have lineargradient stroke style */
            SPGradient *vector =
                sp_gradient_get_vector(SP_GRADIENT(SP_OBJECT_STYLE_STROKE_SERVER(object)),
                                       FALSE);

            for (GSList const *l = objects->next; l != NULL; l = l->next) {
                SPObject *next = SP_OBJECT(l->data);
                if (sp_gradient_get_vector(SP_GRADIENT(SP_OBJECT_STYLE_STROKE_SERVER(next)),
                                           FALSE) != vector)
                {

                    /* Multiple vectors */
                    sp_paint_selector_set_mode(psel,
                                               SP_PAINT_SELECTOR_MODE_MULTIPLE);
                    gtk_object_set_data( GTK_OBJECT(spw),
                                         "update",
                                         GINT_TO_POINTER(FALSE) );
                    return;
                }
            } // end of for loop


            /* TODO: Probably we should set multiple mode here too */

            sp_paint_selector_set_mode(psel,
                                       SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR);
            sp_paint_selector_set_gradient_linear(psel, vector);
            NRRect fbb;
            sel->boundsInDocument(&fbb);
            sp_paint_selector_set_gradient_bbox( psel, fbb.x0, fbb.y0,
                                                 fbb.x1, fbb.y1 );

            /* TODO: This is plain wrong */

            SPLinearGradient *lg = SP_LINEARGRADIENT(SP_OBJECT_STYLE_STROKE_SERVER(object));
            sp_item_invoke_bbox(SP_ITEM(object), &fbb, NR::identity(), TRUE);
            NRMatrix fctm;
            sp_item_i2doc_affine(SP_ITEM(object), &fctm);
            NRMatrix gs2d;
            sp_gradient_get_gs2d_matrix_f( SP_GRADIENT(lg), &fctm,
                                           &fbb, &gs2d );
            sp_paint_selector_set_gradient_gs2d_matrix_f(psel, &gs2d);
            sp_paint_selector_set_gradient_properties(psel,
                                                      SP_GRADIENT_UNITS(lg),
                                                      SP_GRADIENT_SPREAD(lg));
            sp_paint_selector_set_lgradient_position( psel,
                                                      lg->x1.computed,
                                                      lg->y1.computed,
                                                      lg->x2.computed,
                                                      lg->y2.computed );
            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
        {
            object = SP_OBJECT(objects->data);

            /* We know that all objects have radialgradient stroke style */

            SPGradient *vector =
                sp_gradient_get_vector(SP_GRADIENT(SP_OBJECT_STYLE_STROKE_SERVER(object)),
                                       FALSE);

            for (GSList const *l = objects->next; l != NULL; l = l->next) {
                SPObject *next = SP_OBJECT(l->data);
                if (sp_gradient_get_vector(SP_GRADIENT(SP_OBJECT_STYLE_STROKE_SERVER(next)),
                                           FALSE) != vector)
                {
                    /* Multiple vectors */
                    sp_paint_selector_set_mode(psel,
                                               SP_PAINT_SELECTOR_MODE_MULTIPLE);
                    gtk_object_set_data( GTK_OBJECT(spw), "update",
                                         GINT_TO_POINTER(FALSE) );
                    return;
                }
            }

            /* TODO: Probably we should set multiple mode here too */

            sp_paint_selector_set_gradient_radial(psel, vector);
            NRRect fbb;
            sel->boundsInDocument(&fbb);
            sp_paint_selector_set_gradient_bbox( psel, fbb.x0, fbb.y0,
                                                 fbb.x1, fbb.y1);

            /* TODO: This is plain wrong */

            SPRadialGradient *rg = SP_RADIALGRADIENT(SP_OBJECT_STYLE_STROKE_SERVER(object));
            sp_item_invoke_bbox(SP_ITEM(object), &fbb, NR::identity(), TRUE);
            NRMatrix fctm;
            sp_item_i2doc_affine(SP_ITEM(object), &fctm);
            NRMatrix gs2d;
            sp_gradient_get_gs2d_matrix_f( SP_GRADIENT(rg), &fctm,
                                           &fbb, &gs2d );
            sp_paint_selector_set_gradient_gs2d_matrix_f(psel, &gs2d);
            sp_paint_selector_set_gradient_properties(psel,
                                                      SP_GRADIENT_UNITS(rg),
                                                      SP_GRADIENT_SPREAD(rg) );

            sp_paint_selector_set_rgradient_position( psel,
                                                      rg->cx.computed,
                                                      rg->cy.computed,
                                                      rg->fx.computed,
                                                      rg->fy.computed,
                                                      rg->r.computed );
            break;
        }

        case SP_PAINT_SELECTOR_MODE_PATTERN:
        {
            sp_paint_selector_set_mode ( psel, SP_PAINT_SELECTOR_MODE_PATTERN);
            SPPattern *pat = pattern_getroot (SP_PATTERN (SP_OBJECT_STYLE_STROKE_SERVER (object)));
            sp_update_pattern_list ( psel, pat );
            break;
        }

        default:
            sp_paint_selector_set_mode( psel, SP_PAINT_SELECTOR_MODE_MULTIPLE );
            break;
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));

}



static void
sp_stroke_style_paint_update_repr(SPWidget *spw, SPRepr *repr)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    SPPaintSelector *psel = SP_PAINT_SELECTOR(gtk_object_get_data( GTK_OBJECT(spw),
                                                                   "paint-selector") );

    SPStyle *style = sp_style_new();
    sp_style_read_from_repr(style, repr);

    SPPaintSelectorMode pselmode = sp_stroke_style_determine_paint_selector_mode(style);
#ifdef SP_SS_VERBOSE
    g_print("StrokeStyleWidget: paint selector mode %d\n", pselmode);
#endif

    switch (pselmode) {
        case SP_PAINT_SELECTOR_MODE_NONE:
            /* No paint at all */
            sp_paint_selector_set_mode(psel, SP_PAINT_SELECTOR_MODE_NONE);
            break;

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
            sp_paint_selector_set_mode(psel, pselmode);
            sp_paint_selector_set_color_alpha(psel, &style->stroke.value.color,
                                              SP_SCALE24_TO_FLOAT(style->stroke_opacity.value) );
            break;

        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
            /* fixme: Think about it (Lauris) */
            break;

        default:
            break;
    }

    sp_style_unref(style);

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));

}



static void
sp_stroke_style_paint_mode_changed( SPPaintSelector *psel,
                                    SPPaintSelectorMode mode,
                                    SPWidget *spw )
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    /* TODO: Does this work?
     * Not really, here we have to get old color back from object
     * Instead of relying on paint widget having meaningful colors set
     */
    sp_stroke_style_paint_changed(psel, spw);

}



static void
sp_stroke_style_paint_dragged(SPPaintSelector *psel, SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

#ifdef SP_SS_VERBOSE
    g_print("StrokeStyleWidget: paint dragged\n");
#endif

    switch (psel->mode) {
        case SP_PAINT_SELECTOR_MODE_EMPTY:
        case SP_PAINT_SELECTOR_MODE_MULTIPLE:
        case SP_PAINT_SELECTOR_MODE_NONE:
            g_warning( "file %s: line %d: Paint %d should not emit 'dragged'",
                       __FILE__, __LINE__, psel->mode);
            break;

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            SPColor color;
            gfloat alpha;
            sp_paint_selector_get_color_alpha(psel, &color, &alpha);
            GSList const *items = sp_widget_get_item_list(spw);
            for (GSList const *i = items; i != NULL; i = i->next) {
                sp_style_set_stroke_color_alpha( SP_OBJECT_STYLE(i->data),
                                                 &color, alpha,
                                                 TRUE, TRUE );
            }
            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
        {
            SPGradient *vector = sp_paint_selector_get_gradient_vector(psel);
            vector = sp_gradient_ensure_vector_normalized(vector);
            GSList const *items = sp_widget_get_item_list(spw);
            for (GSList const *i = items; i != NULL; i = i->next) {
                SPGradient *lg
                    = sp_item_force_stroke_lineargradient_vector(SP_ITEM(i->data),
                                                                 vector);
                sp_paint_selector_write_lineargradient(psel,
                                                       SP_LINEARGRADIENT(lg),
                                                       SP_ITEM(i->data) );
            }
            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
        {
            SPGradient *vector = sp_paint_selector_get_gradient_vector(psel);
            vector = sp_gradient_ensure_vector_normalized(vector);
            GSList const *items = sp_widget_get_item_list(spw);
            for (GSList const *i = items; i != NULL; i = i->next) {
                SPGradient *rg
                    = sp_item_force_stroke_radialgradient_vector(SP_ITEM(i->data),
                                                                 vector);
                sp_paint_selector_write_radialgradient(psel,
                                                       SP_RADIALGRADIENT(rg),
                                                       SP_ITEM(i->data));
            }
            break;
        }

        case SP_PAINT_SELECTOR_MODE_PATTERN:
            /*user selected existing pattern from list*/
            break;


        default:
            g_warning( "file %s: line %d: Paint selector should not be in "
                       "mode %d",
                       __FILE__, __LINE__,
                       psel->mode );
            break;
    }

}



static void
sp_stroke_style_paint_changed(SPPaintSelector *psel, SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

#ifdef SP_SS_VERBOSE
    g_print("StrokeStyleWidget: paint changed\n");
#endif

    GSList const *items;
    GSList *reprs;
    if (spw->inkscape) {
        /* fixme: */
        if (!SP_WIDGET_DOCUMENT(spw))
            return;

        reprs = NULL;
        items = sp_widget_get_item_list(spw);
        for (GSList const *i = items; i != NULL; i = i->next) {
            reprs = g_slist_prepend(reprs, SP_OBJECT_REPR(i->data));
        }
    } else {
        reprs = g_slist_prepend(NULL, spw->repr);
        items = NULL;
    }

    switch (psel->mode) {
        case SP_PAINT_SELECTOR_MODE_EMPTY:
        case SP_PAINT_SELECTOR_MODE_MULTIPLE:
            g_warning( "file %s: line %d: Paint %d should not emit 'changed'",
                       __FILE__, __LINE__, psel->mode);
            break;

        case SP_PAINT_SELECTOR_MODE_NONE:
        {
            SPCSSAttr *css = sp_repr_css_attr_new();
            sp_repr_css_set_property(css, "stroke", "none");
            for (GSList const *r = reprs; r != NULL; r = r->next) {
                sp_repr_css_change_recursive((SPRepr *) r->data, css, "style");
                sp_repr_set_attr_recursive((SPRepr *) r->data,
                                           "sodipodi:stroke-cmyk", NULL);
            }
            sp_repr_css_attr_unref(css);
            if (spw->inkscape) {
                sp_document_done(SP_WIDGET_DOCUMENT(spw));
            }
            break;
        }

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            SPCSSAttr *css = sp_repr_css_attr_new();
            SPColor color;
            gfloat alpha;
            sp_paint_selector_get_color_alpha(psel, &color, &alpha);
            gchar b[64];
            sp_svg_write_color( b, 64,
                                sp_color_get_rgba32_falpha(&color, alpha) );
            sp_repr_css_set_property(css, "stroke", b);
            Inkscape::SVGOStringStream osalpha;
            osalpha << alpha;
            sp_repr_css_set_property(css, "stroke-opacity", osalpha.str().c_str());

            Inkscape::SVGOStringStream oscolour;
            if ( sp_color_get_colorspace_type(&color) == SP_COLORSPACE_TYPE_CMYK ) {
                gfloat cmyk[4];
                sp_color_get_cmyk_floatv(&color, cmyk);
                oscolour << "(" << cmyk[0] << " " << cmyk[1] << " " << cmyk[2] << " " << cmyk[3] << ")";
            }

            for (GSList const *r = reprs; r != NULL; r = r->next) {
                sp_repr_set_attr_recursive((SPRepr *) r->data,
                                           "sodipodi:stroke-cmyk",
                                           ( oscolour.str().length() > 0
                                             ? oscolour.str().c_str()
                                             : NULL ));
                sp_repr_css_change_recursive((SPRepr *) r->data, css, "style");
            }
            sp_repr_css_attr_unref(css);

            if (spw->inkscape) {
                sp_document_done(SP_WIDGET_DOCUMENT(spw));
            }

            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:

            if (items) {
                SPGradient *vector = sp_paint_selector_get_gradient_vector(psel);

                if (!vector) {
                    /* No vector in paint selector should mean that we just
                     * changed mode
                     */
                    vector =
                        sp_document_default_gradient_vector(SP_WIDGET_DOCUMENT(spw));

                    for (GSList const *i = items; i != NULL; i = i->next) {
                        sp_item_force_stroke_lineargradient_vector(SP_ITEM(i->data), vector);
                    }
                } else {
                    vector = sp_gradient_ensure_vector_normalized(vector);
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        SPGradient *lg;
                        lg = sp_item_force_stroke_lineargradient_vector(SP_ITEM(i->data), vector);
                        sp_paint_selector_write_lineargradient(psel,
                                                               SP_LINEARGRADIENT(lg),
                                                               SP_ITEM(i->data));

                        SP_OBJECT(lg)->updateRepr();
                    }
                }
                sp_document_done(SP_WIDGET_DOCUMENT(spw));
            }
            break;

        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
            if (items) {
                SPGradient *vector = sp_paint_selector_get_gradient_vector(psel);
                if (!vector) {
                    /* No vector in paint selector should mean that we just
                     * changed mode
                     */
                    vector = sp_document_default_gradient_vector(SP_WIDGET_DOCUMENT(spw));
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        sp_item_force_stroke_radialgradient_vector(SP_ITEM(i->data), vector);
                    }
                } else {
                    vector = sp_gradient_ensure_vector_normalized(vector);
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        SPGradient *lg
                            = sp_item_force_stroke_radialgradient_vector(SP_ITEM(i->data), vector);
                        sp_paint_selector_write_radialgradient(psel,
                                                               SP_RADIALGRADIENT(lg),
                                                               SP_ITEM(i->data));
                        SP_OBJECT(lg)->updateRepr();
                    }
                }
                sp_document_done(SP_WIDGET_DOCUMENT(spw));
            }
            break;

        case SP_PAINT_SELECTOR_MODE_PATTERN:

            if (items) {

                SPPattern *pattern = sp_paint_selector_get_pattern (psel);
                if (!pattern) {

                    /* No Pattern in paint selector should mean that we just
                     * changed mode - dont do jack.
                     */

                } else {
                    SPRepr *patrepr = SP_OBJECT_REPR(pattern);
                    SPCSSAttr *css = sp_repr_css_attr_new ();
                    gchar *urltext = g_strdup_printf ("url(#%s)", sp_repr_attr (patrepr, "id"));
                    sp_repr_css_set_property (css, "stroke", urltext);

                    for (const GSList *i = items; i != NULL; i = i->next) {
                         SPRepr *selrepr = SP_OBJECT_REPR (i->data);
                         SPObject *selobj = SP_OBJECT (i->data);
                         if (!selrepr)
                             continue;

                         SPStyle *style = SP_OBJECT_STYLE (selobj);
                         if (style && style->stroke.type == SP_PAINT_TYPE_PAINTSERVER) {
                             SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (selobj);
                             if (SP_IS_PATTERN (server) && pattern_getroot (SP_PATTERN(server)) == pattern)
                                 // only if this object's pattern is not rooted in our selected pattern, apply
                                 continue;
                         }

                         sp_repr_css_change_recursive (selrepr, css, "style");
                     }

                    sp_repr_css_attr_unref (css);
                    g_free (urltext);

                } // end if

                sp_document_done (SP_WIDGET_DOCUMENT (spw));
            } // end if

            break;

        case SP_PAINT_SELECTOR_MODE_CLONE:
            g_warning( "file %s: line %d: Paint %d should not emit 'changed'",
                       __FILE__, __LINE__, psel->mode);
            break;

        default:
            g_warning( "file %s: line %d: Paint selector should not be in "
                       "mode %d",
                       __FILE__, __LINE__,
                       psel->mode );
            break;
    }

    g_slist_free(reprs);

}





/* Line */

static void sp_stroke_style_line_construct(SPWidget *spw, gpointer data);
static void sp_stroke_style_line_selection_modified (SPWidget *spw,
                                                  SPSelection *selection,
                                                  guint flags,
                                                  gpointer data);

static void sp_stroke_style_line_selection_changed (SPWidget *spw,
                                                   SPSelection *selection,
                                                   gpointer data );

static void sp_stroke_style_line_attr_changed (SPWidget *spw,
                                              gchar const *key,
                                              gchar const *oldval,
                                              gchar const *newval);

static void sp_stroke_style_line_update(SPWidget *spw, SPSelection *sel);
static void sp_stroke_style_line_update_repr(SPWidget *spw, SPRepr *repr);

static void sp_stroke_style_set_join_buttons(SPWidget *spw,
                                             GtkWidget *active);

static void sp_stroke_style_set_cap_buttons(SPWidget *spw,
                                            GtkWidget *active);

static void sp_stroke_style_width_changed(GtkAdjustment *adj, SPWidget *spw);
static void sp_stroke_style_miterlimit_changed(GtkAdjustment *adj, SPWidget *spw);
static void sp_stroke_style_any_toggled(GtkToggleButton *tb, SPWidget *spw);
static void sp_stroke_style_line_dash_changed(SPDashSelector *dsel,
                                              SPWidget *spw);

static void sp_stroke_style_update_marker_menus(SPWidget *spw, GSList const *objects);

static SPObject *ink_extract_marker_name(gchar const *n);


/**
 * Helper function for creating radio buttons.  This should probably be re-thought out
 * when reimplementing this with Gtkmm.
 */
static GtkWidget *
sp_stroke_radio_button(GtkWidget *tb, char const *n, char const *xpm,
                       GtkWidget *hb, GtkWidget *spw,
                       gchar const *key, gchar const *data)
{
    g_assert(xpm != NULL);
    g_assert(hb  != NULL);
    g_assert(spw != NULL);

    if (tb == NULL) {
        tb = gtk_radio_button_new(NULL);
    } else {
        tb = gtk_radio_button_new(gtk_radio_button_group(GTK_RADIO_BUTTON(tb)) );
    }

    gtk_widget_show(tb);
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(tb), FALSE);
    gtk_box_pack_start(GTK_BOX(hb), tb, FALSE, FALSE, 0);
    gtk_object_set_data(GTK_OBJECT(spw), n, tb);
    gtk_object_set_data(GTK_OBJECT(tb), key, (gpointer*)data);
    gtk_signal_connect(GTK_OBJECT(tb), "toggled",
                       GTK_SIGNAL_FUNC(sp_stroke_style_any_toggled),
                       spw);
    GtkWidget *px = gtk_image_new_from_stock(n, GTK_ICON_SIZE_LARGE_TOOLBAR);
    g_assert(px != NULL);
    gtk_widget_show(px);
    gtk_container_add(GTK_CONTAINER(tb), px);

    return tb;

}

void mm_print (gchar *say, NR::Matrix m)
{ g_print ("%s %g %g %g %g %g %g\n", say, m[0], m[1], m[2], m[3], m[4], m[5]); }


static GtkWidget *
sp_marker_prev_new (unsigned int size, gchar const *mname, SPDocument *source, SPDocument *sandbox, gchar *menu_id, const NRArena *arena, unsigned int visionkey, NRArenaItem *root)
{
    // the object of the marker
    const SPObject *marker = source->getObjectById(mname);
    if (marker == NULL)
        return NULL;

    // the repr of the marker; make a copy with id="sample"
    SPRepr *mrepr = sp_repr_duplicate (SP_OBJECT_REPR (marker));
    sp_repr_set_attr (mrepr, "id", "sample");

    // replace the old sample in the sandbox by the new one
    SPRepr *defsrepr = SP_OBJECT_REPR (sandbox->getObjectById("defs"));
    SPObject *oldmarker = sandbox->getObjectById("sample");
    if (oldmarker)
        oldmarker->deleteObject(false);
    sp_repr_append_child (defsrepr, mrepr);
    sp_repr_unref (mrepr);

// Uncomment this to get the sandbox documents saved (useful for debugging)
    //FILE *fp = fopen (g_strconcat(mname, ".svg", NULL), "w");
    //sp_repr_save_stream (sp_document_repr_doc (sandbox), fp);
    //fclose (fp);

    // object to render; note that the id is the same as that of the menu we're building
    SPObject *object = sandbox->getObjectById(menu_id);
    sp_document_root (sandbox)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    sp_document_ensure_up_to_date(sandbox);

    if (object == NULL || !SP_IS_ITEM(object))
        return NULL; // sandbox broken?

    // Find object's bbox in document
    NR::Matrix const i2doc(sp_item_i2doc_affine(SP_ITEM(object)));

    NRRect dbox;
    sp_item_invoke_bbox(SP_ITEM(object), &dbox, i2doc, TRUE);

    if (nr_rect_d_test_empty(&dbox))
        return NULL;

    /* Update to renderable state */
    NRMatrix t;
    double sf = 0.8;
    nr_matrix_set_scale(&t, sf, sf);
    nr_arena_item_set_transform(root, &t);
    NRGC gc(NULL);
    nr_matrix_set_identity(&gc.transform);
    nr_arena_item_invoke_update( root, NULL, &gc,
                                 NR_ARENA_ITEM_STATE_ALL,
                                 NR_ARENA_ITEM_STATE_NONE );

    /* Item integer bbox in points */
    NRRectL ibox;
    ibox.x0 = (int) floor(sf * dbox.x0 + 0.5);
    ibox.y0 = (int) floor(sf * dbox.y0 + 0.5);
    ibox.x1 = (int) floor(sf * dbox.x1 + 0.5);
    ibox.y1 = (int) floor(sf * dbox.y1 + 0.5);

    /* Find visible area */
    int width = ibox.x1 - ibox.x0;
    int height = ibox.y1 - ibox.y0;
    int dx = size;
    int dy = size;
    dx=(dx - width)/2; // watch out for size, since 'unsigned'-'signed' can cause problems if the result is negative
    dy=(dy - height)/2;

    NRRectL area;
    area.x0 = ibox.x0 - dx;
    area.y0 = ibox.y0 - dy;
    area.x1 = area.x0 + size;
    area.y1 = area.y0 + size;

    /* Actual renderable area */
    NRRectL ua;
    ua.x0 = MAX(ibox.x0, area.x0);
    ua.y0 = MAX(ibox.y0, area.y0);
    ua.x1 = MIN(ibox.x1, area.x1);
    ua.y1 = MIN(ibox.y1, area.y1);

    /* Set up pixblock */
    guchar *px = nr_new(guchar, 4 * size * size);
    memset(px, 0x00, 4 * size * size);

    /* Render */
    NRPixBlock B;
    nr_pixblock_setup_extern( &B, NR_PIXBLOCK_MODE_R8G8B8A8N,
                              ua.x0, ua.y0, ua.x1, ua.y1,
                              px + 4 * size * (ua.y0 - area.y0) +
                              4 * (ua.x0 - area.x0),
                              4 * size, FALSE, FALSE );
    nr_arena_item_invoke_render( root, &ua, &B,
                                 NR_ARENA_ITEM_RENDER_NO_CACHE );
    nr_pixblock_release(&B);

    // Create widget
    GtkWidget *pb = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(px,
                              GDK_COLORSPACE_RGB,
                              TRUE,
                              8, size, size, size * 4,
                              (GdkPixbufDestroyNotify)nr_free,
                              NULL));
    return pb;
}


#define MARKER_ITEM_MARGIN 0


/**
 * sp_marker_list_from_doc()
 *
 * \brief Pick up all markers from source, except those that are in
 * current_doc (if non-NULL), and add items to the m menu
 *
 */
static void
sp_marker_list_from_doc (GtkWidget *m, SPDocument *current_doc, SPDocument *source, SPDocument *markers_doc, SPDocument *sandbox, gchar *menu_id)
{

    // search through defs
    GSList *ml = NULL;
    SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS (source);
    for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(defs)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT (ochild) ) {
        if (SP_IS_MARKER(ochild)) {
            ml = g_slist_prepend (ml, ochild);
        }
    }

    // Do this here, outside of loop, to speed up preview generation:
    /* Create new arena */
    const NRArena *arena = NRArena::create();
    /* Create ArenaItem and set transform */
    const unsigned int visionkey = sp_item_display_key_new(1);
    NRArenaItem *root =  sp_item_invoke_show( SP_ITEM(SP_DOCUMENT_ROOT (sandbox)), (NRArena *) arena, visionkey, SP_ITEM_SHOW_PRINT );

    for (; ml != NULL; ml = ml->next) {

        if (!SP_IS_MARKER(ml->data))
            continue;

        SPRepr *repr = SP_OBJECT_REPR((SPItem *) ml->data);

        if (markers_doc && sp_repr_attr(repr,"inkscape:stockid")) {
            // find out if markers_doc has a marker with the same stockid, and if so, skip this
            for (SPObject *child = sp_object_first_child(SP_OBJECT(SP_DOCUMENT_DEFS(markers_doc))) ;
                 child != NULL;
                 child = SP_OBJECT_NEXT(child) )
            {
                if (sp_repr_attr(SP_OBJECT_REPR(child),"inkscape:stockid") &&
                    !strcmp(sp_repr_attr(repr,"inkscape:stockid"), sp_repr_attr(SP_OBJECT_REPR(child),"inkscape:stockid")) &&
                    SP_IS_MARKER(child))
                    continue; // stock item, dont add to list from current doc
            }
        }


        GtkWidget *i = gtk_menu_item_new();
        gtk_widget_show(i);

        if (sp_repr_attr(repr, "inkscape:stockid"))
            g_object_set_data (G_OBJECT(i), "stockid", (void *) "true");
        else
            g_object_set_data (G_OBJECT(i), "stockid", (void *) "false");

        const gchar *markid = sp_repr_attr (repr, "id");
        g_object_set_data (G_OBJECT(i), "marker", (void *) markid);

        GtkWidget *hb = gtk_hbox_new(FALSE, MARKER_ITEM_MARGIN);
        gtk_widget_show(hb);

        // generate preview
        GtkWidget *prv = sp_marker_prev_new (22, markid, source, sandbox, menu_id, arena, visionkey, root);
        gtk_widget_show(prv);
        gtk_box_pack_start(GTK_BOX(hb), prv, FALSE, FALSE, 6);

        // create label
        GtkWidget *l = gtk_label_new(sp_repr_attr(repr, "id"));
        gtk_widget_show(l);
        gtk_misc_set_alignment(GTK_MISC(l), 0.0, 0.5);

        gtk_box_pack_start(GTK_BOX(hb), l, TRUE, TRUE, 0);

        gtk_widget_show(hb);
        gtk_container_add(GTK_CONTAINER(i), hb);

        gtk_menu_append(GTK_MENU(m), i);
    }

    g_slist_free (ml);
}


SPDocument *
ink_markers_preview_doc ()
{
const gchar *buffer = "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
"  <defs id=\"defs\" />"

"  <g id=\"marker-start\">"
"    <path style=\"fill:none;stroke:black;stroke-width:1.7;marker-start:url(#sample);marker-mid:none;marker-end:none;\""
"       d=\"M 12.5,13 L 25,13\" id=\"path1\" />"
"    <rect style=\"fill:none;stroke:none;\" id=\"rect2\""
"       width=\"25\" height=\"25\" x=\"0\" y=\"0\" />"
"  </g>"

"  <g id=\"marker-mid\">"
"    <path style=\"fill:none;stroke:black;stroke-width:1.7;marker-start:none;marker-mid:url(#sample);marker-end:none;\""
"       d=\"M 0,113 L 12.5,113 L 25,113\" id=\"path11\" />"
"    <rect style=\"fill:none;stroke:none;\" id=\"rect22\""
"       width=\"25\" height=\"25\" x=\"0\" y=\"100\" />"
"  </g>"

"  <g id=\"marker-end\">"
"    <path style=\"fill:none;stroke:black;stroke-width:1.7;marker-start:none;marker-mid:none;marker-end:url(#sample);\""
"       d=\"M 0,213 L 12.5,213\" id=\"path111\" />"
"    <rect style=\"fill:none;stroke:none;\" id=\"rect222\""
"       width=\"25\" height=\"25\" x=\"0\" y=\"200\" />"
"  </g>"

"</svg>";

    return sp_document_new_from_mem (buffer, strlen(buffer), FALSE, FALSE);
}



static GtkWidget *
ink_marker_menu( GtkWidget *tbl, gchar *menu_id, SPDocument *sandbox)
{
    SPDesktop *desktop = inkscape_active_desktop();
    SPDocument *doc = SP_DT_DOCUMENT(desktop);
    GtkWidget *mnu = gtk_option_menu_new();

    /* Create new menu widget */
    GtkWidget *m = gtk_menu_new();
    gtk_widget_show(m);

    g_object_set_data(G_OBJECT(mnu), "updating", (gpointer) FALSE);

    if (!doc) {
        GtkWidget *i = gtk_menu_item_new_with_label(_("No document selected"));
        gtk_widget_show(i);
        gtk_menu_append(GTK_MENU(m), i);
        gtk_widget_set_sensitive(mnu, FALSE);

    } else {

        // add "None"
        {
            GtkWidget *i = gtk_menu_item_new();
            gtk_widget_show(i);

            g_object_set_data(G_OBJECT(i), "marker", (void *) "none");

            GtkWidget *hb = gtk_hbox_new(FALSE,  MARKER_ITEM_MARGIN);
            gtk_widget_show(hb);

            GtkWidget *l = gtk_label_new( _("None") );
            gtk_widget_show(l);
            gtk_misc_set_alignment(GTK_MISC(l), 0.0, 0.5);

            gtk_box_pack_start(GTK_BOX(hb), l, TRUE, TRUE, 0);

            gtk_widget_show(hb);
            gtk_container_add(GTK_CONTAINER(i), hb);
            gtk_menu_append(GTK_MENU(m), i);
        }

        // find and load  markers.svg
        static SPDocument *markers_doc = NULL;
        char *markers_source = g_build_filename(INKSCAPE_MARKERSDIR, "/markers.svg", NULL);
        if (g_file_test (markers_source, G_FILE_TEST_IS_REGULAR)) {
            markers_doc = sp_document_new(markers_source, FALSE, FALSE);
        }
        g_free(markers_source);

        // suck in from current doc
        sp_marker_list_from_doc ( m, NULL, doc, markers_doc, sandbox, menu_id );

        // add separator
        {
            GtkWidget *i = gtk_separator_menu_item_new();
            gtk_widget_show(i);
            gtk_menu_append(GTK_MENU(m), i);
        }

        // suck in from markers.svg
        if (markers_doc) {
            sp_document_ensure_up_to_date(doc);
            sp_marker_list_from_doc ( m, doc, markers_doc, NULL, sandbox, menu_id );
        }

        gtk_widget_set_sensitive(mnu, TRUE);
    }

    gtk_object_set_data(GTK_OBJECT(mnu), "menu_id", menu_id);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(mnu), m);

    /* Set history */
    gtk_option_menu_set_history(GTK_OPTION_MENU(mnu), 0);

    return mnu;
}


/*user selected existing marker from list*/
static void
sp_marker_select(GtkOptionMenu *mnu, GtkWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    SPDesktop *desktop = inkscape_active_desktop();
    SPDocument *doc = SP_DT_DOCUMENT(desktop);
    if (!SP_IS_DOCUMENT(doc)) {
        return;
    }

    /* Get Marker */
    if (!g_object_get_data(G_OBJECT(gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(mnu)))),
                           "marker"))
    {
        return;
    }
    gchar *markid = (gchar *) g_object_get_data(G_OBJECT(gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(mnu)))),
                                                "marker");
    gchar *marker = "";
    if (strcmp(markid, "none")){
       gchar *stockid = (gchar *) g_object_get_data(G_OBJECT(gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(mnu)))),
                                                "stockid");

       gchar *markurn = markid;
       if (!strcmp(stockid,"true")) markurn = g_strconcat("urn:inkscape:marker:",markid,NULL);
       SPObject *mark = get_stock_item(markurn);
       if (mark) {
            SPRepr *repr = SP_OBJECT_REPR(mark);
            marker = g_strconcat("url(#", sp_repr_attr(repr,"id"), ")", NULL);
        }
    } else {
        marker = markid;
    }

    SPCSSAttr *css = sp_repr_css_attr_new();
    gchar *menu_id = (gchar *) g_object_get_data(G_OBJECT(mnu), "menu_id");
    sp_repr_css_set_property(css, menu_id, marker);

    SPSelection *selection = SP_DT_SELECTION(desktop);
    GSList const *items = selection->itemList();
    for (; items != NULL; items = items->next) {
        SPRepr *selrepr = SP_OBJECT_REPR((SPItem *) items->data);
        if (selrepr) {
            sp_repr_css_change_recursive(selrepr, css, "style");
        }
        SP_OBJECT(items->data)->requestModified(SP_OBJECT_MODIFIED_FLAG);
        SP_OBJECT(items->data)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
    }

    sp_repr_css_attr_unref(css);

    sp_document_done(SP_WIDGET_DOCUMENT(spw));
}

/** Determine average stroke width */
gdouble
stroke_average_width (GSList const *objects)
{
    if (g_slist_length ((GSList *) objects) == 0)
        return NR_HUGE;

    gdouble avgwidth = 0.0;
    bool notstroked = true;

    for (GSList const *l = objects; l != NULL; l = l->next) {
        if (!SP_IS_ITEM (l->data))
            continue;

        NR::Matrix i2d = sp_item_i2d_affine (SP_ITEM(l->data));

        SPObject *object = SP_OBJECT(l->data);

        if ( object->style->stroke.type == SP_PAINT_TYPE_NONE ) {
            continue;
        } else {
            notstroked = false;
        }

        avgwidth += SP_OBJECT_STYLE (object)->stroke_width.computed * i2d.expansion();
    }

    if (notstroked)
        return NR_HUGE;

    return avgwidth / g_slist_length ((GSList *) objects);
}

bool
stroke_width_varying (GSList const *objects)
{
    if (g_slist_length ((GSList *) objects) <= 1)
        return false;

    gdouble width = NR_HUGE;

    for (GSList const *l = objects; l != NULL; l = l->next) {

        if (!SP_IS_ITEM (l->data))
            continue;

        SPObject *object = SP_OBJECT(l->data);

        if ( SP_OBJECT_STYLE (object)->stroke.type == SP_PAINT_TYPE_NONE ) {
            continue;
        }

        NR::Matrix i2d = sp_item_i2d_affine (SP_ITEM(l->data));

        if (width == NR_HUGE) {
            width = SP_OBJECT_STYLE (object)->stroke_width.computed * i2d.expansion();
        } else {
            if (fabs (width - SP_OBJECT_STYLE (object)->stroke_width.computed * i2d.expansion()) > 1e-3)
                return true;
        }

    }

    return false;
}

/** Determine average miterlimit */
gdouble
stroke_average_miterlimit (GSList const *objects)
{
    if (g_slist_length ((GSList *)objects) == 0)
        return NR_HUGE;

    gdouble avgml = 0.0;
    bool notstroked = true;

    for (GSList const *l = objects; l != NULL; l = l->next) {
        if (!SP_IS_ITEM (l->data))
            continue;

        SPObject *object = SP_OBJECT(l->data);

        avgml += object->style->stroke_miterlimit.value;

        if ( object->style->stroke.type != SP_PAINT_TYPE_NONE ) {
            notstroked = false;
        }
    }

    if (notstroked)
        return NR_HUGE;

    return avgml / g_slist_length ((GSList *) objects);
}

static gboolean stroke_width_set_unit(SPUnitSelector *,
                                                 SPUnit const *old,
                                                 SPUnit const *new_units,
                                                 GObject *dlg)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    if (!desktop) {
        return FALSE;
    }

    SPSelection *selection = SP_DT_SELECTION (desktop);

    if (selection->isEmpty())
        return FALSE;

    GSList const *objects = selection->itemList();

    if ((old->base == SP_UNIT_ABSOLUTE) &&
       (new_units->base == SP_UNIT_DIMENSIONLESS)) {

        /* Absolute to percentage */
        g_object_set_data (dlg, "update", GUINT_TO_POINTER (TRUE));

        GtkAdjustment *a = GTK_ADJUSTMENT(g_object_get_data (dlg, "width"));
        float w = sp_units_get_points (a->value, old);

        gdouble average = stroke_average_width (objects);

        if (average == NR_HUGE || average == 0)
            return FALSE;

        gtk_adjustment_set_value (a, 100.0 * w / average);

        g_object_set_data (dlg, "update", GUINT_TO_POINTER (FALSE));
        return TRUE;

    } else if ((old->base == SP_UNIT_DIMENSIONLESS) &&
              (new_units->base == SP_UNIT_ABSOLUTE)) {

        /* Percentage to absolute */
        g_object_set_data (dlg, "update", GUINT_TO_POINTER (TRUE));

        GtkAdjustment *a = GTK_ADJUSTMENT(g_object_get_data (dlg, "width"));

        gdouble average = stroke_average_width (objects);

        gtk_adjustment_set_value (a, sp_points_get_units (0.01 * a->value * average, new_units));

        g_object_set_data (dlg, "update", GUINT_TO_POINTER (FALSE));
        return TRUE;
    }

    return FALSE;
}


/**
 * \brief  Creates a new widget for the line stroke style.
 *
 */
GtkWidget *
sp_stroke_style_line_widget_new(void)
{
    GtkWidget *spw, *f, *t, *hb, *sb, *us, *tb, *ds;
    GtkObject *a;

    GtkTooltips *tt = gtk_tooltips_new();

    spw = sp_widget_new_global(INKSCAPE);

    f = gtk_hbox_new (FALSE, 0);
    gtk_widget_show(f);
    gtk_container_add(GTK_CONTAINER(spw), f);

    t = gtk_table_new(3, 6, FALSE);
    gtk_widget_show(t);
    gtk_container_set_border_width(GTK_CONTAINER(t), 4);
    gtk_table_set_row_spacings(GTK_TABLE(t), 4);
    gtk_container_add(GTK_CONTAINER(f), t);
    gtk_object_set_data(GTK_OBJECT(spw), "stroke", t);

    gint i = 0;

    /* Stroke width */
    spw_label(t, _("Width:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    a = gtk_adjustment_new(1.0, 0.0, 1000.0, 0.1, 10.0, 10.0);
    gtk_object_set_data(GTK_OBJECT(spw), "width", a);
    sb = gtk_spin_button_new(GTK_ADJUSTMENT(a), 0.1, 2);
    gtk_tooltips_set_tip(tt, sb, _("Stroke width"), NULL);
    gtk_widget_show(sb);

    sp_dialog_defocus_on_enter(sb);

    gtk_box_pack_start(GTK_BOX(hb), sb, FALSE, FALSE, 0);
    us = sp_unit_selector_new(SP_UNIT_ABSOLUTE);
    sp_unit_selector_add_unit(SP_UNIT_SELECTOR(us), &sp_unit_get_by_id(SP_UNIT_PERCENT), 0);
    g_signal_connect ( G_OBJECT (us), "set_unit", G_CALLBACK (stroke_width_set_unit), spw );
    gtk_widget_show(us);
    sp_unit_selector_add_adjustment( SP_UNIT_SELECTOR(us), GTK_ADJUSTMENT(a) );
    gtk_box_pack_start(GTK_BOX(hb), us, FALSE, FALSE, 0);
    gtk_object_set_data(GTK_OBJECT(spw), "units", us);

    gtk_signal_connect( GTK_OBJECT(a), "value_changed", GTK_SIGNAL_FUNC(sp_stroke_style_width_changed), spw );
    i++;

    /* Join type */
    // TRANSLATORS: The line join style specifies the shape to be used at the
    //  corners of paths. It can be "miter", "round" or "bevel".
    spw_label(t, _("Join:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    tb = NULL;

    char *path = g_build_filename(INKSCAPE_PIXMAPDIR, "/join_miter.xpm", NULL);
    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_JOIN_MITER,
                                path, hb, spw, "join", "miter");
    g_free(path);

    // TRANSLATORS: Miter join: joining lines with a sharp (pointed) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    gtk_tooltips_set_tip(tt, tb, _("Miter join"), NULL);

    path = g_build_filename(INKSCAPE_PIXMAPDIR, "/join_round.xpm", NULL);
    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_JOIN_ROUND,
                                path, hb, spw, "join", "round");
    g_free(path);

    // TRANSLATORS: Round join: joining lines with a rounded corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    gtk_tooltips_set_tip(tt, tb, _("Round join"), NULL);

    path = g_build_filename(INKSCAPE_PIXMAPDIR, "/join_bevel.xpm", NULL);
    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_JOIN_BEVEL,
                                path, hb, spw, "join", "bevel");
    g_free(path);

    // TRANSLATORS: Bevel join: joining lines with a blunted (flattened) corner.
    //  For an example, draw a triangle with a large stroke width and modify the
    //  "Join" option (in the Fill and Stroke dialog).
    gtk_tooltips_set_tip(tt, tb, _("Bevel join"), NULL);

    i++;

    /* Miterlimit  */
    // TRANSLATORS: Miter limit: only for "miter join", this limits the length
    //  of the sharp "spike" when the lines connect at too sharp an angle.
    // When two line segments meet at a sharp angle, a miter join results in a
    //  spike that extends well beyond the connection point. The purpose of the
    //  miter limit is to cut off such spikes (i.e. convert them into bevels)
    //  when they become too long.
    spw_label(t, _("Miter limit:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    a = gtk_adjustment_new(4.0, 0.0, 100.0, 0.1, 10.0, 10.0);
    gtk_object_set_data(GTK_OBJECT(spw), "miterlimit", a);

    sb = gtk_spin_button_new(GTK_ADJUSTMENT(a), 0.1, 2);
    gtk_tooltips_set_tip(tt, sb, _("Maximum length of the miter (in units of stroke width)"), NULL);
    gtk_widget_show(sb);
    gtk_object_set_data(GTK_OBJECT(spw), "miterlimit_sb", sb);
    sp_dialog_defocus_on_enter(sb);

    gtk_box_pack_start(GTK_BOX(hb), sb, FALSE, FALSE, 0);

    gtk_signal_connect( GTK_OBJECT(a), "value_changed",
                        GTK_SIGNAL_FUNC(sp_stroke_style_miterlimit_changed), spw );
    i++;

    /* Cap type */
    // TRANSLATORS: cap type specifies the shape for the ends of lines
    spw_label(t, _("Cap:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    tb = NULL;

    path = g_build_filename(INKSCAPE_PIXMAPDIR, "/cap_butt.xpm", NULL);
    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_CAP_BUTT,
                                path, hb, spw, "cap", "butt");
    g_free(path);

    // TRANSLATORS: Butt cap: the line shape does not extend beyond the end point
    //  of the line; the ends of the line are square
    gtk_tooltips_set_tip(tt, tb, _("Butt cap"), NULL);

    path = g_build_filename(INKSCAPE_PIXMAPDIR, "/cap_round.xpm", NULL);
    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_CAP_ROUND,
                                path, hb, spw, "cap", "round");
    g_free(path);

    // TRANSLATORS: Round cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are rounded
    gtk_tooltips_set_tip(tt, tb, _("Round cap"), NULL);

    path = g_build_filename(INKSCAPE_PIXMAPDIR, "/cap_square.xpm", NULL);
    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_CAP_SQUARE,
                                path, hb, spw, "cap", "square");
    g_free(path);

    // TRANSLATORS: Square cap: the line shape extends beyond the end point of the
    //  line; the ends of the line are square
    gtk_tooltips_set_tip(tt, tb, _("Square cap"), NULL);

    i++;


    /* Dash */
    spw_label(t, _("Dashes:"), 0, i);
    ds = sp_dash_selector_new( inkscape_get_repr( INKSCAPE,
                                                  "palette.dashes") );

    gtk_widget_show(ds);
    gtk_table_attach( GTK_TABLE(t), ds, 1, 4, i, i+1,
                      (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                      (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data(GTK_OBJECT(spw), "dash", ds);
    gtk_signal_connect( GTK_OBJECT(ds), "changed",
                        GTK_SIGNAL_FUNC(sp_stroke_style_line_dash_changed),
                        spw );
    i++;

    /* Drop down marker selectors*/

    // doing this here once, instead of for each preview, to speed things up
    SPDocument *sandbox = ink_markers_preview_doc ();

    // TRANSLATORS: Path markers are an SVG feature that allows you to attach arbitrary shapes
    // (arrowheads, bullets, faces, whatever) to the start, end, or middle nodes of a path.
    spw_label(t, _("Start Markers:"), 0, i);
    GtkWidget *mnu  = ink_marker_menu( spw ,"marker-start", sandbox);
    gtk_signal_connect( GTK_OBJECT(mnu), "changed", GTK_SIGNAL_FUNC(sp_marker_select), spw );
    gtk_widget_show(mnu);
    gtk_table_attach( GTK_TABLE(t), mnu, 1, 4, i, i+1,
                      (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                      (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data(GTK_OBJECT(spw), "start_mark_menu", mnu);

    i++;
    spw_label(t, _("Mid Markers:"), 0, i);
    mnu = NULL;
    mnu  = ink_marker_menu( spw ,"marker-mid", sandbox);
    gtk_signal_connect( GTK_OBJECT(mnu), "changed", GTK_SIGNAL_FUNC(sp_marker_select), spw );
    gtk_widget_show(mnu);
    gtk_table_attach( GTK_TABLE(t), mnu, 1, 4, i, i+1,
                      (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                      (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data(GTK_OBJECT(spw), "mid_mark_menu", mnu);

    i++;
    spw_label(t, _("End Markers:"), 0, i);
    mnu = NULL;
    mnu  = ink_marker_menu( spw ,"marker-end", sandbox);
    gtk_signal_connect( GTK_OBJECT(mnu), "changed", GTK_SIGNAL_FUNC(sp_marker_select), spw );
    gtk_widget_show(mnu);
    gtk_table_attach( GTK_TABLE(t), mnu, 1, 4, i, i+1,
                      (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                      (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data(GTK_OBJECT(spw), "end_mark_menu", mnu);

    i++;



    /* General (I think) style dialog signals */
    gtk_signal_connect( GTK_OBJECT(spw), "construct",
                        GTK_SIGNAL_FUNC(sp_stroke_style_line_construct),
                        NULL );
    gtk_signal_connect( GTK_OBJECT(spw), "modify_selection",
                        GTK_SIGNAL_FUNC(sp_stroke_style_line_selection_modified),
                        NULL );
    gtk_signal_connect( GTK_OBJECT(spw), "change_selection",
                        GTK_SIGNAL_FUNC(sp_stroke_style_line_selection_changed),
                        NULL );
    gtk_signal_connect( GTK_OBJECT(spw), "attr_changed",
                        GTK_SIGNAL_FUNC(sp_stroke_style_line_attr_changed),
                        NULL );

    SPDesktop *desktop = inkscape_active_desktop();
    sp_stroke_style_line_update( SP_WIDGET(spw), desktop ? SP_DT_SELECTION(desktop) : NULL);

    return spw;

}



static void
sp_stroke_style_line_construct(SPWidget *spw, gpointer data)
{

#ifdef SP_SS_VERBOSE
    g_print( "Stroke style widget constructed: inkscape %p repr %p\n",
             spw->inkscape, spw->repr );
#endif
    if (spw->inkscape) {
        sp_stroke_style_line_update(spw,
                                    ( SP_ACTIVE_DESKTOP
                                      ? SP_DT_SELECTION(SP_ACTIVE_DESKTOP)
                                      : NULL ));
    } else if (spw->repr) {
        sp_stroke_style_line_update_repr(spw, spw->repr);
    }
}



static void
sp_stroke_style_line_selection_modified ( SPWidget *spw,
                                       SPSelection *selection,
                                       guint flags,
                                       gpointer data )
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG)) {
        sp_stroke_style_line_update (spw, selection);
    }

}



static void
sp_stroke_style_line_selection_changed ( SPWidget *spw,
                                       SPSelection *selection,
                                       gpointer data )
{
    sp_stroke_style_line_update (spw, selection);
}



static void
sp_stroke_style_line_attr_changed(SPWidget *spw,
                                  gchar const *key,
                                  gchar const *oldval,
                                  gchar const *newval)
{
    if (!strcmp(key, "style")) {
        /* This sounds interesting. */
        sp_stroke_style_line_update_repr(spw, spw->repr);
    }
}

static void
sp_dash_selector_set_from_style (GtkWidget *dsel, SPStyle *style)
{
    if (style->stroke_dash.n_dash > 0) {
        double d[64];
        int len = MIN(style->stroke_dash.n_dash, 64);
        for (int i = 0; i < len; i++) {
            if (style->stroke_width.computed != 0)
                d[i] = style->stroke_dash.dash[i] / style->stroke_width.computed;
            else
                d[i] = style->stroke_dash.dash[i]; // is there a better thing to do for stroke_width==0?
        }
        sp_dash_selector_set_dash(SP_DASH_SELECTOR(dsel), len, d,
               style->stroke_width.computed != 0?
                    style->stroke_dash.offset / style->stroke_width.computed  :
                    style->stroke_dash.offset);
    } else {
        sp_dash_selector_set_dash(SP_DASH_SELECTOR(dsel), 0, NULL, 0.0);
    }
}

static void
sp_jointype_set (SPWidget *spw, unsigned const jointype)
{
    GtkWidget *tb = NULL;
    switch (jointype) {
        case SP_STROKE_LINEJOIN_MITER:
            tb = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), INKSCAPE_STOCK_JOIN_MITER));
            break;
        case SP_STROKE_LINEJOIN_ROUND:
            tb = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), INKSCAPE_STOCK_JOIN_ROUND));
            break;
        case SP_STROKE_LINEJOIN_BEVEL:
            tb = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), INKSCAPE_STOCK_JOIN_BEVEL));
            break;
        default:
            break;
    }
    sp_stroke_style_set_join_buttons (spw, tb);
}

static void
sp_captype_set (SPWidget *spw, unsigned const captype)
{
    GtkWidget *tb = NULL;
    switch (captype) {
        case SP_STROKE_LINECAP_BUTT:
            tb = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), INKSCAPE_STOCK_CAP_BUTT));
            break;
        case SP_STROKE_LINECAP_ROUND:
            tb = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), INKSCAPE_STOCK_CAP_ROUND));
            break;
        case SP_STROKE_LINECAP_SQUARE:
            tb = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), INKSCAPE_STOCK_CAP_SQUARE));
            break;
        default:
            break;
    }
    sp_stroke_style_set_cap_buttons (spw, tb);
}

static void
sp_stroke_style_line_update(SPWidget *spw, SPSelection *sel)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    GtkWidget *sset = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "stroke"));
    GtkObject *width = GTK_OBJECT(gtk_object_get_data(GTK_OBJECT(spw), "width"));
    GtkObject *ml = GTK_OBJECT(gtk_object_get_data(GTK_OBJECT(spw), "miterlimit"));
    GtkWidget *us = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "units"));
    GtkWidget *dsel = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "dash"));

    if ( !sel || sel->isEmpty() ) {
        /* No objects, set empty */
        gtk_widget_set_sensitive(sset, FALSE);
        gtk_object_set_data( GTK_OBJECT(spw), "update",
                             GINT_TO_POINTER(FALSE) );
        return;
    }

    GSList const *objects = sel->itemList();

    gdouble avgwidth = stroke_average_width (objects);
    gdouble avgml = stroke_average_miterlimit (objects);

    if (avgwidth != NR_HUGE) {
        gtk_widget_set_sensitive(sset, TRUE);
    } else {
        /* No objects stroked, set insensitive */
        gtk_widget_set_sensitive(sset, FALSE);

        gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
        return;
    }

    const SPUnit *unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (us));

    if (stroke_width_varying (objects)) {
        sp_unit_selector_set_unit(SP_UNIT_SELECTOR(us), &sp_unit_get_by_id(SP_UNIT_PERCENT));
    } else {
        // only one object; no sense to keep percent, switch to absolute
        if (unit->base != SP_UNIT_ABSOLUTE) {
            // FIXME: use some other default absolute unit
            sp_unit_selector_set_unit(SP_UNIT_SELECTOR(us), &sp_unit_get_by_id(SP_UNIT_PT));
        }
    }

    unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (us));

    if (unit->base == SP_UNIT_ABSOLUTE) {
        sp_convert_distance(&avgwidth, SP_PS_UNIT, sp_unit_selector_get_unit(SP_UNIT_SELECTOR(us)));
        gtk_adjustment_set_value(GTK_ADJUSTMENT(width), avgwidth);
    } else {
        gtk_adjustment_set_value(GTK_ADJUSTMENT(width), 100);
    }

    gtk_adjustment_set_value(GTK_ADJUSTMENT(ml), avgml);

    /* Join & Cap */
    SPObject * const object = SP_OBJECT(objects->data);
    SPStyle * const style = SP_OBJECT_STYLE(object);
    unsigned const jointype = object->style->stroke_linejoin.value;
    unsigned const captype = object->style->stroke_linecap.value;

    bool joinValid = true;
    bool capValid = true;
    for (GSList const *l = objects->next; l != NULL; l = l->next) {
        SPObject *o = SP_OBJECT(l->data);
        if ( o->style->stroke_linejoin.value != jointype ) {
            joinValid = false;
        }
        if ( o->style->stroke_linecap.value != captype ) {
            capValid = false;
        }
    }

    if (joinValid) {
        sp_jointype_set (spw, jointype);
    } else {
        sp_stroke_style_set_join_buttons(spw, NULL);
    }

    if (capValid) {
        sp_captype_set (spw, captype);
    } else {
        sp_stroke_style_set_cap_buttons(spw, NULL);
    }

    /* Markers */
    sp_stroke_style_update_marker_menus(spw, objects);

    /* Dash */
    sp_dash_selector_set_from_style (dsel, style);

    gtk_widget_set_sensitive(sset, TRUE);

    gtk_object_set_data(GTK_OBJECT(spw), "update",
                        GINT_TO_POINTER(FALSE));
}


/**
 * \brief  This routine updates the GUI widgets from data in the repr for the
 * line styles. It retrieves the current width, units, etc. from the dialog
 * and then pulls in the data from the repr and updates the widgets
 * accordingly.
 *
 */
static void
sp_stroke_style_line_update_repr(SPWidget *spw, SPRepr *repr)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    GtkWidget *sset = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "stroke"));
    GtkObject *width = GTK_OBJECT(gtk_object_get_data(GTK_OBJECT(spw), "width"));
    GtkWidget *units = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "units"));
    GtkWidget *dsel = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), "dash"));

    SPStyle *style = sp_style_new();
    sp_style_read_from_repr(style, repr);

    if (style->stroke.type == SP_PAINT_TYPE_NONE) {
        gtk_widget_set_sensitive(sset, FALSE);
        gtk_object_set_data(GTK_OBJECT(spw), "update",
                            GINT_TO_POINTER(FALSE));
        return;
    }

    /* We need points */
    gdouble swidth = style->stroke_width.computed / 1.25;
    SPUnit const *unit = sp_unit_selector_get_unit(SP_UNIT_SELECTOR(units));
    sp_convert_distance(&swidth, SP_PS_UNIT, unit);
    gtk_adjustment_set_value(GTK_ADJUSTMENT(width), swidth);

    /* Join */
    sp_jointype_set (spw, style->stroke_linejoin.value);

    /* Cap */
    sp_captype_set (spw, style->stroke_linecap.value);

    /* Dash */
    sp_dash_selector_set_from_style (dsel, style);

    gtk_widget_set_sensitive(sset, TRUE);

    sp_style_unref(style);

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));

}



static void
sp_stroke_style_set_scaled_dash(SPCSSAttr *css,
                                int ndash, double *dash, double offset,
                                double scale)
{
    if (ndash > 0) {
        Inkscape::SVGOStringStream osarray;
        for (int i = 0; i < ndash; i++) {
            osarray << dash[i] * scale;
            if (i < (ndash - 1)) {
                osarray << ",";
            }
        }
        sp_repr_css_set_property(css, "stroke-dasharray", osarray.str().c_str());

        Inkscape::SVGOStringStream osoffset;
        osoffset << offset * scale;
        sp_repr_css_set_property(css, "stroke-dashoffset", osoffset.str().c_str());
    } else {
        sp_repr_css_set_property(css, "stroke-dasharray", "none");
        sp_repr_css_set_property(css, "stroke-dashoffset", NULL);
    }
}



static void
sp_stroke_style_scale_line(SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(TRUE));

    GtkAdjustment *wadj = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(spw), "width"));
    SPUnitSelector *us = SP_UNIT_SELECTOR(gtk_object_get_data(GTK_OBJECT(spw), "units"));
    SPDashSelector *dsel = SP_DASH_SELECTOR(gtk_object_get_data(GTK_OBJECT(spw), "dash"));
    GtkAdjustment *ml = GTK_ADJUSTMENT(gtk_object_get_data(GTK_OBJECT(spw), "miterlimit"));

    GSList *reprs;
    GSList const *items;
    if (spw->inkscape) {
        /* TODO: */
        if (!SP_WIDGET_DOCUMENT(spw))
            return;

        reprs = NULL;
        items = sp_widget_get_item_list(spw);
        for (GSList const *i = items; i != NULL; i = i->next) {
            reprs = g_slist_prepend(reprs, SP_OBJECT_REPR(i->data));
        }
    } else {
        reprs = g_slist_prepend(NULL, spw->repr);
        items = NULL;
    }

    /* TODO: Create some standardized method */
    SPCSSAttr *css = sp_repr_css_attr_new();

    if (items) {

        double width_typed = wadj->value;
        double const miterlimit = ml->value;

        const SPUnit *unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (us));

        double *dash, offset;
        int ndash;
        sp_dash_selector_get_dash(dsel, &ndash, &dash, &offset);

        for (GSList const *i = items; i != NULL; i = i->next) {
            /* Set stroke width */
            double width;
            if (unit->base == SP_UNIT_ABSOLUTE) {
                sp_convert_distance( &width_typed, sp_unit_selector_get_unit(us), SP_PS_UNIT );
                NR::Matrix i2d = sp_item_i2d_affine (SP_ITEM(i->data));
                width = width_typed / expansion(i2d);
            } else { // percentage
                gdouble old_w = SP_OBJECT_STYLE (i->data)->stroke_width.computed;
                width = old_w * width_typed / 100;
            }

            {
                Inkscape::SVGOStringStream os_width;
                os_width << width;
                sp_repr_css_set_property(css, "stroke-width", os_width.str().c_str());
            }

            {
                Inkscape::SVGOStringStream os_ml;
                os_ml << miterlimit;
                sp_repr_css_set_property(css, "stroke-miterlimit", os_ml.str().c_str());
            }

            /* Set dash */
            sp_stroke_style_set_scaled_dash(css, ndash, dash, offset, width);

            sp_repr_css_change_recursive(SP_OBJECT_REPR(i->data), css, "style");
        }

        g_free(dash);

        if (unit->base != SP_UNIT_ABSOLUTE) {
            // reset to 100 percent
            gtk_adjustment_set_value (wadj, 100.0);
        }

    } else {

         // FIXME: seems like this reprs list can only contain one repr, that of the selected tool's pref when you "apply to" some tool.
         // That stupidity will be eliminated when we have current style in SPDesktop.
        for (GSList const *r = reprs; r != NULL; r = r->next) {
            double length = wadj->value;
            double const miterlimit = ml->value;
            double *dash, offset;
            int ndash;
            sp_dash_selector_get_dash(dsel, &ndash, &dash, &offset);

            sp_convert_distance( &length, sp_unit_selector_get_unit(us),
                                 SP_PS_UNIT );

            /* Set stroke width. */
            {
                Inkscape::SVGOStringStream os_width;
                os_width << length * 1.25;
                sp_repr_css_set_property(css, "stroke-width", os_width.str().c_str());
            }

            /* Set stroke miterlimit. */
            {
                Inkscape::SVGOStringStream os_ml;
                os_ml << miterlimit;
                sp_repr_css_set_property(css, "stroke-miterlimit", os_ml.str().c_str());
            }

            sp_stroke_style_set_scaled_dash(css, ndash, dash, offset, length);

            sp_repr_css_change_recursive((SPRepr *) r->data, css, "style");
            g_free(dash);
        }
    }

    sp_repr_css_attr_unref(css);

    if (spw->inkscape) {
        sp_document_done(SP_WIDGET_DOCUMENT(spw));
    }

    g_slist_free(reprs);

    gtk_object_set_data(GTK_OBJECT(spw), "update", GINT_TO_POINTER(FALSE));
}



static void
sp_stroke_style_width_changed(GtkAdjustment *adj, SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    sp_stroke_style_scale_line(spw);
}

static void
sp_stroke_style_miterlimit_changed(GtkAdjustment *adj, SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    sp_stroke_style_scale_line(spw);
}

static void
sp_stroke_style_line_dash_changed(SPDashSelector *dsel, SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    sp_stroke_style_scale_line(spw);
}



/**
 * \brief  This routine handles toggle events for buttons in the stroke style
 *         dialog.
 * When activated, this routine gets the data for the various widgets, and then
 * calls the respective routines to update css properties, etc.
 *
 */
static void
sp_stroke_style_any_toggled(GtkToggleButton *tb, SPWidget *spw)
{
    if (gtk_object_get_data(GTK_OBJECT(spw), "update")) {
        return;
    }

    if (gtk_toggle_button_get_active(tb)) {
        /* XXX: It looks wrong that we assign to items here and then
           never use it: it seems that we unconditionally overwrite it
           in the if-then-else below. */
        GSList const *items = sp_widget_get_item_list(spw);
        gchar const *join
            = static_cast<gchar const *>(gtk_object_get_data(GTK_OBJECT(tb),
                                                             "join"));
        gchar const *cap
            = static_cast<gchar const *>(gtk_object_get_data(GTK_OBJECT(tb),
                                                             "cap"));
        if (join) {
            GtkWidget *ml = GTK_WIDGET(g_object_get_data(G_OBJECT(spw), "miterlimit_sb"));
            gtk_widget_set_sensitive(ml, !strcmp(join, "miter"));
        }

        GSList *reprs;
        if (spw->inkscape) {
            reprs = NULL;
            items = sp_widget_get_item_list(spw);
            for (GSList const *i = items; i != NULL; i = i->next) {
                reprs = g_slist_prepend(reprs, SP_OBJECT_REPR(i->data));
            }
        } else {
            reprs = g_slist_prepend(NULL, spw->repr);
            items = NULL;
        }

        /* TODO: Create some standardized method */
        SPCSSAttr *css = sp_repr_css_attr_new();

        if (join) {
            sp_repr_css_set_property(css, "stroke-linejoin", join);
            for (GSList const *r = reprs; r != NULL; r = r->next) {
                sp_repr_css_change_recursive((SPRepr *) r->data, css, "style");
            }
            sp_stroke_style_set_join_buttons(spw, GTK_WIDGET(tb));
        } else if (cap) {
            sp_repr_css_set_property(css, "stroke-linecap", cap);
            for (GSList const *r = reprs; r != NULL; r = r->next) {
                sp_repr_css_change_recursive((SPRepr *) r->data, css, "style");
            }
            sp_stroke_style_set_cap_buttons(spw, GTK_WIDGET(tb));
        }

        sp_repr_css_attr_unref(css);

        if (spw->inkscape) {
            sp_document_done(SP_WIDGET_DOCUMENT(spw));
        }

        g_slist_free(reprs);
    }

}



/* Helpers */



static void
sp_stroke_style_get_average_color_rgba(GSList const *objects, gfloat c[4])
{
    c[0] = 0.0;
    c[1] = 0.0;
    c[2] = 0.0;
    c[3] = 0.0;
    gint num = 0;
    while (objects) {
        gfloat d[3];
        SPObject *object = SP_OBJECT(objects->data);
        if (object->style->stroke.type == SP_PAINT_TYPE_COLOR) {
            sp_color_get_rgb_floatv(&object->style->stroke.value.color, d);
            c[0] += d[0];
            c[1] += d[1];
            c[2] += d[2];
            c[3] += SP_SCALE24_TO_FLOAT(object->style->stroke_opacity.value);
        }
        num += 1;
        objects = objects->next;
    }

    c[0] /= num;
    c[1] /= num;
    c[2] /= num;
    c[3] /= num;

} // end of sp_stroke_style_get_average_color_rgba()



static void
sp_stroke_style_get_average_color_cmyka(GSList const *objects, gfloat c[5])
{
    c[0] = 0.0;
    c[1] = 0.0;
    c[2] = 0.0;
    c[3] = 0.0;
    c[4] = 0.0;
    gint num = 0;
    while (objects) {
        gfloat d[4];
        SPObject *object = SP_OBJECT(objects->data);
        if (object->style->stroke.type == SP_PAINT_TYPE_COLOR) {
            sp_color_get_cmyk_floatv(&object->style->stroke.value.color, d);
            c[0] += d[0];
            c[1] += d[1];
            c[2] += d[2];
            c[3] += d[3];
            c[4] += SP_SCALE24_TO_FLOAT(object->style->stroke_opacity.value);
        }
        num += 1;
        objects = objects->next;
    }

    c[0] /= num;
    c[1] /= num;
    c[2] /= num;
    c[3] /= num;
    c[4] /= num;

} // end of sp_stroke_style_get_average_color_cmyka()



static SPPaintSelectorMode
sp_stroke_style_determine_paint_selector_mode(SPStyle *style)
{
    SPSelection *sel = SP_DT_SELECTION (SP_ACTIVE_DESKTOP);
    // prevent change of style on clones.
    for (GSList const *l = sel->itemList(); l != NULL; l = l->next) {
          if (SP_IS_USE(l->data)) {
               return SP_PAINT_SELECTOR_MODE_CLONE;
          }
    }

    switch (style->stroke.type) {
        case SP_PAINT_TYPE_NONE:
            return SP_PAINT_SELECTOR_MODE_NONE;
        case SP_PAINT_TYPE_COLOR:
        {
            SPColorSpaceType cstype = sp_color_get_colorspace_type(&style->stroke.value.color);
            switch (cstype) {
                case SP_COLORSPACE_TYPE_RGB:
                    return SP_PAINT_SELECTOR_MODE_COLOR_RGB;
                case SP_COLORSPACE_TYPE_CMYK:
                    return SP_PAINT_SELECTOR_MODE_COLOR_CMYK;
                default:
                    g_warning( "file %s: line %d: Unknown colorspace type %d",
                               __FILE__, __LINE__, cstype );
                    return SP_PAINT_SELECTOR_MODE_NONE;
            }
        }

        case SP_PAINT_TYPE_PAINTSERVER:
            if (SP_IS_LINEARGRADIENT(SP_STYLE_STROKE_SERVER(style))) {
                return SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR;
            } else if (SP_IS_RADIALGRADIENT(SP_STYLE_STROKE_SERVER(style))) {
                return SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL;
            } else if (SP_IS_PATTERN(SP_STYLE_STROKE_SERVER(style))) {
                return SP_PAINT_SELECTOR_MODE_PATTERN;
            }
            return SP_PAINT_SELECTOR_MODE_NONE;

        default:
            g_warning( "file %s: line %d: Unknown paint type %d",
                       __FILE__, __LINE__, style->stroke.type );
            break;
    }

    return SP_PAINT_SELECTOR_MODE_NONE;


} // end of sp_stroke_style_determine_paint_selector_mode()



static void
sp_stroke_style_set_join_buttons(SPWidget *spw, GtkWidget *active)
{
    GtkWidget *tb;

    tb = GTK_WIDGET(gtk_object_get_data( GTK_OBJECT(spw),
                                         INKSCAPE_STOCK_JOIN_MITER) );
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), (active == tb));

    GtkWidget *ml = GTK_WIDGET(g_object_get_data(G_OBJECT(spw), "miterlimit_sb"));
    gtk_widget_set_sensitive(ml, (active == tb));

    tb = GTK_WIDGET(gtk_object_get_data( GTK_OBJECT(spw),
                                         INKSCAPE_STOCK_JOIN_ROUND) );
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), (active == tb));
    tb = GTK_WIDGET(gtk_object_get_data( GTK_OBJECT(spw),
                                         INKSCAPE_STOCK_JOIN_BEVEL) );
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), (active == tb));
}



static void
sp_stroke_style_set_cap_buttons(SPWidget *spw, GtkWidget *active)
{
    GtkWidget *tb;

    tb = GTK_WIDGET(gtk_object_get_data( GTK_OBJECT(spw),
                                         INKSCAPE_STOCK_CAP_BUTT));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), (active == tb));
    tb = GTK_WIDGET(gtk_object_get_data( GTK_OBJECT(spw),
                                         INKSCAPE_STOCK_CAP_ROUND) );
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), (active == tb));
    tb = GTK_WIDGET(gtk_object_get_data( GTK_OBJECT(spw),
                                         INKSCAPE_STOCK_CAP_SQUARE) );
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), (active == tb));
}

static void
ink_marker_menu_set_current(SPObject *marker, GtkOptionMenu *mnu)
{
    gtk_object_set_data(GTK_OBJECT(mnu), "update", GINT_TO_POINTER(TRUE));

    GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu(mnu));
    if (marker != NULL) {
        bool mark_is_stock = false;
        if (sp_repr_attr(SP_OBJECT_REPR(marker), "inkscape:stockid"))
            mark_is_stock = true;

        gchar *markname;
        if (mark_is_stock)
            markname = g_strdup(sp_repr_attr(SP_OBJECT_REPR(marker), "inkscape:stockid"));
        else
            markname = g_strdup(sp_repr_attr(SP_OBJECT_REPR(marker), "id"));

        int markpos = 0;
        GList *kids = GTK_MENU_SHELL(m)->children;
        int i = 0;
        for (; kids != NULL; kids = kids->next) {
            gchar *mark = (gchar *) g_object_get_data(G_OBJECT(kids->data), "marker");
            if ( mark && strcmp(mark, markname) == 0 ) {
                if ( mark_is_stock && !strcmp((gchar *) g_object_get_data(G_OBJECT(kids->data), "stockid"), "true"))
                    markpos = i;
                if ( !mark_is_stock && !strcmp((gchar *) g_object_get_data(G_OBJECT(kids->data), "stockid"), "false"))
                    markpos = i;
            }
            i++;
        }
        gtk_option_menu_set_history(GTK_OPTION_MENU(mnu), markpos);

        g_free (markname);
    }
    else {
        gtk_option_menu_set_history(GTK_OPTION_MENU(mnu), 0);
    }
    gtk_object_set_data(GTK_OBJECT(mnu), "update", GINT_TO_POINTER(FALSE));
}

static void
sp_stroke_style_update_marker_menus( SPWidget *spw,
                                     GSList const *objects)
{
    struct { char const *key; int loc; } const keyloc[] = {
        { "start_mark_menu", SP_MARKER_LOC_START },
        { "mid_mark_menu", SP_MARKER_LOC_MID },
        { "end_mark_menu", SP_MARKER_LOC_END }
    };

    bool all_texts = true;
    for (GSList *i = (GSList *) objects; i != NULL; i = i->next) {
        if (!SP_IS_TEXT (i->data)) {
            all_texts = false;
        }
    }

    for (unsigned i = 0; i < G_N_ELEMENTS(keyloc); ++i) {
        GtkOptionMenu *mnu = (GtkOptionMenu *) g_object_get_data(G_OBJECT(spw), keyloc[i].key);
        if (all_texts) {
            // Per SVG spec, text objects cannot have markers; disable menus if only texts are selected
            gtk_widget_set_sensitive (GTK_WIDGET(mnu), FALSE);
        } else {
            gtk_widget_set_sensitive (GTK_WIDGET(mnu), TRUE);
        }
    }

    // We show markers of the first object in the list only
    // FIXME: use the first in the list that has the marker of each type, if any
    SPObject *object = SP_OBJECT(objects->data);

    for (unsigned i = 0; i < G_N_ELEMENTS(keyloc); ++i) {
        // For all three marker types,

        // find the corresponding menu
        GtkOptionMenu *mnu = (GtkOptionMenu *) g_object_get_data(G_OBJECT(spw), keyloc[i].key);

        // Quit if we're in update state
        if (gtk_object_get_data(GTK_OBJECT(mnu), "update")) {
            return;
        }

        if (object->style->marker[keyloc[i].loc].value != NULL && !all_texts) {
            // If the object has this type of markers,

            // Extract the name of the marker that the object uses
            SPObject *marker = ink_extract_marker_name(object->style->marker[keyloc[i].loc].value);
            // Scroll the menu to that marker
            ink_marker_menu_set_current (marker, mnu);

        } else {
            gtk_option_menu_set_history(GTK_OPTION_MENU(mnu), 0);
        }
    }
}


/** Extract the actual name of the link
 * e.g. get mTriangle from url(#mTriangle).
 * \return Buffer containing the actual name, allocated from GLib;
 * the caller should free the buffer when they no longer need it.
 */
static SPObject*
ink_extract_marker_name(gchar const *n)
{

    gchar const *p = n;
    while (*p != '\0' && *p != '#') {
        p++;
    }

    if (*p == '\0' || p[1] == '\0') {
        return NULL;
    }

    p++;
    int c = 0;
    while (p[c] != '\0' && p[c] != ')') {
        c++;
    }

    if (p[c] == '\0') {
        return NULL;
    }

    gchar* b = g_strdup(p);
    b[c] = '\0';


    SPDesktop *desktop = inkscape_active_desktop();
    SPDocument *doc = SP_DT_DOCUMENT(desktop);
    SPObject *marker = doc->getObjectById(b);
    return marker;
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
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
