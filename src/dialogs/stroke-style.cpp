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

#include <config.h>

#include <string.h>
#include <glib.h>

#include <libnr/nr-values.h>
#include <libnr/nr-matrix.h>

#include <gtk/gtk.h>

#include "helper/sp-intl.h"
#include "helper/unit-menu.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "widgets/sp-widget.h"
#include "widgets/spw-utilities.h"
#include "sp-gradient.h"
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
#include "document-private.h"
#include "xml/repr-private.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"
#include "widgets/icon.h"

#include "dialogs/stroke-style.h"

/* Paint */

static void sp_stroke_style_paint_construct (SPWidget *spw, SPPaintSelector *psel);
static void sp_stroke_style_paint_modify_selection (SPWidget *spw, SPSelection *selection, guint flags, SPPaintSelector *psel);
static void sp_stroke_style_paint_change_selection (SPWidget *spw, SPSelection *selection, SPPaintSelector *psel);
static void sp_stroke_style_paint_attr_changed (SPWidget *spw, const gchar *key, const gchar *oldval, const gchar *newval);
static void sp_stroke_style_paint_update (SPWidget *spw, SPSelection *sel);
static void sp_stroke_style_paint_update_repr (SPWidget *spw, SPRepr *repr);

static void sp_stroke_style_paint_mode_changed (SPPaintSelector *psel, SPPaintSelectorMode mode, SPWidget *spw);
static void sp_stroke_style_paint_dragged (SPPaintSelector *psel, SPWidget *spw);
static void sp_stroke_style_paint_changed (SPPaintSelector *psel, SPWidget *spw);

static void sp_stroke_style_get_average_color_rgba (const GSList *objects, gfloat *c);
static void sp_stroke_style_get_average_color_cmyka (const GSList *objects, gfloat *c);
static SPPaintSelectorMode sp_stroke_style_determine_paint_selector_mode (SPStyle *style);



GtkWidget *
sp_stroke_style_paint_widget_new (void)
{
    GtkWidget *spw, *psel;

    spw = sp_widget_new_global (INKSCAPE);

    psel = sp_paint_selector_new ();
    gtk_widget_show (psel);
    gtk_container_add (GTK_CONTAINER (spw), psel);
    gtk_object_set_data (GTK_OBJECT (spw), "paint-selector", psel);

    gtk_signal_connect ( GTK_OBJECT (spw), "construct", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_paint_construct), 
                         psel );
    gtk_signal_connect ( GTK_OBJECT (spw), "modify_selection", 
                         GTK_SIGNAL_FUNC 
                             (sp_stroke_style_paint_modify_selection), 
                         psel );
    gtk_signal_connect ( GTK_OBJECT (spw), "change_selection", 
                         GTK_SIGNAL_FUNC 
                             (sp_stroke_style_paint_change_selection), 
                         psel );
    gtk_signal_connect ( GTK_OBJECT (spw), "attr_changed", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_paint_attr_changed), 
                         psel );

    gtk_signal_connect ( GTK_OBJECT (psel), "mode_changed", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_paint_mode_changed), 
                         spw );
    gtk_signal_connect ( GTK_OBJECT (psel), "dragged", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_paint_dragged), 
                         spw );
    gtk_signal_connect ( GTK_OBJECT (psel), "changed", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_paint_changed), 
                         spw );

    sp_stroke_style_paint_update ( SP_WIDGET (spw), 
            SP_ACTIVE_DESKTOP ? SP_DT_SELECTION (SP_ACTIVE_DESKTOP) : NULL );

    return spw;
    
} // end of sp_stroke_style_paint_widget_new()



void
sp_stroke_style_paint_system_color_set (GtkWidget *widget, SPColor *color, float opacity)
{
    SPPaintSelector *psel;

    psel = SP_PAINT_SELECTOR( g_object_get_data (G_OBJECT (widget), 
                              "paint-selector") );

    switch (psel->mode) {
    case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
    case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        sp_paint_selector_system_color_set (psel, color, opacity);
        break;
    default:
        break;
    }
}



static void
sp_stroke_style_paint_construct (SPWidget *spw, SPPaintSelector *psel)
{
#ifdef SP_SS_VERBOSE
    g_print ( "Stroke style widget constructed: inkscape %p repr %p\n", 
              spw->inkscape, spw->repr );
#endif
    if (spw->inkscape) {
        sp_stroke_style_paint_update ( spw, 
                SP_ACTIVE_DESKTOP ? SP_DT_SELECTION (SP_ACTIVE_DESKTOP) : NULL);
    } else if (spw->repr) {
        sp_stroke_style_paint_update_repr (spw, spw->repr);
    }
}



static void
sp_stroke_style_paint_modify_selection ( SPWidget *spw, 
                                         SPSelection *selection, 
                                         guint flags, 
                                         SPPaintSelector *psel)
{
    if (flags & ( SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG | 
                  SP_OBJECT_STYLE_MODIFIED_FLAG) )
    {
        sp_stroke_style_paint_update (spw, selection);
    }

} // end of sp_stroke_style_paint_modify_selection()



static void
sp_stroke_style_paint_change_selection ( SPWidget *spw, 
                                         SPSelection *selection, 
                                         SPPaintSelector *psel )
{
    sp_stroke_style_paint_update (spw, selection);
}



static void
sp_stroke_style_paint_attr_changed ( SPWidget *spw, 
                                     const gchar *key, 
                                     const gchar *oldval, 
                                     const gchar *newval )
{
    if (!strcmp (key, "style")) {
        /* This sounds interesting */
        sp_stroke_style_paint_update_repr (spw, spw->repr);
    }
}



static void
sp_stroke_style_paint_update (SPWidget *spw, SPSelection *sel)
{
    SPPaintSelector *psel;
    SPPaintSelectorMode pselmode;
    const GSList *objects, *l;
    SPObject *object;
    SPGradient *vector;
    SPColor color;
    gfloat c[5];
    SPLinearGradient *lg;
    SPRadialGradient *rg;
    NRMatrix fctm, gs2d;
    NRRect fbb;

    if (gtk_object_get_data (GTK_OBJECT (spw), "update"))
        return;

    gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    psel = SP_PAINT_SELECTOR( gtk_object_get_data (GTK_OBJECT (spw), 
                              "paint-selector") );

    if (!sel || sp_selection_is_empty (sel)) {
        /* No objects, set empty */
        sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_EMPTY);
        gtk_object_set_data ( GTK_OBJECT (spw), "update", 
                              GINT_TO_POINTER (FALSE) );
        return;
    }

    objects = sp_selection_item_list (sel);
    object = SP_OBJECT (objects->data);
    pselmode = 
        sp_stroke_style_determine_paint_selector_mode 
            (SP_OBJECT_STYLE (object));

    for (l = objects->next; l != NULL; l = l->next) {
        SPPaintSelectorMode nextmode;
        nextmode = 
            sp_stroke_style_determine_paint_selector_mode 
                (SP_OBJECT_STYLE (l->data));
        if (nextmode != pselmode) {
            /* Multiple styles */
            sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_MULTIPLE);
            gtk_object_set_data ( GTK_OBJECT (spw), 
                                  "update", 
                                  GINT_TO_POINTER (FALSE) );
            return;
        }
    }
#ifdef SP_SS_VERBOSE
    g_print ("StrokeStyleWidget: paint selector mode %d\n", pselmode);
#endif
    switch (pselmode) {
    
        case SP_PAINT_SELECTOR_MODE_NONE:
            /* No paint at all */
            sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_NONE);
            break;
            
        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
            sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_COLOR_RGB);
            sp_stroke_style_get_average_color_rgba (objects, c);
            sp_color_set_rgb_float (&color, c[0], c[1], c[2]);
            sp_paint_selector_set_color_alpha (psel, &color, c[3]);
            break;
            
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
            sp_paint_selector_set_mode ( psel, 
                                         SP_PAINT_SELECTOR_MODE_COLOR_CMYK);
            sp_stroke_style_get_average_color_cmyka (objects, c);
            sp_color_set_cmyk_float (&color, c[0], c[1], c[2], c[3]);
            sp_paint_selector_set_color_alpha (psel, &color, c[4]);
            break;
            
        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
            object = SP_OBJECT (objects->data);
            /* We know that all objects have lineargradient stroke style */
            vector = 
                sp_gradient_get_vector (SP_GRADIENT 
                                           (SP_OBJECT_STYLE_STROKE_SERVER 
                                               (object)), 
                                        FALSE );
                                        
            for (l = objects->next; l != NULL; l = l->next) {
                SPObject *next;
                next = SP_OBJECT (l->data);
                if (sp_gradient_get_vector 
                       ( SP_GRADIENT (SP_OBJECT_STYLE_STROKE_SERVER (next)), 
                         FALSE) != vector)
                {
                    
                    /* Multiple vectors */
                    sp_paint_selector_set_mode (psel, 
                                               SP_PAINT_SELECTOR_MODE_MULTIPLE);
                    gtk_object_set_data ( GTK_OBJECT (spw), 
                                          "update", 
                                          GINT_TO_POINTER (FALSE) );
                    return;
                }
            } // end of for loop
            
            
            /* TODO: Probably we should set multiple mode here too */
            
            sp_paint_selector_set_mode (psel, 
                                        SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR);
            sp_paint_selector_set_gradient_linear (psel, vector);
            sp_selection_bbox_document (sel, &fbb);
            sp_paint_selector_set_gradient_bbox ( psel, fbb.x0, fbb.y0, 
                                                  fbb.x1, fbb.y1 );
            
            /* TODO: This is plain wrong */
            
            lg = SP_LINEARGRADIENT (SP_OBJECT_STYLE_STROKE_SERVER (object));
            sp_item_invoke_bbox (SP_ITEM (object), &fbb, NULL, TRUE);
            sp_item_i2doc_affine (SP_ITEM (object), &fctm);
            sp_gradient_get_gs2d_matrix_f ( SP_GRADIENT (lg), &fctm, 
                                            &fbb, &gs2d );
            sp_paint_selector_set_gradient_gs2d_matrix_f (psel, &gs2d);
            sp_paint_selector_set_gradient_properties (psel, 
                                                       SP_GRADIENT_UNITS (lg), 
                                                       SP_GRADIENT_SPREAD (lg));
            sp_paint_selector_set_lgradient_position ( psel, 
                                                       lg->x1.computed, 
                                                       lg->y1.computed, 
                                                       lg->x2.computed, 
                                                       lg->y2.computed );
            break;
            
        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
        
            object = SP_OBJECT (objects->data);
            
            /* We know that all objects have radialgradient stroke style */
            
            vector = 
                sp_gradient_get_vector ( SP_GRADIENT 
                                          (SP_OBJECT_STYLE_STROKE_SERVER 
                                              (object)),
                                         FALSE );
            
            for (l = objects->next; l != NULL; l = l->next) {
                SPObject *next;
                next = SP_OBJECT (l->data);
                if (sp_gradient_get_vector ( SP_GRADIENT 
                                              (SP_OBJECT_STYLE_STROKE_SERVER 
                                                  (next)), 
                                             FALSE) != vector)
                {
                    /* Multiple vectors */
                    sp_paint_selector_set_mode (psel, 
                                               SP_PAINT_SELECTOR_MODE_MULTIPLE);
                    gtk_object_set_data ( GTK_OBJECT (spw), "update", 
                                          GINT_TO_POINTER (FALSE) );
                    return;
                }
            }
            
            /* TODO: Probably we should set multiple mode here too */
            
            sp_paint_selector_set_gradient_radial (psel, vector);
            sp_selection_bbox_document (sel, &fbb);
            sp_paint_selector_set_gradient_bbox ( psel, fbb.x0, fbb.y0, 
                                                  fbb.x1, fbb.y1);
            
            /* TODO: This is plain wrong */
            
            rg = SP_RADIALGRADIENT (SP_OBJECT_STYLE_STROKE_SERVER (object));
            sp_item_invoke_bbox (SP_ITEM (object), &fbb, NULL, TRUE);
            sp_item_i2doc_affine (SP_ITEM (object), &fctm);
            sp_gradient_get_gs2d_matrix_f ( SP_GRADIENT (rg), &fctm, 
                                            &fbb, &gs2d );
            sp_paint_selector_set_gradient_gs2d_matrix_f (psel, &gs2d);
            sp_paint_selector_set_gradient_properties (psel, 
                                                      SP_GRADIENT_UNITS (rg), 
                                                      SP_GRADIENT_SPREAD (rg) );
                                                      
            sp_paint_selector_set_rgradient_position ( psel, 
                                                       rg->cx.computed, 
                                                       rg->cy.computed, 
                                                       rg->fx.computed, 
                                                       rg->fy.computed, 
                                                       rg->r.computed );
            break;
            
        default:
            sp_paint_selector_set_mode ( psel, SP_PAINT_SELECTOR_MODE_MULTIPLE );
            break;
    }

    gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));

} // end of sp_stroke_style_paint_update()



