#ifndef __SP_UNIT_MENU_H__
#define __SP_UNIT_MENU_H__

/*
 * SPUnitMenu
 *
 * Generic (and quite unintelligent) grid item for gnome canvas
 *
 * Copyright (C) Lauris Kaplinski 2000
 *
 */

#include <glib.h>
#include <gtk/gtkoptionmenu.h>
#include "units.h"

G_BEGIN_DECLS

/* Unit selector Widget */

#define SP_TYPE_UNIT_SELECTOR (sp_unit_selector_get_type ())
#define SP_UNIT_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_UNIT_SELECTOR, SPUnitSelector))
#define SP_UNIT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_UNIT_SELECTOR, SPUnitSelectorClass))
#define SP_IS_UNIT_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_UNIT_SELECTOR))
#define SP_IS_UNIT_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_UNIT_SELECTOR))

typedef struct _SPUnitSelector SPUnitSelector;
typedef struct _SPUnitSelectorClass SPUnitSelectorClass;

GType sp_unit_selector_get_type (void);

GtkWidget *sp_unit_selector_new (guint bases);

const SPUnit *sp_unit_selector_get_unit (SPUnitSelector *selector);

void sp_unit_selector_set_bases (SPUnitSelector *selector, guint bases);
void sp_unit_selector_add_unit (SPUnitSelector *selector, const SPUnit *unit, int position);

void sp_unit_selector_set_unit (SPUnitSelector *selector, const SPUnit *unit);
void sp_unit_selector_add_adjustment (SPUnitSelector *selector, GtkAdjustment *adjustment);
void sp_unit_selector_remove_adjustment (SPUnitSelector *selector, GtkAdjustment *adjustment);

gboolean sp_unit_selector_update_test (SPUnitSelector *selector);

float sp_unit_selector_get_value_in_points (SPUnitSelector *selector, GtkAdjustment *adj);
void sp_unit_selector_set_value_in_points (SPUnitSelector *selector, GtkAdjustment *adj, float value);

G_END_DECLS

#endif
