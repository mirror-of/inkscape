#include <config.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkcolorsel.h>
#include "../color.h"
#include "../helper/sp-intl.h"
#include "../dialogs/dialog-events.h"
#include "sp-color-preview.h"
#include "sp-color-wheel-selector.h"


G_BEGIN_DECLS

static void sp_color_wheel_selector_class_init (SPColorWheelSelectorClass *klass);
static void sp_color_wheel_selector_init (SPColorWheelSelector *cs);
static void sp_color_wheel_selector_destroy (GtkObject *object);

static void sp_color_wheel_selector_show_all (GtkWidget *widget);
static void sp_color_wheel_selector_hide_all (GtkWidget *widget);


G_END_DECLS

static SPColorSelectorClass *parent_class;

#define XPAD 4
#define YPAD 1

GType
sp_color_wheel_selector_get_type (void)
{
    static GType type = 0;
    if (!type) {
        static const GTypeInfo info = {
            sizeof (SPColorWheelSelectorClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) sp_color_wheel_selector_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (SPColorWheelSelector),
            0,    /* n_preallocs */
            (GInstanceInitFunc) sp_color_wheel_selector_init,
            0,    /* value_table */
        };

        type = g_type_register_static (SP_TYPE_COLOR_SELECTOR,
                                       "SPColorWheelSelector",
                                       &info,
                                       static_cast< GTypeFlags > (0) );
    }
    return type;
}

static void
sp_color_wheel_selector_class_init (SPColorWheelSelectorClass *klass)
{
    static const gchar* nameset[] = {N_("Wheel"), 0};
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    SPColorSelectorClass *selector_class;

    object_class = (GtkObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;
    selector_class = SP_COLOR_SELECTOR_CLASS (klass);

    parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));

    selector_class->name = nameset;
    selector_class->submode_count = 1;

    object_class->destroy = sp_color_wheel_selector_destroy;

    widget_class->show_all = sp_color_wheel_selector_show_all;
    widget_class->hide_all = sp_color_wheel_selector_hide_all;
}

ColorWheelSelector::ColorWheelSelector( SPColorSelector* csel )
    : ColorSelector( csel ),
      _updating( FALSE ),
      _dragging( FALSE ),
      _adj(0),
      _wheel(0),
      _sbtn(0),
      _label(0)
{
}

ColorWheelSelector::~ColorWheelSelector()
{
    _adj = 0;
    _wheel = 0;
    _sbtn = 0;
    _label = 0;
}

void sp_color_wheel_selector_init (SPColorWheelSelector *cs)
{
    SP_COLOR_SELECTOR(cs)->base = new ColorWheelSelector( SP_COLOR_SELECTOR(cs) );

    if ( SP_COLOR_SELECTOR(cs)->base )
    {
        SP_COLOR_SELECTOR(cs)->base->init();
    }
}

void ColorWheelSelector::init()
{
    GtkWidget *t;
    gint row = 0;

    _updating = FALSE;
    _dragging = FALSE;

    t = gtk_table_new (5, 3, FALSE);
    gtk_widget_show (t);
    gtk_box_pack_start (GTK_BOX (_csel), t, TRUE, TRUE, 0);

    /* Create components */
    row = 0;

    _wheel = sp_color_wheel_new ();
    gtk_widget_show (_wheel);
    gtk_table_attach (GTK_TABLE (t), _wheel, 0, 3, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), XPAD, YPAD);

    row++;

    /* Label */
    _label = gtk_label_new ("Alpha:");
    gtk_misc_set_alignment (GTK_MISC (_label), 1.0, 0.5);
    gtk_widget_show (_label);
    gtk_table_attach (GTK_TABLE (t), _label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
    /* Adjustment */
    _adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 1.0, 0.01, 0.1, 0.1);
    gtk_adjustment_set_value (_adj, 0.5);
    /* Slider */
    _slider = sp_color_slider_new (_adj);
    gtk_widget_show (_slider);
    gtk_table_attach (GTK_TABLE (t), _slider, 1, 2, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)GTK_FILL, XPAD, YPAD);

    sp_color_slider_set_colors (SP_COLOR_SLIDER (_slider),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 1.0, 0.0),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 1.0, 1.0));


    /* Spinbutton */
    _sbtn = gtk_spin_button_new (GTK_ADJUSTMENT (_adj), 0.01, 2);
    sp_dialog_defocus_on_enter (_sbtn);
    gtk_widget_show (_sbtn);
    gtk_table_attach (GTK_TABLE (t), _sbtn, 2, 3, row, row + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD);

    /* Signals */
    gtk_signal_connect (GTK_OBJECT (_adj), "value_changed",
                        GTK_SIGNAL_FUNC (_adjustmentChanged), _csel);

    gtk_signal_connect (GTK_OBJECT (_slider), "grabbed",
                        GTK_SIGNAL_FUNC (_sliderGrabbed), _csel);
    gtk_signal_connect (GTK_OBJECT (_slider), "released",
                        GTK_SIGNAL_FUNC (_sliderReleased), _csel);
    gtk_signal_connect (GTK_OBJECT (_slider), "changed",
                        GTK_SIGNAL_FUNC (_sliderChanged), _csel);

    gtk_signal_connect (GTK_OBJECT(_wheel), "changed",
                        GTK_SIGNAL_FUNC (_wheelChanged), _csel);
}

