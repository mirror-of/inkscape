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
		klass = g_type_class_peek_parent (klass);
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
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_ITEM), sp_item_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_GROUP), sp_group_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_ANCHOR), sp_anchor_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_IMAGE), sp_image_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_SHAPE), sp_shape_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_RECT), sp_rect_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_STAR), sp_star_menu);
		g_hash_table_insert (t2m, GUINT_TO_POINTER (SP_TYPE_SPIRAL), sp_spiral_menu);
	}
	handler = g_hash_table_lookup (t2m, GUINT_TO_POINTER (type));
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
sp_item_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
	SPItem *item;
	GtkWidget *i, *m, *w;
	gboolean insensitive;

	item = (SPItem *) object;

	/* Create toplevel menuitem */
	i = gtk_menu_item_new_with_label (_("Item"));
	m = gtk_menu_new ();
	/* Item dialog */
	w = gtk_menu_item_new_with_label (_("Item Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Separator */
	w = gtk_menu_item_new ();
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Select item */
	w = gtk_menu_item_new_with_label (_("Select this"));
	if (sp_selection_item_selected (SP_DT_SELECTION (desktop), item)) {
		gtk_widget_set_sensitive (w, FALSE);
	} else {
		gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
		gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_select_this), item);
	}
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Reset transformations */
	w = gtk_menu_item_new_with_label (_("Reset transformation"));
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_reset_transformation), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Toggle sensitivity */
	insensitive = (sp_repr_attr (SP_OBJECT_REPR (item), "sodipodi:insensitive") != NULL);
	w = gtk_menu_item_new_with_label (insensitive ? _("Make sensitive") : _("Make insensitive"));
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_toggle_sensitivity), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Create link */
	w = gtk_menu_item_new_with_label (_("Create link"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_create_link), item);
	gtk_widget_set_sensitive (w, !SP_IS_ANCHOR (item));
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Show menu */
	gtk_widget_show (m);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), m);

	gtk_menu_append (menu, i);
	gtk_widget_show (i);
}

static void
sp_item_properties (GtkMenuItem *menuitem, SPItem *item)
{
	SPDesktop *desktop;

	g_assert (SP_IS_ITEM (item));

	desktop = gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	sp_selection_set_item (SP_DT_SELECTION (desktop), item);

	sp_item_dialog ();
}

static void
sp_item_select_this (GtkMenuItem *menuitem, SPItem *item)
{
	SPDesktop *desktop;

	g_assert (SP_IS_ITEM (item));

	desktop = gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	sp_selection_set_item (SP_DT_SELECTION (desktop), item);
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
	SPRepr *repr, *child;
	SPObject *object;
	SPDesktop *desktop;

	g_assert (SP_IS_ITEM (item));
	g_assert (!SP_IS_ANCHOR (item));

	desktop = gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	repr = sp_repr_new ("a");
	sp_repr_add_child (SP_OBJECT_REPR (SP_OBJECT_PARENT (item)), repr, SP_OBJECT_REPR (item));
	object = sp_document_lookup_id (SP_OBJECT_DOCUMENT (item), sp_repr_attr (repr, "id"));
	g_return_if_fail (SP_IS_ANCHOR (object));
	child = sp_repr_duplicate (SP_OBJECT_REPR (item));
	sp_repr_unparent (SP_OBJECT_REPR (item));
	sp_repr_add_child (repr, child, NULL);
	sp_document_done (SP_OBJECT_DOCUMENT (object));

	sp_object_attributes_dialog (object, "SPAnchor");

	sp_selection_set_item (SP_DT_SELECTION (desktop), SP_ITEM (object));
}

/* SPGroup */

static void sp_item_group_ungroup_activate (GtkMenuItem *menuitem, SPGroup *group);

static void
sp_group_menu (SPObject *object, SPDesktop *desktop, GtkMenu * menu)
{
	SPItem *item;
	GtkWidget * i, * m, * w;

	item = (SPItem *) object;

	i = gtk_menu_item_new_with_label (_("Group"));
	m = gtk_menu_new ();

	/* Group dialog */
	w = gtk_menu_item_new_with_label (_("Group Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
#if 0
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_properties), item);
#endif
	gtk_widget_set_sensitive (w, FALSE);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Separator */
	w = gtk_menu_item_new ();
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* "Ungroup" */
	w = gtk_menu_item_new_with_label (_("Ungroup"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_item_group_ungroup_activate), item);
	gtk_widget_show (w);

	gtk_menu_append (GTK_MENU (m), w);
	gtk_widget_show (m);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), m);

	gtk_menu_append (menu, i);
	gtk_widget_show (i);
}

static void
sp_item_group_ungroup_activate (GtkMenuItem *menuitem, SPGroup *group)
{
	SPDesktop *desktop;
	GSList *children;

	g_assert (SP_IS_GROUP (group));

	desktop = gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	children = NULL;
	sp_item_group_ungroup (group, &children);

	sp_selection_set_item_list (SP_DT_SELECTION (desktop), children);
	g_slist_free (children);
}

/* SPAnchor */

