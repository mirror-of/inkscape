#define __SP_FILL_STYLE_C__

/**
 * \brief  Fill style widget
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_FS_VERBOSE

#include <config.h>

#include <string.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmisc.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtktooltips.h>

#include <libnr/nr-values.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-path.h>

#include <glibmm/i18n.h>
#include <helper/window.h>
#include <svg/svg.h>
#include <svg/stringstream.h>
#include <widgets/sp-widget.h>
#include <sp-gradient.h>
#include <sp-linear-gradient.h>
#include <sp-pattern.h>
#include <sp-radial-gradient.h>
#include <sp-use.h>
#include <widgets/paint-selector.h>
#include <style.h>
#include <gradient-chemistry.h>
#include "common-style.h"
#include <document.h>
#include <desktop-style.h>
#include <desktop-handles.h>
#include <selection.h>
#include <sp-item.h>
#include <inkscape.h>
#include <document-private.h>
#include <file.h>


#include "fill-style.h"

// These can be deleted once we sort out the libart dependence.

#define ART_WIND_RULE_NONZERO 0

static void sp_fill_style_widget_construct          ( SPWidget *spw,
                                                      SPPaintSelector *psel );

static void sp_fill_style_widget_modify_selection   ( SPWidget *spw,
                                                      Inkscape::Selection *selection,
                                                      guint flags,
                                                      SPPaintSelector *psel );
static void
sp_fill_style_widget_change_subselection ( Inkscape::Application *inkscape, 
                                        SPDesktop *desktop,
                                           SPWidget *spw );

static void sp_fill_style_widget_change_selection   ( SPWidget *spw,
                                                      Inkscape::Selection *selection,
                                                      SPPaintSelector *psel );

static void sp_fill_style_widget_update             ( SPWidget *spw,
                                                      Inkscape::Selection *sel );

static void sp_fill_style_widget_paint_mode_changed ( SPPaintSelector *psel,
                                                      SPPaintSelectorMode mode,
                                                      SPWidget *spw );
static void sp_fill_style_widget_fillrule_changed ( SPPaintSelector *psel,
                                          SPPaintSelectorFillRule mode,
                                                    SPWidget *spw );

static void sp_fill_style_widget_paint_dragged (SPPaintSelector *psel, SPWidget *spw );
static void sp_fill_style_widget_paint_changed (SPPaintSelector *psel, SPWidget *spw );
static void sp_fill_style_get_average_color_rgba (GSList const *objects, gfloat *c);
static SPPaintSelectorMode sp_fill_style_determine_paint_selector_mode ( SPStyle *style );


GtkWidget *
sp_fill_style_widget_new (void)
{
    GtkWidget *spw = sp_widget_new_global (INKSCAPE);

    GtkWidget *vb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vb);
    gtk_container_add (GTK_CONTAINER (spw), vb);

    GtkWidget *psel = sp_paint_selector_new (true); // with fillrule selector
    gtk_widget_show (psel);
    gtk_box_pack_start (GTK_BOX (vb), psel, TRUE, TRUE, 0);
    g_object_set_data (G_OBJECT (spw), "paint-selector", psel);

    g_signal_connect ( G_OBJECT (psel), "mode_changed",
                       G_CALLBACK (sp_fill_style_widget_paint_mode_changed),
                       spw );

    g_signal_connect ( G_OBJECT (psel), "dragged",
                       G_CALLBACK (sp_fill_style_widget_paint_dragged),
                       spw );

    g_signal_connect ( G_OBJECT (psel), "changed",
                       G_CALLBACK (sp_fill_style_widget_paint_changed),
                       spw );

    g_signal_connect ( G_OBJECT (psel), "fillrule_changed",
                       G_CALLBACK (sp_fill_style_widget_fillrule_changed),
                       spw );


    g_signal_connect ( G_OBJECT (spw), "construct",
                       G_CALLBACK (sp_fill_style_widget_construct), psel);

//FIXME: switch these from spw signals to global inkscape object signals; spw just retranslates
//those anyway; then eliminate spw
    g_signal_connect ( G_OBJECT (spw), "modify_selection",
                       G_CALLBACK (sp_fill_style_widget_modify_selection), psel);

    g_signal_connect ( G_OBJECT (spw), "change_selection",
                       G_CALLBACK (sp_fill_style_widget_change_selection), psel);

    g_signal_connect (INKSCAPE, "change_subselection", G_CALLBACK (sp_fill_style_widget_change_subselection), spw);

    sp_fill_style_widget_update (SP_WIDGET (spw),
        SP_ACTIVE_DESKTOP ? SP_DT_SELECTION (SP_ACTIVE_DESKTOP) : NULL);

    return spw;

} // end of sp_fill_style_widget_new()



static void
sp_fill_style_widget_construct ( SPWidget *spw, SPPaintSelector *psel )
{

#ifdef SP_FS_VERBOSE
    g_print ( "Fill style widget constructed: inkscape %p repr %p\n",
              spw->inkscape, spw->repr );
#endif
    if (spw->inkscape) {

        sp_fill_style_widget_update ( spw,
            SP_ACTIVE_DESKTOP ? SP_DT_SELECTION (SP_ACTIVE_DESKTOP) : NULL );

    } 

} // end of sp_fill_style_widget_construct()

static void
sp_fill_style_widget_modify_selection ( SPWidget *spw,
                                        Inkscape::Selection *selection,
                                        guint flags,
                                        SPPaintSelector *psel )
{
    if (flags & ( SP_OBJECT_MODIFIED_FLAG |
                  SP_OBJECT_PARENT_MODIFIED_FLAG |
                  SP_OBJECT_STYLE_MODIFIED_FLAG) )
    {
        sp_fill_style_widget_update (spw, selection);
    }
}

static void
sp_fill_style_widget_change_subselection ( Inkscape::Application *inkscape, 
                                        SPDesktop *desktop,
                                        SPWidget *spw )
{
    sp_fill_style_widget_update (spw, SP_DT_SELECTION(desktop));
}

static void
sp_fill_style_widget_change_selection ( SPWidget *spw,
                                        Inkscape::Selection *selection,
                                        SPPaintSelector *psel )
{
    sp_fill_style_widget_update (spw, selection);
}

/**
* \param sel Selection to use, or NULL.
*/
static void
sp_fill_style_widget_update ( SPWidget *spw, Inkscape::Selection *sel )
{
    if (g_object_get_data (G_OBJECT (spw), "update"))
        return;

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    SPPaintSelector *psel = SP_PAINT_SELECTOR (g_object_get_data (G_OBJECT (spw), "paint-selector"));

    // create temporary style
    SPStyle *query = sp_style_new ();
    // query into it
    int result = sp_desktop_query_style (SP_ACTIVE_DESKTOP, query, QUERY_STYLE_PROPERTY_FILLSTROKE); 

    if (result) { 
// for now this is mostly a duplication of the else branch; eventually the else will be eliminated,
// after sp_desktop_query_style can query selection (for now it only works for subselections which implement it)
        SPPaintSelectorMode pselmode = sp_fill_style_determine_paint_selector_mode (query);
        sp_paint_selector_set_mode (psel, pselmode);

        sp_paint_selector_set_fillrule (psel, query->fill_rule.computed == ART_WIND_RULE_NONZERO? 
                                        SP_PAINT_SELECTOR_FILLRULE_NONZERO : SP_PAINT_SELECTOR_FILLRULE_EVENODD);

        if (query->fill.type == SP_PAINT_TYPE_COLOR) {
            gfloat d[3];
            sp_color_get_rgb_floatv (&query->fill.value.color, d);
            SPColor color;
            sp_color_set_rgb_float (&color, d[0], d[1], d[2]);
            sp_paint_selector_set_color_alpha (psel, &color, SP_SCALE24_TO_FLOAT (query->fill_opacity.value));
        }

    } else { 

    if ( !sel || sel->isEmpty() ) {
        /* No objects, set empty */
        sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_EMPTY);
        g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));
        return;
    }

    GSList const *objects = sel->itemList();
    SPObject *object = SP_OBJECT (objects->data);

    // prevent trying to modify objects with multiple fill modes
    SPPaintSelectorMode pselmode =
        sp_fill_style_determine_paint_selector_mode(SP_OBJECT_STYLE (object));

    for (GSList const *l = objects->next; l != NULL; l = l->next) {
        SPPaintSelectorMode nextmode =
            sp_fill_style_determine_paint_selector_mode(SP_OBJECT_STYLE (l->data));

        if (nextmode != pselmode) {
            /* Multiple styles */
            sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_MULTIPLE);
            g_object_set_data ( G_OBJECT (spw), "update",
                                GINT_TO_POINTER (FALSE));
            return;
        }
    }

