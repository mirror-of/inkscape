#include "config.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <glib/gprintf.h>

#include <map>
#include <string>


#include "recolorwheel.h"
#include <math.h>
#include "recolor-wheel-node.h"

/* Default ring fraction */
#define DEFAULT_FRACTION 0.1

/* Default width/height */
#define DEFAULT_SIZE 100

/* Default ring width */
#define DEFAULT_RING_WIDTH 10

//--
/* Maximum Selectable objects for recoloring */
#define RECOLOR_MAX_OBJECTS 5

/*Outer Radius of Node*/
#define NODE_RADIUS_OUTER 8

/*Inner Radius of Node*/
#define NODE_RADIUS_INNER 4
//--

//RecolorWheelNode* _nodes1[RECOLOR_MAX_OBJECTS];
RecolorWheelNode* _activeNode = NULL ;

/* Dragging modes */
typedef enum
{
  DRAG_NONE,
  DRAG_HS,
  DRAG_RECOLOR_NODE,
  DRAG_V
} DragMode;

/* Private part of the RecolorWheel structure */
typedef struct
{
  /* Color value */
  gdouble h;
  gdouble s;
  gdouble v;

  std::map<std::string,RecolorWheelNode> nodes;
    
/* ring_width is this fraction of size */
  gdouble ring_fraction;
  //-- Don't need ring fraction anymore as the Wheel is painted completely.

  /* Size and ring width */
  gint size;
  gint ring_width;
  //-- Don't need ring fraction anymore as the Wheel is painted completely.

  /* Window for capturing events */
  GdkWindow *window;
  
  /* Dragging mode */
  DragMode mode;

  guint focus_on_ring : 1;
  //-- Should be renamed to focus_on_wheel
   
} RecolorWheelPrivate;

enum
{
  CHANGED,
  MOVE,
  LAST_SIGNAL
};

static void     recolor_wheel_map            (GtkWidget          *widget);
static void     recolor_wheel_unmap          (GtkWidget          *widget);
static void     recolor_wheel_realize        (GtkWidget          *widget);
static void     recolor_wheel_unrealize      (GtkWidget          *widget);
static void     recolor_wheel_size_allocate  (GtkWidget          *widget,
                                                 GtkAllocation      *allocation);
static gboolean recolor_wheel_button_press   (GtkWidget          *widget,
                                                 GdkEventButton     *event);
static gboolean recolor_wheel_button_release (GtkWidget          *widget,
                                                 GdkEventButton     *event);
static gboolean recolor_wheel_motion         (GtkWidget          *widget,
                                                 GdkEventMotion     *event);
#if GTK_CHECK_VERSION(3,0,0)
static gboolean recolor_wheel_draw           (GtkWidget          *widget,
                                                 cairo_t            *cr);
static void     recolor_wheel_get_preferred_width (GtkWidget     *widget,
		                                 gint               *minimum_width,
						 gint               *natural_width);
static void     recolor_wheel_get_preferred_height (GtkWidget    *widget,
		                                 gint               *minimum_height,
						 gint               *natural_height);
#else
static gboolean recolor_wheel_expose         (GtkWidget          *widget,
                                                 GdkEventExpose     *event);
static void     recolor_wheel_size_request   (GtkWidget          *widget,
                                                 GtkRequisition     *requisition);
#endif

static gboolean recolor_wheel_grab_broken    (GtkWidget          *widget,
                                                 GdkEventGrabBroken *event);
static gboolean recolor_wheel_focus          (GtkWidget          *widget,
                                                 GtkDirectionType    direction);
static void     recolor_wheel_move           (RecolorWheel     *wheel,
                                                 GtkDirectionType    dir);


static guint wheel_signals[LAST_SIGNAL];

G_DEFINE_TYPE (RecolorWheel, recolor_wheel, GTK_TYPE_WIDGET)

#define parent_class recolor_wheel_parent_class

void add_node_to_recolor_wheel (RecolorWheel *wheel, std::string name, RecolorWheelNode node)
{
    RecolorWheelPrivate *priv  = (RecolorWheelPrivate*) wheel->priv;
    priv->nodes.insert( std::pair<std::string,RecolorWheelNode>(name,node) );
}

void remove_node_to_recolor_wheel (RecolorWheel *wheel, std::string name)
{
    RecolorWheelPrivate *priv  = (RecolorWheelPrivate*) wheel->priv;

    std::map<std::string,RecolorWheelNode>::iterator iter;
    iter = priv->nodes.find(name);
    priv->nodes.erase( iter );   
}


static void
recolor_wheel_nodes_check1()
{
    //g_printf("\n\t\tChecking nodes ");
    int i;
    for(i=0; i< RECOLOR_MAX_OBJECTS ; i++)
    {   
        //g_printf("\n Id = %d\n r = %6.3f\n g = %6.3f\n b = %6.3f\n x = %f\n y = %f\n", i, _nodes1[i]->_color[0],_nodes1[i]->_color[1],
        //          _nodes1[i]->_color[2],_nodes1[i]->x,_nodes1[i]->y
         //         );
    }
    //g_printf("\n\t\tOver!-----");    
}

/*Initialize RecolorWheelNode*/
void recolor_wheel_node_set_color1 (RecolorWheelNode* node, gfloat h, gfloat s, gfloat v)
{
    node->_color[0] = h;
    node->_color[1] = s;
    node->_color[2] = v;
} 

