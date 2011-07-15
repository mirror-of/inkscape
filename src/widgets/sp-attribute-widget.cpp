/** @file
 * @brief Widget that listens and modifies repr attributes
 */
/* Authors:
 *  Lauris Kaplinski <lauris@ximian.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 2001 Ximian, Inc.
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtk.h>
#include "xml/repr.h"
#include "macros.h"
#include "document.h"
#include "sp-object.h"
#include <glibmm/i18n.h>

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "sp-attribute-widget.h"
#include "inkscape.h"
#include <glib.h>

using Inkscape::DocumentUndo;

static void sp_attribute_widget_class_init (SPAttributeWidgetClass *klass);
static void sp_attribute_widget_init (SPAttributeWidget *widget);
static void sp_attribute_widget_destroy (GtkObject *object);

static void sp_attribute_widget_changed (GtkEditable *editable);

static void sp_attribute_widget_object_modified ( SPObject *object,
                                                  guint flags,
                                                  SPAttributeWidget *spaw );
static void sp_attribute_widget_object_release ( SPObject *object,
                                                 SPAttributeWidget *spaw );

static GtkEntryClass *parent_class;




GType sp_attribute_widget_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPAttributeWidgetClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_attribute_widget_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPAttributeWidget),
            0, // n_preallocs
            (GInstanceInitFunc)sp_attribute_widget_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_ENTRY, "SPAttributeWidget", &info, static_cast<GTypeFlags>(0));
    }
    return type;
} // end of sp_attribute_widget_get_type()



static void
sp_attribute_widget_class_init (SPAttributeWidgetClass *klass)
{
    GtkObjectClass *object_class;
    GtkEditableClass *editable_class;

    object_class = GTK_OBJECT_CLASS (klass);
    editable_class = GTK_EDITABLE_CLASS (klass);

    parent_class = (GtkEntryClass*)g_type_class_peek_parent (klass);

    object_class->destroy = sp_attribute_widget_destroy;

    editable_class->changed = sp_attribute_widget_changed;

} // end of sp_attribute_widget_class_init()



static void
sp_attribute_widget_init (SPAttributeWidget *spaw)
{
    spaw->blocked = FALSE;
    spaw->hasobj = FALSE;

    spaw->src.object = NULL;

    spaw->attribute = NULL;

    new (&spaw->modified_connection) sigc::connection();
    new (&spaw->release_connection) sigc::connection();
}



static void
sp_attribute_widget_destroy (GtkObject *object)
{

    SPAttributeWidget *spaw;

    spaw = SP_ATTRIBUTE_WIDGET (object);

    if (spaw->attribute) {
        g_free (spaw->attribute);
        spaw->attribute = NULL;
    }


    if (spaw->hasobj) {

        if (spaw->src.object) {
            spaw->modified_connection.disconnect();
            spaw->release_connection.disconnect();
            spaw->src.object = NULL;
        }
    } else {

        if (spaw->src.repr) {
            spaw->src.repr = Inkscape::GC::release(spaw->src.repr);
        }
    } // end of if()

    spaw->modified_connection.~connection();
    spaw->release_connection.~connection();

    ((GtkObjectClass *) parent_class)->destroy (object);

}



static void
sp_attribute_widget_changed (GtkEditable *editable)
{

    SPAttributeWidget *spaw;

    spaw = SP_ATTRIBUTE_WIDGET (editable);

    if (!spaw->blocked) {

        const gchar *text;
        spaw->blocked = TRUE;
        text = gtk_entry_get_text (GTK_ENTRY (spaw));
        if (!*text)
            text = NULL;

        if (spaw->hasobj && spaw->src.object) {        
            spaw->src.object->getRepr()->setAttribute(spaw->attribute, text, false);
            DocumentUndo::done(spaw->src.object->document, SP_VERB_NONE,
                                _("Set attribute"));

        } else if (spaw->src.repr) {
            spaw->src.repr->setAttribute(spaw->attribute, text, false);
            /* TODO: Warning! Undo will not be flushed in given case */
        }
        spaw->blocked = FALSE;
    }

} // end of sp_attribute_widget_changed()



