#define __SP_OBJECT_UI_C__

/*
 * Unser-interface related object extension
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <config.h>

#include "object-ui.h"

static void sp_object_type_menu (GType type, SPObject *object, SPDesktop *desktop, GtkMenu *menu);

/* Append object-specific part to context menu */

void
sp_object_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
	GObjectClass *klass;
	klass = G_OBJECT_GET_CLASS (object);
	while (G_TYPE_CHECK_CLASS_TYPE ((klass), SP_TYPE_OBJECT)) {
		GType type;
		type = G_TYPE_FROM_CLASS (klass);
		sp_object_type_menu (type, object, desktop, menu);
		klass = (GObjectClass*)g_type_class_peek_parent (klass);
	}
}

/* Implementation */

#include <gtk/gtkmenuitem.h>
#include <gtk/gtksignal.h>

#include "helper/sp-intl.h"

#include "sp-item-group.h"
#include "sp-anchor.h"
#include "sp-image.h"
#include "sp-rect.h"
#include "sp-star.h"
#include "sp-spiral.h"

#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"

#include "dialogs/item-properties.h"
#include "dialogs/object-attributes.h"
#include "dialogs/fill-style.h"
#include "dialogs/object-properties.h"

static void sp_item_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_group_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_anchor_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_image_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_shape_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_rect_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_star_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);
static void sp_spiral_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu);

static void
sp_object_type_menu (GType type, SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
	static GHashTable *t2m = NULL;
	void (* handler) (SPObject *object, SPDesktop *desktop, GtkMenu *menu);
	if (!t2m) {
		t2m = g_hash_table_new (NULL, NULL);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_ITEM), (void*)sp_item_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_GROUP), (void*)sp_group_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_ANCHOR), (void*)sp_anchor_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_IMAGE), (void*)sp_image_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_SHAPE), (void*)sp_shape_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_RECT), (void*)sp_rect_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_STAR), (void*)sp_star_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_SPIRAL), (void*)sp_spiral_menu);
	}
	handler = (void (*)(SPObject*, SPDesktop*, GtkMenu*))g_hash_table_lookup (t2m, GUINT_TO_POINTER (type));
	if (handler) handler (object, desktop, menu);
}

/* SPItem */

static void sp_item_properties (GtkMenuItem *menuitem, SPItem *item);
static void sp_item_select_this (GtkMenuItem *menuitem, SPItem *item);
static void sp_item_reset_transformation (GtkMenuItem *menuitem, SPItem *item);
static void sp_item_toggle_sensitivity (GtkMenuItem *menuitem, SPItem *item);
static void sp_item_create_link (GtkMenuItem *menuitem, SPItem *item);

/* Generate context menu item section */

static void
sp_item_menu (SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
	SPItem *item;
	GtkWidget *w;
	gboolean insensitive;

	item = (SPItem *) object;

	/* Item dialog */
	w = gtk_menu_item_new_with_mnemonic (_("Item _Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Separator */
	w = gtk_menu_item_new ();
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Select item */
	w = gtk_menu_item_new_with_mnemonic (_("_Select This"));
	if (SP_DT_SELECTION (desktop)->includesItem(item)) {
		gtk_widget_set_sensitive (w, FALSE);
	} else {
		gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
		gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_select_this), item);
	}
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Reset transformations */
	w = gtk_menu_item_new_with_mnemonic (_("Reset _Transformation"));
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_reset_transformation), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Toggle sensitivity */
	insensitive = (sp_repr_attr (SP_OBJECT_REPR (item), "sodipodi:insensitive") != NULL);
	w = gtk_menu_item_new_with_mnemonic (insensitive ? _("Make s_ensitive") : _("Make i_nsensitive"));
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_toggle_sensitivity), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Create link */
	w = gtk_menu_item_new_with_mnemonic (_("_Create Link"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_create_link), item);
	gtk_widget_set_sensitive (w, !SP_IS_ANCHOR (item));
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
}