static void
recolor_wheel_class_init (RecolorWheelClass *klass)
{
  GObjectClass        *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass      *widget_class = GTK_WIDGET_CLASS (klass);
  RecolorWheelClass *wheel_class  = RECOLOR_WHEEL_CLASS (klass);
  GtkBindingSet       *binding_set;

  widget_class->map                  = recolor_wheel_map;
  widget_class->unmap                = recolor_wheel_unmap;
  widget_class->realize              = recolor_wheel_realize;
  widget_class->unrealize            = recolor_wheel_unrealize;
  widget_class->size_allocate        = recolor_wheel_size_allocate;
  widget_class->button_press_event   = recolor_wheel_button_press;
  widget_class->button_release_event = recolor_wheel_button_release;
  widget_class->motion_notify_event  = recolor_wheel_motion;

#if GTK_CHECK_VERSION(3,0,0)
  widget_class->get_preferred_width  = recolor_wheel_get_preferred_width;
  widget_class->get_preferred_height = recolor_wheel_get_preferred_height;
  widget_class->draw                 = recolor_wheel_draw;
#else
  widget_class->size_request         = recolor_wheel_size_request;
  widget_class->expose_event         = recolor_wheel_expose;
#endif

  widget_class->focus                = recolor_wheel_focus;
  widget_class->grab_broken_event    = recolor_wheel_grab_broken;

  wheel_class->move                  = recolor_wheel_move;
  
  wheel_signals[CHANGED] =
    g_signal_new ("changed",
                  G_OBJECT_CLASS_TYPE (object_class),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (RecolorWheelClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  wheel_signals[MOVE] =
    g_signal_new ("move",
                  G_OBJECT_CLASS_TYPE (object_class),
                  (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                  G_STRUCT_OFFSET (RecolorWheelClass, move),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1,
                  GTK_TYPE_DIRECTION_TYPE);
  
  //-- added for simulation
      /* recolor_wheel_node_set_color1(_nodes1[0], 0.65, 0.5, 0.33);
      recolor_wheel_node_set_color1(_nodes1[1], 0.12, 0.67, 0.45);
      recolor_wheel_node_set_color1(_nodes1[2], 1.0, 0.54, 0.167);
      recolor_wheel_node_set_color1(_nodes1[3], 0.34, 0.91, 0.43);
      recolor_wheel_node_set_color1(_nodes1[4], 1.0, 0.27, 0.88); */
  //--
  
  binding_set = gtk_binding_set_by_class (klass);

  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Up, (GdkModifierType) 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_UP);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_KP_Up, (GdkModifierType) 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_UP);

  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Down, (GdkModifierType) 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_DOWN);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_KP_Down, (GdkModifierType) 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_DOWN);


  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Right, (GdkModifierType) 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_RIGHT);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_KP_Right, (GdkModifierType) 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_RIGHT);

  gtk_binding_entry_add_signal (binding_set, GDK_KEY_Left, (GdkModifierType) 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_LEFT);
  gtk_binding_entry_add_signal (binding_set, GDK_KEY_KP_Left, (GdkModifierType) 0,
                                "move", 1,
                                G_TYPE_ENUM, GTK_DIR_LEFT);

  g_type_class_add_private (object_class, sizeof (RecolorWheelPrivate));
}

static void
recolor_wheel_init (RecolorWheel *wheel)
{
  g_printf("\nWe are here: recolor_wheel_init (RecolorWheel *wheel) ! ");
  
  RecolorWheelPrivate *priv;

  priv = G_TYPE_INSTANCE_GET_PRIVATE (wheel, RECOLOR_TYPE_COLOR_WHEEL,
                                      RecolorWheelPrivate);

  wheel->priv = (RecolorWheelPrivate*)priv;
  priv = (RecolorWheelPrivate*)wheel->priv ;
  new (&(priv->nodes)) std::map<std::string,RecolorWheelNode>();
  //priv->nodes = new std::map<std::string,RecolorWheelNode>();

  gtk_widget_set_has_window (GTK_WIDGET (wheel), FALSE);
  gtk_widget_set_can_focus (GTK_WIDGET (wheel), TRUE);

  priv->ring_fraction = DEFAULT_FRACTION;
  priv->size          = DEFAULT_SIZE;
  priv->ring_width    = DEFAULT_RING_WIDTH;
  //-- Again ring_fraction and ring_width will not be needed
  //--
  
  //--
  g_printf("\nWe are here: recolor_wheel_init (RecolorWheel *wheel) Ending....! ");
  
}

static void
recolor_wheel_map (GtkWidget *widget)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv  = (RecolorWheelPrivate*) wheel->priv;

  GTK_WIDGET_CLASS (parent_class)->map (widget);

  gdk_window_show (priv->window);
}

static void
recolor_wheel_unmap (GtkWidget *widget)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv  = (RecolorWheelPrivate*) wheel->priv;

  gdk_window_hide (priv->window);

  GTK_WIDGET_CLASS (parent_class)->unmap (widget);
}

static void
recolor_wheel_realize (GtkWidget *widget)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv  = (RecolorWheelPrivate*) wheel->priv;
  GtkAllocation          allocation;
  GdkWindowAttr          attr;
  gint                   attr_mask;
  GdkWindow             *parent_window;

  gtk_widget_get_allocation (widget, &allocation);

  gtk_widget_set_realized (widget, TRUE);

  attr.window_type = GDK_WINDOW_CHILD;
  attr.x           = allocation.x;
  attr.y           = allocation.y;
  attr.width       = allocation.width;
  attr.height      = allocation.height;
  attr.wclass      = GDK_INPUT_ONLY;
  attr.event_mask  = (gtk_widget_get_events (widget) |
                      GDK_KEY_PRESS_MASK      |
                      GDK_BUTTON_PRESS_MASK   |
                      GDK_BUTTON_RELEASE_MASK |
                      GDK_POINTER_MOTION_MASK |
                      GDK_ENTER_NOTIFY_MASK   |
                      GDK_LEAVE_NOTIFY_MASK);
   //-- The events that will be reported. Consider Scrollup and down for hue values.

  attr_mask = GDK_WA_X | GDK_WA_Y;

  parent_window = gtk_widget_get_parent_window (widget);

  gtk_widget_set_window (widget, parent_window);
  g_object_ref (parent_window);

  priv->window = gdk_window_new (parent_window, &attr, attr_mask);
  gdk_window_set_user_data (priv->window, wheel);

  gtk_widget_style_attach (widget);
}

