/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * SPWrapBox: Wrapping box widget
 * Copyright (C) 1999 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __SP_WRAP_BOX_H__
#define __SP_WRAP_BOX_H__


#include <gtk/gtkcontainer.h>

G_BEGIN_DECLS


/* --- type macros --- */
#define SP_TYPE_WRAP_BOX	     (sp_wrap_box_get_type ())
#define SP_WRAP_BOX(obj)	     (GTK_CHECK_CAST ((obj), SP_TYPE_WRAP_BOX, SPWrapBox))
#define SP_WRAP_BOX_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_WRAP_BOX, SPWrapBoxClass))
#define GTK_IS_WRAP_BOX(obj)	     (GTK_CHECK_TYPE ((obj), SP_TYPE_WRAP_BOX))
#define GTK_IS_WRAP_BOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_WRAP_BOX))
#define SP_WRAP_BOX_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), SP_TYPE_WRAP_BOX, SPWrapBoxClass))


/* --- typedefs --- */
typedef struct _SPWrapBox      SPWrapBox;
typedef struct _SPWrapBoxClass SPWrapBoxClass;
typedef struct _SPWrapBoxChild SPWrapBoxChild;

/* --- SPWrapBox --- */
struct _SPWrapBox
{
  GtkContainer     container;
  
  guint            homogeneous : 1;
  guint            justify : 4;
  guint            line_justify : 4;
  guint8           hspacing;
  guint8           vspacing;
  guint16          n_children;
  SPWrapBoxChild *children;
  gfloat           aspect_ratio; /* 1/256..256 */
  guint            child_limit;
};

struct _SPWrapBoxClass
{
  GtkContainerClass parent_class;

  GSList* (*rlist_line_children) (SPWrapBox       *wbox,
				  SPWrapBoxChild **child_p,
				  GtkAllocation    *area,
				  guint            *max_child_size,
				  gboolean         *expand_line);
};

struct _SPWrapBoxChild
{
  GtkWidget *widget;
  guint      hexpand : 1;
  guint      hfill : 1;
  guint      vexpand : 1;
  guint      vfill : 1;
  guint      wrapped : 1;
  
  SPWrapBoxChild *next;
};

#define GTK_JUSTIFY_TOP    GTK_JUSTIFY_LEFT
#define GTK_JUSTIFY_BOTTOM GTK_JUSTIFY_RIGHT


/* --- prototypes --- */
GtkType	   sp_wrap_box_get_type            (void) G_GNUC_CONST;
void	   sp_wrap_box_set_homogeneous     (SPWrapBox      *wbox,
					     gboolean         homogeneous);
void	   sp_wrap_box_set_hspacing        (SPWrapBox      *wbox,
					     guint            hspacing);
void	   sp_wrap_box_set_vspacing        (SPWrapBox      *wbox,
					     guint            vspacing);
void	   sp_wrap_box_set_justify         (SPWrapBox      *wbox,
					     GtkJustification justify);
void	   sp_wrap_box_set_line_justify    (SPWrapBox      *wbox,
					     GtkJustification line_justify);
void	   sp_wrap_box_set_aspect_ratio    (SPWrapBox      *wbox,
					     gfloat           aspect_ratio);
void	   sp_wrap_box_pack	            (SPWrapBox      *wbox,
					     GtkWidget       *child,
					     gboolean         hexpand,
					     gboolean         hfill,
					     gboolean         vexpand,
					     gboolean         vfill);
void	   sp_wrap_box_pack_wrapped        (SPWrapBox      *wbox,
					     GtkWidget       *child,
					     gboolean         hexpand,
					     gboolean         hfill,
					     gboolean         vexpand,
					     gboolean         vfill,
					     gboolean         wrapped);
void       sp_wrap_box_reorder_child       (SPWrapBox      *wbox,
					     GtkWidget       *child,
					     gint             position);
void       sp_wrap_box_query_child_packing (SPWrapBox      *wbox,
					     GtkWidget       *child,
					     gboolean        *hexpand,
					     gboolean        *hfill,
					     gboolean        *vexpand,
					     gboolean        *vfill,
					     gboolean        *wrapped);
void       sp_wrap_box_set_child_packing   (SPWrapBox      *wbox,
					     GtkWidget       *child,
					     gboolean         hexpand,
					     gboolean         hfill,
					     gboolean         vexpand,
					     gboolean         vfill,
					     gboolean         wrapped);
guint*	   sp_wrap_box_query_line_lengths  (SPWrapBox	     *wbox,
					     guint           *n_lines);


G_END_DECLS

#endif /* __SP_WRAP_BOX_H__ */
