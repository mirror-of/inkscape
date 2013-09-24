#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <math.h>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>
#include "../dialogs/dialog-events.h"
#include "recolor-selector.h"
#include "sp-color-scales.h"
#include "sp-color-icc-selector.h"
#include "../svg/svg-icc-color.h"
#include "ui/widget/recolorwheel.h"
// --
#include <glib/gprintf.h>
// --
G_BEGIN_DECLS

static void sp_recolor_wheel_selector_class_init (SPRecolorWheelSelectorClass *klass);
static void sp_recolor_wheel_selector_init (SPRecolorWheelSelector *cs);
static void sp_recolor_wheel_selector_dispose(GObject *object);

static void sp_recolor_wheel_selector_show_all (GtkWidget *widget);
static void sp_recolor_wheel_selector_hide(GtkWidget *widget);

G_END_DECLS

enum {

    GRABBED,
    DRAGGED,
    RELEASED,
    CHANGED,
    LAST_SIGNAL
};


static SPColorSelectorClass *parent_class;
static guint rsel_signals[LAST_SIGNAL] = {0};


#define XPAD 4
#define YPAD 1

GType
sp_recolor_wheel_selector_get_type (void)
{
    static GType type = 0;
    if (!type) {
        static const GTypeInfo info = {
            sizeof (SPRecolorWheelSelectorClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) sp_recolor_wheel_selector_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (SPRecolorWheelSelector),
            0,    /* n_preallocs */
            (GInstanceInitFunc) sp_recolor_wheel_selector_init,
            0,    /* value_table */
        };

        type = g_type_register_static (SP_TYPE_COLOR_SELECTOR,
                                       "SPRecolorWheelSelector",
                                       &info,
                                       static_cast< GTypeFlags > (0) );
    }
    return type;
}

static void sp_recolor_wheel_selector_class_init(SPRecolorWheelSelectorClass *klass)
{
    static const gchar* nameset[] = {N_("Wheel"), 0};
    GObjectClass   *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    SPColorSelectorClass *selector_class = SP_COLOR_SELECTOR_CLASS (klass);

    parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));

    selector_class->name = nameset;
    selector_class->submode_count = 1;

    object_class->dispose = sp_recolor_wheel_selector_dispose;

    widget_class->show_all = sp_recolor_wheel_selector_show_all;
    widget_class->hide = sp_recolor_wheel_selector_hide;
    
    rsel_signals[GRABBED] =  g_signal_new("grabbed",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPRecolorWheelSelectorClass, grabbed),
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
    rsel_signals[DRAGGED] =  g_signal_new("dragged",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPRecolorWheelSelectorClass, dragged),
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
    rsel_signals[RELEASED] = g_signal_new("released",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPRecolorWheelSelectorClass, released),
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
    rsel_signals[CHANGED] =  g_signal_new("changed",
                                            G_TYPE_FROM_CLASS(object_class),
                                            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE),
                                            G_STRUCT_OFFSET(SPRecolorWheelSelectorClass, changed),
                                            NULL, NULL,
                                            g_cclosure_marshal_VOID__VOID,
                                            G_TYPE_NONE, 0);
        
}

RecolorWheelSelector::RecolorWheelSelector( SPColorSelector* csel )
    : ColorSelector( csel ),
      _updating( FALSE ),
      _dragging( FALSE ),
      _adj(0),
      _adjB(0),
      _wheel(0),
      _slider(0),
      _sliderB(0),
      _sbtn(0),
      _sbtnB(0),
      _label(0),
      _labelB(0)
{
    
}

RecolorWheelSelector::~RecolorWheelSelector()
{
    _adj = 0;
    _adjB = 0;
    _wheel = 0;
    _sbtn = 0;
    _sbtnB = 0;
    _label = 0;
    _labelB = 0;
}

void sp_recolor_wheel_selector_init (SPRecolorWheelSelector *cs)
{
    SP_COLOR_SELECTOR(cs)->base = new RecolorWheelSelector( SP_COLOR_SELECTOR(cs) );

    if ( SP_COLOR_SELECTOR(cs)->base )
    {
        SP_COLOR_SELECTOR(cs)->base->init();
    }
}