static void
recolor_wheel_unrealize (GtkWidget *widget)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv  = (RecolorWheelPrivate*) wheel->priv;

  gdk_window_set_user_data (priv->window, NULL);
  gdk_window_destroy (priv->window);
  priv->window = NULL;

  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

#if GTK_CHECK_VERSION(3,0,0)
static void
recolor_wheel_get_preferred_width (GtkWidget *widget,
                                      gint      *minimum_width,
				      gint      *natural_width)
{
  gint focus_width;
  gint focus_pad;

  gtk_widget_style_get (widget,
                        "focus-line-width", &focus_width,
                        "focus-padding", &focus_pad,
                        NULL);
  
  *minimum_width = *natural_width = DEFAULT_SIZE + 2 * (focus_width + focus_pad);
}

static void
recolor_wheel_get_preferred_height (GtkWidget *widget,
                                       gint      *minimum_height,
				       gint      *natural_height)
{
  gint focus_width;
  gint focus_pad;

  gtk_widget_style_get (widget,
                        "focus-line-width", &focus_width,
                        "focus-padding", &focus_pad,
                        NULL);
  
  *minimum_height = *natural_height = DEFAULT_SIZE + 2 * (focus_width + focus_pad);
}
#else
static void
recolor_wheel_size_request (GtkWidget      *widget,
                               GtkRequisition *requisition)
{
  gint focus_width;
  gint focus_pad;

  gtk_widget_style_get (widget,
                        "focus-line-width", &focus_width,
                        "focus-padding", &focus_pad,
                        NULL);

  requisition->width  = DEFAULT_SIZE + 2 * (focus_width + focus_pad);
  requisition->height = DEFAULT_SIZE + 2 * (focus_width + focus_pad);
}
#endif

static void
recolor_wheel_size_allocate (GtkWidget     *widget,
                                GtkAllocation *allocation)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv  = (RecolorWheelPrivate*) wheel->priv;
  gint                   focus_width;
  gint                   focus_pad;

  gtk_widget_set_allocation (widget, allocation);

  gtk_widget_style_get (widget,
                        "focus-line-width", &focus_width,
                        "focus-padding",    &focus_pad,
                        NULL);

  priv->size = MIN (allocation->width  - 2 * (focus_width + focus_pad),
                    allocation->height - 2 * (focus_width + focus_pad));

  priv->ring_width = priv->size * priv->ring_fraction;

  if (gtk_widget_get_realized (widget))
    gdk_window_move_resize (priv->window,
                            allocation->x,
                            allocation->y,
                            allocation->width,
                            allocation->height);
}


/* Utility functions */

#define INTENSITY(r, g, b) ((r) * 0.30 + (g) * 0.59 + (b) * 0.11)


//-- redundant code TODO: Remove
static void
rgb_to_hsv_floatv (float *hsv, float r, float g, float b)
{
    float max, min, delta;

    max = MAX (MAX (r, g), b);
    min = MIN (MIN (r, g), b);
    delta = max - min;

    hsv[2] = max;

    if (max > 0) {
        hsv[1] = delta / max;
    } else {
        hsv[1] = 0.0;
    }

    if (hsv[1] != 0.0) {
        if (r == max) {
            hsv[0] = (g - b) / delta;
        } else if (g == max) {
            hsv[0] = 2.0 + (b - r) / delta;
        } else {
            hsv[0] = 4.0 + (r - g) / delta;
        }

        hsv[0] = hsv[0] / 6.0;

        if (hsv[0] < 0) hsv[0] += 1.0;
    }
    else
        hsv[0] = 0.0;
}

/* Converts from HSV to RGB */
static void
hsv_to_rgb (gdouble *h,
            gdouble *s,
            gdouble *v)
{
  gdouble hue, saturation, value;
  gdouble f, p, q, t;

  if (*s == 0.0)
    {
      *h = *v;
      *s = *v;
      *v = *v; /* heh */
    }
  else
    {
      hue = *h * 6.0;
      saturation = *s;
      value = *v;

      if (hue == 6.0)
        hue = 0.0;

      f = hue - (int) hue;
      p = value * (1.0 - saturation);
      q = value * (1.0 - saturation * f);
      t = value * (1.0 - saturation * (1.0 - f));

      switch ((int) hue)
        {
        case 0:
          *h = value;
          *s = t;
          *v = p;
          break;

        case 1:
          *h = q;
          *s = value;
          *v = p;
          break;

        case 2:
          *h = p;
          *s = value;
          *v = t;
          break;

        case 3:
          *h = p;
          *s = q;
          *v = value;
          break;

        case 4:
          *h = t;
          *s = p;
          *v = value;
          break;

        case 5:
          *h = value;
          *s = p;
          *v = q;
          break;

        default:
          g_assert_not_reached ();
        }
    }
}

/* Computes whether a point is inside the hue ring */
//-- question its very existence ? Because the focus_on_ring gint
//-- was just to make sure the hue was being changed.
static gboolean
is_in_ring (RecolorWheel *wheel,
            gdouble         x,
            gdouble         y)
{
  RecolorWheelPrivate *priv =(RecolorWheelPrivate*) wheel->priv;
  GtkAllocation          allocation;
  gdouble                dx, dy, dist;
  gdouble                center_x;
  gdouble                center_y;
  gdouble                /*inner,*/ outer;
  //-- Inner not needed in Recolor Artwork wheel.

  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;

  outer = priv->size / 2.0;
  //inner = outer - priv->ring_width;
  //-- Inner not needed in Recolor Artwork wheel.

  dx = x - center_x;
  dy = center_y - y;
  dist = dx * dx + dy * dy;
  
  return ( dist <= outer * outer );
}