GtkWidget *
sp_attribute_widget_new ( SPObject *object, const gchar *attribute )
{
    SPAttributeWidget *spaw;

    g_return_val_if_fail (!object || SP_IS_OBJECT (object), NULL);
    g_return_val_if_fail (!object || attribute, NULL);

    spaw = (SPAttributeWidget*)g_object_new (SP_TYPE_ATTRIBUTE_WIDGET, NULL);

    sp_attribute_widget_set_object (spaw, object, attribute);

    return GTK_WIDGET (spaw);

} // end of sp_attribute_widget_new()



GtkWidget *
sp_attribute_widget_new_repr ( Inkscape::XML::Node *repr, const gchar *attribute )
{
    SPAttributeWidget *spaw;

    spaw = (SPAttributeWidget*)g_object_new (SP_TYPE_ATTRIBUTE_WIDGET, NULL);

    sp_attribute_widget_set_repr (spaw, repr, attribute);

    return GTK_WIDGET (spaw);
}



void
sp_attribute_widget_set_object ( SPAttributeWidget *spaw,
                                 SPObject *object,
                                 const gchar *attribute )
{

    g_return_if_fail (spaw != NULL);
    g_return_if_fail (SP_IS_ATTRIBUTE_WIDGET (spaw));
    g_return_if_fail (!object || SP_IS_OBJECT (object));
    g_return_if_fail (!object || attribute);
    g_return_if_fail (attribute != NULL);

    if (spaw->attribute) {
        g_free (spaw->attribute);
        spaw->attribute = NULL;
    }

    if (spaw->hasobj) {

        if (spaw->src.object) {
            spaw->modified_connection.disconnect();
            spaw->release_connection.disconnect();
            spaw->src.object = NULL;
        }
    } else {

        if (spaw->src.repr) {
            spaw->src.repr = Inkscape::GC::release(spaw->src.repr);
        }
    }

    spaw->hasobj = TRUE;

    if (object) {
        const gchar *val;

        spaw->blocked = TRUE;
        spaw->src.object = object;

        spaw->modified_connection = object->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_attribute_widget_object_modified), spaw));
        spaw->release_connection = object->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_attribute_widget_object_release), spaw));

        spaw->attribute = g_strdup (attribute);

        val = object->getRepr()->attribute(attribute);
        gtk_entry_set_text (GTK_ENTRY (spaw), val ? val : (const gchar *) "");
        spaw->blocked = FALSE;
    }

    gtk_widget_set_sensitive (GTK_WIDGET (spaw), (spaw->src.object != NULL));

} // end of sp_attribute_widget_set_object()



void
sp_attribute_widget_set_repr ( SPAttributeWidget *spaw,
                               Inkscape::XML::Node *repr,
                               const gchar *attribute )
{

    g_return_if_fail (spaw != NULL);
    g_return_if_fail (SP_IS_ATTRIBUTE_WIDGET (spaw));
    g_return_if_fail (attribute != NULL);

    if (spaw->attribute) {
        g_free (spaw->attribute);
        spaw->attribute = NULL;
    }

    if (spaw->hasobj) {

        if (spaw->src.object) {
            spaw->modified_connection.disconnect();
            spaw->release_connection.disconnect();
            spaw->src.object = NULL;
        }
    } else {

        if (spaw->src.repr) {
            spaw->src.repr = Inkscape::GC::release(spaw->src.repr);
        }
    }

    spaw->hasobj = FALSE;

    if (repr) {
        const gchar *val;

        spaw->blocked = TRUE;
        spaw->src.repr = Inkscape::GC::anchor(repr);
        spaw->attribute = g_strdup (attribute);

        val = repr->attribute(attribute);
        gtk_entry_set_text (GTK_ENTRY (spaw), val ? val : (const gchar *) "");
        spaw->blocked = FALSE;
    }

    gtk_widget_set_sensitive (GTK_WIDGET (spaw), (spaw->src.repr != NULL));

} // end of sp_attribute_widget_set_repr()