#ifdef SP_FS_VERBOSE
    g_print ("FillStyleWidget: paint selector mode %d\n", pselmode);
#endif

    switch (pselmode) {

        case SP_PAINT_SELECTOR_MODE_NONE:
            /* No paint at all */
            sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_NONE);
            break;

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        {
            sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_COLOR_RGB);
            gfloat c[5];
            sp_fill_style_get_average_color_rgba (objects, c);
            SPColor color;
            sp_color_set_rgb_float (&color, c[0], c[1], c[2]);
            sp_paint_selector_set_color_alpha (psel, &color, c[3]);
            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
        {
            SPObject *object = SP_OBJECT (objects->data);
            /* We know that all objects have lineargradient fill style */
            SPGradient *vector =
                sp_gradient_get_vector ( SP_GRADIENT
                                            (SP_OBJECT_STYLE_FILL_SERVER
                                                (object)),
                                         FALSE );

            for (GSList const *l = objects->next; l != NULL; l = l->next) {
                SPObject const *next = SP_OBJECT(l->data);

                if (sp_gradient_get_vector ( SP_GRADIENT
                                             (SP_OBJECT_STYLE_FILL_SERVER
                                                 (next)),
                                             FALSE) != vector )
                {
                    /* Multiple vectors */
                    sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_MULTIPLE);

                    g_object_set_data ( G_OBJECT (spw), "update",
                                        GINT_TO_POINTER (FALSE));
                    return;

                } // end of if

            } // end of for()

            sp_paint_selector_set_gradient_linear (psel, vector);

            SPLinearGradient *lg = SP_LINEARGRADIENT (SP_OBJECT_STYLE_FILL_SERVER (object));
            sp_paint_selector_set_gradient_properties (psel,
                                                       SP_GRADIENT_UNITS (lg),
                                                       SP_GRADIENT_SPREAD (lg));
            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
        {
            /* We know that all objects have radialgradient fill style */
            SPGradient *vector =
                sp_gradient_get_vector ( SP_GRADIENT
                                            (SP_OBJECT_STYLE_FILL_SERVER
                                                (object)),
                                         FALSE );

            for (GSList const *l = objects->next; l != NULL; l = l->next) {
                SPObject const *next = SP_OBJECT(l->data);
                if (sp_gradient_get_vector ( SP_GRADIENT
                                                (SP_OBJECT_STYLE_FILL_SERVER
                                                    (next)),
                                             FALSE) != vector )
                {
                    /* Multiple vectors */
                    sp_paint_selector_set_mode (psel, SP_PAINT_SELECTOR_MODE_MULTIPLE);
                    g_object_set_data ( G_OBJECT (spw),
                                        "update", GINT_TO_POINTER (FALSE) );
                    return;
                }

            } // end of for loop

            sp_paint_selector_set_gradient_radial (psel, vector);

            SPRadialGradient *rg = SP_RADIALGRADIENT (SP_OBJECT_STYLE_FILL_SERVER (object));
            sp_paint_selector_set_gradient_properties (psel,
                                                       SP_GRADIENT_UNITS (rg),
                                                       SP_GRADIENT_SPREAD (rg));

            break;
        }

        case SP_PAINT_SELECTOR_MODE_PATTERN:
        {
            sp_paint_selector_set_mode ( psel, SP_PAINT_SELECTOR_MODE_PATTERN );

            SPPattern *pat = pattern_getroot (SP_PATTERN (SP_OBJECT_STYLE_FILL_SERVER (object)));
            sp_update_pattern_list ( psel, pat);

            break;
        }

        case SP_PAINT_SELECTOR_MODE_UNSET:
        {
            sp_paint_selector_set_mode ( psel, SP_PAINT_SELECTOR_MODE_UNSET );
            break;
        }

        default:
            sp_paint_selector_set_mode ( psel, SP_PAINT_SELECTOR_MODE_MULTIPLE );
            break;

    } // end of switch

    sp_paint_selector_set_fillrule (psel, SP_OBJECT_STYLE(object)->fill_rule.computed == ART_WIND_RULE_NONZERO? 
        SP_PAINT_SELECTOR_FILLRULE_NONZERO : SP_PAINT_SELECTOR_FILLRULE_EVENODD);

    }

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));

} // end of sp_fill_style_widget_update()


