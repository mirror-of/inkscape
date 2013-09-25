/* HSV color selector for GTK+
 *
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Simon Budig <Simon.Budig@unix-ag.org> (original code)
 *          Federico Mena-Quintero <federico@gimp.org> (cleanup for GTK+)
 *          Jonathan Blandford <jrb@redhat.com> (cleanup for GTK+)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#include <sigc++/sigc++.h>

#ifndef __RECOLOR_WHEEL_H__
#define __RECOLOR_WHEEL_H__

G_BEGIN_DECLS

#define RECOLOR_TYPE_COLOR_WHEEL            (recolor_wheel_get_type ())
#define RECOLOR_WHEEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), RECOLOR_TYPE_COLOR_WHEEL, RecolorWheel))
#define RECOLOR_WHEEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), RECOLOR_TYPE_COLOR_WHEEL, RecolorWheelClass))
#define RECOLOR_IS_COLOR_WHEEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), RECOLOR_TYPE_COLOR_WHEEL))
#define RECOLOR_IS_COLOR_WHEEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), RECOLOR_TYPE_COLOR_WHEEL))
#define RECOLOR_WHEEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), RECOLOR_TYPE_COLOR_WHEEL, RecolorWheelClass))

#include "recolor-wheel-node.h"
#include <sigc++/connection.h>
#include <sigc++/functors/slot.h>
#include <sigc++/signal.h>
#include <color.h>


typedef struct _RecolorWheel      RecolorWheel;
typedef struct _RecolorWheelClass RecolorWheelClass;

struct _RecolorWheel
{
  GtkWidget parent_instance;

  /* Private data */
  gpointer priv;
};

struct _RecolorWheelClass
{
  GtkWidgetClass parent_class;
  
  /* Notification signals */
  void (* changed) (RecolorWheel   *wheel);

  /* Keybindings */
  void (* move)    (RecolorWheel   *wheel,
                    GtkDirectionType  type);

  /* Padding for future expansion */
  void (*_recolor_reserved1) (void);
  void (*_recolor_reserved2) (void);
  void (*_recolor_reserved3) (void);
  void (*_recolor_reserved4) (void);
};

GType       recolor_wheel_get_type          (void) G_GNUC_CONST;
GtkWidget * recolor_wheel_new               (void);

void        recolor_wheel_set_color         (RecolorWheel *wheel,
                                                double          h,
                                                double          s,
                                                double          v);
void        recolor_wheel_get_color         (RecolorWheel *wheel,
                                                gdouble        *h,
                                                gdouble        *s,
                                                gdouble        *v);

gboolean    recolor_wheel_is_adjusting      (RecolorWheel *wheel);

//=============================
//void            recolor_wheel_nodes_init();
//=============================

void add_node_to_recolor_wheel (RecolorWheel *wheel, std::string name, RecolorWheelNode node);

void remove_node_to_recolor_wheel (RecolorWheel *wheel, std::string name);

void remove_all_nodes_recolor_wheel (RecolorWheel *wheel);

guint get_sel_obj (RecolorWheel *wheel);

guint get_priv_count (RecolorWheel *wheel);

sigc::connection connectNodeReleased(sigc::slot<void, std::string, gfloat*> slot, RecolorWheel *wheel);

G_END_DECLS

#endif /* __RECOLOR_WHEEL_H__ */