static void sp_anchor_link_properties (GtkMenuItem *menuitem, SPAnchor *anchor);
static void sp_anchor_link_follow (GtkMenuItem *menuitem, SPAnchor *anchor);
static void sp_anchor_link_remove (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_anchor_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
	SPItem *item;
	GtkWidget *i, *m, *w;

	item = (SPItem *) object;

	/* Create toplevel menuitem */
	i = gtk_menu_item_new_with_label (_("Link"));
	m = gtk_menu_new ();
	/* Link dialog */
	w = gtk_menu_item_new_with_label (_("Link Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_anchor_link_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Separator */
	w = gtk_menu_item_new ();
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Select item */
	w = gtk_menu_item_new_with_label (_("Follow link"));
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_anchor_link_follow), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Reset transformations */
	w = gtk_menu_item_new_with_label (_("Remove link"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_anchor_link_remove), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Show menu */
	gtk_widget_show (m);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), m);

	gtk_menu_append (menu, i);
	gtk_widget_show (i);
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

#if 0
	if (anchor->href) {
		gnome_url_show (anchor->href, NULL);
	}
#endif
}

static void
sp_anchor_link_remove (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	GSList *children;

	g_return_if_fail (anchor != NULL);
	g_return_if_fail (SP_IS_ANCHOR (anchor));

	children = NULL;
	sp_item_group_ungroup (SP_GROUP (anchor), &children);

#if 0
	sp_selection_set_item_list (SP_DT_SELECTION (desktop), children);
#endif
	g_slist_free (children);
}

/* Image */

static void sp_image_image_properties (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_image_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
	SPItem *item;
	GtkWidget *i, *m, *w;

	item = (SPItem *) object;

	/* Create toplevel menuitem */
	i = gtk_menu_item_new_with_label (_("Image"));
	m = gtk_menu_new ();
	/* Link dialog */
	w = gtk_menu_item_new_with_label (_("Image Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_image_image_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);

#ifdef ENABLE_AUTOTRACE
	/* Autotrace dialog */
	w = gtk_menu_item_new_with_label (_("Trace"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_image_autotrace), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
#endif /* Def: ENABLE_AUTOTRACE */

	/* Show menu */
	gtk_widget_show (m);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), m);

	gtk_menu_append (menu, i);
	gtk_widget_show (i);
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

	desktop = gtk_object_get_data (GTK_OBJECT (menuitem), "desktop");
	g_return_if_fail (desktop != NULL);
	g_return_if_fail (SP_IS_DESKTOP (desktop));

	sp_selection_set_item (SP_DT_SELECTION (desktop), item);

	sp_fill_style_dialog ();
}

static void
sp_shape_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
	SPItem *item;
	GtkWidget *i, *m, *w;

	item = (SPItem *) object;

	/* Create toplevel menuitem */
	i = gtk_menu_item_new_with_label (_("Shape"));
	m = gtk_menu_new ();
	/* Item dialog */
	w = gtk_menu_item_new_with_label (_("Fill settings"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_shape_fill_settings), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Show menu */
	gtk_widget_show (m);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), m);

	gtk_menu_append (menu, i);
	gtk_widget_show (i);
}

/* SPRect */

static void sp_rect_rect_properties (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_rect_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
	SPItem *item;
	GtkWidget *i, *m, *w;

	item = (SPItem *) object;

	/* Create toplevel menuitem */
	i = gtk_menu_item_new_with_label (_("Rect"));
	m = gtk_menu_new ();
	/* Link dialog */
	w = gtk_menu_item_new_with_label (_("Rect Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_rect_rect_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Show menu */
	gtk_widget_show (m);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), m);

	gtk_menu_append (menu, i);
	gtk_widget_show (i);
}

static void
sp_rect_rect_properties (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	sp_object_attributes_dialog (SP_OBJECT (anchor), "SPRect");
}

/* SPStar */

static void sp_star_star_properties (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_star_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
	SPItem *item;
	GtkWidget *i, *m, *w;

	item = (SPItem *) object;

	/* Create toplevel menuitem */
	i = gtk_menu_item_new_with_label (_("Star"));
	m = gtk_menu_new ();
	/* Link dialog */
	w = gtk_menu_item_new_with_label (_("Star Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_star_star_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Show menu */
	gtk_widget_show (m);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), m);

	gtk_menu_append (menu, i);
	gtk_widget_show (i);
}

static void
sp_star_star_properties (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	sp_object_attributes_dialog (SP_OBJECT (anchor), "SPStar");
}

/* SPSpiral */

static void sp_spiral_spiral_properties (GtkMenuItem *menuitem, SPAnchor *anchor);

static void
sp_spiral_menu (SPObject *object, SPDesktop *desktop, GtkMenu *menu)
{
	SPItem *item;
	GtkWidget *i, *m, *w;

	item = (SPItem *) object;

	/* Create toplevel menuitem */
	i = gtk_menu_item_new_with_label (_("Spiral"));
	m = gtk_menu_new ();
	/* Link dialog */
	w = gtk_menu_item_new_with_label (_("Spiral Properties"));
	gtk_object_set_data (GTK_OBJECT (w), "desktop", desktop);
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (sp_spiral_spiral_properties), item);
	gtk_widget_show (w);
	gtk_menu_append (GTK_MENU (m), w);
	/* Show menu */
	gtk_widget_show (m);

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (i), m);

	gtk_menu_append (menu, i);
	gtk_widget_show (i);
}

static void
sp_spiral_spiral_properties (GtkMenuItem *menuitem, SPAnchor *anchor)
{
	sp_object_attributes_dialog (SP_OBJECT (anchor), "SPSpiral");
}