static gboolean
is_in_recolor_node (RecolorWheel *wheel,
                        gdouble         x,
                        gdouble         y)
{
  RecolorWheelPrivate   *priv = (RecolorWheelPrivate*)wheel->priv;
  GtkAllocation               allocation;
  gdouble                       dx, dy, dist;
  gdouble                       center_x;
  gdouble                       center_y;
  gdouble                       outer;
  gint                             iter;
  
  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;

  outer = priv->size / 2.0;
  
  for (std::map<std::string,RecolorWheelNode>::iterator it = priv->nodes.begin(); it != priv->nodes.end(); ++it)
  {
#define RECOLOR_DELTA 25
     //g_printf("iteration of %d",iter); 
     
     if ( (ABS((int)(x-(*it).second.x) ) < RECOLOR_DELTA) && (ABS((int) ( y-(*it).second.y) ) < RECOLOR_DELTA) )
     {   
        
        //g_printf("\ni=%d id=%d x=%6.2f _x=%6.2f y=%6.2f _y=%6.2f",iter,_nodes1[iter]->_id,x,_nodes1[iter]->x,y,_nodes1[iter]->y);
        
        //_activeNode = &((*it).second);
        return TRUE;
     }
  }
  return FALSE;
}
                     
/*Computes a saturation value based the new recolor artwork wheel*/
static double
compute_s (RecolorWheel *wheel,
            gdouble         x,
            gdouble         y
            /* gdouble        *s */)
{
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  GtkAllocation          allocation;
  gdouble                dx, dy, dist;
  gdouble                center_x;
  gdouble                center_y;
  gdouble                outer;
  
  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;

  outer = priv->size / 2.0;
  
  dx = x - center_x;
  dy = center_y - y;
  dist = dx * dx + dy * dy;
  return  ( sqrt(dist) / (outer) );
  
  }            

/* Computes a value based on the mouse coordinates */
static double
compute_v (RecolorWheel *wheel,
           gdouble         x,
           gdouble         y)
{
  GtkAllocation allocation;
  gdouble       center_x;
  gdouble       center_y;
  gdouble       dx, dy;
  gdouble       angle;

  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;

  dx = x - center_x;
  dy = center_y - y;

  angle = atan2 (dy, dx);
  if (angle < 0.0)
    angle += 2.0 * G_PI;

  return angle / (2.0 * G_PI);
}
//--TODO: Remove this function to add a bar that controls the brightness.
//-- the new compute_hs function is what should be replaced

static void
compute_hs (RecolorWheel *wheel,
           gdouble         x,
           gdouble         y,
           gdouble *h,
           gdouble *s)
{
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  GtkAllocation allocation;
  gdouble       center_x;
  gdouble       center_y;
  gdouble       dx, dy, dist;
  gdouble       angle;
  gdouble       outer;
  

  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);

  center_x = allocation.width / 2.0;
  center_y = allocation.height / 2.0;
  
  outer=priv->size / 2.0;

  dx = x - center_x;
  dy = center_y - y;
  dist = dx * dx + dy * dy;

  angle = atan2 (dy, dx);
  if (angle < 0.0)
    angle += 2.0 * G_PI;

  *h = angle / (2.0 * G_PI);
  *s = sqrt(dist) / (outer);
}
//-- Added Utility function that shall be used in recolor artwork
//-- 2:05 PM 6/19/2013 yet to be tested

static void
set_cross_grab (RecolorWheel *wheel,
                guint32         time)
//Description: Sets the ' + ' cursor that facilitates GUI for selection.
{
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  GdkCursor             *cursor;

  cursor =
    gdk_cursor_new_for_display (gtk_widget_get_display (GTK_WIDGET (wheel)),
                                GDK_CROSSHAIR);

#if GTK_CHECK_VERSION(3,0,0)
  gdk_device_grab (gtk_get_current_event_device(),
		   priv->window,
		   GDK_OWNERSHIP_NONE,
		   FALSE,
		   GDK_POINTER_MOTION_MASK      |
		   GDK_POINTER_MOTION_HINT_MASK |
		   GDK_BUTTON_RELEASE_MASK,
		   cursor, time);
  g_object_unref (cursor);
#else
  gdk_pointer_grab (priv->window, FALSE,
                    (GdkEventMask)(GDK_POINTER_MOTION_MASK      |
                    GDK_POINTER_MOTION_HINT_MASK |
                    GDK_BUTTON_RELEASE_MASK),
                    NULL, cursor, time);
  gdk_cursor_unref (cursor);
#endif
}
//-- purpose unclear

static gboolean recolor_wheel_grab_broken(GtkWidget *widget, GdkEventGrabBroken *event)
{
    (void)event;
    RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
    RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;

    priv->mode = DRAG_NONE;

    return TRUE;
}

static void recolor_drag_node( RecolorWheel *wheel, gdouble x, gdouble y)
{
  _activeNode->x = x;
  _activeNode->y = y;
  
  gdouble        h, s, v;
  
  recolor_wheel_get_color(wheel, &h, &s, &v);
  
  //hsv_to_rgb (&h, &s, &v);
  
  _activeNode->_color[0] = h;
  _activeNode->_color[1] = s;
  _activeNode->_color[2] = v;  
}