static void
sp_color_wheel_selector_destroy (GtkObject *object)
{
    if (((GtkObjectClass *) (parent_class))->destroy)
        (* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_wheel_selector_show_all (GtkWidget *widget)
{
    gtk_widget_show (widget);
}

static void
sp_color_wheel_selector_hide_all (GtkWidget *widget)
{
    gtk_widget_hide (widget);
}

GtkWidget *
sp_color_wheel_selector_new (void)
{
    SPColorWheelSelector *csel;

    csel = (SPColorWheelSelector*)gtk_type_new (SP_TYPE_COLOR_WHEEL_SELECTOR);

    return GTK_WIDGET (csel);
}

/* Helpers for setting color value */

void ColorWheelSelector::_colorChanged( const SPColor& color, gfloat alpha )
{
    _updating = TRUE;
    sp_color_wheel_set_color( SP_COLOR_WHEEL( _wheel ), &color );

    guint32 start = sp_color_get_rgba32_ualpha( &color, 0x00 );
    guint32 end = sp_color_get_rgba32_ualpha( &color, 0xff );

    sp_color_slider_set_colors (SP_COLOR_SLIDER(_slider), start, end);

    gtk_adjustment_set_value (_adj, alpha);

    _updating = FALSE;
}

void ColorWheelSelector::_adjustmentChanged( GtkAdjustment *adjustment, SPColorWheelSelector *cs )
{
    ColorWheelSelector* wheelSelector = (ColorWheelSelector*)(SP_COLOR_SELECTOR(cs)->base);
    if (wheelSelector->_updating) return;

    wheelSelector->_updating = TRUE;

    wheelSelector->_updateInternals( wheelSelector->_color, wheelSelector->_adj->value, wheelSelector->_dragging );

    wheelSelector->_updating = FALSE;
}

void ColorWheelSelector::_sliderGrabbed( SPColorSlider *slider, SPColorWheelSelector *cs )
{
    ColorWheelSelector* wheelSelector = (ColorWheelSelector*)(SP_COLOR_SELECTOR(cs)->base);
    if (!wheelSelector->_dragging) {
        wheelSelector->_dragging = TRUE;
        wheelSelector->_grabbed();
        wheelSelector->_updateInternals( wheelSelector->_color, wheelSelector->_adj->value, wheelSelector->_dragging );
    }
}

void ColorWheelSelector::_sliderReleased( SPColorSlider *slider, SPColorWheelSelector *cs )
{
    ColorWheelSelector* wheelSelector = (ColorWheelSelector*)(SP_COLOR_SELECTOR(cs)->base);
    if (wheelSelector->_dragging) {
        wheelSelector->_dragging = FALSE;
        wheelSelector->_released();
        wheelSelector->_updateInternals( wheelSelector->_color, wheelSelector->_adj->value, wheelSelector->_dragging );
    }
}

void ColorWheelSelector::_sliderChanged( SPColorSlider *slider, SPColorWheelSelector *cs )
{
    ColorWheelSelector* wheelSelector = (ColorWheelSelector*)(SP_COLOR_SELECTOR(cs)->base);

    wheelSelector->_updateInternals( wheelSelector->_color, wheelSelector->_adj->value, wheelSelector->_dragging );
}

void ColorWheelSelector::_wheelChanged( SPColorWheel *wheel, SPColorWheelSelector *cs )
{
    ColorWheelSelector* wheelSelector = (ColorWheelSelector*)(SP_COLOR_SELECTOR(cs)->base);
    SPColor color;

    sp_color_wheel_get_color( wheel, &color );

    guint32 start = sp_color_get_rgba32_ualpha( &color, 0x00 );
    guint32 end = sp_color_get_rgba32_ualpha( &color, 0xff );

    sp_color_slider_set_colors (SP_COLOR_SLIDER(wheelSelector->_slider), start, end);

    wheelSelector->_updateInternals( color, wheelSelector->_alpha, sp_color_wheel_is_adjusting( wheel ) );
}