static void
sp_stroke_style_paint_update_repr ( SPWidget *spw, SPRepr *repr )
{
    SPPaintSelector *psel;
    SPPaintSelectorMode pselmode;
    SPStyle *style;

    if (gtk_object_get_data (GTK_OBJECT (spw), "update"))
        return;

    gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    psel = SP_PAINT_SELECTOR(gtk_object_get_data ( GTK_OBJECT (spw), 
                                                   "paint-selector") );

    style = sp_style_new ();
    sp_style_read_from_repr (style, repr);

    pselmode = sp_stroke_style_determine_paint_selector_mode (style);
#ifdef SP_SS_VERBOSE
    g_print ("StrokeStyleWidget: paint selector mode %d\n", pselmode);
#endif

    switch (pselmode) {
        case SP_PAINT_SELECTOR_MODE_NONE:
            /* No paint at all */
            sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_NONE);
            break;
            
        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
            sp_paint_selector_set_mode (psel, pselmode);
            sp_paint_selector_set_color_alpha(psel, &style->stroke.value.color,
                            SP_SCALE24_TO_FLOAT(style->stroke_opacity.value) );
            break;
            
        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
            /* fixme: Think about it (Lauris) */
            break;
            
        default:
            break;
    }

    sp_style_unref (style);

    gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));

} // end of sp_stroke_style_paint_update_repr()



static void
sp_stroke_style_paint_mode_changed ( SPPaintSelector *psel, 
                                     SPPaintSelectorMode mode, 
                                     SPWidget *spw )
{
    if (gtk_object_get_data (GTK_OBJECT (spw), "update"))
        return;

    /* TODO: Does this work?
     * Not really, here we have to get old color back from object
     * Instead of relying on paint widget having meaningful colors set 
     */
    sp_stroke_style_paint_changed (psel, spw);
    
}



static void
sp_stroke_style_paint_dragged (SPPaintSelector *psel, SPWidget *spw)
{
    const GSList *items, *i;
    SPGradient *vector;
    SPColor color;
    gfloat alpha;

    if (gtk_object_get_data (GTK_OBJECT (spw), "update")) 
        return;

#ifdef SP_SS_VERBOSE
    g_print ("StrokeStyleWidget: paint dragged\n");
#endif

    switch (psel->mode) {
        case SP_PAINT_SELECTOR_MODE_EMPTY:
        case SP_PAINT_SELECTOR_MODE_MULTIPLE:
        case SP_PAINT_SELECTOR_MODE_NONE:
            g_warning ( "file %s: line %d: Paint %d should not emit 'dragged'",
                        __FILE__, __LINE__, psel->mode);
            break;
            
        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
            sp_paint_selector_get_color_alpha (psel, &color, &alpha);
            items = sp_widget_get_item_list (spw);
            for (i = items; i != NULL; i = i->next) {
                sp_style_set_stroke_color_alpha ( SP_OBJECT_STYLE (i->data),
                                                  &color, alpha, 
                                                  TRUE, TRUE );
            }
            break;
            
        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
            vector = sp_paint_selector_get_gradient_vector (psel);
            vector = sp_gradient_ensure_vector_normalized (vector);
            items = sp_widget_get_item_list (spw);
            for (i = items; i != NULL; i = i->next) {
                SPGradient *lg;
                lg = sp_item_force_stroke_lineargradient_vector ( SP_ITEM 
                                                                    (i->data),
                                                                  vector );
                sp_paint_selector_write_lineargradient ( psel, 
                                                         SP_LINEARGRADIENT (lg),
                                                         SP_ITEM (i->data) );
            }
            break;
            
        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
            vector = sp_paint_selector_get_gradient_vector (psel);
            vector = sp_gradient_ensure_vector_normalized (vector);
            items = sp_widget_get_item_list (spw);
            for (i = items; i != NULL; i = i->next) {
                SPGradient *rg;
                rg = sp_item_force_stroke_radialgradient_vector ( SP_ITEM 
                                                                      (i->data),
                                                                  vector );
                sp_paint_selector_write_radialgradient ( psel, 
                                                         SP_RADIALGRADIENT (rg),
                                                         SP_ITEM (i->data) );
            }
            break;
            
        default:
            g_warning ( "file %s: line %d: Paint selector should not be in " 
                        "mode %d", 
                        __FILE__, __LINE__, 
                        psel->mode );
            break;
    }
    
} // end of sp_stroke_style_paint_dragged()



static void
sp_stroke_style_paint_changed ( SPPaintSelector *psel, SPWidget *spw )
{

    const GSList *items, *i, *r;
    GSList *reprs;
    SPCSSAttr *css;
    SPColor color;
    gfloat alpha;
    SPGradient *vector;
    gchar b[64];
	Inkscape::SVGOStringStream osalpha, oscolour;
            
    if (gtk_object_get_data (GTK_OBJECT (spw), "update"))
        return;

#ifdef SP_SS_VERBOSE
    g_print ("StrokeStyleWidget: paint changed\n");
#endif
    
    if (spw->inkscape) {
        /* fixme: */
        if (!SP_WIDGET_DOCUMENT (spw))
            return;
        
        reprs = NULL;
        items = sp_widget_get_item_list (spw);
        
        for (i = items; i != NULL; i = i->next) {
            reprs = g_slist_prepend (reprs, SP_OBJECT_REPR (i->data));
        }
    } else {
        reprs = g_slist_prepend (NULL, spw->repr);
        items = NULL;
    }

    switch (psel->mode) {
        case SP_PAINT_SELECTOR_MODE_EMPTY:
        case SP_PAINT_SELECTOR_MODE_MULTIPLE:
            g_warning ( "file %s: line %d: Paint %d should not emit 'changed'", 
                        __FILE__, __LINE__, psel->mode);
            break;
            
        case SP_PAINT_SELECTOR_MODE_NONE:
            css = sp_repr_css_attr_new ();
            sp_repr_css_set_property (css, "stroke", "none");
            for (r = reprs; r != NULL; r = r->next) {
                sp_repr_css_change_recursive ((SPRepr *) r->data, css, "style");
                sp_repr_set_attr_recursive ( (SPRepr *) r->data, 
                                             "sodipodi:stroke-cmyk", NULL );
            }
            sp_repr_css_attr_unref (css);
            if (spw->inkscape) sp_document_done (SP_WIDGET_DOCUMENT (spw));
            break;
            
        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
            css = sp_repr_css_attr_new ();
            sp_paint_selector_get_color_alpha (psel, &color, &alpha);
            sp_svg_write_color ( b, 64, 
                                 sp_color_get_rgba32_falpha (&color, alpha) );
            sp_repr_css_set_property (css, "stroke", b);
            osalpha << alpha;
            sp_repr_css_set_property (css, "stroke-opacity", osalpha.str().c_str());
            
	     if (sp_color_get_colorspace_type (&color) == 
                    SP_COLORSPACE_TYPE_CMYK)
            {
                gfloat cmyk[4];
                sp_color_get_cmyk_floatv (&color, cmyk);
				oscolour << "(" << cmyk[0] << " " << cmyk[1] << " " << cmyk[2] << " " << cmyk[3] << ")";
            }
            
            for (r = reprs; r != NULL; r = r->next) {
                if(oscolour.str().length() > 0)
					sp_repr_set_attr_recursive ( (SPRepr *) r->data, 
                                             "sodipodi:stroke-cmyk", oscolour.str().c_str() );
				else
					sp_repr_set_attr_recursive ( (SPRepr *) r->data, 
                                             "sodipodi:stroke-cmyk", NULL );
											 
                sp_repr_css_change_recursive ((SPRepr *) r->data, css, "style");
            }
            sp_repr_css_attr_unref (css);
            
            if (spw->inkscape) 
                sp_document_done ( SP_WIDGET_DOCUMENT (spw) );
            
            break;
            
        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
        
            if (items) {
                vector = sp_paint_selector_get_gradient_vector (psel);
                
                if (!vector) {
                    /* No vector in paint selector should mean that we just 
                     * changed mode 
                     */
                    vector = 
                        sp_document_default_gradient_vector 
                            ( SP_WIDGET_DOCUMENT (spw) );
                            
                    for (i = items; i != NULL; i = i->next) {
                        sp_item_force_stroke_lineargradient_vector 
                            ( SP_ITEM (i->data), vector );
                    }
                } else {
                    vector = sp_gradient_ensure_vector_normalized (vector);
                    for (i = items; i != NULL; i = i->next) {
                        SPGradient *lg;
                        lg = sp_item_force_stroke_lineargradient_vector 
                                 ( SP_ITEM (i->data), vector );
                        sp_paint_selector_write_lineargradient ( psel, 
                                    SP_LINEARGRADIENT (lg), SP_ITEM (i->data) );
                                    
                        sp_object_invoke_write ( SP_OBJECT (lg), 
                                                 SP_OBJECT_REPR (lg), 
                                                 SP_OBJECT_WRITE_EXT );
                    }
                }
                sp_document_done (SP_WIDGET_DOCUMENT (spw));
            }
            break;
            
        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
            if (items) {
                vector = sp_paint_selector_get_gradient_vector (psel);
                if (!vector) {
                    /* No vector in paint selector should mean that we just 
                     * changed mode 
                     */
                    vector = 
                        sp_document_default_gradient_vector 
                            ( SP_WIDGET_DOCUMENT (spw) );
                            
                    for (i = items; i != NULL; i = i->next) {
                        sp_item_force_stroke_radialgradient_vector 
                            ( SP_ITEM (i->data), vector );
                    }
                } else {
                    vector = sp_gradient_ensure_vector_normalized (vector);
                    for (i = items; i != NULL; i = i->next) {
                        SPGradient *lg;
                        lg = sp_item_force_stroke_radialgradient_vector 
                                 ( SP_ITEM (i->data), vector );
                        sp_paint_selector_write_radialgradient (psel, 
                                    SP_RADIALGRADIENT (lg), SP_ITEM (i->data) );
                        sp_object_invoke_write ( SP_OBJECT (lg), 
                                                 SP_OBJECT_REPR (lg), 
                                                 SP_OBJECT_WRITE_EXT );
                    }
                }
                sp_document_done (SP_WIDGET_DOCUMENT (spw));
            }
            break;
            
        default:
            g_warning ( "file %s: line %d: Paint selector should not be in " 
                        "mode %d", 
                        __FILE__, __LINE__, 
                        psel->mode );
            break;
    }

    g_slist_free (reprs);
    
} // end of sp_stroke_style_paint_changed()