static void
sp_attribute_widget_object_modified ( SPObject */*object*/,
                                      guint flags,
                                      SPAttributeWidget *spaw )
{

    if (flags && SP_OBJECT_MODIFIED_FLAG) {

        const gchar *val, *text;
        val = spaw->src.object->getRepr()->attribute(spaw->attribute);
        text = gtk_entry_get_text (GTK_ENTRY (spaw));

        if (val || text) {

            if (!val || !text || strcmp (val, text)) {
                /* We are different */
                spaw->blocked = TRUE;
                gtk_entry_set_text ( GTK_ENTRY (spaw),
                                     val ? val : (const gchar *) "");
                spaw->blocked = FALSE;
            } // end of if()

        } // end of if()

    } //end of if()

} // end of sp_attribute_widget_object_modified()



static void
sp_attribute_widget_object_release ( SPObject */*object*/,
                                     SPAttributeWidget *spaw )
{
    sp_attribute_widget_set_object (spaw, NULL, NULL);
}



/* SPAttributeTable */

static void sp_attribute_table_class_init (SPAttributeTableClass *klass);
static void sp_attribute_table_init (SPAttributeTable *widget);
static void sp_attribute_table_destroy (GtkObject *object);

static void sp_attribute_table_object_modified (SPObject *object, guint flags, SPAttributeTable *spaw);
static void sp_attribute_table_object_release (SPObject *object, SPAttributeTable *spaw);
static void sp_attribute_table_entry_changed (GtkEditable *editable, SPAttributeTable *spat);

static GtkVBoxClass *table_parent_class;




GType sp_attribute_table_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPAttributeTableClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_attribute_table_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPAttributeTable),
            0, // n_preallocs
            (GInstanceInitFunc)sp_attribute_table_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_VBOX, "SPAttributeTable", &info, static_cast<GTypeFlags>(0));
    }
    return type;
} // end of sp_attribute_table_get_type()



static void
sp_attribute_table_class_init (SPAttributeTableClass *klass)
{
    GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass);

    table_parent_class = (GtkVBoxClass*)g_type_class_peek_parent (klass);

    object_class->destroy = sp_attribute_table_destroy;

} // end of sp_attribute_table_class_init()



static void
sp_attribute_table_init ( SPAttributeTable *spat )
{
    spat->blocked = FALSE;
    spat->hasobj = FALSE;
    spat->table = NULL;
    spat->src.object = NULL;
    spat->num_attr = 0;
    spat->attributes = NULL;
    spat->entries = NULL;

    new (&spat->modified_connection) sigc::connection();
    new (&spat->release_connection) sigc::connection();
}

static void
sp_attribute_table_destroy ( GtkObject *object )
{
    SPAttributeTable *spat;

    spat = SP_ATTRIBUTE_TABLE (object);

    if (spat->attributes) {
        gint i;
        for (i = 0; i < spat->num_attr; i++) {
            g_free (spat->attributes[i]);
        }
        g_free (spat->attributes);
        spat->attributes = NULL;
    }

    if (spat->hasobj) {

        if (spat->src.object) {
            spat->modified_connection.disconnect();
            spat->release_connection.disconnect();
            spat->src.object = NULL;
        }
    } else {
        if (spat->src.repr) {
            spat->src.repr = Inkscape::GC::release(spat->src.repr);
        }
    } // end of if()

    spat->modified_connection.~connection();
    spat->release_connection.~connection();

    if (spat->entries) {
        g_free (spat->entries);
        spat->entries = NULL;
    }

    spat->table = NULL;

    if (((GtkObjectClass *) table_parent_class)->destroy) {
        (* ((GtkObjectClass *) table_parent_class)->destroy) (object);
    }

} // end of sp_attribute_table_destroy()