static void
sp_item_properties (GtkMenuItem *menuitem, SPItem *item)
{
	SPDesktop *desktop;

	g_assert (SP_IS_ITEM (item));

	desktop = (SPDesktop*)gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	SP_DT_SELECTION(desktop)->setItem(item);

	sp_item_dialog ();
}

static void
sp_item_select_this (GtkMenuItem *menuitem, SPItem *item)
{
	SPDesktop *desktop;

	g_assert (SP_IS_ITEM (item));

	desktop = (SPDesktop*)gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	SP_DT_SELECTION(desktop)->setItem(item);
}

static void
sp_item_reset_transformation (GtkMenuItem * menuitem, SPItem * item)
{
	g_assert (SP_IS_ITEM (item));

	sp_repr_set_attr (((SPObject *) item)->repr, "transform", NULL);
	sp_document_done (SP_OBJECT_DOCUMENT (item));
}

static void
sp_item_toggle_sensitivity (GtkMenuItem * menuitem, SPItem * item)
{
	const gchar * val;

	g_assert (SP_IS_ITEM (item));

	/* fixme: reprs suck */
	val = sp_repr_attr (SP_OBJECT_REPR (item), "sodipodi:insensitive");
	if (val != NULL) {
		val = NULL;
	} else {
		val = "1";
	}
	sp_repr_set_attr (SP_OBJECT_REPR (item), "sodipodi:insensitive", val);
	sp_document_done (SP_OBJECT_DOCUMENT (item));
}

static void
sp_item_create_link (GtkMenuItem *menuitem, SPItem *item)
{
	g_assert (SP_IS_ITEM (item));
	g_assert (!SP_IS_ANCHOR (item));

	SPDesktop *desktop = (SPDesktop*)gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	SPRepr *repr = sp_repr_new ("a");
	sp_repr_add_child (SP_OBJECT_REPR (SP_OBJECT_PARENT (item)), repr, SP_OBJECT_REPR (item));
	SPObject *object = sp_document_lookup_id (SP_OBJECT_DOCUMENT (item), sp_repr_attr (repr, "id"));
	g_return_if_fail (SP_IS_ANCHOR (object));

	const char *id = sp_repr_attr (SP_OBJECT_REPR (item), "id");
	SPRepr *child = sp_repr_duplicate (SP_OBJECT_REPR (item));
	SP_OBJECT (item)->deleteObject(false);
	sp_repr_add_child (repr, child, NULL);
	sp_repr_set_attr (child, "id", id);
	sp_document_done (SP_OBJECT_DOCUMENT (object));

	sp_object_attributes_dialog (object, "SPAnchor");

	SP_DT_SELECTION(desktop)->setItem(SP_ITEM(object));
}

/* SPGroup */

static void sp_item_group_ungroup_activate (GtkMenuItem *menuitem, SPGroup *group);

static void
sp_group_menu (SPObject *object, SPDesktop *desktop, GtkMenu * menu)
{
	SPItem *item;
	GtkWidget *w;

	item = (SPItem *)object;

	/* "Ungroup" */
	w = gtk_menu_item_new_with_mnemonic (_("_Ungroup"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_group_ungroup_activate), item);
	gtk_widget_show (w);

	gtk_menu_append (GTK_MENU (menu), w);
}

static void
sp_item_group_ungroup_activate (GtkMenuItem *menuitem, SPGroup *group)
{
	SPDesktop *desktop;
	GSList *children;

	g_assert (SP_IS_GROUP (group));

	desktop = (SPDesktop*)gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	children = NULL;
	sp_item_group_ungroup (group, &children);

	SP_DT_SELECTION(desktop)->setItemList(children);
	g_slist_free (children);
}

/* SPAnchor */