/* Line */

static void sp_stroke_style_line_construct (SPWidget *spw, gpointer data);
static void sp_stroke_style_line_modify_selection ( SPWidget *spw, 
                                                    SPSelection *selection, 
                                                    guint flags, 
                                                    gpointer data );
                                                    
static void sp_stroke_style_line_change_selection ( SPWidget *spw, 
                                                    SPSelection *selection, 
                                                    gpointer data );
                                                    
static void sp_stroke_style_line_attr_changed ( SPWidget *spw, 
                                                const gchar *key, 
                                                const gchar *oldval, 
                                                const gchar *newval );

static void sp_stroke_style_line_update (SPWidget *spw, SPSelection *sel);
static void sp_stroke_style_line_update_repr (SPWidget *spw, SPRepr *repr);

static void sp_stroke_style_set_join_buttons ( SPWidget *spw, 
                                               GtkWidget *active );

static void sp_stroke_style_set_cap_buttons ( SPWidget *spw, 
                                              GtkWidget *active );

static void sp_stroke_style_set_marker_buttons ( SPWidget *spw, 
                                                 GtkWidget *active, 
                                                 const gchar *marker_name );
static void sp_stroke_style_width_changed (GtkAdjustment *adj, SPWidget *spw);
static void sp_stroke_style_miterlimit_changed (GtkAdjustment *adj, SPWidget *spw);
static void sp_stroke_style_any_toggled (GtkToggleButton *tb, SPWidget *spw);
static void sp_stroke_style_line_dash_changed ( SPDashSelector *dsel, 
                                                SPWidget *spw );
static void sp_stroke_style_update_marker_buttons( SPWidget *spw, 
                                                   const GSList *objects, 
                                                   unsigned int loc, 
                                                   const gchar *stock_type );

static void sp_stroke_style_ensure_marker (const gchar* n);
static gchar* ink_extract_marker_name(const gchar *n);

                                                   
/**
* Helper function for creating radio buttons.  This should probably be re-thought out
* when reimplementing this with Gtkmm.  
*/
static GtkWidget *
sp_stroke_radio_button ( GtkWidget* tb, const char* n, const char* xpm,
                         GtkWidget* hb, GtkWidget* spw,
                         gchar const *key, gchar const *data )
{
    g_assert(xpm != NULL);
    g_assert(hb  != NULL);
    g_assert(spw != NULL);

    if (tb == NULL) {
        tb = gtk_radio_button_new (NULL);
    } else {
        tb = gtk_radio_button_new 
                 ( gtk_radio_button_group (GTK_RADIO_BUTTON (tb)) );
    }
    
    gtk_widget_show (tb);
    gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (tb), FALSE);
    gtk_box_pack_start (GTK_BOX (hb), tb, FALSE, FALSE, 0);
    gtk_object_set_data (GTK_OBJECT (spw), n, tb);
    gtk_object_set_data (GTK_OBJECT (tb), key, (gpointer*)data);
    gtk_signal_connect (GTK_OBJECT (tb), "toggled",
                GTK_SIGNAL_FUNC (sp_stroke_style_any_toggled),
                spw);
    GtkWidget *px = gtk_image_new_from_stock(n, GTK_ICON_SIZE_LARGE_TOOLBAR);
    g_assert(px != NULL);
    gtk_widget_show (px);
    gtk_container_add (GTK_CONTAINER (tb), px);

    return (tb);

} // end of sp_stroke_radio_button()

static bool
marker_not_in_doc (SPRepr *repr, GSList *ml)
{
    for (; ml != NULL; ml = ml->next){
        SPRepr *item_repr = SP_OBJECT_REPR((SPItem *) ml->data);
        if (sp_repr_attr(repr,"id")==sp_repr_attr(item_repr,"id")) return FALSE;
    }
    return TRUE;
}

static guchar *
sp_marker_preview_from_svg( const gchar *name,
                         unsigned int size,
                         unsigned int scale )
{
    static SPDocument *doc = NULL;
    static NRArena *arena = NULL;
    static NRArenaItem *root = NULL;
    static unsigned int edoc = FALSE;
    guchar *px;
    /* Try to load from document */
    if (!edoc && !doc) {
        if (g_file_test(INKSCAPE_MARKERSDIR "/markers.svg", G_FILE_TEST_IS_REGULAR)) {
            doc = sp_document_new (INKSCAPE_MARKERSDIR "/markers.svg", FALSE, FALSE);
        }
        if ( !doc && g_file_test( INKSCAPE_MARKERSDIR "/markers.svg",
                                  G_FILE_TEST_IS_REGULAR) )
        {
            doc = sp_document_new ( INKSCAPE_MARKERSDIR "/markers.svg",
                                    FALSE, FALSE );
        }
        if (doc) {
            unsigned int visionkey;
            sp_document_ensure_up_to_date (doc);
            /* Create new arena */
            arena = (NRArena *) nr_object_new (NR_TYPE_ARENA);
            /* Create ArenaItem and set transform */
            visionkey = sp_item_display_key_new (1);
            /* fixme: Memory manage root if needed (Lauris) */
            root = sp_item_invoke_show ( SP_ITEM (SP_DOCUMENT_ROOT (doc)),
                                         arena, visionkey, SP_ITEM_SHOW_PRINT );
        } else {
            edoc = TRUE;
        }
    }

    if (!edoc && doc) {
        SPObject *object;
        object = sp_document_lookup_id (doc, name);
        if (object && SP_IS_ITEM (object)) {
            /* Find bbox in document */
            NRMatrix i2doc;
            sp_item_i2doc_affine(SP_ITEM(object), &i2doc);
            NRRect dbox;
            sp_item_invoke_bbox(SP_ITEM(object), &dbox, &i2doc, TRUE);
            /* This is in document coordinates, i.e. pixels */
            if (!nr_rect_d_test_empty (&dbox))
            {
                NRRectL ibox, area, ua;
                NRMatrix t;
                NRPixBlock B;
                NRGC gc;
                double sf;
                int width, height, dx, dy;
                /* Update to renderable state */
                sf = 0.8 * size / scale;
                nr_matrix_set_scale (&t, sf, sf);
                nr_arena_item_set_transform (root, &t);
                nr_matrix_set_identity (&gc.transform);
                nr_arena_item_invoke_update ( root, NULL, &gc,
                                              NR_ARENA_ITEM_STATE_ALL,
                                              NR_ARENA_ITEM_STATE_NONE );
                /* Item integer bbox in points */
                ibox.x0 = (int) floor (sf * dbox.x0 + 0.5);
                ibox.y0 = (int) floor (sf * dbox.y0 + 0.5);
                ibox.x1 = (int) floor (sf * dbox.x1 + 0.5);
                ibox.y1 = (int) floor (sf * dbox.y1 + 0.5);
                /* Find button visible area */
                width = ibox.x1 - ibox.x0;
                height = ibox.y1 - ibox.y0;
                //dx = (size - width) / 2;
                //dy = (size - height) / 2;
                dx=dy=size;
                dx=(dx-width)/2; // watch out for size, since 'unsigned'-'signed' can cause problems if the result is negative
                dy=(dy-height)/2;
                area.x0 = ibox.x0 - dx;
                area.y0 = ibox.y0 - dy;
                area.x1 = area.x0 + size;
                area.y1 = area.y0 + size;
                /* Actual renderable area */
                ua.x0 = MAX (ibox.x0, area.x0);
                ua.y0 = MAX (ibox.y0, area.y0);
                ua.x1 = MIN (ibox.x1, area.x1);
                ua.y1 = MIN (ibox.y1, area.y1);
                /* Set up pixblock */
                px = nr_new (guchar, 4 * size * size);
                memset (px, 0x00, 4 * size * size);
                /* Render */
                nr_pixblock_setup_extern ( &B, NR_PIXBLOCK_MODE_R8G8B8A8N,
                                           ua.x0, ua.y0, ua.x1, ua.y1,
                                           px + 4 * size * (ua.y0 - area.y0) +
                                           4 * (ua.x0 - area.x0),
                                           4 * size, FALSE, FALSE );
                nr_arena_item_invoke_render ( root, &ua, &B,
                                              NR_ARENA_ITEM_RENDER_NO_CACHE );
                nr_pixblock_release (&B);
                return px;
            }
        }
    }
    return NULL;

} // end of sp_marker_preview_from_svg()


GtkWidget *
sp_marker_prev_new (unsigned int size, const gchar *name)
{
    guchar *pixels;

    gchar *prevname = g_strconcat ( name , "_prev",NULL);
    pixels = sp_marker_preview_from_svg (prevname, size, 1);
    /* TODO: If pixels == NULL then write to stderr that we couldn't find NAME.xpm,
       and suggest doing `make install'. */
    GtkWidget *pb = gtk_image_new_from_pixbuf(gdk_pixbuf_new_from_data(pixels, GDK_COLORSPACE_RGB, TRUE, 8, size, size, size * 4, (GdkPixbufDestroyNotify)nr_free, NULL));

    return  pb;
}


static bool
sp_marker_load_from_svg ( const gchar *name, SPDocument *current_doc )
{
    static SPDocument *doc = NULL;
    static unsigned int edoc = FALSE;
    if (!current_doc) return FALSE;
    /* Try to load from document */
    if (!edoc && !doc) {
        if (g_file_test(INKSCAPE_MARKERSDIR "/markers.svg", G_FILE_TEST_IS_REGULAR)) {
            doc = sp_document_new (INKSCAPE_MARKERSDIR "/markers.svg", FALSE, FALSE);
        }
        if ( !doc && g_file_test( INKSCAPE_MARKERSDIR "/markers.svg",
                                  G_FILE_TEST_IS_REGULAR) )
        {
            doc = sp_document_new ( INKSCAPE_MARKERSDIR "/markers.svg",
                                    FALSE, FALSE );
        }
        if (doc) {
            sp_document_ensure_up_to_date (doc);
            } else {
            edoc = TRUE;
        }
    }
    if (!edoc && doc) {
        /* Get the marker we want */
        SPObject *object;
        object = sp_document_lookup_id (doc, name);
        if (object && SP_IS_MARKER (object)) {
             SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS (current_doc);
             SPRepr *mark_repr = sp_repr_duplicate(SP_OBJECT_REPR(object));
             sp_repr_add_child (SP_OBJECT_REPR (defs), mark_repr, NULL);
             sp_repr_unref (mark_repr);
             return TRUE;
         }
   }
   return FALSE;
}