GtkWidget *
sp_attribute_table_new ( SPObject *object,
                         gint num_attr,
                         const gchar **labels,
                         const gchar **attributes,
                         bool script)
{
    SPAttributeTable *spat;

    g_return_val_if_fail (!object || SP_IS_OBJECT (object), NULL);
    g_return_val_if_fail (!object || (num_attr > 0), NULL);
    g_return_val_if_fail (!num_attr || (labels && attributes), NULL);

    spat = (SPAttributeTable*)g_object_new (SP_TYPE_ATTRIBUTE_TABLE, NULL);

    sp_attribute_table_set_object (spat, object, num_attr, labels, attributes, script);

    return GTK_WIDGET (spat);

} // end of sp_attribute_table_new()



GtkWidget *
sp_attribute_table_new_repr ( Inkscape::XML::Node *repr,
                              gint num_attr,
                              const gchar **labels,
                              const gchar **attributes )
{
    SPAttributeTable *spat;

    g_return_val_if_fail (!num_attr || (labels && attributes), NULL);

    spat = (SPAttributeTable*)g_object_new (SP_TYPE_ATTRIBUTE_TABLE, NULL);

    sp_attribute_table_set_repr (spat, repr, num_attr, labels, attributes);

    return GTK_WIDGET (spat);

} // end of sp_attribute_table_new_repr()



#define XPAD 4
#define YPAD 0

