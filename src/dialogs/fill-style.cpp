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

static void sp_fill_style_widget_change_selection   ( SPWidget *spw,
                                                      Inkscape::Selection *selection,
                                                      SPPaintSelector *psel );

static void sp_fill_style_widget_update             ( SPWidget *spw,
                                                      Inkscape::Selection *sel );

static void sp_fill_style_widget_paint_mode_changed ( SPPaintSelector *psel,
                                                      SPPaintSelectorMode mode,
                                                      SPWidget *spw );

static void sp_fill_style_widget_paint_dragged (SPPaintSelector *psel, SPWidget *spw );
static void sp_fill_style_widget_paint_changed (SPPaintSelector *psel, SPWidget *spw );
static void sp_fill_style_widget_fill_rule_activate (GtkWidget *w, SPWidget *spw);
static void sp_fill_style_get_average_color_rgba (GSList const *objects, gfloat *c);
static void sp_fill_style_get_average_color_cmyka (GSList const *objects, gfloat *c);
static SPPaintSelectorMode sp_fill_style_determine_paint_selector_mode ( SPStyle *style );


GtkWidget *
sp_fill_style_widget_new (void)
{
    GtkWidget *spw = sp_widget_new_global (INKSCAPE);

    GtkWidget *vb = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vb);
    gtk_container_add (GTK_CONTAINER (spw), vb);

    GtkWidget *psel = sp_paint_selector_new ();
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

    GtkWidget *hb = gtk_hbox_new (FALSE, 4);
    gtk_widget_show (hb);
    gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
    GtkTooltips *ttips = gtk_tooltips_new ();

    GtkWidget *l = gtk_label_new (_("Fill:"));
    gtk_widget_show (l);
    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);

    GtkWidget *om = gtk_option_menu_new ();
    gtk_widget_show (om);
    gtk_box_pack_start (GTK_BOX (hb), om, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (spw), "fill-rule", om);
    gtk_tooltips_set_tip (ttips, om,
    // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/painting.html#FillRuleProperty
                          _("Specifies the method of filling overlapping areas when an object intersects itself. "
                            "With the \"winding fill\" method (fill-rule:nonzero), all overlapping areas are filled; "
                            "with the \"alternating fill\" method (fill-rule:evenodd), every other of them is filled."), NULL);

    /* 0 - nonzero 1 - evenodd */
    GtkWidget *m = gtk_menu_new ();
    gtk_widget_show (m);

    /* TRANSLATORS: Use the "winding fill" method (fill-rule:nonzero): all overlapping areas are filled. */
    GtkWidget *mi = gtk_menu_item_new_with_label (_("winding"));
    gtk_widget_show (mi);
    gtk_menu_append (GTK_MENU (m), mi);
    g_object_set_data ( G_OBJECT (mi), "fill-rule",
                        (void *)"nonzero" );
    g_signal_connect ( G_OBJECT (mi), "activate",
                       G_CALLBACK (sp_fill_style_widget_fill_rule_activate),
                       spw );
    /* TRANSLATORS: Use the "alternating fill" method (fill-rule:evenodd): areas with odd number of overlaps are filled. */
    mi = gtk_menu_item_new_with_label (_("alternating"));
    gtk_widget_show (mi);
    gtk_menu_append (GTK_MENU (m), mi);
    g_object_set_data (G_OBJECT (mi), "fill-rule", (void *)"evenodd");
    g_signal_connect ( G_OBJECT (mi), "activate",
                       G_CALLBACK (sp_fill_style_widget_fill_rule_activate),
                       spw );

    gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);

    g_signal_connect ( G_OBJECT (spw), "construct",
                       G_CALLBACK (sp_fill_style_widget_construct), psel);

    g_signal_connect ( G_OBJECT (spw), "modify_selection",
                       G_CALLBACK (sp_fill_style_widget_modify_selection), psel);

    g_signal_connect ( G_OBJECT (spw), "change_selection",
                       G_CALLBACK (sp_fill_style_widget_change_selection), psel);

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

    SPPaintSelector *psel = SP_PAINT_SELECTOR (g_object_get_data ( G_OBJECT (spw),
                                                                   "paint-selector"));

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
            sp_paint_selector_set_mode ( psel,
                                         SP_PAINT_SELECTOR_MODE_COLOR_RGB);
            gfloat c[5];
            sp_fill_style_get_average_color_rgba (objects, c);
            SPColor color;
            sp_color_set_rgb_float (&color, c[0], c[1], c[2]);
            sp_paint_selector_set_color_alpha (psel, &color, c[3]);
            break;
        }

        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            sp_paint_selector_set_mode ( psel,
                                         SP_PAINT_SELECTOR_MODE_COLOR_CMYK);
            gfloat c[5];
            sp_fill_style_get_average_color_cmyka (objects, c);
            SPColor color;
            sp_color_set_cmyk_float (&color, c[0], c[1], c[2], c[3]);
            sp_paint_selector_set_color_alpha (psel, &color, c[4]);
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
                    sp_paint_selector_set_mode ( psel,
                            SP_PAINT_SELECTOR_MODE_MULTIPLE);

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
            SPObject *object = SP_OBJECT (objects->data);

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
                    sp_paint_selector_set_mode ( psel,
                            SP_PAINT_SELECTOR_MODE_MULTIPLE);
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
            SPObject *object = SP_OBJECT (objects->data);

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

    GtkWidget *fillrule = GTK_WIDGET(g_object_get_data (G_OBJECT (spw), "fill-rule"));
    gtk_option_menu_set_history ( GTK_OPTION_MENU (fillrule),
            (SP_OBJECT_STYLE
                (object)->fill_rule.computed == ART_WIND_RULE_NONZERO) ? 0 : 1);

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