static gboolean
recolor_wheel_button_press (GtkWidget      *widget,
                               GdkEventButton *event)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  gdouble                x, y;

  if (priv->mode != DRAG_NONE || event->button != 1)
  //-- event->button != 1 ? GdkDestroy ?? Why put here ??
    return FALSE;

  x = event->x;
  y = event->y;
  
  if (is_in_recolor_node (wheel, x, y))
    {
      priv->mode = DRAG_RECOLOR_NODE;
                
      set_cross_grab (wheel, event->time);

      
      recolor_wheel_set_color (wheel,
                                  compute_v (wheel, x, y),
                                  compute_s (wheel, x, y),
                                  priv->v);
      
      gtk_widget_grab_focus (widget);
      priv->focus_on_ring = TRUE;

      return TRUE;
    }
    
    
  /*if (is_in_triangle (wheel, x, y))
    {
      gdouble s, v;

      priv->mode = DRAG_SV;
      set_cross_grab (wheel, event->time);

      compute_sv (wheel, x, y, &s, &v);
      recolor_wheel_set_color (wheel, priv->h, s, v);

      gtk_widget_grab_focus (widget);
      priv->focus_on_ring = FALSE;

      return TRUE;
    }*/
  return FALSE;
}

static gboolean
recolor_wheel_button_release (GtkWidget      *widget,
                                 GdkEventButton *event)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  DragMode               mode;
  gdouble                x, y;

  if (priv->mode == DRAG_NONE || event->button != 1)
    return FALSE;

  /* Set the drag mode to DRAG_NONE so that signal handlers for "catched"
   * can see that this is the final color state.
   */
  mode = priv->mode;
  priv->mode = DRAG_NONE;

  x = event->x;
  y = event->y;

  /*if (mode == DRAG_HS)
    {
      recolor_wheel_set_color (wheel,
                                  compute_v (wheel, x, y), compute_s (wheel, x, y), priv->v);   
    }
  */
  if (mode == DRAG_RECOLOR_NODE)
    {
      recolor_wheel_set_color (wheel,
                                  compute_v (wheel, x, y), compute_s (wheel, x, y), priv->v);
      //recolor_drag_node(wheel, x, y);
    }
  else
    g_assert_not_reached ();

#if GTK_CHECK_VERSION(3,0,0)
  gdk_device_ungrab (gtk_get_current_event_device(),
                     event->time);
#else
  gdk_display_pointer_ungrab (gdk_window_get_display (event->window),
                              event->time);
#endif
  
  recolor_wheel_nodes_check1();
  
  return TRUE;
}

static gboolean
recolor_wheel_motion (GtkWidget      *widget,
                         GdkEventMotion *event)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  gdouble                x, y;

  if (priv->mode == DRAG_NONE)
    return FALSE;

  gdk_event_request_motions (event);
  x = event->x;
  y = event->y;
  
  gint id ;
  
  if (priv->mode == DRAG_RECOLOR_NODE)
  {
     recolor_wheel_set_color (wheel,
                                      compute_v (wheel, x, y),
                                      compute_s (wheel, x, y),
                                      priv->v);
        
    recolor_drag_node(wheel, x, y);
    gtk_widget_grab_focus (widget);
    priv->focus_on_ring = TRUE;

    return TRUE;
  }

  /*else if (priv->mode == DRAG_HS)
    {
      recolor_wheel_set_color (wheel,
                                  compute_v (wheel, x, y), compute_s (wheel, x, y), priv->v);
      return TRUE;
    }*/
  
  g_assert_not_reached ();

  return FALSE;
}

static void
paint_recolor_nodes_to_wheel (RecolorWheel *wheel,
                              cairo_t        *cr )
{
  //g_printf("\nPainting nodes");
  GtkWidget             *widget = GTK_WIDGET (wheel);
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  gdouble                r, g, b;
  gdouble                center_x;
  gdouble                center_y;
  gdouble                xx, yy, dist, outer;
  gdouble                width, height;
  gfloat                 hsv[3];
  gint                   iter;   

#if GTK_CHECK_VERSION(3,0,0)
  GtkWidget             *widget = GTK_WIDGET (wheel);
  GtkStyleContext       *context;
  width  = gtk_widget_get_allocated_width  (widget);
  height = gtk_widget_get_allocated_height (widget);
#else
  GtkAllocation          allocation;
  gchar                 *detail;  
  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);
  width  = allocation.width;
  height = allocation.height;
#endif
  
  center_x = width  / 2.0;
  center_y = height / 2.0;
  
  outer = priv->size / 2.0;
  g_printf("\nWe are here: paint_nodes() ! ");
  for (std::map<std::string,RecolorWheelNode>::iterator iter = priv->nodes.begin(); iter != priv->nodes.end(); ++iter)
  {
  
    
    dist = (*iter).second._color[1] * outer ;
    
    xx = center_x + cos ((*iter).second._color[0] * 2.0 * G_PI) * dist;
    yy = center_y - sin ((*iter).second._color[0] * 2.0 * G_PI) * dist;
    
    (*iter).second.x = (gfloat)xx;
    (*iter).second.y = (gfloat)yy;
    
    r = (*iter).second._color[0];
    g = (*iter).second._color[1];
    b = (*iter).second._color[2];
    hsv_to_rgb (&r, &g, &b);
    
#if GTK_CHECK_VERSION(3,0,0)
    context = gtk_widget_get_style_context (widget);

    gtk_style_context_save (context);
#endif

#define RADIUS 4

#define FOCUS_RADIUS 6
    
    cairo_set_source_rgb( cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr,center_x, center_y);
    cairo_line_to (cr,xx,yy);
    cairo_arc (cr, xx, yy, NODE_RADIUS_OUTER+1, 0, 2 * G_PI);
    cairo_stroke (cr);
    cairo_new_path (cr);
    cairo_set_source_rgb( cr, 1.0, 1.0, 1.0);
    cairo_arc (cr, xx, yy, NODE_RADIUS_OUTER, 0, 2 * G_PI);
    cairo_fill (cr);
    cairo_new_path (cr);
    cairo_set_source_rgb( cr, r, g, b);
    cairo_arc (cr, xx, yy, NODE_RADIUS_INNER, 0, 2 * G_PI);
    cairo_fill (cr);
    cairo_set_source_rgb( cr, 0.0, 0.0, 0.0);
    cairo_arc (cr, xx, yy, NODE_RADIUS_INNER, 0, 2 * G_PI);
    cairo_set_line_width(cr,cairo_get_line_width(cr)/2.0);
    cairo_stroke (cr);
    cairo_set_line_width(cr,cairo_get_line_width(cr)*2.0);
    
    
#if GTK_CHECK_VERSION(3,0,0)
    gtk_style_context_restore (context);
#endif
  }

}