void RecolorWheelSelector::init()
{
    gint row = 0;

    _updating = FALSE;
    _dragging = FALSE;

#if GTK_CHECK_VERSION(3,0,0)
    GtkWidget *t = gtk_grid_new();
#else
    GtkWidget *t = gtk_table_new (6, 3, FALSE);
#endif

    gtk_widget_show (t);
    gtk_box_pack_start (GTK_BOX (_csel), t, TRUE, TRUE, 0);

    /* Create components */
    row = 0;

    _wheel = recolor_wheel_new();
    gtk_widget_show( _wheel );

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_halign(_wheel, GTK_ALIGN_FILL);
    gtk_widget_set_valign(_wheel, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(_wheel, TRUE);
    gtk_widget_set_vexpand(_wheel, TRUE);
    gtk_grid_attach(GTK_GRID(t), _wheel, 0, row, 3, 1);
#else
    gtk_table_attach(GTK_TABLE(t), _wheel, 0, 3, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 0, 0);
#endif

    row++;

    /* Label */
    _label = gtk_label_new_with_mnemonic (_("_A:"));
    gtk_misc_set_alignment (GTK_MISC (_label), 1.0, 0.5);
    gtk_widget_show (_label);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_margin_left(_label, XPAD);
    gtk_widget_set_margin_right(_label, XPAD);
    gtk_widget_set_margin_top(_label, YPAD);
    gtk_widget_set_margin_bottom(_label, YPAD);
    gtk_widget_set_halign(_label, GTK_ALIGN_FILL);
    gtk_widget_set_valign(_label, GTK_ALIGN_FILL);
    gtk_grid_attach(GTK_GRID(t), _label, 0, row, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (t), _label, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
#endif

    /* Adjustment */
    _adj = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 255.0, 1.0, 10.0, 10.0));

    /* Slider */
    _slider = sp_color_slider_new (_adj);
    gtk_widget_set_tooltip_text (_slider, _("Alpha (opacity)"));
    gtk_widget_show (_slider);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_margin_left(_slider, XPAD);
    gtk_widget_set_margin_right(_slider, XPAD);
    gtk_widget_set_margin_top(_slider, YPAD);
    gtk_widget_set_margin_bottom(_slider, YPAD);
    gtk_widget_set_hexpand(_slider, TRUE);
    gtk_widget_set_halign(_slider, GTK_ALIGN_FILL);
    gtk_widget_set_valign(_slider, GTK_ALIGN_FILL);
    gtk_grid_attach(GTK_GRID(t), _slider, 1, row, 1, 1);
#else
    gtk_table_attach(GTK_TABLE (t), _slider, 1, 2, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL, XPAD, YPAD);
#endif

    sp_color_slider_set_colors (SP_COLOR_SLIDER (_slider),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 1.0, 0.0),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 1.0, 0.5),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 1.0, 1.0));


    /* Spinbutton */
    _sbtn = gtk_spin_button_new (GTK_ADJUSTMENT (_adj), 1.0, 0);
    gtk_widget_set_tooltip_text (_sbtn, _("Alpha (opacity)"));
    sp_dialog_defocus_on_enter (_sbtn);
    gtk_label_set_mnemonic_widget (GTK_LABEL(_label), _sbtn);
    gtk_widget_show (_sbtn);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_margin_left(_sbtn, XPAD);
    gtk_widget_set_margin_right(_sbtn, XPAD);
    gtk_widget_set_margin_top(_sbtn, YPAD);
    gtk_widget_set_margin_bottom(_sbtn, YPAD);
    gtk_widget_set_halign(_sbtn, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(_sbtn, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(t), _sbtn, 2, row, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (t), _sbtn, 2, 3, row, row + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD);
#endif

    row++;
    
    /*New SLider for Brightness Value */
    /* Label */
    _labelB = gtk_label_new_with_mnemonic (_("_B:"));
    gtk_misc_set_alignment (GTK_MISC (_labelB), 1.0, 0.5);
    gtk_widget_show (_labelB);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_margin_left(_labelB, XPAD);
    gtk_widget_set_margin_right(_labelB, XPAD);
    gtk_widget_set_margin_top(_labelB, YPAD);
    gtk_widget_set_margin_bottom(_labelB, YPAD);
    gtk_widget_set_halign(_label, GTK_ALIGN_FILL);
    gtk_widget_set_valign(_label, GTK_ALIGN_FILL);
    gtk_grid_attach(GTK_GRID(t), _labelB, 0, row, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (t), _labelB, 0, 1, row, row + 1, GTK_FILL, GTK_FILL, XPAD, YPAD);
#endif

    /* Adjustment */
    _adjB = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 255.0, 1.0, 10.0, 10.0));

    /* Slider */
    _sliderB = sp_color_slider_new (_adjB);
    gtk_widget_set_tooltip_text (_sliderB, _("Lightness or Brightness in HSV"));
    gtk_widget_show (_sliderB);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_margin_left(_sliderB, XPAD);
    gtk_widget_set_margin_right(_sliderB, XPAD);
    gtk_widget_set_margin_top(_sliderB, YPAD);
    gtk_widget_set_margin_bottom(_sliderB, YPAD);
    gtk_widget_set_hexpand(_sliderB, TRUE);
    gtk_widget_set_halign(_sliderB, GTK_ALIGN_FILL);
    gtk_widget_set_valign(_sliderB, GTK_ALIGN_FILL);
    gtk_grid_attach(GTK_GRID(t), _sliderB, 1, row, 1, 1);