static void sp_anchor_link_properties (GtkMenuItem *menuitem, SPAnchor *anchor);
static void sp_anchor_link_follow (GtkMenuItem *menuitem, SPAnchor *anchor);
static void sp_anchor_link_remove (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_anchor_menu (SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
	SPItem *item;
	GtkWidget *w;

	item = (SPItem *) object;

	/* Link dialog */
	w = gtk_menu_item_new_with_mnemonic (_("Link _Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_anchor_link_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Separator */
	w = gtk_menu_item_new ();
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Select item */
	w = gtk_menu_item_new_with_mnemonic (_("_Follow Link"));
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_anchor_link_follow), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Reset transformations */
	w = gtk_menu_item_new_with_mnemonic (_("_Remove Link"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_anchor_link_remove), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
}

static void
sp_anchor_link_properties (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	sp_object_attributes_dialog (SP_OBJECT (anchor), "SPAnchor");
}

static void
sp_anchor_link_follow (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	g_return_if_fail (anchor != NULL);
	g_return_if_fail (SP_IS_ANCHOR (anchor));

	/* shell out to an external browser here */
}

static void
sp_anchor_link_remove (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	GSList *children;

	g_return_if_fail (anchor != NULL);
	g_return_if_fail (SP_IS_ANCHOR (anchor));

	children = NULL;
	sp_item_group_ungroup (SP_GROUP (anchor), &children);

	g_slist_free (children);
}

/* Image */

static void sp_image_image_properties (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_image_menu (SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
	SPItem *item;
	GtkWidget *w;

	item = (SPItem *) object;

	/* Link dialog */
	w = gtk_menu_item_new_with_mnemonic (_("Image _Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_image_image_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);

#ifdef ENABLE_AUTOTRACE
	/* Autotrace dialog */
	w = gtk_menu_item_new_with_mnemonic (_("_Trace"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_image_autotrace), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
#endif /* Def: ENABLE_AUTOTRACE */
}

static void
sp_image_image_properties (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	sp_object_attributes_dialog (SP_OBJECT (anchor), "SPImage");
}

/* SPShape */

static void
sp_shape_fill_settings (GtkMenuItem *menuitem, SPItem *item)
{
	SPDesktop *desktop;

	g_assert (SP_IS_ITEM (item));

	desktop = (SPDesktop*)gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	SP_DT_SELECTION(desktop)->setItem(item);

	sp_object_properties_dialog ();
}

static void
sp_shape_menu (SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
	SPItem *item;
	GtkWidget *w;

	item = (SPItem *) object;

	/* Item dialog */
	w = gtk_menu_item_new_with_mnemonic (_("_Fill and Stroke"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_shape_fill_settings), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
}

/* SPRect */

static void sp_rect_rect_properties (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_rect_menu (SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
	SPItem *item;
	GtkWidget *w;

	item = (SPItem *) object;

	/* Link dialog */
	w = gtk_menu_item_new_with_mnemonic (_("Rect _Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_rect_rect_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
}

static void
sp_rect_rect_properties (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	sp_object_attributes_dialog (SP_OBJECT (anchor), "SPRect");
}

/* SPStar */

static void sp_star_star_properties (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_star_menu (SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
	SPItem *item;
	GtkWidget *w;

	item = (SPItem *) object;

	/* Link dialog */
	w = gtk_menu_item_new_with_mnemonic (_("Star _Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_star_star_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
}

static void
sp_star_star_properties (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	sp_object_attributes_dialog (SP_OBJECT (anchor), "SPStar");
}

/* SPSpiral */

static void sp_spiral_spiral_properties (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_spiral_menu (SPObject *object, SPDesktop *desktop, GtkMenu *m)
{
	SPItem *item;
	GtkWidget *w;

	item = (SPItem *) object;

	/* Link dialog */
	w = gtk_menu_item_new_with_mnemonic (_("Spiral _Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_spiral_spiral_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
}

static void
sp_spiral_spiral_properties (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	sp_object_attributes_dialog (SP_OBJECT (anchor), "SPSpiral");
}