/* Redrawing */

/* Paints the hue ring */
static void
paint_ring (RecolorWheel *wheel,
            cairo_t        *cr)
{
#if GTK_CHECK_VERSION(3,0,0)
  GtkWidget             *widget = GTK_WIDGET (wheel);
#else
  GtkAllocation          allocation;
#endif
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  gint                   width, height;
  gint                   xx, yy;
  gdouble                dx, dy, dist;
  gdouble                center_x;
  gdouble                center_y;
  gdouble                outer, sat_thres;
  guint32               *buf, *p;
  gdouble                angle;
  gdouble                hue;
  gdouble                r, g, b;
  cairo_surface_t       *source;
  cairo_t               *source_cr;
  gint                   stride;

#if GTK_CHECK_VERSION(3,0,0)
  width  = gtk_widget_get_allocated_width  (widget);
  height = gtk_widget_get_allocated_height (widget);
#else
  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);
  width  = allocation.width;
  height = allocation.height;
#endif

  center_x = width  / 2.0;
  center_y = height / 2.0;

  outer = priv->size / 2.0;
  sat_thres=0.35*outer;

  /* Create an image initialized with the ring colors */
  stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, width);
  buf = g_new (guint32, height * stride / 4);

  for (yy = 0; yy < height; yy++)
    {
      p = buf + yy * width;
      dy = -(yy - center_y);

      for (xx = 0; xx < width; xx++)
        {
          dx = xx - center_x;

          dist = dx * dx + dy * dy;
          if ( dist > ((outer+1) * (outer+1)) )
            {
              *p++ = 0;
              continue;
            }

          angle = atan2 (dy, dx);
          if (angle < 0.0)
            angle += 2.0 * G_PI;

          hue = angle / (2.0 * G_PI);

          r = hue;
          g = (sqrt(dist)) / outer;
			    //-- priv->v; //-- to be added later
          b = 1.0;
          hsv_to_rgb (&r, &g, &b);

          *p++ = (((int)floor (r * 255 + 0.5) << 16) |
                  ((int)floor (g * 255 + 0.5) << 8) |
                  (int)floor (b * 255 + 0.5));
        }
    }

  source = cairo_image_surface_create_for_data ((unsigned char *)buf,
                                                CAIRO_FORMAT_RGB24,
                                                width, height, stride);

  /* Now draw the value marker onto the source image, so that it
   * will get properly clipped at the edges of the ring
   */
  //-- do we need the value marker now ? No.
  source_cr = cairo_create (source);

  r = priv->h;
  g = 1.0;
  b = 1.0;
  hsv_to_rgb (&r, &g, &b);

  if (INTENSITY (r, g, b) > 0.5)
    cairo_set_source_rgb (source_cr, 0.0, 0.0, 0.0);
  else
    cairo_set_source_rgb (source_cr, 1.0, 1.0, 1.0);

  //cairo_move_to (source_cr, center_x, center_y);
  /*cairo_line_to (source_cr,
                 center_x + cos (priv->h * 2.0 * G_PI) * priv->size / 2,
                 center_y - sin (priv->h * 2.0 * G_PI) * priv->size / 2);*/
  //cairo_stroke (source_cr);
  cairo_destroy (source_cr);

  /* Draw the wheel using the source image */

  cairo_save (cr);

  cairo_set_source_surface (cr, source, 0, 0);
  cairo_surface_destroy (source);

  cairo_new_path (cr);
  cairo_arc (cr,
             center_x, center_y,
             priv->size / 2.0 - priv->ring_width / 2.0,
             0, 2 * G_PI);
  cairo_fill (cr);

  cairo_restore (cr);

  g_free (buf);
}

/* Converts an HSV triplet to an integer RGB triplet */
static void
get_color (gdouble  h,
           gdouble  s,
           gdouble  v,
           gint    *r,
           gint    *g,
           gint    *b)
{
  hsv_to_rgb (&h, &s, &v);

  *r = floor (h * 255 + 0.5);
  *g = floor (s * 255 + 0.5);
  *b = floor (v * 255 + 0.5);
}

#define SWAP(a, b, t) ((t) = (a), (a) = (b), (b) = (t))

#define LERP(a, b, v1, v2, i) (((v2) - (v1) != 0)                                       \
                               ? ((a) + ((b) - (a)) * ((i) - (v1)) / ((v2) - (v1)))     \
                               : (a))

/* Number of pixels we extend out from the edges when creating
 * color source to avoid artifacts
 */
#define PAD 3