static bool
sp_marker_defaultlist_from_svg ( GtkWidget *m, SPDocument *current_doc )
{
    static SPDocument *doc = NULL;
    static unsigned int edoc = FALSE;

    if (!current_doc) return FALSE;
    /* Try to load from document */
    if (!edoc && !doc) {
        if (g_file_test(INKSCAPE_MARKERSDIR "/markers.svg", G_FILE_TEST_IS_REGULAR)) {
            doc = sp_document_new (INKSCAPE_MARKERSDIR "/markers.svg", FALSE, FALSE);
        }
        if ( !doc && g_file_test( INKSCAPE_MARKERSDIR "/markers.svg",
                                  G_FILE_TEST_IS_REGULAR) )
        {
            doc = sp_document_new ( INKSCAPE_MARKERSDIR "/markers.svg",
                                    FALSE, FALSE );
        }
        if (doc) {
            sp_document_ensure_up_to_date (doc);
            } else {
            edoc = TRUE;
        }
    }
    if (!edoc && doc) {
        /* Pick up all markers ad add items to menu*/
        GSList *ml = NULL;
        SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS (doc);

        for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(defs)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
                if (SP_IS_MARKER (ochild)) {
                    ml = g_slist_prepend (ml, ochild);
                }
        }
        for (; ml != NULL; ml = ml->next){
                        if (SP_IS_MARKER(ml->data)){
                            SPRepr *repr = SP_OBJECT_REPR((SPItem *) ml->data);
                            if (!sp_document_lookup_id (current_doc, sp_repr_attr(repr,"id"))){
                                GtkWidget *i = gtk_menu_item_new ();
                                gtk_widget_show (i);
                                gchar *markid = (gchar *)sp_repr_attr(repr,"id");
                                g_object_set_data (G_OBJECT (i), "marker", markid);
                                GtkWidget *hb = gtk_hbox_new (FALSE, 4);
                                gtk_widget_show (hb);
    //                            GtkWidget *prv = sp_marker_prev_new (32, markid);
    //                            gtk_widget_show (prv);
    //                            gtk_box_pack_start (GTK_BOX (hb), prv, FALSE, FALSE, 0);
                                GtkWidget *l = gtk_label_new (sp_repr_attr(repr,"id"));
                                gtk_widget_show (l);
                                gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
                                gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                                gtk_widget_show (hb);
                                gtk_container_add (GTK_CONTAINER (i), hb);
                                gtk_menu_append (GTK_MENU (m), i);
                             }
                        }
        }

   }
   return FALSE;
}



static GtkWidget*
ink_marker_menu ( GtkWidget *tbl, gchar *menu_id)
{

        SPDesktop *desktop = inkscape_active_desktop();
        SPDocument *doc = SP_DT_DOCUMENT (desktop);
        GtkWidget *mnu = gtk_option_menu_new ();


       /* Create new menu widget */
        GtkWidget *m = gtk_menu_new ();
        gtk_widget_show (m);
        SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS (doc);

        /* Pick up all markers  */
        GSList *ml = NULL;

        for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(defs)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
                if (SP_IS_MARKER (ochild)) {
                    ml = g_slist_prepend (ml, ochild);
                }
        }

        bool *updating= FALSE;
        g_object_set_data (G_OBJECT (mnu), "updating", updating);


       ml = g_slist_reverse (ml);

       if (!doc) {
        GtkWidget *i;
        i = gtk_menu_item_new_with_label (_("No document selected"));
        gtk_widget_show (i);
        gtk_menu_append (GTK_MENU (m), i);
        gtk_widget_set_sensitive (mnu, FALSE);
    } else if (!ml) {
             GtkWidget *i;
             i = gtk_menu_item_new ();
             gtk_widget_show (i);
             gchar *markid = "none";
             g_object_set_data (G_OBJECT (i), "marker", markid);
              GtkWidget *hb = gtk_hbox_new (FALSE, 4);
             gtk_widget_show (hb);
             GtkWidget *l = gtk_label_new ("None");
             gtk_widget_show (l);
             gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
             gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
             gtk_widget_show (hb);
             gtk_container_add (GTK_CONTAINER (i), hb);
             gtk_menu_append (GTK_MENU (m), i);
             sp_marker_defaultlist_from_svg ( m, doc );
    } else {
             SPMarker *mark = NULL;
             GtkWidget *i;
             i = gtk_menu_item_new ();
             gtk_widget_show (i);
             gchar *markid = "none";
             g_object_set_data (G_OBJECT (i), "marker", markid);
             GtkWidget *hb = gtk_hbox_new (FALSE, 4);
             gtk_widget_show (hb);
             GtkWidget *l = gtk_label_new ("None");
             gtk_widget_show (l);
             gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
             gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
             gtk_widget_show (hb);
             gtk_container_add (GTK_CONTAINER (i), hb);
             gtk_menu_append (GTK_MENU (m), i);


             for (; ml != NULL; ml = ml->next){
                if (SP_IS_MARKER(ml->data)){
                    mark = SP_MARKER (ml->data);

                    i = gtk_menu_item_new ();
                    gtk_widget_show (i);
                    SPRepr *repr = SP_OBJECT_REPR((SPItem *) ml->data);
                    gchar *markid = (gchar *)sp_repr_attr(repr,"id");
                    g_object_set_data (G_OBJECT (i), "marker", markid);
                    GtkWidget *hb = gtk_hbox_new (FALSE, 4);
                    gtk_widget_show (hb);
                    GtkWidget *l = gtk_label_new (sp_repr_attr(repr,"id"));
                    gtk_widget_show (l);
                    gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
                    gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                    gtk_widget_show (hb);
                    gtk_container_add (GTK_CONTAINER (i), hb);
                    gtk_menu_append (GTK_MENU (m), i);
                 }

             }
            sp_marker_defaultlist_from_svg ( m, doc );
            gtk_widget_set_sensitive (mnu, TRUE);
      }
    gtk_object_set_data (GTK_OBJECT (mnu), "menu_id", menu_id);
    gtk_option_menu_set_menu (GTK_OPTION_MENU (mnu), m);
    /* Set history */
    gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);

return mnu;
}


/*user selected existing marker from list*/
static void
sp_marker_select (GtkOptionMenu *mnu,  GtkWidget *tbl)
{
    if (gtk_object_get_data (GTK_OBJECT (mnu), "update"))  return;
    SPDesktop *desktop = inkscape_active_desktop();
    SPDocument *doc = SP_DT_DOCUMENT (desktop);
    if (!SP_IS_DOCUMENT(doc)) return;
    /* Get Marker */
    if (!g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "marker")) return;
    gchar *markid = (gchar *) g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "marker");
    gchar *marker ="";
    if (strcmp(markid, "none") != 0){
        if (!sp_document_lookup_id (doc, markid) && !SP_IS_MARKER(sp_document_lookup_id (doc, markid))) sp_marker_load_from_svg ( markid, doc );
        SPMarker *mark = SP_MARKER(sp_document_lookup_id (doc, markid));
        if (mark){
            SPRepr *repr = SP_OBJECT_REPR(mark);
            marker = g_strconcat ("url(#", sp_repr_attr(repr,"id"), ")",NULL);
        }
    }else {
        marker = markid;
    }
    SPCSSAttr *css = sp_repr_css_attr_new ();
    gchar *menu_id = (gchar *) g_object_get_data (G_OBJECT(mnu), "menu_id");
    sp_repr_css_set_property (css, menu_id, marker);
    const GSList *items = sp_selection_item_list (SP_DT_SELECTION (desktop));
    for (; items != NULL; items = items->next){
          SPRepr *selrepr = SP_OBJECT_REPR((SPItem *) items->data);
          if (selrepr) sp_repr_css_change_recursive (selrepr, css, "style");
          sp_object_request_modified(SP_OBJECT(items->data), SP_OBJECT_MODIFIED_FLAG);
          sp_object_request_update (SP_OBJECT(items->data), SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

        }
    sp_repr_css_attr_unref (css);
    g_free (marker);
    g_free (menu_id);


}