#else
    gtk_table_attach(GTK_TABLE (t), _sliderB, 1, 2, row, row + 1, (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL, XPAD, YPAD);
#endif

    sp_color_slider_set_colors (SP_COLOR_SLIDER (_sliderB),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 0.0, 1.0),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 0.5, 1.0),
                                SP_RGBA32_F_COMPOSE (1.0, 1.0, 1.0, 1.0));


    /* Spinbutton */
    _sbtnB = gtk_spin_button_new (GTK_ADJUSTMENT (_adjB), 1.0, 0);
    gtk_widget_set_tooltip_text (_sbtnB, _("Lightness or Brightness in HSV"));
    sp_dialog_defocus_on_enter (_sbtnB);
    gtk_label_set_mnemonic_widget (GTK_LABEL(_labelB), _sbtnB);
    gtk_widget_show (_sbtnB);

#if GTK_CHECK_VERSION(3,0,0)
    gtk_widget_set_margin_left(_sbtnB, XPAD);
    gtk_widget_set_margin_right(_sbtnB, XPAD);
    gtk_widget_set_margin_top(_sbtnB, YPAD);
    gtk_widget_set_margin_bottom(_sbtnB, YPAD);
    gtk_widget_set_halign(_sbtnB, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(_sbtnB, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(t), _sbtnB, 2, row, 1, 1);
#else
    gtk_table_attach (GTK_TABLE (t), _sbtnB, 2, 3, row, row + 1, (GtkAttachOptions)0, (GtkAttachOptions)0, XPAD, YPAD);
#endif
    

    /* Signals */
    g_signal_connect (G_OBJECT (_adj), "value_changed",
                        G_CALLBACK (_adjustmentChanged), _csel);

    g_signal_connect (G_OBJECT (_slider), "grabbed",
                        G_CALLBACK (_sliderGrabbed), _csel);
    g_signal_connect (G_OBJECT (_slider), "released",
                        G_CALLBACK (_sliderReleased), _csel);
    g_signal_connect (G_OBJECT (_slider), "changed",
                        G_CALLBACK (_sliderChanged), _csel);

    g_signal_connect( G_OBJECT(_wheel), "changed",
                        G_CALLBACK (_wheelChanged), _csel );

    /* Signals for Brightness */                      
    g_signal_connect (G_OBJECT (_adjB), "value_changed",
                        G_CALLBACK (_adjustmentChanged), _csel);

    g_signal_connect (G_OBJECT (_sliderB), "grabbed",
                        G_CALLBACK (_sliderGrabbed), _csel);
    g_signal_connect (G_OBJECT (_sliderB), "released",
                        G_CALLBACK (_sliderReleased), _csel);
    g_signal_connect (G_OBJECT (_sliderB), "changed",
                        G_CALLBACK (_sliderChanged), _csel);

}

static void sp_recolor_wheel_selector_dispose(GObject *object)
{
    if ((G_OBJECT_CLASS(parent_class))->dispose)
        (* (G_OBJECT_CLASS(parent_class))->dispose) (object);
}

static void
sp_recolor_wheel_selector_show_all (GtkWidget *widget)
{
    gtk_widget_show (widget);
}

static void sp_recolor_wheel_selector_hide(GtkWidget *widget)
{
    gtk_widget_hide(widget);
}

