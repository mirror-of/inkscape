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

#ifdef __GNUC__
#warning GTK_DISABLE_DEPRECATED
#endif
#undef GTK_DISABLE_DEPRECATED

#include "sp-wrap-box.h"


/* --- arguments --- */
enum {
  ARG_0,
  ARG_HOMOGENEOUS,
  ARG_JUSTIFY,
  ARG_HSPACING,
  ARG_VSPACING,
  ARG_LINE_JUSTIFY,
  ARG_ASPECT_RATIO,
  ARG_CURRENT_RATIO,
  ARG_CHILD_LIMIT
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_POSITION,
  CHILD_PROP_HEXPAND,
  CHILD_PROP_HFILL,
  CHILD_PROP_VEXPAND,
  CHILD_PROP_VFILL,
  CHILD_PROP_WRAPPED
};

/* --- prototypes --- */
static void sp_wrap_box_class_init    (SPWrapBoxClass    *klass);
static void sp_wrap_box_init          (SPWrapBox         *wbox);
static void sp_wrap_box_get_arg       (GtkObject          *object,
					GtkArg             *arg,
					guint               arg_id);
static void sp_wrap_box_set_arg       (GtkObject          *object,
					GtkArg             *arg,
					guint               arg_id);
static void sp_wrap_box_set_child_property (GtkContainer    *container,
					     GtkWidget       *child,
					     guint            property_id,
					     const GValue    *value,
					     GParamSpec      *pspec);
static void sp_wrap_box_get_child_property (GtkContainer    *container,
					     GtkWidget       *child,
					     guint            property_id,
					     GValue          *value,
					     GParamSpec      *pspec);
static void sp_wrap_box_map           (GtkWidget          *widget);
static void sp_wrap_box_unmap         (GtkWidget          *widget);
static gint sp_wrap_box_expose        (GtkWidget          *widget,
					GdkEventExpose     *event);
static void sp_wrap_box_add           (GtkContainer       *container,
					GtkWidget          *widget);
static void sp_wrap_box_remove        (GtkContainer       *container,
					GtkWidget          *widget);
static void sp_wrap_box_forall        (GtkContainer       *container,
					gboolean            include_internals,
					GtkCallback         callback,
					gpointer            callback_data);
static GtkType sp_wrap_box_child_type (GtkContainer       *container);


/* --- variables --- */
static gpointer parent_class = NULL;