/*Paints the nodes*/
static void
paint_nodes (RecolorWheel *wheel,
                cairo_t        *cr,
                gboolean        draw_focus)
{
  GtkWidget             *widget = GTK_WIDGET (wheel);
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  //cairo_surface_t       *source;
  //-- Don't think will need this.
  gdouble                r, g, b;
  gdouble                center_x;
  gdouble                center_y;
  gdouble                xx, yy, dist, outer;
  gdouble                width, height;

#if GTK_CHECK_VERSION(3,0,0)
  GtkWidget             *widget = GTK_WIDGET (wheel);
  GtkStyleContext       *context;
  width  = gtk_widget_get_allocated_width  (widget);
  height = gtk_widget_get_allocated_height (widget);
#else
  GtkAllocation          allocation;
  gchar                 *detail;  
  gtk_widget_get_allocation (GTK_WIDGET (wheel), &allocation);
  width  = allocation.width;
  height = allocation.height;
#endif
  
  center_x = width  / 2.0;
  center_y = height / 2.0;
  
  outer = priv->size / 2.0;
  
  r = priv->h;
  g = priv->s;
  b = priv->v;
  //hsv_to_rgb (&r, &g, &b);
  
  dist = outer * g ;
  
  xx = center_x + cos (priv->h * 2.0 * G_PI) * dist;
  yy = center_y - sin (priv->h * 2.0 * G_PI) * dist;
    
#if GTK_CHECK_VERSION(3,0,0)
  context = gtk_widget_get_style_context (widget);

  gtk_style_context_save (context);
#endif

  hsv_to_rgb (&r, &g, &b);

  if (INTENSITY (r, g, b) > 0.5)
    {
#if GTK_CHECK_VERSION(3,0,0)
      gtk_style_context_add_class (context, "light-area-focus");
#else
      detail = "colorwheel_light";
#endif
      cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    }
  else
    {
#if GTK_CHECK_VERSION(3,0,0)
      gtk_style_context_add_class (context, "dark-area-focus");
#else
      detail = "colorwheel_dark";
#endif
      cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    }

#define RADIUS 4
#define FOCUS_RADIUS 6
    
  cairo_move_to(cr,center_x, center_y);
  cairo_line_to (cr,xx,yy);
  cairo_stroke (cr);
  cairo_new_path (cr);
  cairo_arc (cr, xx, yy, RADIUS, 0, 2 * G_PI);
  cairo_stroke (cr);

#if GTK_CHECK_VERSION(3,0,0)
  gtk_style_context_restore (context);
#endif
}
  
//-- Very well coded , well written function, sadly of no use now. 
//-- Will help you in drawing the nodes which are to be enabled for dragging.
#if GTK_CHECK_VERSION(3,2,0)
static gboolean
recolor_wheel_draw (GtkWidget *widget,
                       cairo_t   *cr)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv  = wheel->priv;
  gboolean               draw_focus;

  draw_focus = gtk_widget_has_visible_focus (widget);
  //-- Purpose in code ??

  paint_ring (wheel, cr);
  //paint_nodes (wheel, cr, draw_focus);
  paint_recolor_nodes_to_wheel(wheel, cr);

  if (draw_focus && priv->focus_on_ring)
    {
      GtkStyleContext *context = gtk_widget_get_style_context (widget);

      gtk_render_focus (context, cr, 0, 0,
                        gtk_widget_get_allocated_width (widget),
                        gtk_widget_get_allocated_height (widget));
    }

  return FALSE;
}
#else
static gint
recolor_wheel_expose (GtkWidget      *widget,
                         GdkEventExpose *event)
{
  cairo_t               *cr    = gdk_cairo_create (gtk_widget_get_window (widget));

  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;
  gboolean               draw_focus;
  GtkAllocation          allocation;

  if (! (event->window == gtk_widget_get_window (widget) &&
         gtk_widget_is_drawable (widget)))
    return FALSE;

  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);
  
  gtk_widget_get_allocation (widget, &allocation);
  cairo_translate (cr, allocation.x, allocation.y);
  
  draw_focus = gtk_widget_has_focus (widget);
  
  paint_ring (wheel, cr);
  //paint_nodes (wheel, cr, draw_focus);
  paint_recolor_nodes_to_wheel(wheel ,cr);
  
  cairo_destroy (cr);
  
  if (draw_focus && priv->focus_on_ring)
    gtk_paint_focus (gtk_widget_get_style (widget),
                     gtk_widget_get_window (widget),
                     gtk_widget_get_state (widget),
                     &event->area, widget, NULL,
                     allocation.x,
                     allocation.y,
                     allocation.width,
                     allocation.height);

  return FALSE;
}
#endif

//-- TODO: Refactor this function for the recolor-wheel functionality.
static gboolean
recolor_wheel_focus (GtkWidget        *widget,
                        GtkDirectionType  dir)
{
  RecolorWheel        *wheel = RECOLOR_WHEEL (widget);
  RecolorWheelPrivate *priv = (RecolorWheelPrivate*) wheel->priv;

  if (!gtk_widget_has_focus (widget))
    {
      if (dir == GTK_DIR_TAB_BACKWARD)
        priv->focus_on_ring = FALSE;
      else
        priv->focus_on_ring = TRUE;

      gtk_widget_grab_focus (widget);
      return TRUE;
    }

  switch (dir)
    {
    case GTK_DIR_UP:
      if (priv->focus_on_ring)
        return FALSE;
      else
        priv->focus_on_ring = TRUE;
      break;

    case GTK_DIR_DOWN:
      if (priv->focus_on_ring)
        priv->focus_on_ring = FALSE;
      else
        return FALSE;
      break;

    case GTK_DIR_LEFT:
    case GTK_DIR_TAB_BACKWARD:
      if (priv->focus_on_ring)
        return FALSE;
      else
        priv->focus_on_ring = TRUE;
      break;

    case GTK_DIR_RIGHT:
    case GTK_DIR_TAB_FORWARD:
      if (priv->focus_on_ring)
        priv->focus_on_ring = FALSE;
      else
        return FALSE;
      break;
    }

  gtk_widget_queue_draw (widget);

  return TRUE;
}