void
sp_attribute_table_set_object ( SPAttributeTable *spat,
                                SPObject *object,
                                gint num_attr,
                                const gchar **labels,
                                const gchar **attributes,
                                bool script)
{

    g_return_if_fail (spat != NULL);
    g_return_if_fail (SP_IS_ATTRIBUTE_TABLE (spat));
    g_return_if_fail (!object || SP_IS_OBJECT (object));
    g_return_if_fail (!object || (num_attr > 0));
    g_return_if_fail (!num_attr || (labels && attributes));

    if (spat->table) {
        gtk_widget_destroy (spat->table);
        spat->table = NULL;
    }

    if (spat->attributes) {
        gint i;
        for (i = 0; i < spat->num_attr; i++) {
            g_free (spat->attributes[i]);
        }
        g_free (spat->attributes);
        spat->attributes = NULL;
    }

    if (spat->entries) {
        g_free (spat->entries);
        spat->entries = NULL;
    }

    if (spat->hasobj) {
        if (spat->src.object) {
            spat->modified_connection.disconnect();
            spat->release_connection.disconnect();
            spat->src.object = NULL;
        }
    } else {
        if (spat->src.repr) {
            spat->src.repr = Inkscape::GC::release(spat->src.repr);
        }
    }

    spat->hasobj = TRUE;

    if (object) {
        gint i;

        spat->blocked = TRUE;

        /* Set up object */
        spat->src.object = object;
        spat->num_attr = num_attr;

        spat->modified_connection = object->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_attribute_table_object_modified), spat));
        spat->release_connection = object->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_attribute_table_object_release), spat));

        /* Create table */
        spat->table = gtk_table_new (num_attr, 2, FALSE);
        gtk_container_add (GTK_CONTAINER (spat), spat->table);
        /* Arrays */
        spat->attributes = g_new0 (gchar *, num_attr);
        spat->entries = g_new0 (GtkWidget *, num_attr);
        /* Fill rows */
        for (i = 0; i < num_attr; i++) {
            GtkWidget *w, *w2;
            const gchar *val;

            spat->attributes[i] = g_strdup (attributes[i]);
            w = gtk_label_new (_(labels[i]));
            gtk_widget_show (w);
            gtk_misc_set_alignment (GTK_MISC (w), 1.0, 0.5);
            gtk_table_attach ( GTK_TABLE (spat->table), w, 0, 1, i, i + 1,
                               GTK_FILL,
                               (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                               XPAD, YPAD );
            val = object->getRepr()->attribute(attributes[i]);
            if (!script) {
                w = gtk_entry_new ();
                w2 = w;
            } else {
                w = gtk_combo_box_text_new_with_entry ();
                w2 = gtk_bin_get_child ( GTK_BIN (w) );
                if (val) gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT (w), val);
                // Find all functions and add to the combo box
                const GSList *current = SP_ACTIVE_DOCUMENT->getResourceList( "script" );
                while ( current ) {
                    SPObject* obj = SP_OBJECT(current->data);
                    int count=0;
                    for ( SPObject *child = obj->children ; child; child = child->next ) {
                        count++;
                    }
                    if (count>1)
                        g_warning("TODO: Found a script element with multiple (%d) child nodes! We must implement support for that!", count);

                    //XML Tree being used directly here while it shouldn't be.
                    SPObject* child = obj->firstChild();
                    //TODO: shouldnt we get all children instead of simply the first child?

                    if (child && child->getRepr()){
                        const gchar* content = child->getRepr()->content();
                        if (content){
                            // Parse the script content to get the functions
                            GRegex *regex;
                            GMatchInfo *match_info;

                            // vim style: function\s*\(\w*\)\s*(\(.*\))
                            regex = g_regex_new ("function\\s*(\\w*)\\s*\\((.*?)\\)", (GRegexCompileFlags)0, (GRegexMatchFlags)0, NULL);
                            g_regex_match (regex, content, (GRegexMatchFlags)0, &match_info);
                            gchar *function;
                            gchar *params;
                            while (g_match_info_matches (match_info))
                            {
                                function = g_match_info_fetch (match_info, 1);
                                params = g_match_info_fetch (match_info, 2);
                                gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT (w), g_strconcat(function, " (", params, ")", NULL) );
                                g_match_info_next (match_info, NULL);
                                g_free (function);
                                g_free (params);
                            }
                            g_match_info_free (match_info);
                            g_regex_unref (regex);

                        }
                    }
                    current = g_slist_next(current);
                }

            }
            gtk_widget_show (w);
            gtk_entry_set_text (GTK_ENTRY (w2), val ? val : (const gchar *) "");
            gtk_table_attach ( GTK_TABLE (spat->table), w, 1, 2, i, i + 1,
                               (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                               (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                               XPAD, YPAD );
            spat->entries[i] = w2;
            g_signal_connect ( G_OBJECT (w2), "changed",
                               G_CALLBACK (sp_attribute_table_entry_changed),
                               spat );
        }
        /* Show table */
        gtk_widget_show (spat->table);

        spat->blocked = FALSE;
    }

    gtk_widget_set_sensitive ( GTK_WIDGET (spat),
                               (spat->src.object != NULL) );

} // end of sp_attribute_table_set_object()