GtkWidget *sp_recolor_wheel_selector_new()
{
    SPRecolorWheelSelector *csel = SP_RECOLOR_WHEEL_SELECTOR(g_object_new (SP_TYPE_RECOLOR_WHEEL_SELECTOR, NULL));

    return GTK_WIDGET (csel);
}

/* Helpers for setting color value */

static void preserve_icc(SPColor *color, SPRecolorWheelSelector *cs){
    ColorSelector* selector = static_cast<ColorSelector*>(SP_COLOR_SELECTOR(cs)->base);
    color->icc = selector->getColor().icc ? new SVGICCColor(*selector->getColor().icc) : 0;
}

GtkWidget* RecolorWheelSelector::getWheel (SPRecolorWheelSelector *cs)
{
    RecolorWheelSelector* wheelSelector = static_cast<RecolorWheelSelector*>(SP_COLOR_SELECTOR(cs)->base);
    
    return GTK_WIDGET(wheelSelector->_wheel) ;
}

void RecolorWheelSelector::_colorChanged()
{
#ifdef DUMP_CHANGE_INFO
    g_message("RecolorWheelSelector::_colorChanged( this=%p, %f, %f, %f,   %f)", this, color.v.c[0], color.v.c[1], color.v.c[2], alpha );
#endif
    _updating = TRUE;
    //{
        float hsv[3] = {0,0,0};
        sp_color_rgb_to_hsv_floatv(hsv, _color.v.c[0], _color.v.c[1], _color.v.c[2]);
        recolor_wheel_set_color( RECOLOR_WHEEL(_wheel), hsv[0], hsv[1], hsv[2] );
    //}
    //-- don't know why blocking this achieves anything.

    guint32 start = _color.toRGBA32( 0x00 );
    guint32 mid = _color.toRGBA32( 0x7f );
    guint32 end = _color.toRGBA32( 0xff );
    //g_printf("\n---Changed the alpha colors1---\n");
    
    float rgb[3]={0,0,0};
    sp_color_hsv_to_rgb_floatv(rgb, hsv[0], hsv[1], 0.0);
    guint32 startB = SP_RGBA32_F_COMPOSE (rgb[0], rgb[1], rgb[2], 1.0);
    sp_color_hsv_to_rgb_floatv(rgb, hsv[0], hsv[1], 0.5);
    guint32 midB = SP_RGBA32_F_COMPOSE (rgb[0], rgb[1], rgb[2], 1.0);
    sp_color_hsv_to_rgb_floatv(rgb, hsv[0], hsv[1], 1.0);
    guint32 endB = SP_RGBA32_F_COMPOSE (rgb[0], rgb[1], rgb[2], 1.0);

    ColorScales::setScaled(_adj, _alpha);
    //g_printf("\n%f",_brightness);
    _brightness = hsv[2];
    ColorScales::setScaled(_adjB, _brightness);

    sp_color_slider_set_colors(SP_COLOR_SLIDER(_slider), start, mid, end);
    sp_color_slider_set_colors(SP_COLOR_SLIDER(_sliderB), startB, midB, endB);
    
    _updating = FALSE;
}

void RecolorWheelSelector::_adjustmentChanged( GtkAdjustment *adjustment, SPRecolorWheelSelector *cs )
{
// TODO check this. It looks questionable:
    // if a value is entered between 0 and 1 exclusive, normalize it to (int) 0..255  or 0..100
    gdouble value = gtk_adjustment_get_value (adjustment);
    gdouble upper = gtk_adjustment_get_upper (adjustment);
    
    if (value > 0.0 && value < 1.0) {
        gtk_adjustment_set_value( adjustment, floor (value * upper + 0.5) );
    }

    RecolorWheelSelector* wheelSelector = static_cast<RecolorWheelSelector*>(SP_COLOR_SELECTOR(cs)->base);
    if (wheelSelector->_updating) return;

    wheelSelector->_updating = TRUE;

    preserve_icc(&wheelSelector->_color, cs);
    wheelSelector->_updateBrightness( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adjB ), wheelSelector->_dragging );
    wheelSelector->_updateInternals( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adj ), wheelSelector->_dragging );
    wheelSelector->_colorChanged();

    //g_signal_emit_by_name(GTK_WIDGET(wheelSelector->_wheel),"changed");
    //was added in the wake of updating the color inside the nodes. Doesn't seem to work.
    
    wheelSelector->_updating = FALSE;
}