/**
* \brief  Creates a new widget for the line stroke style.
*
*/
GtkWidget *
sp_stroke_style_line_widget_new (void)
{
    GtkWidget *spw, *f, *t, *hb, *sb, *us, *tb, *ds;
    GtkObject *a;
    gint i;

    GtkTooltips *tt = gtk_tooltips_new();

    spw = sp_widget_new_global (INKSCAPE);

    f = gtk_frame_new (_("Stroke settings"));
    gtk_widget_show (f);
    gtk_container_set_border_width (GTK_CONTAINER (f), 4);
    gtk_container_add (GTK_CONTAINER (spw), f);

    t = gtk_table_new (3, 6, FALSE);
    gtk_widget_show (t);
    gtk_container_set_border_width (GTK_CONTAINER (t), 4);
    gtk_table_set_row_spacings (GTK_TABLE (t), 4);
    gtk_container_add (GTK_CONTAINER (f), t);
    gtk_object_set_data (GTK_OBJECT (spw), "stroke", t);

    i=0;

    /* Stroke width */
    spw_label(t, _("Width:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    a = gtk_adjustment_new (1.0, 0.0, 100.0, 0.1, 10.0, 10.0);
    gtk_object_set_data (GTK_OBJECT (spw), "width", a);
    sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 2);
    gtk_tooltips_set_tip (tt, sb, _("Stroke width"), NULL);
    gtk_widget_show (sb);

    sp_dialog_defocus_on_enter (sb); 

    gtk_box_pack_start (GTK_BOX (hb), sb, FALSE, FALSE, 0);
    us = sp_unit_selector_new (SP_UNIT_ABSOLUTE);
    gtk_widget_show (us);
    sp_unit_selector_add_adjustment ( SP_UNIT_SELECTOR (us), 
                                      GTK_ADJUSTMENT (a) );
    gtk_box_pack_start (GTK_BOX (hb), us, FALSE, FALSE, 0);
    gtk_object_set_data (GTK_OBJECT (spw), "units", us);

    gtk_signal_connect ( GTK_OBJECT (a), "value_changed", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_width_changed), spw );
    i++;

    /* Join type */
    spw_label(t, _("Join:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    tb = NULL;

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_JOIN_MITER,
                INKSCAPE_PIXMAPDIR "/join_miter.xpm",
                hb, spw, "join", "miter");
    gtk_tooltips_set_tip (tt, tb, _("Miter join"), NULL);

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_JOIN_ROUND,
                INKSCAPE_PIXMAPDIR "/join_round.xpm",
                hb, spw, "join", "round");
    gtk_tooltips_set_tip (tt, tb, _("Round join"), NULL);

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_JOIN_BEVEL,
                INKSCAPE_PIXMAPDIR "/join_bevel.xpm",
                hb, spw, "join", "bevel");
    gtk_tooltips_set_tip (tt, tb, _("Bevel join"), NULL);

    i++;  

    /* Miterlimit  */
    spw_label(t, _("Miter limit:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    a = gtk_adjustment_new (4.0, 0.0, 100.0, 0.1, 10.0, 10.0);
    gtk_object_set_data (GTK_OBJECT (spw), "miterlimit", a);

    sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 2);
    gtk_tooltips_set_tip (tt, sb, _("Maximum length of the miter (in units of stroke width)"), NULL);
    gtk_widget_show (sb);
    gtk_object_set_data (GTK_OBJECT (spw), "miterlimit_sb", sb);
    sp_dialog_defocus_on_enter (sb); 

    gtk_box_pack_start (GTK_BOX (hb), sb, FALSE, FALSE, 0);

    gtk_signal_connect ( GTK_OBJECT (a), "value_changed", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_miterlimit_changed), spw );
    i++;

    /* Cap type */
    spw_label(t, _("Cap:"), 0, i);

    hb = spw_hbox(t, 3, 1, i);

    tb = NULL;
    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_CAP_BUTT,
                INKSCAPE_PIXMAPDIR "/cap_butt.xpm",
                hb, spw, "cap", "butt");
    gtk_tooltips_set_tip (tt, tb, _("Butt cap"), NULL);

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_CAP_ROUND,
                INKSCAPE_PIXMAPDIR "/cap_round.xpm",
                hb, spw, "cap", "round");
    gtk_tooltips_set_tip (tt, tb, _("Round cap"), NULL);

    tb = sp_stroke_radio_button(tb, INKSCAPE_STOCK_CAP_SQUARE,
                INKSCAPE_PIXMAPDIR "/cap_square.xpm",
                hb, spw, "cap", "square");
    gtk_tooltips_set_tip (tt, tb, _("Square cap"), NULL);

    i++;


    /* Dash */
    spw_label(t, _("Pattern:"), 0, i);
    ds = sp_dash_selector_new ( inkscape_get_repr ( INKSCAPE, 
                                                    "palette.dashes") );

    gtk_widget_show (ds);
    gtk_table_attach ( GTK_TABLE (t), ds, 1, 4, i, i+1, 
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ), 
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "dash", ds);
    gtk_signal_connect ( GTK_OBJECT (ds), "changed", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_line_dash_changed), 
                         spw );
    i++;

    /* Start Marker */
    spw_label(t, _("Start Markers:"), 0, i);
    hb = spw_hbox(t, 3, 1, i);
    g_assert(hb != NULL);

    tb = NULL;
    tb = sp_stroke_radio_button ( tb, 
                INKSCAPE_STOCK_START_MARKER INKSCAPE_STOCK_MARKER_NONE,
                INKSCAPE_PIXMAPDIR "/marker_none_start.xpm",
                hb, spw, "start_marker", "none" );
    tb = sp_stroke_radio_button ( tb, 
                INKSCAPE_STOCK_START_MARKER INKSCAPE_STOCK_MARKER_FILLED_ARROW,
                INKSCAPE_PIXMAPDIR "/marker_triangle_start.xpm",
                hb, spw, "start_marker", "url(#mTriangle)" );
    tb = sp_stroke_radio_button ( tb, 
                INKSCAPE_STOCK_START_MARKER INKSCAPE_STOCK_MARKER_HOLLOW_ARROW,
                INKSCAPE_PIXMAPDIR "/marker_arrow_start.xpm",
                hb, spw, "start_marker", "url(#mArrow)" );
    i++;

    /* Mid Marker */
    spw_label(t, _("Mid Markers:"), 0, i);
    hb = spw_hbox(t, 3, 1, i);
    g_assert(hb != NULL);

    tb = NULL;
    tb = sp_stroke_radio_button ( tb, 
                INKSCAPE_STOCK_MID_MARKER INKSCAPE_STOCK_MARKER_NONE,
                INKSCAPE_PIXMAPDIR "/marker_none_end.xpm",
                hb, spw, "mid_marker", "none" );
    tb = sp_stroke_radio_button ( tb, 
                INKSCAPE_STOCK_MID_MARKER INKSCAPE_STOCK_MARKER_FILLED_ARROW,
                INKSCAPE_PIXMAPDIR "/marker_triangle_end.xpm",
                hb, spw, "mid_marker", "url(#mTriangle)" );
    tb = sp_stroke_radio_button ( tb, 
                INKSCAPE_STOCK_MID_MARKER INKSCAPE_STOCK_MARKER_HOLLOW_ARROW,
                INKSCAPE_PIXMAPDIR "/marker_triangle_end.xpm",
                hb, spw, "mid_marker", "url(#mArrow)" );
    i++;

    /* End Marker */
    spw_label(t, _("End Markers:"), 0, i);
    hb = spw_hbox(t, 3, 1, i);
    g_assert(hb != NULL);

    tb = NULL;
    tb = sp_stroke_radio_button ( tb, 
                INKSCAPE_STOCK_END_MARKER INKSCAPE_STOCK_MARKER_NONE,
                INKSCAPE_PIXMAPDIR "/marker_none_end.xpm",
                hb, spw, "end_marker", "none" );
    tb = sp_stroke_radio_button ( tb, 
                INKSCAPE_STOCK_END_MARKER INKSCAPE_STOCK_MARKER_FILLED_ARROW,
                INKSCAPE_PIXMAPDIR "/marker_triangle_end.xpm",
                hb, spw, "end_marker", "url(#mTriangle)" );
    tb = sp_stroke_radio_button ( tb, 
                INKSCAPE_STOCK_END_MARKER INKSCAPE_STOCK_MARKER_HOLLOW_ARROW,
                INKSCAPE_PIXMAPDIR "/marker_arrow_end.xpm",
                hb, spw, "end_marker", "url(#mArrow)" );
    i++;

    /* Drop down marker selectors*/
    spw_label(t, _("Start Markers:"), 0, i);
    GtkWidget *mnu  = ink_marker_menu ( spw ,"marker-start");
    gtk_signal_connect ( GTK_OBJECT (mnu), "changed", GTK_SIGNAL_FUNC (sp_marker_select), spw );
    gtk_widget_show (mnu);
    gtk_table_attach ( GTK_TABLE (t), mnu, 1, 4, i, i+1,
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "start_mark_menu", mnu);

    i++;
    spw_label(t, _("Mid Markers:"), 0, i);
    mnu = NULL;
    mnu  = ink_marker_menu ( spw ,"marker-mid");
    gtk_signal_connect ( GTK_OBJECT (mnu), "changed", GTK_SIGNAL_FUNC (sp_marker_select), spw );
    gtk_widget_show (mnu);
    gtk_table_attach ( GTK_TABLE (t), mnu, 1, 4, i, i+1,
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "mid_mark_menu", mnu);

    i++;
    spw_label(t, _("End Markers:"), 0, i);
    mnu = NULL;
    mnu  = ink_marker_menu ( spw ,"marker-end");
    gtk_signal_connect ( GTK_OBJECT (mnu), "changed", GTK_SIGNAL_FUNC (sp_marker_select), spw );
    gtk_widget_show (mnu);
    gtk_table_attach ( GTK_TABLE (t), mnu, 1, 4, i, i+1,
                       (GtkAttachOptions)( GTK_EXPAND | GTK_FILL ),
                       (GtkAttachOptions)0, 0, 0 );
    gtk_object_set_data (GTK_OBJECT (spw), "end_mark_menu", mnu);

    i++;



    /* General (I think) style dialog signals */
    gtk_signal_connect ( GTK_OBJECT (spw), "construct", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_line_construct), 
                         NULL );
    gtk_signal_connect ( GTK_OBJECT (spw), "modify_selection", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_line_modify_selection), 
                         NULL );
    gtk_signal_connect ( GTK_OBJECT (spw), "change_selection", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_line_change_selection), 
                         NULL );
    gtk_signal_connect ( GTK_OBJECT (spw), "attr_changed", 
                         GTK_SIGNAL_FUNC (sp_stroke_style_line_attr_changed), 
                         NULL );

    SPDesktop *desktop = inkscape_active_desktop();
    sp_stroke_style_line_update ( SP_WIDGET (spw), desktop ? SP_DT_SELECTION (desktop) : NULL);

    return spw;

} // end of sp_stroke_style_line_widget_new()



static void
sp_stroke_style_line_construct (SPWidget *spw, gpointer data)
{

#ifdef SP_SS_VERBOSE
    g_print ( "Stroke style widget constructed: inkscape %p repr %p\n", 
              spw->inkscape, spw->repr );
#endif
    if (spw->inkscape) {
        sp_stroke_style_line_update (spw, 
                SP_ACTIVE_DESKTOP ? SP_DT_SELECTION (SP_ACTIVE_DESKTOP) : NULL);
    } else if (spw->repr) {
        sp_stroke_style_line_update_repr (spw, spw->repr);
    }
}



static void
sp_stroke_style_line_modify_selection ( SPWidget *spw, 
                                        SPSelection *selection, 
                                        guint flags, 
                                        gpointer data )
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG)) {
        sp_stroke_style_line_update (spw, selection);
    }

}



static void
sp_stroke_style_line_change_selection ( SPWidget *spw, 
                                        SPSelection *selection, 
                                        gpointer data )
{
    sp_stroke_style_line_update (spw, selection);
}



static void
sp_stroke_style_line_attr_changed ( SPWidget *spw, 
                                    const gchar *key, 
                                    const gchar *oldval, 
                                    const gchar *newval )
{
    if (!strcmp (key, "style")) {
    
        /* This sounds interesting */
        sp_stroke_style_line_update_repr (spw, spw->repr);
    }
}