static void
sp_fill_style_widget_paint_mode_changed ( SPPaintSelector *psel,
                                          SPPaintSelectorMode mode,
                                          SPWidget *spw )
{
    if (g_object_get_data (G_OBJECT (spw), "update"))
        return;

    /* TODO: Does this work? */
    /* TODO: Not really, here we have to get old color back from object */
    /* Instead of relying on paint widget having meaningful colors set */
    sp_fill_style_widget_paint_changed (psel, spw);
}

static void
sp_fill_style_widget_fillrule_changed ( SPPaintSelector *psel,
                                          SPPaintSelectorFillRule mode,
                                          SPWidget *spw )
{
    if (g_object_get_data (G_OBJECT (spw), "update"))
        return;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_set_property (css, "fill-rule", mode == SP_PAINT_SELECTOR_FILLRULE_EVENODD? "evenodd":"nonzero");

    sp_desktop_set_style (desktop, css);

    sp_repr_css_attr_unref (css);

    sp_document_done (SP_ACTIVE_DOCUMENT);
}

static gchar *undo_label_1 = "fill:flatcolor:1";
static gchar *undo_label_2 = "fill:flatcolor:2";
static gchar *undo_label = undo_label_1;

/**
This is called repeatedly while you are dragging a color slider, only for flat color
modes. Previously it set the color in style but did not update the repr for efficiency, however
this was flakey and didn't buy us almost anything. So now it does the same as _changed, except
lumps all its changes for undo.
 */