void
sp_attribute_table_set_repr ( SPAttributeTable *spat,
                              Inkscape::XML::Node *repr,
                              gint num_attr,
                              const gchar **labels,
                              const gchar **attributes )
{
    g_return_if_fail (spat != NULL);
    g_return_if_fail (SP_IS_ATTRIBUTE_TABLE (spat));
    g_return_if_fail (!num_attr || (labels && attributes));

    if (spat->table) {
        gtk_widget_destroy (spat->table);
        spat->table = NULL;
    }

    if (spat->attributes) {
        gint i;
        for (i = 0; i < spat->num_attr; i++) {
            g_free (spat->attributes[i]);
        }
        g_free (spat->attributes);
        spat->attributes = NULL;
    }

    if (spat->entries) {
        g_free (spat->entries);
        spat->entries = NULL;
    }

    if (spat->hasobj) {
        if (spat->src.object) {
            spat->modified_connection.disconnect();
            spat->release_connection.disconnect();
            spat->src.object = NULL;
        }
    } else {
        if (spat->src.repr) {
            spat->src.repr = Inkscape::GC::release(spat->src.repr);
        }
    }

    spat->hasobj = FALSE;

    if (repr) {
        gint i;

        spat->blocked = TRUE;

        /* Set up repr */
        spat->src.repr = Inkscape::GC::anchor(repr);
        spat->num_attr = num_attr;
        /* Create table */
        spat->table = gtk_table_new (num_attr, 2, FALSE);
        gtk_container_add (GTK_CONTAINER (spat), spat->table);
        /* Arrays */
        spat->attributes = g_new0 (gchar *, num_attr);
        spat->entries = g_new0 (GtkWidget *, num_attr);

        /* Fill rows */
        for (i = 0; i < num_attr; i++) {
            GtkWidget *w;
            const gchar *val;

            spat->attributes[i] = g_strdup (attributes[i]);
            w = gtk_label_new (labels[i]);
            gtk_widget_show (w);
            gtk_misc_set_alignment (GTK_MISC (w), 1.0, 0.5);
            gtk_table_attach ( GTK_TABLE (spat->table), w, 0, 1, i, i + 1,
                               GTK_FILL,
                               (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                               XPAD, YPAD );
            w = gtk_entry_new ();
            gtk_widget_show (w);
            val = repr->attribute(attributes[i]);
            gtk_entry_set_text (GTK_ENTRY (w), val ? val : (const gchar *) "");
            gtk_table_attach ( GTK_TABLE (spat->table), w, 1, 2, i, i + 1,
                               (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                               (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                               XPAD, YPAD );
            spat->entries[i] = w;
            g_signal_connect ( G_OBJECT (w), "changed",
                               G_CALLBACK (sp_attribute_table_entry_changed),
                               spat );
        }
        /* Show table */
        gtk_widget_show (spat->table);

        spat->blocked = FALSE;
    }

    gtk_widget_set_sensitive (GTK_WIDGET (spat), (spat->src.repr != NULL));

} // end of sp_attribute_table_set_repr()



static void
sp_attribute_table_object_modified ( SPObject */*object*/,
                                     guint flags,
                                     SPAttributeTable *spat )
{
    if (flags && SP_OBJECT_MODIFIED_FLAG)
    {
        gint i;
        for (i = 0; i < spat->num_attr; i++) {
            const gchar *val, *text;
            val = spat->src.object->getRepr()->attribute(spat->attributes[i]);
            text = gtk_entry_get_text (GTK_ENTRY (spat->entries[i]));
            if (val || text) {
                if (!val || !text || strcmp (val, text)) {
                    /* We are different */
                    spat->blocked = TRUE;
                    gtk_entry_set_text ( GTK_ENTRY (spat->entries[i]),
                                         val ? val : (const gchar *) "");
                    spat->blocked = FALSE;
                }
            }
        }
    } // end of if()

} // end of sp_attribute_table_object_modified()



static void
sp_attribute_table_object_release (SPObject */*object*/, SPAttributeTable *spat)
{
    sp_attribute_table_set_object (spat, NULL, 0, NULL, NULL);
}



static void
sp_attribute_table_entry_changed ( GtkEditable *editable,
                                   SPAttributeTable *spat )
{
    if (!spat->blocked)
    {
        gint i;
        for (i = 0; i < spat->num_attr; i++) {

            if (GTK_WIDGET (editable) == spat->entries[i]) {
                const gchar *text;
                spat->blocked = TRUE;
                text = gtk_entry_get_text (GTK_ENTRY (spat->entries[i]));

                if (!*text)
                    text = NULL;

                if (spat->hasobj && spat->src.object) {
                    spat->src.object->getRepr()->setAttribute(spat->attributes[i], text, false);
                    DocumentUndo::done(spat->src.object->document, SP_VERB_NONE,
                                       _("Set attribute"));

                } else if (spat->src.repr) {

                    spat->src.repr->setAttribute(spat->attributes[i], text, false);
                    /* TODO: Warning! Undo will not be flushed in given case */
                }
                spat->blocked = FALSE;
                return;
            }
        }
        g_warning ("file %s: line %d: Entry signalled change, but there is no such entry", __FILE__, __LINE__);
    } // end of if()

} // end of sp_attribute_table_entry_changed()

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