static void
sp_stroke_style_line_update ( SPWidget *spw, SPSelection *sel )
{
    const SPUnit *unit;
    const GSList *objects, *l;
    SPObject *object;
    SPStyle *style;
    gboolean stroked;
    gboolean joinValid = TRUE;
    unsigned int jointype;
    gboolean capValid = TRUE;
    unsigned int captype;
    GtkWidget *tb;

    
    if (gtk_object_get_data (GTK_OBJECT (spw), "update")) {
    return;
    }

    gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    GtkWidget *sset = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "stroke"));
    GtkObject *width = GTK_OBJECT(gtk_object_get_data (GTK_OBJECT (spw), "width"));
    GtkObject *ml = GTK_OBJECT(gtk_object_get_data (GTK_OBJECT (spw), "miterlimit"));
    GtkWidget *units = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "units"));
    GtkWidget *dsel = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "dash"));

    if (!sel || sp_selection_is_empty (sel)) {
        /* No objects, set empty */
        gtk_widget_set_sensitive (sset, FALSE);
        gtk_object_set_data ( GTK_OBJECT (spw), "update", 
                              GINT_TO_POINTER (FALSE) );
        return;
    }

    objects = sp_selection_item_list (sel);

    /* Determine average stroke width and miterlimit */
    gdouble avgwidth = 0.0;
    gdouble avgml = 0.0;
    stroked = TRUE;
    for (l = objects; l != NULL; l = l->next) {

        NRMatrix i2d;
        sp_item_i2d_affine (SP_ITEM (l->data), &i2d);

        object = SP_OBJECT (l->data);

        gdouble dist = object->style->stroke_width.computed * NR_MATRIX_DF_EXPANSION (&i2d);
        gdouble ml = object->style->stroke_miterlimit.value;
               
#ifdef SP_SS_VERBOSE
        g_print ( "%g in user is %g on desktop\n", 
                  object->style->stroke_width.computed, dist );
#endif
        avgwidth += dist;
        avgml += ml;
        if (object->style->stroke.type == SP_PAINT_TYPE_NONE) stroked = FALSE;
    }

    if (stroked) {
        gtk_widget_set_sensitive (sset, TRUE);
    } else {
        /* Some objects not stroked, set insensitive */
        gtk_widget_set_sensitive (sset, FALSE);
        gtk_object_set_data ( GTK_OBJECT (spw), "update", 
                              GINT_TO_POINTER (FALSE) );
        return;
    }

    avgwidth /= g_slist_length ((GSList *) objects);
    unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (units));
    sp_convert_distance (&avgwidth, SP_PS_UNIT, unit);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (width), avgwidth);

    avgml /= g_slist_length ((GSList *) objects);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (ml), avgml);

    /* Join & Cap */
    object = SP_OBJECT (objects->data);
    style = SP_OBJECT_STYLE (object);
    jointype = object->style->stroke_linejoin.value;
    captype = object->style->stroke_linecap.value;

    for (l = objects->next; l != NULL; l = l->next) {
        SPObject *o;
        o = SP_OBJECT (l->data);
        if (o->style->stroke_linejoin.value != jointype)
        {
            joinValid = FALSE;
        }
        if (o->style->stroke_linecap.value != captype)
        {
            capValid = FALSE;
        }
    }

    tb = NULL;
    if ( joinValid )
    {
        switch (jointype) {
            case SP_STROKE_LINEJOIN_MITER:
                tb = GTK_WIDGET( gtk_object_get_data ( GTK_OBJECT (spw), 
                                                INKSCAPE_STOCK_JOIN_MITER) );
                break;

            case SP_STROKE_LINEJOIN_ROUND:
                tb = GTK_WIDGET( gtk_object_get_data ( GTK_OBJECT (spw), 
                                                INKSCAPE_STOCK_JOIN_ROUND) );
                break;

            case SP_STROKE_LINEJOIN_BEVEL:
                tb = GTK_WIDGET( gtk_object_get_data ( GTK_OBJECT (spw), 
                                                INKSCAPE_STOCK_JOIN_BEVEL) );
                break;

            default:
                break;
        }
    }
    sp_stroke_style_set_join_buttons (spw, tb);

    tb = NULL;
    if ( capValid )
    {
        switch (captype) {
            case SP_STROKE_LINECAP_BUTT:
                tb = GTK_WIDGET( gtk_object_get_data ( GTK_OBJECT (spw), 
                                                INKSCAPE_STOCK_CAP_BUTT) );
                break;
                
            case SP_STROKE_LINECAP_ROUND:
                tb = GTK_WIDGET( gtk_object_get_data ( GTK_OBJECT (spw), 
                                                INKSCAPE_STOCK_CAP_ROUND) );
                break;
                
            case SP_STROKE_LINECAP_SQUARE:
                tb = GTK_WIDGET( gtk_object_get_data ( GTK_OBJECT (spw), 
                                                INKSCAPE_STOCK_CAP_SQUARE) );
                break;
                
            default:
                break;
        }
    } // end of if()
    
    sp_stroke_style_set_cap_buttons (spw, tb);

    /* Markers */
    sp_stroke_style_update_marker_buttons ( spw, objects, 
                                            SP_MARKER_LOC_START, 
                                            INKSCAPE_STOCK_START_MARKER );
    sp_stroke_style_update_marker_buttons ( spw, objects, 
                                            SP_MARKER_LOC_MID, 
                                            INKSCAPE_STOCK_MID_MARKER );
    sp_stroke_style_update_marker_buttons ( spw, objects, 
                                            SP_MARKER_LOC_END, 
                                            INKSCAPE_STOCK_END_MARKER );

    /* Dash */
    if (style->stroke_dash.n_dash > 0) {
        double d[64];
        int len, i;
        len = MIN (style->stroke_dash.n_dash, 64);
        for (i = 0; i < len; i++) {
            d[i] = style->stroke_dash.dash[i] / style->stroke_width.computed;
        }
        sp_dash_selector_set_dash ( SP_DASH_SELECTOR (dsel), len, d, 
                                    style->stroke_dash.offset / 
                                    style->stroke_width.computed );
    } else {
        sp_dash_selector_set_dash (SP_DASH_SELECTOR (dsel), 0, NULL, 0.0);
    }

    gtk_widget_set_sensitive (sset, TRUE);

    gtk_object_set_data ( GTK_OBJECT (spw), "update", 
                          GINT_TO_POINTER (FALSE) );

} // end of sp_stroke_style_line_update()


/**
 * \brief  This routine updates the GUI widgets from data in the repr for the 
 * line styles. It retrieves the current width, units, etc. from the dialog 
 * and then pulls in the data from the repr and updates the widgets 
 * accordingly.
 *
 */
static void
sp_stroke_style_line_update_repr (SPWidget *spw, SPRepr *repr)
{
    GtkWidget *sset, *units, *dsel;
    GtkObject *width;
    SPStyle *style;
    const SPUnit *unit;
    gdouble swidth;
    GtkWidget *tb;

    if (gtk_object_get_data (GTK_OBJECT (spw), "update"))
        return;

    gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    sset = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "stroke"));
    width = GTK_OBJECT(gtk_object_get_data (GTK_OBJECT (spw), "width"));
    units = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "units"));
    dsel = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), "dash"));

    style = sp_style_new ();
    sp_style_read_from_repr (style, repr);

    if (style->stroke.type == SP_PAINT_TYPE_NONE) {
        gtk_widget_set_sensitive (sset, FALSE);
        gtk_object_set_data ( GTK_OBJECT (spw), "update", 
                              GINT_TO_POINTER (FALSE) );
        return;
    }

    /* We need points */
    swidth = style->stroke_width.computed / 1.25;
    unit = sp_unit_selector_get_unit (SP_UNIT_SELECTOR (units));
    sp_convert_distance (&swidth, SP_PS_UNIT, unit);
    gtk_adjustment_set_value (GTK_ADJUSTMENT (width), swidth);

    /* Join & Cap */
    switch (style->stroke_linejoin.value) {
        case SP_STROKE_LINEJOIN_MITER:
            tb = GTK_WIDGET( gtk_object_get_data (GTK_OBJECT (spw), 
                             INKSCAPE_STOCK_JOIN_MITER) );
            break;
            
        case SP_STROKE_LINEJOIN_ROUND:
            tb = GTK_WIDGET( gtk_object_get_data (GTK_OBJECT (spw), 
                             INKSCAPE_STOCK_JOIN_ROUND) );
            break;
            
        case SP_STROKE_LINEJOIN_BEVEL:
            tb = GTK_WIDGET( gtk_object_get_data (GTK_OBJECT (spw), 
                             INKSCAPE_STOCK_JOIN_BEVEL) );
            break;
            
        default:
            tb = NULL;
            break;
    }
    sp_stroke_style_set_join_buttons (spw, tb);

    switch (style->stroke_linecap.value) {
        case SP_STROKE_LINECAP_BUTT:
            tb = GTK_WIDGET(gtk_object_get_data ( GTK_OBJECT (spw), 
                                                INKSCAPE_STOCK_CAP_BUTT) );
            break;

        case SP_STROKE_LINECAP_ROUND:
            tb = GTK_WIDGET(gtk_object_get_data ( GTK_OBJECT (spw), 
                                                INKSCAPE_STOCK_CAP_ROUND) );
            break;

        case SP_STROKE_LINECAP_SQUARE:
            tb = GTK_WIDGET(gtk_object_get_data ( GTK_OBJECT (spw), 
                                                INKSCAPE_STOCK_CAP_SQUARE) );
            break;

        default:
            tb = NULL;
            break;
            
    } // end of switch()
    
    sp_stroke_style_set_cap_buttons (spw, tb);

    /* Toggle buttons for markers - marker-start, marker-mid, and marker-end */
    /* TODO:  There's also a generic 'marker' that applies to all, but we'll 
     * leave that for later 
     */
    // tb = (GtkWidget*)gtk_object_get_data (GTK_OBJECT (spw), 
    //      style->marker[SP_MARKER_LOC_START].value);
    
    tb = (GtkWidget*)gtk_object_get_data ( GTK_OBJECT (spw), 
                                           INKSCAPE_STOCK_START_MARKER );
    sp_stroke_style_set_marker_buttons ( spw, tb, 
                                         INKSCAPE_STOCK_START_MARKER );

    tb = (GtkWidget*)gtk_object_get_data ( GTK_OBJECT (spw), 
                                           INKSCAPE_STOCK_MID_MARKER );
    sp_stroke_style_set_marker_buttons (spw, tb, INKSCAPE_STOCK_MID_MARKER);

    tb = (GtkWidget*)gtk_object_get_data ( GTK_OBJECT (spw), 
                                           INKSCAPE_STOCK_END_MARKER );
    sp_stroke_style_set_marker_buttons (spw, tb, INKSCAPE_STOCK_END_MARKER);



    /* Dash */
    if (style->stroke_dash.n_dash > 0) {
        double d[64];
        int len, i;
        len = MIN (style->stroke_dash.n_dash, 64);
        for (i = 0; i < len; i++) {
            d[i] = style->stroke_dash.dash[i] / style->stroke_width.computed;
        }
        sp_dash_selector_set_dash ( SP_DASH_SELECTOR (dsel), len, d, 
                                    style->stroke_dash.offset / 
                                        style->stroke_width.computed );
    } else {
        sp_dash_selector_set_dash (SP_DASH_SELECTOR (dsel), 0, NULL, 0.0);
    }

    gtk_widget_set_sensitive (sset, TRUE);

    sp_style_unref (style);

    gtk_object_set_data (GTK_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));

} // end of sp_stroke_style_line_update_repr()



static void
sp_stroke_style_set_scaled_dash ( SPCSSAttr *css, 
                                  int ndash, double *dash, double offset, 
                                  double scale )
{

    if (ndash > 0) {
        int i;
        Inkscape::SVGOStringStream osoffset, osarray;
	 for (i = 0; i < ndash; i++) {
	     osarray << dash[i] * scale;
            if (i < (ndash - 1)) {
		osarray << ",";
            }
        }
        sp_repr_css_set_property (css, "stroke-dasharray", osarray.str().c_str());
        osoffset << offset * scale;
	 sp_repr_css_set_property (css, "stroke-dashoffset", osoffset.str().c_str());
        
    } else {
        sp_repr_css_set_property (css, "stroke-dasharray", "none");
        sp_repr_css_set_property (css, "stroke-dashoffset", NULL);
    }
    
} // end of sp_stroke_style_set_scaled_dash()