static void
sp_fill_style_widget_paint_dragged (SPPaintSelector *psel, SPWidget *spw)
{
    if (!spw->inkscape) {
        return;
    }

    if (g_object_get_data (G_OBJECT (spw), "update")) {
        return;
    }
    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    switch (psel->mode) {

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            sp_paint_selector_set_flat_color (psel, SP_ACTIVE_DESKTOP, "fill", "fill-opacity");
            sp_document_maybe_done (SP_DT_DOCUMENT(SP_ACTIVE_DESKTOP), undo_label);
            break;
        }

        default:
            g_warning ( "file %s: line %d: Paint %d should not emit 'dragged'",
                        __FILE__, __LINE__, psel->mode );
            break;

    }
    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));
}


/**
This is called (at least) when:
1  paint selector mode is switched (e.g. flat color -> gradient)
2  you finished dragging a gradient node and released mouse
3  you changed a gradient selector parameter (e.g. spread)
Must update repr.
 */
static void
sp_fill_style_widget_paint_changed ( SPPaintSelector *psel,
                                     SPWidget *spw )
{
    if (g_object_get_data (G_OBJECT (spw), "update")) {
        return;
    }
    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (TRUE));

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        return;
    }
    SPDocument *document = SP_DT_DOCUMENT (desktop);
    Inkscape::Selection *selection = SP_DT_SELECTION (desktop);

    GSList const *items = selection->itemList();

    switch (psel->mode) {

        case SP_PAINT_SELECTOR_MODE_EMPTY:
            // This should not happen.
            g_warning ( "file %s: line %d: Paint %d should not emit 'changed'",
                        __FILE__, __LINE__, psel->mode);
            break;
        case SP_PAINT_SELECTOR_MODE_MULTIPLE:
            // This happens when you switch multiple objects with different gradients to flat color;
            // nothing to do here.
            break;

        case SP_PAINT_SELECTOR_MODE_NONE:
        {
            SPCSSAttr *css = sp_repr_css_attr_new ();
            sp_repr_css_set_property (css, "fill", "none");

            sp_desktop_set_style (desktop, css);

            sp_repr_css_attr_unref (css);

            sp_document_done (document);
            break;
        }

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            sp_paint_selector_set_flat_color (psel, desktop, "fill", "fill-opacity");
            sp_document_maybe_done (SP_DT_DOCUMENT(desktop), undo_label);

            // on release, toggle undo_label so that the next drag will not be lumped with this one
            if (undo_label == undo_label_1)
                undo_label = undo_label_2;
            else 
                undo_label = undo_label_1;

            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
            if (items) {
                SPGradientType const gradient_type = ( psel->mode == SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR
                                                       ? SP_GRADIENT_TYPE_LINEAR
                                                       : SP_GRADIENT_TYPE_RADIAL );

                // HACK: reset fill-opacity - that 0.75 is annoying; BUT remove this when we have an opacity slider for all tabs
                SPCSSAttr *css = sp_repr_css_attr_new();
                sp_repr_css_set_property(css, "fill-opacity", "1.0");

                SPGradient *vector = sp_paint_selector_get_gradient_vector(psel);
                if (!vector) {
                    /* No vector in paint selector should mean that we just
                     * changed mode
                     */

                    guint32 common_rgb = objects_get_common_rgb(items, FILL);
                    if (common_rgb != DIFFERENT_COLORS) {
                        if (common_rgb == NO_COLOR) {
                            common_rgb = sp_desktop_get_color(desktop, true);
                        }
                        vector = sp_document_default_gradient_vector(document, common_rgb);
                    }

                    for (GSList const *i = items; i != NULL; i = i->next) {
                        //FIXME: see above
                        sp_repr_css_change_recursive(SP_OBJECT_REPR(i->data), css, "style");

                        if (common_rgb == DIFFERENT_COLORS) {
                            vector = sp_gradient_vector_for_object(document, desktop, SP_OBJECT(i->data), true);
                        }

                        sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, true);
                    }
                } else {
                    /* We have changed from another gradient type, or modified spread/units within
                     * this gradient type. */
                    vector = sp_gradient_ensure_vector_normalized (vector);
                    for (GSList const *i = items; i != NULL; i = i->next) {
                        //FIXME: see above
                        sp_repr_css_change_recursive (SP_OBJECT_REPR (i->data), css, "style");

                        SPGradient *gr = sp_item_set_gradient(SP_ITEM(i->data), vector, gradient_type, true);
                        sp_gradient_selector_attrs_to_gradient (gr, psel);
                    }
                }

                sp_document_done (document);
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
                    Inkscape::XML::Node *patrepr = SP_OBJECT_REPR(pattern);
                    SPCSSAttr *css = sp_repr_css_attr_new ();
                    gchar *urltext = g_strdup_printf ("url(#%s)", patrepr->attribute("id"));
                    sp_repr_css_set_property (css, "fill", urltext);

                    // cannot just call sp_desktop_set_style, because we don't want to touch those
                    // objects who already have the same root pattern but through a different href
                    // chain. FIXME: move this to a sp_item_set_pattern
                    for (GSList const *i = items; i != NULL; i = i->next) {
                         SPObject *selobj = SP_OBJECT (i->data);

                         SPStyle *style = SP_OBJECT_STYLE (selobj);
                         if (style && style->fill.type == SP_PAINT_TYPE_PAINTSERVER) {
                             SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (selobj);
                             if (SP_IS_PATTERN (server) && pattern_getroot (SP_PATTERN(server)) == pattern)
                                // only if this object's pattern is not rooted in our selected pattern, apply
                                 continue;
                         }

                         sp_desktop_apply_css_recursive (selobj, css, true);
                     }

                    sp_repr_css_attr_unref (css);
                    g_free (urltext);

                } // end if

                sp_document_done (document);

            } // end if

            break;

        case SP_PAINT_SELECTOR_MODE_UNSET:
            if (items) {
                    SPCSSAttr *css = sp_repr_css_attr_new ();
                    sp_repr_css_unset_property (css, "fill");

                    sp_desktop_set_style (desktop, css);
                    sp_repr_css_attr_unref (css);

                    sp_document_done (document);
            }
            break;

        default:
            g_warning ( "file %s: line %d: Paint selector should not be in "
                        "mode %d",
                        __FILE__, __LINE__, psel->mode );
            break;
    }

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));
}