/* --- functions --- */
GtkType
sp_wrap_box_get_type (void)
{
  static GtkType wrap_box_type = 0;
  
  if (!wrap_box_type)
    {
      static const GtkTypeInfo wrap_box_info =
      {
	"SPWrapBox",
	sizeof (SPWrapBox),
	sizeof (SPWrapBoxClass),
	(GtkClassInitFunc) sp_wrap_box_class_init,
	(GtkObjectInitFunc) sp_wrap_box_init,
        /* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
	(GtkClassInitFunc) NULL,
      };
      
      wrap_box_type = gtk_type_unique (GTK_TYPE_CONTAINER, &wrap_box_info);
    }
  
  return wrap_box_type;
}

static void
sp_wrap_box_class_init (SPWrapBoxClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;
  
  object_class = GTK_OBJECT_CLASS (class);
  widget_class = GTK_WIDGET_CLASS (class);
  container_class = GTK_CONTAINER_CLASS (class);
  
  parent_class = g_type_class_peek_parent (class);
  
  object_class->set_arg = sp_wrap_box_set_arg;
  object_class->get_arg = sp_wrap_box_get_arg;
  
  widget_class->map = sp_wrap_box_map;
  widget_class->unmap = sp_wrap_box_unmap;
  widget_class->expose_event = sp_wrap_box_expose;
  
  container_class->add = sp_wrap_box_add;
  container_class->remove = sp_wrap_box_remove;
  container_class->forall = sp_wrap_box_forall;
  container_class->child_type = sp_wrap_box_child_type;
  container_class->set_child_property = sp_wrap_box_set_child_property;
  container_class->get_child_property = sp_wrap_box_get_child_property;

  class->rlist_line_children = NULL;
  
  gtk_object_add_arg_type ("SPWrapBox::homogeneous",
			   GTK_TYPE_BOOL, GTK_ARG_READWRITE, ARG_HOMOGENEOUS);
  gtk_object_add_arg_type ("SPWrapBox::justify",
			   GTK_TYPE_JUSTIFICATION, GTK_ARG_READWRITE, ARG_JUSTIFY);
  gtk_object_add_arg_type ("SPWrapBox::hspacing",
			   GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_HSPACING);
  gtk_object_add_arg_type ("SPWrapBox::vspacing",
			   GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_VSPACING);
  gtk_object_add_arg_type ("SPWrapBox::line_justify",
			   GTK_TYPE_JUSTIFICATION, GTK_ARG_READWRITE, ARG_LINE_JUSTIFY);
  gtk_object_add_arg_type ("SPWrapBox::aspect_ratio",
			   GTK_TYPE_FLOAT, GTK_ARG_READWRITE, ARG_ASPECT_RATIO);
  gtk_object_add_arg_type ("SPWrapBox::current_ratio",
			   GTK_TYPE_FLOAT, GTK_ARG_READABLE, ARG_CURRENT_RATIO);
  gtk_object_add_arg_type ("SPWrapBox::max_children_per_line",
			   GTK_TYPE_UINT, GTK_ARG_READWRITE, ARG_CHILD_LIMIT);

  gtk_container_class_install_child_property (container_class, CHILD_PROP_POSITION,
					      g_param_spec_int ("position", NULL, NULL,
								-1, G_MAXINT, 0,
								G_PARAM_READWRITE));
  gtk_container_class_install_child_property (container_class, CHILD_PROP_HEXPAND,
					      g_param_spec_boolean ("hexpand", NULL, NULL,
								    FALSE,
								    G_PARAM_READWRITE));
  gtk_container_class_install_child_property (container_class, CHILD_PROP_HFILL,
					      g_param_spec_boolean ("hfill", NULL, NULL,
								    FALSE,
								    G_PARAM_READWRITE));
  gtk_container_class_install_child_property (container_class, CHILD_PROP_VEXPAND,
					      g_param_spec_boolean ("vexpand", NULL, NULL,
								    FALSE,
								    G_PARAM_READWRITE));
  gtk_container_class_install_child_property (container_class, CHILD_PROP_VFILL,
					      g_param_spec_boolean ("vfill", NULL, NULL,
								    FALSE,
								    G_PARAM_READWRITE));
  gtk_container_class_install_child_property (container_class, CHILD_PROP_VFILL,
					      g_param_spec_boolean ("wrapped", NULL, NULL,
								    FALSE,
								    G_PARAM_READWRITE));
}

static void
sp_wrap_box_init (SPWrapBox *wbox)
{
  GTK_WIDGET_SET_FLAGS (wbox, GTK_NO_WINDOW);
  
  wbox->homogeneous = FALSE;
  wbox->hspacing = 0;
  wbox->vspacing = 0;
  wbox->justify = GTK_JUSTIFY_LEFT;
  wbox->line_justify = GTK_JUSTIFY_BOTTOM;
  wbox->n_children = 0;
  wbox->children = NULL;
  wbox->aspect_ratio = 1;
  wbox->child_limit = 32767;
}

static void
sp_wrap_box_set_arg (GtkObject *object,
		      GtkArg    *arg,
		      guint      arg_id)
{
  SPWrapBox *wbox = SP_WRAP_BOX (object);
  
  switch (arg_id)
    {
    case ARG_HOMOGENEOUS:
      sp_wrap_box_set_homogeneous (wbox, GTK_VALUE_BOOL (*arg));
      break;
    case ARG_JUSTIFY:
      sp_wrap_box_set_justify (wbox, GTK_VALUE_ENUM (*arg));
      break;
    case ARG_LINE_JUSTIFY:
      sp_wrap_box_set_line_justify (wbox, GTK_VALUE_ENUM (*arg));
      break;
    case ARG_HSPACING:
      sp_wrap_box_set_hspacing (wbox, GTK_VALUE_UINT (*arg));
      break;
    case ARG_VSPACING:
      sp_wrap_box_set_vspacing (wbox, GTK_VALUE_UINT (*arg));
      break;
    case ARG_ASPECT_RATIO:
      sp_wrap_box_set_aspect_ratio (wbox, GTK_VALUE_FLOAT (*arg));
      break;
    case ARG_CHILD_LIMIT:
      if (wbox->child_limit != GTK_VALUE_UINT (*arg))
	{
	  wbox->child_limit = CLAMP (GTK_VALUE_UINT (*arg), 1, 32767);
	  gtk_widget_queue_resize (GTK_WIDGET (wbox));
	}
      break;
    }
}

static void
sp_wrap_box_get_arg (GtkObject *object,
		      GtkArg    *arg,
		      guint      arg_id)
{
  SPWrapBox *wbox = SP_WRAP_BOX (object);
  GtkWidget *widget = GTK_WIDGET (object);
  
  switch (arg_id)
    {
    case ARG_HOMOGENEOUS:
      GTK_VALUE_BOOL (*arg) = wbox->homogeneous;
      break;
    case ARG_JUSTIFY:
      GTK_VALUE_ENUM (*arg) = wbox->justify;
      break;
    case ARG_LINE_JUSTIFY:
      GTK_VALUE_ENUM (*arg) = wbox->line_justify;
      break;
    case ARG_HSPACING:
      GTK_VALUE_UINT (*arg) = wbox->hspacing;
      break;
    case ARG_VSPACING:
      GTK_VALUE_UINT (*arg) = wbox->vspacing;
      break;
    case ARG_ASPECT_RATIO:
      GTK_VALUE_FLOAT (*arg) = wbox->aspect_ratio;
      break;
    case ARG_CURRENT_RATIO:
      GTK_VALUE_FLOAT (*arg) = (((gfloat) widget->allocation.width) /
				((gfloat) widget->allocation.height));
      break;
    case ARG_CHILD_LIMIT:
      GTK_VALUE_UINT (*arg) = wbox->child_limit;
      break;
    default:
      arg->type = GTK_TYPE_INVALID;
      break;
    }
}

static void
sp_wrap_box_set_child_property (GtkContainer    *container,
				 GtkWidget       *child,
				 guint            property_id,
				 const GValue    *value,
				 GParamSpec      *pspec)
{
  SPWrapBox *wbox = SP_WRAP_BOX (container);
  gboolean hexpand = FALSE, hfill = FALSE, vexpand = FALSE, vfill = FALSE, wrapped = FALSE;
  
  if (property_id != CHILD_PROP_POSITION)
    sp_wrap_box_query_child_packing (wbox, child, &hexpand, &hfill, &vexpand, &vfill, &wrapped);
  
  switch (property_id)
    {
    case CHILD_PROP_POSITION:
      sp_wrap_box_reorder_child (wbox, child, g_value_get_int (value));
      break;
    case CHILD_PROP_HEXPAND:
      sp_wrap_box_set_child_packing (wbox, child,
				      g_value_get_boolean (value), hfill,
				      vexpand, vfill,
				      wrapped);
      break;
    case CHILD_PROP_HFILL:
      sp_wrap_box_set_child_packing (wbox, child,
				      hexpand, g_value_get_boolean (value),
				      vexpand, vfill,
				      wrapped);
      break;
    case CHILD_PROP_VEXPAND:
      sp_wrap_box_set_child_packing (wbox, child,
				      hexpand, hfill,
				      g_value_get_boolean (value), vfill,
				      wrapped);
      break;
    case CHILD_PROP_VFILL:
      sp_wrap_box_set_child_packing (wbox, child,
				      hexpand, hfill,
				      vexpand, g_value_get_boolean (value),
				      wrapped);
      break;
    case CHILD_PROP_WRAPPED:
      sp_wrap_box_set_child_packing (wbox, child,
				      hexpand, hfill,
				      vexpand, vfill,
				      g_value_get_boolean (value));
      break;
    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
      break;
    }
}

static void
sp_wrap_box_get_child_property (GtkContainer    *container,
				 GtkWidget       *child,
				 guint            property_id,
				 GValue 	 *value,
				 GParamSpec      *pspec)
{
  SPWrapBox *wbox = SP_WRAP_BOX (container);
  gboolean hexpand = FALSE, hfill = FALSE, vexpand = FALSE, vfill = FALSE, wrapped = FALSE;
  
  if (property_id != CHILD_PROP_POSITION)
    sp_wrap_box_query_child_packing (wbox, child, &hexpand, &hfill, &vexpand, &vfill, &wrapped);
  
  switch (property_id)
    {
      SPWrapBoxChild *child_info;
      guint i;
    case CHILD_PROP_POSITION:
      i = 0;
      for (child_info = wbox->children; child_info; child_info = child_info->next)
	{
	  if (child_info->widget == child)
	    break;
	  i += 1;
	}
      g_value_set_int (value, child_info ? i : -1);
      break;
    case CHILD_PROP_HEXPAND:
      g_value_set_boolean (value, hexpand);
      break;
    case CHILD_PROP_HFILL:
      g_value_set_boolean (value, hfill);
      break;
    case CHILD_PROP_VEXPAND:
      g_value_set_boolean (value, vexpand);
      break;
    case CHILD_PROP_VFILL:
      g_value_set_boolean (value, vfill);
      break;
    case CHILD_PROP_WRAPPED:
      g_value_set_boolean (value, wrapped);
      break;
    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
      break;
    }
}

static GtkType
sp_wrap_box_child_type	(GtkContainer *container)
{
  return GTK_TYPE_WIDGET;
}

void
sp_wrap_box_set_homogeneous (SPWrapBox *wbox,
			      gboolean    homogeneous)
{
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  
  homogeneous = homogeneous != FALSE;
  if (wbox->homogeneous != homogeneous)
    {
      wbox->homogeneous = homogeneous;
      gtk_widget_queue_resize (GTK_WIDGET (wbox));
    }
}

void
sp_wrap_box_set_hspacing (SPWrapBox *wbox,
			   guint       hspacing)
{
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  
  if (wbox->hspacing != hspacing)
    {
      wbox->hspacing = hspacing;
      gtk_widget_queue_resize (GTK_WIDGET (wbox));
    }
}

void
sp_wrap_box_set_vspacing (SPWrapBox *wbox,
			   guint       vspacing)
{
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  
  if (wbox->vspacing != vspacing)
    {
      wbox->vspacing = vspacing;
      gtk_widget_queue_resize (GTK_WIDGET (wbox));
    }
}

void
sp_wrap_box_set_justify (SPWrapBox      *wbox,
			  GtkJustification justify)
{
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  g_return_if_fail (justify <= GTK_JUSTIFY_FILL);
  
  if (wbox->justify != justify)
    {
      wbox->justify = justify;
      gtk_widget_queue_resize (GTK_WIDGET (wbox));
    }
}

void
sp_wrap_box_set_line_justify (SPWrapBox      *wbox,
			       GtkJustification line_justify)
{
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  g_return_if_fail (line_justify <= GTK_JUSTIFY_FILL);
  
  if (wbox->line_justify != line_justify)
    {
      wbox->line_justify = line_justify;
      gtk_widget_queue_resize (GTK_WIDGET (wbox));
    }
}

void
sp_wrap_box_set_aspect_ratio (SPWrapBox *wbox,
			       gfloat      aspect_ratio)
{
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  
  aspect_ratio = CLAMP (aspect_ratio, 1.0 / 256.0, 256.0);
  
  if (wbox->aspect_ratio != aspect_ratio)
    {
      wbox->aspect_ratio = aspect_ratio;
      gtk_widget_queue_resize (GTK_WIDGET (wbox));
    }
}

void
sp_wrap_box_pack (SPWrapBox *wbox,
		   GtkWidget  *child,
		   gboolean    hexpand,
		   gboolean    hfill,
		   gboolean    vexpand,
		   gboolean    vfill)
{
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (child->parent == NULL);

  sp_wrap_box_pack_wrapped (wbox, child, hexpand, hfill, vexpand, vfill, FALSE);
}

void
sp_wrap_box_pack_wrapped (SPWrapBox *wbox,
			   GtkWidget  *child,
			   gboolean    hexpand,
			   gboolean    hfill,
			   gboolean    vexpand,
			   gboolean    vfill,
			   gboolean    wrapped)
{
  SPWrapBoxChild *child_info;
  
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (child->parent == NULL);
  
  child_info = g_new (SPWrapBoxChild, 1);
  child_info->widget = child;
  child_info->hexpand = hexpand ? TRUE : FALSE;
  child_info->hfill = hfill ? TRUE : FALSE;
  child_info->vexpand = vexpand ? TRUE : FALSE;
  child_info->vfill = vfill ? TRUE : FALSE;
  child_info->wrapped = wrapped ? TRUE : FALSE;
  child_info->next = NULL;
  if (wbox->children)
    {
      SPWrapBoxChild *last = wbox->children;
      
      while (last->next)
	last = last->next;
      last->next = child_info;
    }
  else
    wbox->children = child_info;
  wbox->n_children++;
  
  gtk_widget_set_parent (child, GTK_WIDGET (wbox));
  
  if (GTK_WIDGET_REALIZED (wbox))
    gtk_widget_realize (child);
  
  if (GTK_WIDGET_VISIBLE (wbox) && GTK_WIDGET_VISIBLE (child))
    {
      if (GTK_WIDGET_MAPPED (wbox))
	gtk_widget_map (child);
      
      gtk_widget_queue_resize (child);
    }
}

void
sp_wrap_box_reorder_child (SPWrapBox *wbox,
			    GtkWidget  *child,
			    gint        position)
{
  SPWrapBoxChild *child_info, *last = NULL;
  
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  g_return_if_fail (GTK_IS_WIDGET (child));
  
  for (child_info = wbox->children; child_info; last = child_info, child_info = last->next)
    if (child_info->widget == child)
      break;
  
  if (child_info && wbox->children->next)
    {
      SPWrapBoxChild *tmp;
      
      if (last)
	last->next = child_info->next;
      else
	wbox->children = child_info->next;
      
      last = NULL;
      tmp = wbox->children;
      while (position && tmp->next)
	{
	  position--;
	  last = tmp;
	  tmp = last->next;
	}
      
      if (position)
	{
	  tmp->next = child_info;
	  child_info->next = NULL;
	}
      else
	{
	  child_info->next = tmp;
	  if (last)
	    last->next = child_info;
	  else
	    wbox->children = child_info;
	}
      
      if (GTK_WIDGET_VISIBLE (child) && GTK_WIDGET_VISIBLE (wbox))
	gtk_widget_queue_resize (child);
    }
}

void
sp_wrap_box_query_child_packing (SPWrapBox *wbox,
				  GtkWidget  *child,
				  gboolean   *hexpand,
				  gboolean   *hfill,
				  gboolean   *vexpand,
				  gboolean   *vfill,
				  gboolean   *wrapped)
{
  SPWrapBoxChild *child_info;
  
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  g_return_if_fail (GTK_IS_WIDGET (child));
  
  for (child_info = wbox->children; child_info; child_info = child_info->next)
    if (child_info->widget == child)
      break;
  
  if (child_info)
    {
      if (hexpand)
	*hexpand = child_info->hexpand;
      if (hfill)
	*hfill = child_info->hfill;
      if (vexpand)
	*vexpand = child_info->vexpand;
      if (vfill)
	*vfill = child_info->vfill;
      if (wrapped)
	*wrapped = child_info->wrapped;
    }
}

void
sp_wrap_box_set_child_packing (SPWrapBox *wbox,
				GtkWidget  *child,
				gboolean    hexpand,
				gboolean    hfill,
				gboolean    vexpand,
				gboolean    vfill,
				gboolean    wrapped)
{
  SPWrapBoxChild *child_info;
  
  g_return_if_fail (GTK_IS_WRAP_BOX (wbox));
  g_return_if_fail (GTK_IS_WIDGET (child));
  
  hexpand = hexpand != FALSE;
  hfill = hfill != FALSE;
  vexpand = vexpand != FALSE;
  vfill = vfill != FALSE;
  wrapped = wrapped != FALSE;

  for (child_info = wbox->children; child_info; child_info = child_info->next)
    if (child_info->widget == child)
      break;
  
  if (child_info &&
      (child_info->hexpand != hexpand || child_info->vexpand != vexpand ||
       child_info->hfill != hfill || child_info->vfill != vfill ||
       child_info->wrapped != wrapped))
    {
      child_info->hexpand = hexpand;
      child_info->hfill = hfill;
      child_info->vexpand = vexpand;
      child_info->vfill = vfill;
      child_info->wrapped = wrapped;
      
      if (GTK_WIDGET_VISIBLE (child) && GTK_WIDGET_VISIBLE (wbox))
	gtk_widget_queue_resize (child);
    }
}

guint*
sp_wrap_box_query_line_lengths (SPWrapBox *wbox,
				 guint      *_n_lines)
{
  SPWrapBoxChild *next_child = NULL;
  GtkAllocation area, *allocation;
  gboolean expand_line;
  GSList *slist;
  guint max_child_size, border, n_lines = 0, *lines = NULL;

  if (_n_lines)
    *_n_lines = 0;
  g_return_val_if_fail (GTK_IS_WRAP_BOX (wbox), NULL);

  allocation = &GTK_WIDGET (wbox)->allocation;
  border = GTK_CONTAINER (wbox)->border_width;
  area.x = allocation->x + border;
  area.y = allocation->y + border;
  area.width = MAX (1, (gint) allocation->width - border * 2);
  area.height = MAX (1, (gint) allocation->height - border * 2);

  next_child = wbox->children;
  slist = SP_WRAP_BOX_GET_CLASS (wbox)->rlist_line_children (wbox,
							      &next_child,
							      &area,
							      &max_child_size,
							      &expand_line);
  while (slist)
    {
      guint l = n_lines++;

      lines = g_renew (guint, lines, n_lines);
      lines[l] = g_slist_length (slist);
      g_slist_free (slist);

      slist = SP_WRAP_BOX_GET_CLASS (wbox)->rlist_line_children (wbox,
								  &next_child,
								  &area,
								  &max_child_size,
								  &expand_line);
    }

  if (_n_lines)
    *_n_lines = n_lines;

  return lines;
}

static void
sp_wrap_box_map (GtkWidget *widget)
{
  SPWrapBox *wbox = SP_WRAP_BOX (widget);
  SPWrapBoxChild *child;
  
  GTK_WIDGET_SET_FLAGS (wbox, GTK_MAPPED);
  
  for (child = wbox->children; child; child = child->next)
    if (GTK_WIDGET_VISIBLE (child->widget) &&
	!GTK_WIDGET_MAPPED (child->widget))
      gtk_widget_map (child->widget);
}

static void
sp_wrap_box_unmap (GtkWidget *widget)
{
  SPWrapBox *wbox = SP_WRAP_BOX (widget);
  SPWrapBoxChild *child;
  
  GTK_WIDGET_UNSET_FLAGS (wbox, GTK_MAPPED);
  
  for (child = wbox->children; child; child = child->next)
    if (GTK_WIDGET_VISIBLE (child->widget) &&
	GTK_WIDGET_MAPPED (child->widget))
      gtk_widget_unmap (child->widget);
}

static gint
sp_wrap_box_expose (GtkWidget      *widget,
		     GdkEventExpose *event)
{
  return GTK_WIDGET_CLASS (parent_class)->expose_event (widget, event);
}

static void
sp_wrap_box_add (GtkContainer *container,
		  GtkWidget    *widget)
{
  sp_wrap_box_pack (SP_WRAP_BOX (container), widget, FALSE, TRUE, FALSE, TRUE);
}

static void
sp_wrap_box_remove (GtkContainer *container,
		     GtkWidget    *widget)
{
  SPWrapBox *wbox = SP_WRAP_BOX (container);
  SPWrapBoxChild *child, *last = NULL;
  
  child = wbox->children;
  while (child)
    {
      if (child->widget == widget)
	{
	  gboolean was_visible;
	  
	  was_visible = GTK_WIDGET_VISIBLE (widget);
	  gtk_widget_unparent (widget);
	  
	  if (last)
	    last->next = child->next;
	  else
	    wbox->children = child->next;
	  g_free (child);
	  wbox->n_children--;
	  
	  if (was_visible)
	    gtk_widget_queue_resize (GTK_WIDGET (container));
	  
	  break;
	}
      
      last = child;
      child = last->next;
    }
}

static void
sp_wrap_box_forall (GtkContainer *container,
		     gboolean      include_internals,
		     GtkCallback   callback,
		     gpointer      callback_data)
{
  SPWrapBox *wbox = SP_WRAP_BOX (container);
  SPWrapBoxChild *child;
  
  child = wbox->children;
  while (child)
    {
      GtkWidget *widget = child->widget;
      
      child = child->next;
      
      callback (widget, callback_data);
    }
}