static void
sp_stroke_style_scale_line (SPWidget *spw)
{
    const GSList *items, *i, *r;
    GSList *reprs;
    SPCSSAttr *css;

    GtkAdjustment *wadj = GTK_ADJUSTMENT(gtk_object_get_data (GTK_OBJECT (spw), "width"));
    SPUnitSelector *us = SP_UNIT_SELECTOR(gtk_object_get_data (GTK_OBJECT (spw), "units"));
    SPDashSelector *dsel = SP_DASH_SELECTOR(gtk_object_get_data (GTK_OBJECT (spw), "dash"));
    GtkAdjustment *ml = GTK_ADJUSTMENT(gtk_object_get_data (GTK_OBJECT (spw), "miterlimit"));

    if (spw->inkscape) {
        /* TODO: */
        if (!SP_WIDGET_DOCUMENT (spw)) 
            return;
        
        reprs = NULL;
        items = sp_widget_get_item_list (spw);
        for (i = items; i != NULL; i = i->next) {
            reprs = g_slist_prepend (reprs, SP_OBJECT_REPR (i->data));
        }
    } else {
        reprs = g_slist_prepend (NULL, spw->repr);
        items = NULL;
    }

    /* TODO: Create some standardized method */
    css = sp_repr_css_attr_new ();

    if (items) {
        for (i = items; i != NULL; i = i->next) {
            NRMatrix i2d, d2i;
            double length, dist, miterlimit;
            double *dash, offset;
            int ndash;
	     Inkscape::SVGOStringStream os_width, os_ml;

            length = wadj->value;
            miterlimit = ml->value;
            sp_dash_selector_get_dash (dsel, &ndash, &dash, &offset);

            /* Set stroke width */
            sp_convert_distance ( &length, sp_unit_selector_get_unit (us), 
                                  SP_PS_UNIT );
            sp_item_i2d_affine (SP_ITEM (i->data), &i2d);
            nr_matrix_invert (&d2i, &i2d);
            dist = length * NR_MATRIX_DF_EXPANSION (&d2i);

            os_width << dist;
            sp_repr_css_set_property (css, "stroke-width", os_width.str().c_str());

            os_ml << miterlimit;
            sp_repr_css_set_property (css, "stroke-miterlimit", os_ml.str().c_str());

            /* Set dash */
            sp_stroke_style_set_scaled_dash (css, ndash, dash, offset, dist);

            sp_repr_css_change_recursive ( SP_OBJECT_REPR (i->data), css, 
                                           "style" );
            g_free (dash);
        }
    } else {
        for (r = reprs; r != NULL; r = r->next) {
            double length, miterlimit;
            double *dash, offset;
            int ndash;
	     Inkscape::SVGOStringStream os_width, os_ml;

            length = wadj->value;
            miterlimit = ml->value;
            sp_dash_selector_get_dash (dsel, &ndash, &dash, &offset);

            sp_convert_distance ( &length, sp_unit_selector_get_unit (us), 
                                  SP_PS_UNIT );
            os_width << length * 1.25;
            sp_repr_css_set_property (css, "stroke-width", os_width.str().c_str());

            os_ml << miterlimit;
            sp_repr_css_set_property (css, "stroke-miterlimit", os_ml.str().c_str());

            sp_stroke_style_set_scaled_dash (css, ndash, dash, offset, length);

            sp_repr_css_change_recursive ((SPRepr *) r->data, css, "style");
            g_free (dash);
        }
    }

    sp_repr_css_attr_unref (css);
    
    if (spw->inkscape) 
        sp_document_done (SP_WIDGET_DOCUMENT (spw));

    g_slist_free (reprs);
    
} // end of sp_stroke_style_scale_line()



static void
sp_stroke_style_width_changed (GtkAdjustment *adj, SPWidget *spw)
{
    if (gtk_object_get_data (GTK_OBJECT (spw), "update")) 
        return;

    sp_stroke_style_scale_line (spw);
}

static void
sp_stroke_style_miterlimit_changed (GtkAdjustment *adj, SPWidget *spw)
{
    if (gtk_object_get_data (GTK_OBJECT (spw), "update")) 
        return;

    sp_stroke_style_scale_line (spw);
}

static void
sp_stroke_style_line_dash_changed (SPDashSelector *dsel, SPWidget *spw)
{
    if (gtk_object_get_data (GTK_OBJECT (spw), "update")) 
        return;

    sp_stroke_style_scale_line (spw);
}



/**
 * \brief  This routine handles toggle events for buttons in the stroke style 
 *         dialog.
 * When activated, this routine gets the data for the various widgets, and then
 * calls the respective routines to update css properties, etc.
 *
 */
static void
sp_stroke_style_any_toggled (GtkToggleButton *tb, SPWidget *spw)
{

    if (gtk_object_get_data (GTK_OBJECT (spw), "update")) 
        return;

    if (gtk_toggle_button_get_active (tb)) {
    
        const GSList *items, *i, *r;
        GSList *reprs;
        const gchar *join, *cap, *start_marker, *mid_marker, *end_marker;
        SPCSSAttr *css;

        items = sp_widget_get_item_list (spw);
        join = (const gchar *)gtk_object_get_data (GTK_OBJECT (tb), "join");
        cap = (const gchar *)gtk_object_get_data (GTK_OBJECT (tb), "cap");
        start_marker = (const gchar*)gtk_object_get_data ( GTK_OBJECT (tb), 
                                                           "start_marker");
        mid_marker = (const gchar*)gtk_object_get_data ( GTK_OBJECT (tb), 
                                                         "mid_marker");
        end_marker = (const gchar*)gtk_object_get_data ( GTK_OBJECT (tb), 
                                                         "end_marker");
        if (join) {
            GtkWidget *ml = GTK_WIDGET (g_object_get_data (G_OBJECT (spw), "miterlimit_sb"));
            gtk_widget_set_sensitive (ml, !strcmp(join, "miter"));
        }

        if (spw->inkscape) {
            reprs = NULL;
            items = sp_widget_get_item_list (spw);
            for (i = items; i != NULL; i = i->next) {
                reprs = g_slist_prepend (reprs, SP_OBJECT_REPR (i->data));
            }
        } else {
            reprs = g_slist_prepend (NULL, spw->repr);
            items = NULL;
        }

        /* TODO: Create some standardized method */
        css = sp_repr_css_attr_new ();

        if (join) {
            sp_repr_css_set_property (css, "stroke-linejoin", join);
            for (r = reprs; r != NULL; r = r->next) {
                sp_repr_css_change_recursive ((SPRepr *) r->data, css, "style");
            }
            sp_stroke_style_set_join_buttons (spw, GTK_WIDGET (tb));
        } else if (cap) {
            sp_repr_css_set_property (css, "stroke-linecap", cap);
            for (r = reprs; r != NULL; r = r->next) {
                sp_repr_css_change_recursive ((SPRepr *) r->data, css, "style");
            }
            sp_stroke_style_set_cap_buttons (spw, GTK_WIDGET (tb));
        } else if (start_marker) {
        
            /* Update the start marker style value based on what's set in 
            * the widget 
            */
                
            if (start_marker[0] != '\0') {
                sp_stroke_style_ensure_marker (start_marker);
            }

            sp_repr_css_set_property (css, "marker-start", start_marker);
            
            for (r = reprs; r != NULL; r = r->next) {
                    sp_repr_css_change_recursive ( (SPRepr *) r->data, css, 
                                                   "style" );
            }
            sp_stroke_style_set_marker_buttons (spw, GTK_WIDGET (tb), 
                                                 INKSCAPE_STOCK_START_MARKER);
        
        } else if (mid_marker) {
                sp_repr_css_set_property (css, "marker-mid", mid_marker);
            for (r = reprs; r != NULL; r = r->next) {
                    sp_repr_css_change_recursive ( (SPRepr *) r->data, css, 
                                                   "style" );
            }
            sp_stroke_style_set_marker_buttons ( spw, GTK_WIDGET (tb), 
                                                 INKSCAPE_STOCK_MID_MARKER );

            if (mid_marker[0] != '\0') {
                sp_stroke_style_ensure_marker (mid_marker);
            }

        } else if (end_marker) {
            
            sp_repr_css_set_property (css, "marker-end", end_marker);
                
            for (r = reprs; r != NULL; r = r->next) {
                    sp_repr_css_change_recursive ( (SPRepr *) r->data, css, 
                                                   "style" );
            }
            sp_stroke_style_set_marker_buttons ( spw, GTK_WIDGET (tb), 
                                                 INKSCAPE_STOCK_END_MARKER );

            if (end_marker[0] != '\0') {
                sp_stroke_style_ensure_marker (end_marker);
            }
        }

        sp_repr_css_attr_unref (css);
        
        if (spw->inkscape) {
            sp_document_done (SP_WIDGET_DOCUMENT (spw));
        }

        g_slist_free (reprs);
    }
    
} // end of sp_stroke_style_any_toggled()



/* Helpers */



static void
sp_stroke_style_get_average_color_rgba (const GSList *objects, gfloat *c)
{
    gint num;

    c[0] = 0.0;
    c[1] = 0.0;
    c[2] = 0.0;
    c[3] = 0.0;
    num = 0;

    while (objects) {
        SPObject *object;
        gfloat d[3];
        object = SP_OBJECT (objects->data);
        if (object->style->stroke.type == SP_PAINT_TYPE_COLOR) {
            sp_color_get_rgb_floatv (&object->style->stroke.value.color, d);
            c[0] += d[0];
            c[1] += d[1];
            c[2] += d[2];
            c[3] += SP_SCALE24_TO_FLOAT (object->style->stroke_opacity.value);
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
sp_stroke_style_get_average_color_cmyka (const GSList *objects, gfloat *c)
{
    gint num;

    c[0] = 0.0;
    c[1] = 0.0;
    c[2] = 0.0;
    c[3] = 0.0;
    c[4] = 0.0;
    num = 0;

    while (objects) {
        SPObject *object;
        gfloat d[4];
        object = SP_OBJECT (objects->data);
        if (object->style->stroke.type == SP_PAINT_TYPE_COLOR) {
            sp_color_get_cmyk_floatv (&object->style->stroke.value.color, d);
            c[0] += d[0];
            c[1] += d[1];
            c[2] += d[2];
            c[3] += d[3];
            c[4] += SP_SCALE24_TO_FLOAT (object->style->stroke_opacity.value);
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
sp_stroke_style_determine_paint_selector_mode (SPStyle *style)
{
    SPColorSpaceType cstype;

    switch (style->stroke.type) {
    case SP_PAINT_TYPE_NONE:
        return SP_PAINT_SELECTOR_MODE_NONE;
    case SP_PAINT_TYPE_COLOR:
        cstype = sp_color_get_colorspace_type (&style->stroke.value.color);
        switch (cstype) {
        case SP_COLORSPACE_TYPE_RGB:
            return SP_PAINT_SELECTOR_MODE_COLOR_RGB;
        case SP_COLORSPACE_TYPE_CMYK:
            return SP_PAINT_SELECTOR_MODE_COLOR_CMYK;
        default:
            g_warning ( "file %s: line %d: Unknown colorspace type %d", 
                        __FILE__, __LINE__, cstype );
            return SP_PAINT_SELECTOR_MODE_NONE;
        }
    case SP_PAINT_TYPE_PAINTSERVER:
        if (SP_IS_LINEARGRADIENT (SP_STYLE_STROKE_SERVER (style))) {
            return SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR;
        } else if (SP_IS_RADIALGRADIENT (SP_STYLE_STROKE_SERVER (style))) {
            return SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL;
        }
        return SP_PAINT_SELECTOR_MODE_NONE;
    default:
        g_warning ( "file %s: line %d: Unknown paint type %d", 
                    __FILE__, __LINE__, style->stroke.type );
        break;
    }

    return SP_PAINT_SELECTOR_MODE_NONE;
    
} // end of sp_stroke_style_determine_paint_selector_mode()



static void
sp_stroke_style_set_join_buttons (SPWidget *spw, GtkWidget *active)
{
    GtkWidget *tb;

    tb = GTK_WIDGET(gtk_object_get_data ( GTK_OBJECT (spw), 
                                          INKSCAPE_STOCK_JOIN_MITER) );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tb), (active == tb));

    GtkWidget *ml = GTK_WIDGET (g_object_get_data (G_OBJECT (spw), "miterlimit_sb"));
    gtk_widget_set_sensitive (ml, (active == tb));

    tb = GTK_WIDGET(gtk_object_get_data ( GTK_OBJECT (spw), 
                                          INKSCAPE_STOCK_JOIN_ROUND) );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tb), (active == tb));
    tb = GTK_WIDGET(gtk_object_get_data ( GTK_OBJECT (spw), 
                                          INKSCAPE_STOCK_JOIN_BEVEL) );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tb), (active == tb));
}