static void
sp_fill_style_get_average_color_rgba(GSList const *objects, gfloat *c)
{
    c[0] = 0.0;
    c[1] = 0.0;
    c[2] = 0.0;
    c[3] = 0.0;
    gint num = 0;

    while (objects) {
        gfloat d[3];
        SPObject *object = SP_OBJECT (objects->data);
        if (object->style->fill.type == SP_PAINT_TYPE_COLOR) {
            sp_color_get_rgb_floatv (&object->style->fill.value.color, d);
            c[0] += d[0];
            c[1] += d[1];
            c[2] += d[2];
            c[3] += SP_SCALE24_TO_FLOAT (object->style->fill_opacity.value);
        }
        num += 1;
        objects = objects->next;
    }

    c[0] /= num;
    c[1] /= num;
    c[2] /= num;
    c[3] /= num;

} // end of sp_fill_style_get_average_color_rgba()


static SPPaintSelectorMode
sp_fill_style_determine_paint_selector_mode (SPStyle *style)
{
    if (!style->fill.set)
        return SP_PAINT_SELECTOR_MODE_UNSET;

    switch (style->fill.type) {

        case SP_PAINT_TYPE_NONE:
            return SP_PAINT_SELECTOR_MODE_NONE;

        case SP_PAINT_TYPE_COLOR:
        {
            return SP_PAINT_SELECTOR_MODE_COLOR_RGB; // so far only rgb can be read from svg
        }

        case SP_PAINT_TYPE_PAINTSERVER:

            if (SP_IS_LINEARGRADIENT (SP_STYLE_FILL_SERVER (style))) {
                return SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR;
            } else if (SP_IS_RADIALGRADIENT (SP_STYLE_FILL_SERVER (style))) {
                return SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL;
            } else if (SP_IS_PATTERN (SP_STYLE_FILL_SERVER (style))) {
                return SP_PAINT_SELECTOR_MODE_PATTERN;
            }

            g_warning ( "file %s: line %d: Unknown paintserver",
                        __FILE__, __LINE__ );
            return SP_PAINT_SELECTOR_MODE_NONE;

        default:
            g_warning ( "file %s: line %d: Unknown paint type %d",
                        __FILE__, __LINE__, style->fill.type );
            break;
    }

    return SP_PAINT_SELECTOR_MODE_NONE;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