void RecolorWheelSelector::_sliderGrabbed( SPColorSlider *slider, SPRecolorWheelSelector *cs )
{
    (void)slider;
    RecolorWheelSelector* wheelSelector = static_cast<RecolorWheelSelector*>(SP_COLOR_SELECTOR(cs)->base);
    if (!wheelSelector->_dragging) {
        wheelSelector->_dragging = TRUE;
        wheelSelector->_grabbed();

        preserve_icc(&wheelSelector->_color, cs);
        wheelSelector->_updateBrightness( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adjB ), wheelSelector->_dragging );
        wheelSelector->_updateInternals( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adj ), wheelSelector->_dragging );       

    }
}

void RecolorWheelSelector::_sliderReleased( SPColorSlider *slider, SPRecolorWheelSelector *cs )
{
    (void)slider;
    RecolorWheelSelector* wheelSelector = static_cast<RecolorWheelSelector*>(SP_COLOR_SELECTOR(cs)->base);
    if (wheelSelector->_dragging) {
        wheelSelector->_dragging = FALSE;
        wheelSelector->_released();

        preserve_icc(&wheelSelector->_color, cs);
        wheelSelector->_updateBrightness( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adjB ), wheelSelector->_dragging );
        wheelSelector->_updateInternals( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adj ), wheelSelector->_dragging );
        //g_signal_emit_by_name(GTK_WIDGET(wheelSelector->_wheel),"changed");
        //was added in the wake of updating the color inside the nodes. Doesn't seem to work.
    
    }
}

void RecolorWheelSelector::_sliderChanged( SPColorSlider *slider, SPRecolorWheelSelector *cs )
{
    (void)slider;
    RecolorWheelSelector* wheelSelector = static_cast<RecolorWheelSelector*>(SP_COLOR_SELECTOR(cs)->base);
    
    preserve_icc(&wheelSelector->_color, cs);
    wheelSelector->_updateBrightness( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adjB ), wheelSelector->_dragging );
    wheelSelector->_updateInternals( wheelSelector->_color, ColorScales::getScaled( wheelSelector->_adj ), wheelSelector->_dragging );
    wheelSelector->_colorChanged();
}

void RecolorWheelSelector::_wheelChanged( RecolorWheel *wheel, SPRecolorWheelSelector *cs )
{
    RecolorWheelSelector* wheelSelector = static_cast<RecolorWheelSelector*>(SP_COLOR_SELECTOR(cs)->base);

    gdouble h = 0;
    gdouble s = 0;
    gdouble v = 0;
    recolor_wheel_get_color( wheel, &h, &s, &v );
    //v = (wheelSelector)->getBrightness();;
    
    float rgb[3] = {0,0,0};
    sp_color_hsv_to_rgb_floatv (rgb, h, s, v); 

    SPColor color(rgb[0], rgb[1], rgb[2]);

    guint32 start = color.toRGBA32( 0x00 );
    guint32 mid = color.toRGBA32( 0x7f );
    guint32 end = color.toRGBA32( 0xff );
    //g_printf("\n---Changed the alpha colors2:Wheel changed---\n");
    
    //-- handles that the lightness bar also changes everytime the wheel is changed.
    sp_color_hsv_to_rgb_floatv(rgb, h, s, 0.0); //-- Left most (Black)
    guint32 startL = SP_RGBA32_F_COMPOSE (rgb[0], rgb[1], rgb[2], 1.0);
    sp_color_hsv_to_rgb_floatv(rgb, h, s, 0.5); //-- Mid
    guint32 midL = SP_RGBA32_F_COMPOSE (rgb[0], rgb[1], rgb[2], 1.0);
    sp_color_hsv_to_rgb_floatv(rgb, h, s, 1.0);//-- Rightmost (Hue)
    guint32 endL = SP_RGBA32_F_COMPOSE (rgb[0], rgb[1], rgb[2], 1.0);
    //-- end

    sp_color_slider_set_colors (SP_COLOR_SLIDER(wheelSelector->_slider), start, mid, end);
    sp_color_slider_set_colors (SP_COLOR_SLIDER(wheelSelector->_sliderB), startL, midL, endL);
    

    preserve_icc(&color, cs);
    wheelSelector->_updateBrightness( color, wheelSelector->_brightness, recolor_wheel_is_adjusting(wheel) );  
    wheelSelector->_updateInternals( color, wheelSelector->_alpha, recolor_wheel_is_adjusting(wheel) );
    
}

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