static void
sp_stroke_style_set_cap_buttons (SPWidget *spw, GtkWidget *active)
{
    GtkWidget *tb;

    tb = GTK_WIDGET(gtk_object_get_data ( GTK_OBJECT (spw), 
                                          INKSCAPE_STOCK_CAP_BUTT));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tb), (active == tb));
    tb = GTK_WIDGET(gtk_object_get_data ( GTK_OBJECT (spw), 
                                          INKSCAPE_STOCK_CAP_ROUND) );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tb), (active == tb));
    tb = GTK_WIDGET(gtk_object_get_data ( GTK_OBJECT (spw), 
                                          INKSCAPE_STOCK_CAP_SQUARE) );
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tb), (active == tb));
}


/**
* Creates a set of marker buttons.  This routine creates togglebuttons for the
* line markers.  Currently it provides just three options - none, filled or
* hollow arrows.  This is intended only as a quicky way to get proof of concept
* arrowhead functionality and is intended to be replaced by a more powerful and
* flexible system later on.
* 
* spw - the widget to put the buttons onto.
* active - the currently selected button.
*
*/
static void
sp_stroke_style_set_marker_buttons (SPWidget *spw, GtkWidget *active, const gchar *marker_name)
{
    /* Set up the various xpm's as an array so that new kinds of markers 
     * can be added without having to cut and paste the code itself.
     */
    gchar const * const suffixes[] = {
        INKSCAPE_STOCK_MARKER_NONE,
        INKSCAPE_STOCK_MARKER_FILLED_ARROW,
        INKSCAPE_STOCK_MARKER_HOLLOW_ARROW
    };

    for (unsigned i = 0; i < G_N_ELEMENTS(suffixes); ++i) {
        gchar * const marker_xpm = g_strconcat(marker_name, suffixes[i], NULL);
        GtkWidget *tb = GTK_WIDGET(gtk_object_get_data(GTK_OBJECT(spw), marker_xpm));
        g_assert( tb != NULL );
        if ( tb != NULL ) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tb), ( active == tb ));
        }
        g_free(marker_xpm);
    }
}

static void
sp_stroke_style_update_marker_menus ( SPWidget *spw,
                                        const GSList *objects)
{
      SPObject *object = SP_OBJECT(objects->data);
    if (object->style->marker[SP_MARKER_LOC_START].value != NULL){
        GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(spw), "start_mark_menu");

        if (gtk_object_get_data (GTK_OBJECT (mnu), "update"))  return;
        gtk_object_set_data (GTK_OBJECT (mnu), "update", GINT_TO_POINTER (TRUE));

        GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu (mnu));
        gchar *markname = ink_extract_marker_name(object->style->marker[SP_MARKER_LOC_START].value);

        int markpos = 0;
        GList *kids = GTK_MENU_SHELL(m)->children;
        int i=0;
        for (; kids != NULL; kids = kids->next){
             gchar *mark = (gchar *) g_object_get_data (G_OBJECT(kids->data), "marker");
             if (strcmp(mark,markname)==0) markpos =i;
             i++;
        }

        gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), markpos);
        gtk_object_set_data (GTK_OBJECT (mnu), "update", GINT_TO_POINTER (FALSE));
    }else {
         GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(spw), "start_mark_menu");
         gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);
    }
    if (object->style->marker[SP_MARKER_LOC_MID].value != NULL){
        GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(spw), "mid_mark_menu");

        if (gtk_object_get_data (GTK_OBJECT (mnu), "update"))  return;
        gtk_object_set_data (GTK_OBJECT (mnu), "update", GINT_TO_POINTER (TRUE));

        GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu (mnu));
        gchar *markname = ink_extract_marker_name(object->style->marker[SP_MARKER_LOC_MID].value);
        int markpos = 0;
        GList *kids = GTK_MENU_SHELL(m)->children;
         int i=0;
        for (; kids != NULL; kids = kids->next){
             gchar *mark = (gchar *) g_object_get_data (G_OBJECT(kids->data), "marker");
             if (strcmp(mark,markname)==0) markpos =i;
             i++;
        }

        gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), markpos);
        gtk_object_set_data (GTK_OBJECT (mnu), "update", GINT_TO_POINTER (FALSE));
        }else {
         GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(spw), "mid_mark_menu");
         gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);
    }
    if (object->style->marker[SP_MARKER_LOC_END].value != NULL){
        GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(spw), "end_mark_menu");

        if (gtk_object_get_data (GTK_OBJECT (mnu), "update"))  return;
        gtk_object_set_data (GTK_OBJECT (mnu), "update", GINT_TO_POINTER (TRUE));

        GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu (mnu));
        gchar *markname = ink_extract_marker_name(object->style->marker[SP_MARKER_LOC_END].value);
        int markpos = 0;
        GList *kids = GTK_MENU_SHELL(m)->children;
        int i=0;
        for (; kids != NULL; kids = kids->next){
             gchar *mark = (gchar *) g_object_get_data (G_OBJECT(kids->data), "marker");
             if (strcmp(mark,markname)==0) markpos =i;
             i++;
        }

        gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), markpos);
        gtk_object_set_data (GTK_OBJECT (mnu), "update", GINT_TO_POINTER (FALSE));
        }else {
         GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(spw), "end_mark_menu");
         gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);
    }


}


/**
* Checks the marker style settings for the selected objects and updates the
* toggle buttons accordingly.
*/
static void
sp_stroke_style_update_marker_buttons ( SPWidget *spw, 
                                        const GSList *objects,
                                        unsigned int loc, 
                                        const gchar *stock_type) 
{
    SPObject *object = SP_OBJECT(objects->data);
    unsigned int marker_type_set = object->style->marker[loc].set;
    gchar const *marker_type = object->style->marker[loc].value;

    /* Iterate through the objects and check the styles */
    bool marker_valid = true;
    for (GSList *l = objects->next; l != NULL; l = l->next) {
        SPObject *o = SP_OBJECT(l->data);

        if (marker_type_set != object->style->marker[loc].set) {
            marker_valid = false;
        } else if (marker_type_set && object->style->marker[loc].set) {
            if (g_ascii_strcasecmp (o->style->marker[loc].value, marker_type)) {
            marker_valid = false;
        }
    }
    }

    /* Work out the widget name that would have been used for this typ
    ** of marker.  This doesn't seem very neat, but I suppose this code
    ** will be ripped out when we have a more generic marker system.
    */
    gchar* widget_name = NULL;
    if (marker_type && ! g_ascii_strcasecmp (marker_type, "url(#mTriangle)" ))
        widget_name = g_strconcat (stock_type, INKSCAPE_STOCK_MARKER_FILLED_ARROW, NULL);
    else if (marker_type && ! g_ascii_strcasecmp (marker_type, "url(#mArrow)" ))
        widget_name = g_strconcat (stock_type, INKSCAPE_STOCK_MARKER_HOLLOW_ARROW, NULL );
    else
        widget_name = g_strconcat (stock_type, INKSCAPE_STOCK_MARKER_NONE, NULL);

    GtkWidget *tb;
    if (marker_valid) {
        marker_status("Widget name is '%s'", widget_name);
        tb = GTK_WIDGET(gtk_object_get_data (GTK_OBJECT (spw), widget_name));
    } else {
        tb = NULL;
    }

    sp_stroke_style_update_marker_menus ( spw, objects);

    g_free(widget_name);
    sp_stroke_style_set_marker_buttons (spw, GTK_WIDGET (tb), stock_type);

} // end of sp_stroke_style_update_marker_buttons()


/**
* Ensure that the a given marker type is present in the <def> section.
* \param n Marker type (e.g. url(#mTriangle))
*/
static void
sp_stroke_style_ensure_marker (const gchar* n)
{
    /* Extract the actual name of the link
    ** e.g. get mTriangle from url(#mTriangle).
    */
    gchar* buffer = g_strdup (n);
    buffer[strlen(buffer) - 1] = '\0';

    gchar* name = buffer;
    while (*name != '\0' && *name != '#')
        name++;

    if (*name == '\0')
        return;

    name++;

    if (*name == '\0')
        return;

    /* Read the defs to see if one of these markers is already there */
    SPRepr* defs = SP_OBJECT_REPR (SP_DOCUMENT_DEFS (SP_WIDGET_DOCUMENT (spw)));
    SPRepr* r = defs->children;
    while (r && strcmp( sp_repr_attr(r, "id"), name)) {
        r = r->next;
    }

    if (r == NULL) {
        if (!sp_marker_load_from_svg (name, SP_WIDGET_DOCUMENT (spw) ) ){

            SPRepr* repr = sp_repr_new ("marker");
            sp_repr_add_child (defs, repr, NULL);
            sp_repr_unref (repr);

            sp_repr_set_attr(repr, "id", name);
            sp_repr_set_attr(repr, "markerWidth", "12.5");
            sp_repr_set_attr(repr, "markerHeight", "12.5");
            sp_repr_set_attr(repr, "refX", "6.25");
            sp_repr_set_attr(repr, "refY", "6.25");
            sp_repr_set_attr(repr, "orient", "auto");

            SPRepr* path_repr = sp_repr_new ("path");
            sp_repr_add_child (repr, path_repr, NULL);
            sp_repr_unref (path_repr);

            if (strcmp (name, "mArrow") == 0) {
                sp_repr_set_attr (
                    path_repr,
                    "d",
                    "M 0.0068092 6.28628 "
                    "L 12.4563 12.5487 "
                    "C 8.66061 8.58881 8.89012 4.05564 12.2688 0.0725591 "
                    "L 0.0068092 6.28628 z"
                    );
            } else {
                sp_repr_set_attr (
                    path_repr,
                    "d",
                    "M 0.0068092 6.28628 "
                    "L 12.4563 12.5487 "
                    "L 12.2688 0.0725591 "
                    "L 0.0068092 6.28628 z"
                    );
            }
        }
        sp_document_done (SP_WIDGET_DOCUMENT (spw));
    }

    free (buffer);
}


    /* Extract the actual name of the link
    ** e.g. get mTriangle from url(#mTriangle).
    */

static gchar*
ink_extract_marker_name(const gchar *n)
{
    gchar* buffer = g_strdup (n);
    buffer[strlen(buffer) - 1] = '\0';

    gchar* name = buffer;
    while (*name != '\0' && *name != '#')
        name++;

    name++;
    if (*name != '\0' ) return name;

    free(buffer);
    return name;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