/**
 * recolor_wheel_new:
 *
 * Creates a new HSV color selector.
 *
 * Return value: A newly-created HSV color selector.
 *
 * Since: 2.14
 */
GtkWidget*
recolor_wheel_new (void)
{
  return (GtkWidget*) (g_object_new (RECOLOR_TYPE_COLOR_WHEEL, NULL));
}

/**
 * recolor_wheel_set_color:
 * @hsv: An HSV color selector
 * @h: Hue
 * @s: Saturation
 * @v: Value
 *
 * Sets the current color in an HSV color selector.
 * Color component values must be in the [0.0, 1.0] range.
 *
 * Since: 2.14
 */
void
recolor_wheel_set_color (RecolorWheel *wheel,
                            gdouble         h,
                            gdouble         s,
                            gdouble         v)
{
  RecolorWheelPrivate *priv;

  g_return_if_fail (RECOLOR_IS_COLOR_WHEEL (wheel));
  g_return_if_fail (h >= 0.0 && h <= 1.0);
  g_return_if_fail (s >= 0.0 && s <= 1.0);
  g_return_if_fail (v >= 0.0 && v <= 1.0);

  priv = (RecolorWheelPrivate*) wheel->priv;

  priv->h = h;
  priv->s = s;
  priv->v = v;

  g_signal_emit (wheel, wheel_signals[CHANGED], 0);

  gtk_widget_queue_draw (GTK_WIDGET (wheel));
}

/**
 * recolor_wheel_get_color:
 * @hsv: An HSV color selector
 * @h: (out): Return value for the hue
 * @s: (out): Return value for the saturation
 * @v: (out): Return value for the value
 *
 * Queries the current color in an HSV color selector.
 * Returned values will be in the [0.0, 1.0] range.
 *
 * Since: 2.14
 */
void
recolor_wheel_get_color (RecolorWheel *wheel,
                            gdouble        *h,
                            gdouble        *s,
                            gdouble        *v)
{
  RecolorWheelPrivate *priv;

  g_return_if_fail (RECOLOR_IS_COLOR_WHEEL (wheel));

  priv = (RecolorWheelPrivate*) wheel->priv;

  if (h) *h = priv->h;
  if (s) *s = priv->s;
  if (v) *v = priv->v;
}

/**
 * recolor_wheel_set_ring_fraction:
 * @ring: A wheel color selector
 * @fraction: Ring fraction
 *
 * Sets the ring fraction of a wheel color selector.
 *
 * Since: GIMP 2.10
 */
void
recolor_wheel_set_ring_fraction (RecolorWheel *hsv,
                                    gdouble         fraction)
{
  RecolorWheelPrivate *priv;

  g_return_if_fail (RECOLOR_IS_COLOR_WHEEL (hsv));

  priv = (RecolorWheelPrivate*) hsv->priv;

  priv->ring_fraction = CLAMP (fraction, 0.01, 0.99);

  gtk_widget_queue_draw (GTK_WIDGET (hsv));
}

/**
 * recolor_wheel_get_ring_fraction:
 * @ring: A wheel color selector
 *
 * Returns value: The ring fraction of the wheel color selector.
 *
 * Since: GIMP 2.10
 */
gdouble
recolor_wheel_get_ring_fraction (RecolorWheel *wheel)
{
  RecolorWheelPrivate *priv;

  g_return_val_if_fail (RECOLOR_IS_COLOR_WHEEL (wheel), DEFAULT_FRACTION);

  priv = (RecolorWheelPrivate*) wheel->priv;

  return priv->ring_fraction;
}

/**
 * recolor_wheel_is_adjusting:
 * @hsv: A #RecolorWheel
 *
 * An HSV color selector can be said to be adjusting if multiple rapid
 * changes are being made to its value, for example, when the user is
 * adjusting the value with the mouse. This function queries whether
 * the HSV color selector is being adjusted or not.
 *
 * Return value: %TRUE if clients can ignore changes to the color value,
 *     since they may be transitory, or %FALSE if they should consider
 *     the color value status to be final.
 *
 * Since: 2.14
 */
gboolean
recolor_wheel_is_adjusting (RecolorWheel *wheel)
{
  RecolorWheelPrivate *priv;

  g_return_val_if_fail (RECOLOR_IS_COLOR_WHEEL (wheel), FALSE);

  priv = (RecolorWheelPrivate*) wheel->priv;

  return priv->mode != DRAG_NONE;
}

static void
recolor_wheel_move (RecolorWheel   *wheel, 
                       GtkDirectionType  dir)
{
  RecolorWheelPrivate *priv =(RecolorWheelPrivate*) wheel->priv;
  gdouble                hue, sat, val;
  gdouble                angle;
  gdouble                dist;
  gdouble                center_x;
  gdouble                center_y;
  gdouble                outer;

  hue = priv->h;
  sat = priv->s;
  val = priv->v;
  
  outer=priv->size / 2.0;
  
#define HUE_DELTA 0.002
#define SAT_DELTA 0.001

  switch (dir)
    {
    case GTK_DIR_UP:
      if (priv->focus_on_ring)
        hue += HUE_DELTA;
      break;

    case GTK_DIR_DOWN:
      if (priv->focus_on_ring)
        hue -= HUE_DELTA;
      break;

    case GTK_DIR_LEFT:
      if (priv->focus_on_ring)
        sat -= SAT_DELTA;
      break;

    case GTK_DIR_RIGHT:
      if (priv->focus_on_ring)
        sat += SAT_DELTA;
      break;

    default:
      /* we don't care about the tab directions */
      break;
    }

  /* Wrap */
  if (hue < 0.0)
    hue = 1.0;
  else if (hue > 1.0)
    hue = 0.0;

  recolor_wheel_set_color (wheel, hue, sat, val);
}
