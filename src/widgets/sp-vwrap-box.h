/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * SPVWrapBox: Vertical wrapping box widget
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

#ifndef __SP_VWRAP_BOX_H__
#define __SP_VWRAP_BOX_H__


#include "sp-wrap-box.h"

G_BEGIN_DECLS


/* --- type macros --- */
#define SP_TYPE_VWRAP_BOX	      (sp_vwrap_box_get_type ())
#define SP_VWRAP_BOX(obj)	      (GTK_CHECK_CAST ((obj), SP_TYPE_VWRAP_BOX, SPVWrapBox))
#define SP_VWRAP_BOX_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_VWRAP_BOX, SPVWrapBoxClass))
#define GTK_IS_VWRAP_BOX(obj)	      (GTK_CHECK_TYPE ((obj), SP_TYPE_VWRAP_BOX))
#define GTK_IS_VWRAP_BOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_VWRAP_BOX))
#define SP_VWRAP_BOX_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), SP_TYPE_VWRAP_BOX, SPVWrapBoxClass))


/* --- typedefs --- */
typedef struct _SPVWrapBox      SPVWrapBox;
typedef struct _SPVWrapBoxClass SPVWrapBoxClass;


/* --- SPVWrapBox --- */
struct _SPVWrapBox
{
  SPWrapBox parent_widget;
  
  /*<h2v-off>*/
  guint      max_child_width;
  guint      max_child_height;
  /*<h2v-on>*/
};

struct _SPVWrapBoxClass
{
  SPWrapBoxClass parent_class;
};


/* --- prototypes --- */
GtkType	    sp_vwrap_box_get_type  (void) G_GNUC_CONST;
GtkWidget * sp_vwrap_box_new       (gboolean homogeneous);


G_END_DECLS

#endif /* __SP_VWRAP_BOX_H__ */