/**
This is called repeatedly while you are dragging a color slider or a gradient node. Therefore it
must not update repr (for efficiency).
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
#ifdef SP_FS_VERBOSE
    g_print ("FillStyleWidget: paint dragged\n");
#endif
    switch (psel->mode) {
        case SP_PAINT_SELECTOR_MODE_EMPTY:
        case SP_PAINT_SELECTOR_MODE_MULTIPLE:
        case SP_PAINT_SELECTOR_MODE_NONE:
            g_warning ( "file %s: line %d: Paint %d should not emit 'dragged'",
                        __FILE__, __LINE__, psel->mode );
            break;

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            SPColor color;
            gfloat alpha;
            sp_paint_selector_get_color_alpha (psel, &color, &alpha);
            GSList const *items = sp_widget_get_item_list(spw);
            for (GSList const *i = items; i != NULL; i = i->next) {
                sp_style_set_fill_color_alpha ( SP_OBJECT_STYLE (i->data),
                                                &color, alpha, TRUE, TRUE);
            }
            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_LINEAR:
        {
            // gradient in fill&stroke does not drag anymore...
            break;
        }

        case SP_PAINT_SELECTOR_MODE_GRADIENT_RADIAL:
        {
            // gradient in fill&stroke does not drag anymore...
            break;
        }

        default:
            g_warning ( "file %s: line %d: Paint selector should not be in "
                        "mode %d",
                        __FILE__, __LINE__, psel->mode);
            break;

    } // end of switch

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));

} // end of sp_fill_style_widget_paint_dragged()


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

#ifdef SP_FS_VERBOSE
    g_print ("FillStyleWidget: paint changed\n");
#endif

    GSList *reprs = NULL;
    GSList const *items = NULL;
    if (spw->inkscape) {
        /* fixme: */
        if (!SP_WIDGET_DOCUMENT (spw)) {
            g_object_set_data ( G_OBJECT (spw), "update",
                                GINT_TO_POINTER (FALSE) );
            return;
        }
        items = sp_widget_get_item_list (spw);
        for (GSList const *i = items; i != NULL; i = i->next) {
            reprs = g_slist_prepend (reprs, SP_OBJECT_REPR (i->data));
        }
    } 

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

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
            if (spw->inkscape) {
                sp_document_done (SP_WIDGET_DOCUMENT (spw));
            }
            break;
        }

        case SP_PAINT_SELECTOR_MODE_COLOR_RGB:
        {
            SPCSSAttr *css = sp_repr_css_attr_new ();

            SPColor color;
            gfloat alpha;
            sp_paint_selector_get_color_alpha (psel, &color, &alpha);
            guint32 rgba = sp_color_get_rgba32_falpha (&color, alpha);

            gchar b[64];
            sp_svg_write_color (b, 64, rgba);

            sp_repr_css_set_property (css, "fill", b);
            Inkscape::SVGOStringStream osalpha;
            osalpha << alpha;
            sp_repr_css_set_property (css, "fill-opacity", osalpha.str().c_str());

            sp_desktop_set_style (desktop, css);

            sp_repr_css_attr_unref (css);
            if (spw->inkscape) {
                sp_document_done (SP_WIDGET_DOCUMENT (spw));
            }
            break;
        }

        case SP_PAINT_SELECTOR_MODE_COLOR_CMYK:
        {
            SPCSSAttr *css = sp_repr_css_attr_new ();
            SPColor color;
            gfloat alpha;
            sp_paint_selector_get_color_alpha (psel, &color, &alpha);
            guint32 rgba = sp_color_get_rgba32_falpha (&color, alpha);
            gchar b[64];
            sp_svg_write_color (b, 64, rgba);
            sp_repr_css_set_property (css, "fill", b);
            Inkscape::SVGOStringStream osalpha;
            osalpha << alpha;
            sp_repr_css_set_property (css, "fill-opacity", osalpha.str().c_str());

            sp_desktop_set_style (desktop, css);

            sp_repr_css_attr_unref (css);
            if (spw->inkscape) {
                sp_document_done (SP_WIDGET_DOCUMENT (spw));
            }
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
                        vector = sp_document_default_gradient_vector(SP_WIDGET_DOCUMENT(spw), common_rgb);
                    }

                    for (GSList const *i = items; i != NULL; i = i->next) {
                        //FIXME: see above
                        sp_repr_css_change_recursive(SP_OBJECT_REPR(i->data), css, "style");

                        if (common_rgb == DIFFERENT_COLORS) {
                            vector = sp_gradient_vector_for_object(SP_WIDGET_DOCUMENT(spw), desktop, SP_OBJECT(i->data), true);
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

                sp_document_done (SP_WIDGET_DOCUMENT (spw));
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

                    for (GSList const *i = items; i != NULL; i = i->next) {
                         Inkscape::XML::Node *selrepr = SP_OBJECT_REPR (i->data);
                         SPObject *selobj = SP_OBJECT (i->data);
                         if (!selrepr)
                             continue;

                         SPStyle *style = SP_OBJECT_STYLE (selobj);
                         if (style && style->fill.type == SP_PAINT_TYPE_PAINTSERVER) {
                             SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (selobj);
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

        case SP_PAINT_SELECTOR_MODE_UNSET:
            if (items) {
                    SPCSSAttr *css = sp_repr_css_attr_new ();
                    sp_repr_css_unset_property (css, "fill");

                    sp_desktop_set_style (desktop, css);
                    sp_repr_css_attr_unref (css);

                    sp_document_done (SP_WIDGET_DOCUMENT (spw));
            }
            break;

        default:
            g_warning ( "file %s: line %d: Paint selector should not be in "
                        "mode %d",
                        __FILE__, __LINE__, psel->mode );
            break;
    }

    g_slist_free (reprs);

    g_object_set_data (G_OBJECT (spw), "update", GINT_TO_POINTER (FALSE));


} // end of sp_fill_style_widget_paint_changed()



static void
sp_fill_style_widget_fill_rule_activate (GtkWidget *w, SPWidget *spw)
{
    if (g_object_get_data (G_OBJECT (spw), "update"))
        return;

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    SPCSSAttr *css = sp_repr_css_attr_new ();
    sp_repr_css_set_property ( css, "fill-rule",
                               (gchar const *)g_object_get_data (G_OBJECT (w),
                               "fill-rule") );

    sp_desktop_set_style (desktop, css);

    sp_repr_css_attr_unref (css);

    if (spw->inkscape) {
        sp_document_done (SP_WIDGET_DOCUMENT (spw));
    }

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



static void
sp_fill_style_get_average_color_cmyka(GSList const *objects, gfloat *c)
{
    c[0] = 0.0;
    c[1] = 0.0;
    c[2] = 0.0;
    c[3] = 0.0;
    c[4] = 0.0;
    gint num = 0;

    while (objects) {
        gfloat d[4];
        SPObject *object = SP_OBJECT (objects->data);
        if (object->style->fill.type == SP_PAINT_TYPE_COLOR) {
            sp_color_get_cmyk_floatv (&object->style->fill.value.color, d);
            c[0] += d[0];
            c[1] += d[1];
            c[2] += d[2];
            c[3] += d[3];
            c[4] += SP_SCALE24_TO_FLOAT (object->style->fill_opacity.value);
        }
        num += 1;
        objects = objects->next;
    }

    c[0] /= num;
    c[1] /= num;
    c[2] /= num;
    c[3] /= num;
    c[4] /= num;

} // end of sp_fill_style_get_average_color_cmyka()




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
            SPColorSpaceType cstype = sp_color_get_colorspace_type (&style->fill.value.color);

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
