// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * PaintSelector: Generic paint selector widget.
 *//*
 * Authors:
 * see git history
 *   Lauris Kaplinski
 *   bulia byak <buliabyak@users.sf.net>
 *   John Cliff <simarilius@yahoo.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#define noSP_PS_VERBOSE

#include <cstring>
#include <string>
#include <vector>

#include <glibmm/i18n.h>
#include <glibmm/fileutils.h>

#include "desktop-style.h"
#include "inkscape.h"
#include "paint-selector.h"
#include "path-prefix.h"

#include "helper/stock-items.h"
#include "ui/icon-loader.h"

#include "style.h"

#include "io/sys.h"
#include "io/resource.h"

#include "object/sp-hatch.h"
#include "object/sp-linear-gradient.h"
#include "object/sp-mesh-gradient.h"
#include "object/sp-pattern.h"
#include "object/sp-radial-gradient.h"
#include "object/sp-stop.h"

#include "svg/css-ostringstream.h"

#include "ui/icon-names.h"
#include "ui/widget/color-notebook.h"
#include "ui/widget/gradient-selector.h"
#include "ui/widget/swatch-selector.h"

#include "widgets/widget-sizes.h"

#include "xml/repr.h"

#ifdef SP_PS_VERBOSE
#include "svg/svg-icc-color.h"
#endif // SP_PS_VERBOSE

#include <gtkmm/combobox.h>
#include <gtkmm/label.h>

using Inkscape::UI::SelectedColor;

#ifdef SP_PS_VERBOSE
static gchar const *modeStrings[] = {
    "MODE_EMPTY",
    "MODE_MULTIPLE",
    "MODE_NONE",
    "MODE_SOLID_COLOR",
    "MODE_GRADIENT_LINEAR",
    "MODE_GRADIENT_RADIAL",
#ifdef WITH_MESH
    "MODE_GRADIENT_MESH",
#endif
    "MODE_PATTERN",
    "MODE_SWATCH",
    "MODE_UNSET",
    ".",
    ".",
};
#endif

namespace Inkscape {
namespace UI {
namespace Widget {

class FillRuleRadioButton : public Gtk::RadioButton {
  private:
    PaintSelector::FillRule _fillrule;

  public:
    FillRuleRadioButton()
        : Gtk::RadioButton()
    {}

    FillRuleRadioButton(Gtk::RadioButton::Group &group)
        : Gtk::RadioButton(group)
    {}

    inline void set_fillrule(PaintSelector::FillRule fillrule) { _fillrule = fillrule; }
    inline PaintSelector::FillRule get_fillrule() const { return _fillrule; }
};

class StyleToggleButton : public Gtk::ToggleButton {
  private:
    PaintSelector::Mode _style;

  public:
    inline void set_style(PaintSelector::Mode style) { _style = style; }
    inline PaintSelector::Mode get_style() const { return _style; }
};

static bool isPaintModeGradient(PaintSelector::Mode mode)
{
    bool isGrad = (mode == PaintSelector::MODE_GRADIENT_LINEAR) || (mode == PaintSelector::MODE_GRADIENT_RADIAL) ||
                  (mode == PaintSelector::MODE_SWATCH);

    return isGrad;
}

GradientSelector *PaintSelector::getGradientFromData() const
{
    GradientSelector *grad = nullptr;
    if (_mode == PaintSelector::MODE_SWATCH) {
        auto swatchsel = dynamic_cast<SwatchSelector *>(_selector);
        if (swatchsel) {
            grad = swatchsel->getGradientSelector();
        }
    } else {
        grad = dynamic_cast<GradientSelector *>(_selector);
    }
    return grad;
}

#define XPAD 4
#define YPAD 1

PaintSelector::PaintSelector(FillOrStroke kind)
{
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    _mode = static_cast<PaintSelector::Mode>(-1); // huh?  do you mean 0xff?  --  I think this means "not in the enum"

    /* Paint style button box */
    _style = Gtk::manage(new Gtk::Box());
    _style->set_homogeneous(false);
    _style->set_name("PaintSelector");
    _style->show();
    _style->set_border_width(4);
    pack_start(*_style, false, false);

    /* Buttons */
    _none = style_button_add(INKSCAPE_ICON("paint-none"), PaintSelector::MODE_NONE, _("No paint"));
    _solid = style_button_add(INKSCAPE_ICON("paint-solid"), PaintSelector::MODE_SOLID_COLOR, _("Flat color"));
    _gradient = style_button_add(INKSCAPE_ICON("paint-gradient-linear"), PaintSelector::MODE_GRADIENT_LINEAR,
                                 _("Linear gradient"));
    _radial = style_button_add(INKSCAPE_ICON("paint-gradient-radial"), PaintSelector::MODE_GRADIENT_RADIAL,
                               _("Radial gradient"));
#ifdef WITH_MESH
    _mesh =
        style_button_add(INKSCAPE_ICON("paint-gradient-mesh"), PaintSelector::MODE_GRADIENT_MESH, _("Mesh gradient"));
#endif
    _pattern = style_button_add(INKSCAPE_ICON("paint-pattern"), PaintSelector::MODE_PATTERN, _("Pattern"));
    _swatch = style_button_add(INKSCAPE_ICON("paint-swatch"), PaintSelector::MODE_SWATCH, _("Swatch"));
    _unset = style_button_add(INKSCAPE_ICON("paint-unknown"), PaintSelector::MODE_UNSET,
                              _("Unset paint (make it undefined so it can be inherited)"));

    /* Fillrule */
    {
        _fillrulebox = Gtk::manage(new Gtk::Box());
        _fillrulebox->set_homogeneous(false);
        _style->pack_end(*_fillrulebox, false, false, 0);

        _evenodd = Gtk::make_managed<FillRuleRadioButton>();
        _evenodd->set_relief(Gtk::RELIEF_NONE);
        _evenodd->set_mode(false);
        // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/painting.html#FillRuleProperty
        _evenodd->set_tooltip_text(
            _("Any path self-intersections or subpaths create holes in the fill (fill-rule: evenodd)"));
        _evenodd->set_fillrule(PaintSelector::FILLRULE_EVENODD);
        auto w = Glib::wrap(sp_get_icon_image("fill-rule-even-odd", GTK_ICON_SIZE_MENU));
        _evenodd->add(*w);
        _fillrulebox->pack_start(*_evenodd, false, false, 0);
        _evenodd->signal_toggled().connect(
            sigc::bind(sigc::mem_fun(*this, &PaintSelector::fillrule_toggled), _evenodd));

        auto grp = _evenodd->get_group();
        _nonzero = Gtk::manage(new FillRuleRadioButton(grp));
        _nonzero->set_relief(Gtk::RELIEF_NONE);
        _nonzero->set_mode(false);
        // TRANSLATORS: for info, see http://www.w3.org/TR/2000/CR-SVG-20000802/painting.html#FillRuleProperty
        _nonzero->set_tooltip_text(_("Fill is solid unless a subpath is counterdirectional (fill-rule: nonzero)"));
        _nonzero->set_fillrule(PaintSelector::FILLRULE_NONZERO);
        w = Glib::wrap(sp_get_icon_image("fill-rule-nonzero", GTK_ICON_SIZE_MENU));
        _nonzero->add(*w);
        _fillrulebox->pack_start(*_nonzero, false, false, 0);
        _nonzero->signal_toggled().connect(
            sigc::bind(sigc::mem_fun(*this, &PaintSelector::fillrule_toggled), _nonzero));
    }

    /* Frame */
    _label = Gtk::manage(new Gtk::Label(""));
    auto lbbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 4));
    lbbox->set_homogeneous(false);
    _label->show();
    lbbox->pack_start(*_label, false, false, 4);
    pack_start(*lbbox, false, false, 4);

    _frame = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    _frame->set_homogeneous(false);
    _frame->show();
    pack_start(*_frame, true, true, 0);


    /* Last used color */
    _selected_color = new SelectedColor;
    _updating_color = false;

    _selected_color->signal_grabbed.connect(sigc::mem_fun(*this, &PaintSelector::onSelectedColorGrabbed));
    _selected_color->signal_dragged.connect(sigc::mem_fun(*this, &PaintSelector::onSelectedColorDragged));
    _selected_color->signal_released.connect(sigc::mem_fun(*this, &PaintSelector::onSelectedColorReleased));
    _selected_color->signal_changed.connect(sigc::mem_fun(*this, &PaintSelector::onSelectedColorChanged));

    // from _new function
    setMode(PaintSelector::MODE_MULTIPLE);

    if (kind == FILL)
        _fillrulebox->show_all();
    else
        _fillrulebox->hide();
}

PaintSelector::~PaintSelector()
{
    if (_selected_color) {
        delete _selected_color;
        _selected_color = nullptr;
    }
}

StyleToggleButton *PaintSelector::style_button_add(gchar const *pixmap, PaintSelector::Mode mode, gchar const *tip)
{
    auto b = Gtk::manage(new StyleToggleButton());
    b->set_tooltip_text(tip);
    b->show();
    b->set_border_width(0);
    b->set_relief(Gtk::RELIEF_NONE);
    b->set_mode(false);
    b->set_style(mode);

    auto w = Glib::wrap(sp_get_icon_image(pixmap, GTK_ICON_SIZE_BUTTON));
    b->add(*w);

    _style->pack_start(*b, false, false);
    b->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &PaintSelector::style_button_toggled), b));

    return b;
}

void PaintSelector::style_button_toggled(StyleToggleButton *tb)
{
    if (!_update && tb->get_active()) {
        setMode(tb->get_style());
    }
}

void PaintSelector::fillrule_toggled(FillRuleRadioButton *tb)
{
    if (!_update && tb->get_active()) {
        auto fr = tb->get_fillrule();
        _signal_fillrule_changed.emit(fr);
    }
}

void PaintSelector::setMode(Mode mode)
{
    if (_mode != mode) {
        _update = true;
#ifdef SP_PS_VERBOSE
        g_print("Mode change %d -> %d   %s -> %s\n", _mode, mode, modeStrings[_mode], modeStrings[mode]);
#endif
        switch (mode) {
            case MODE_EMPTY:
                set_mode_empty();
                break;
            case MODE_MULTIPLE:
                set_mode_multiple();
                break;
            case MODE_NONE:
                set_mode_none();
                break;
            case MODE_SOLID_COLOR:
                set_mode_color(mode);
                break;
            case MODE_GRADIENT_LINEAR:
            case MODE_GRADIENT_RADIAL:
                set_mode_gradient(mode);
                break;
#ifdef WITH_MESH
            case MODE_GRADIENT_MESH:
                set_mode_mesh(mode);
                break;
#endif
            case MODE_PATTERN:
                set_mode_pattern(mode);
                break;
            case MODE_HATCH:
                set_mode_hatch(mode);
                break;
            case MODE_SWATCH:
                set_mode_swatch(mode);
                break;
            case MODE_UNSET:
                set_mode_unset();
                break;
            default:
                g_warning("file %s: line %d: Unknown paint mode %d", __FILE__, __LINE__, mode);
                break;
        }
        _mode = mode;
        _signal_mode_changed.emit(_mode);
        _update = false;
    }
}

void PaintSelector::setFillrule(FillRule fillrule)
{
    if (_fillrulebox) {
        // TODO this flips widgets but does not use a member to store state. Revisit
        _evenodd->set_active(fillrule == FILLRULE_EVENODD);
        _nonzero->set_active(fillrule == FILLRULE_NONZERO);
    }
}

void PaintSelector::setColorAlpha(SPColor const &color, float alpha)
{
    g_return_if_fail((0.0 <= alpha) && (alpha <= 1.0));
    /*
        guint32 rgba = 0;

        if ( sp_color_get_colorspace_type(color) == SP_COLORSPACE_TYPE_CMYK )
        {
    #ifdef SP_PS_VERBOSE
            g_print("PaintSelector set CMYKA\n");
    #endif
            sp_paint_selector_set_mode(psel, MODE_COLOR_CMYK);
        }
        else
    */
    {
#ifdef SP_PS_VERBOSE
        g_print("PaintSelector set RGBA\n");
#endif
        setMode(MODE_SOLID_COLOR);
    }

    _updating_color = true;
    _selected_color->setColorAlpha(color, alpha);
    _updating_color = false;
    // rgba = color.toRGBA32( alpha );
}

void PaintSelector::setSwatch(SPGradient *vector)
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set SWATCH\n");
#endif
    setMode(MODE_SWATCH);

    auto swatchsel = dynamic_cast<SwatchSelector *>(_selector);
    if (swatchsel) {
        swatchsel->setVector((vector) ? vector->document : nullptr, vector);
    }
}

void PaintSelector::setGradientLinear(SPGradient *vector)
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set GRADIENT LINEAR\n");
#endif
    setMode(MODE_GRADIENT_LINEAR);

    auto gsel = getGradientFromData();

    gsel->setMode(GradientSelector::MODE_LINEAR);
    gsel->setVector((vector) ? vector->document : nullptr, vector);
}

void PaintSelector::setGradientRadial(SPGradient *vector)
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set GRADIENT RADIAL\n");
#endif
    setMode(MODE_GRADIENT_RADIAL);

    auto gsel = getGradientFromData();

    gsel->setMode(GradientSelector::MODE_RADIAL);

    gsel->setVector((vector) ? vector->document : nullptr, vector);
}

#ifdef WITH_MESH
void PaintSelector::setGradientMesh(SPMeshGradient *array)
{
#ifdef SP_PS_VERBOSE
    g_print("PaintSelector set GRADIENT MESH\n");
#endif
    setMode(MODE_GRADIENT_MESH);

    // GradientSelector *gsel = getGradientFromData(this);

    // gsel->setMode(GradientSelector::MODE_GRADIENT_MESH);
    // gsel->setVector((mesh) ? mesh->document : 0, mesh);
}
#endif

void PaintSelector::setGradientProperties(SPGradientUnits units, SPGradientSpread spread)
{
    g_return_if_fail(isPaintModeGradient(_mode));

    auto gsel = getGradientFromData();
    gsel->setUnits(units);
    gsel->setSpread(spread);
}

void PaintSelector::getGradientProperties(SPGradientUnits &units, SPGradientSpread &spread) const
{
    g_return_if_fail(isPaintModeGradient(_mode));

    auto gsel = getGradientFromData();
    units = gsel->getUnits();
    spread = gsel->getSpread();
}


/**
 * \post (alpha == NULL) || (*alpha in [0.0, 1.0]).
 */
void PaintSelector::getColorAlpha(SPColor &color, gfloat &alpha) const
{
    _selected_color->colorAlpha(color, alpha);

    g_assert((0.0 <= alpha) && (alpha <= 1.0));
}

SPGradient *PaintSelector::getGradientVector()
{
    SPGradient *vect = nullptr;

    if (isPaintModeGradient(_mode)) {
        auto gsel = getGradientFromData();
        vect = gsel->getVector();
    }

    return vect;
}


void PaintSelector::pushAttrsToGradient(SPGradient *gr) const
{
    SPGradientUnits units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
    SPGradientSpread spread = SP_GRADIENT_SPREAD_PAD;
    getGradientProperties(units, spread);
    gr->setUnits(units);
    gr->setSpread(spread);
    gr->updateRepr();
}

void PaintSelector::clear_frame()
{
    if (_selector) {
        // This is a hack to work around GtkNotebook bug in ColorSelector. Is sends signal switch-page on destroy
        // The widget is hidden first so it can recognize that it should not process signals from notebook child
        _selector->set_visible(false);
        remove(*_selector);
        //_selector = nullptr;
    }
}

void PaintSelector::set_mode_empty()
{
    set_style_buttons(nullptr);
    _style->set_sensitive(false);
    clear_frame();
    _label->set_markup(_("<b>No objects</b>"));
}

void PaintSelector::set_mode_multiple()
{
    set_style_buttons(nullptr);
    _style->set_sensitive(true);
    clear_frame();
    _label->set_markup(_("<b>Multiple styles</b>"));
}

void PaintSelector::set_mode_unset()
{
    set_style_buttons(_unset);
    _style->set_sensitive(true);
    clear_frame();
    _label->set_markup(_("<b>Paint is undefined</b>"));
}

void PaintSelector::set_mode_none()
{
    set_style_buttons(_none);
    _style->set_sensitive(true);
    clear_frame();
    _label->set_markup(_("<b>No paint</b>"));
}

/* Color paint */

void PaintSelector::onSelectedColorGrabbed() { _signal_grabbed.emit(); }

void PaintSelector::onSelectedColorDragged()
{
    if (_updating_color) {
        return;
    }

    _signal_dragged.emit();
}

void PaintSelector::onSelectedColorReleased() { _signal_released.emit(); }

void PaintSelector::onSelectedColorChanged()
{
    if (_updating_color) {
        return;
    }

    if (_mode == MODE_SOLID_COLOR) {
        _signal_changed.emit();
    } else {
        g_warning("PaintSelector::onSelectedColorChanged(): selected color changed while not in color selection mode");
    }
}

void PaintSelector::set_mode_color(PaintSelector::Mode /*mode*/)
{
    using Inkscape::UI::Widget::ColorNotebook;

    if ((_mode == PaintSelector::MODE_SWATCH) || (_mode == PaintSelector::MODE_GRADIENT_LINEAR) ||
        (_mode == PaintSelector::MODE_GRADIENT_RADIAL)) {
        auto gsel = getGradientFromData();
        if (gsel) {
            SPGradient *gradient = gsel->getVector();

            // Gradient can be null if object paint is changed externally (ie. with a color picker tool)
            if (gradient) {
                SPColor color = gradient->getFirstStop()->getColor();
                float alpha = gradient->getFirstStop()->getOpacity();
                _selected_color->setColorAlpha(color, alpha, false);
            }
        }
    }

    set_style_buttons(_solid);
    _style->set_sensitive(true);

    if (_mode == PaintSelector::MODE_SOLID_COLOR) {
        /* Already have color selector */
        // Do nothing
    } else {

        clear_frame();
        /* Create new color selector */
        /* Create vbox */
        auto vb = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 4));
        vb->set_homogeneous(false);
        vb->show();

        /* Color selector */
        Gtk::Widget *color_selector = Gtk::manage(new ColorNotebook(*(_selected_color)));
        color_selector->show();
        vb->pack_start(*color_selector, true, true, 0);

        /* Pack everything to frame */
        _frame->add(*vb);

        _selector = vb;
    }

    _label->set_markup(_("<b>Flat color</b>"));

#ifdef SP_PS_VERBOSE
    g_print("Color req\n");
#endif
}

/* Gradient */

void PaintSelector::gradient_grabbed() { _signal_grabbed.emit(); }

void PaintSelector::gradient_dragged() { _signal_dragged.emit(); }

void PaintSelector::gradient_released() { _signal_released.emit(); }

void PaintSelector::gradient_changed(SPGradient * /* gr */) { _signal_changed.emit(); }

void PaintSelector::set_mode_gradient(PaintSelector::Mode mode)
{
    if (mode == PaintSelector::MODE_GRADIENT_LINEAR) {
        set_style_buttons(_gradient);
    } else if (mode == PaintSelector::MODE_GRADIENT_RADIAL) {
        set_style_buttons(_radial);
    }
    _style->set_sensitive(true);

    if ((_mode == PaintSelector::MODE_GRADIENT_LINEAR) || (_mode == PaintSelector::MODE_GRADIENT_RADIAL)) {
        // do nothing - the selector should already be a GradientSelector
    } else {
        clear_frame();
        /* Create new gradient selector */
        auto new_gsel = Gtk::manage(new GradientSelector());
        new_gsel->show();
        new_gsel->signal_grabbed().connect(sigc::mem_fun(this, &PaintSelector::gradient_grabbed));
        new_gsel->signal_dragged().connect(sigc::mem_fun(this, &PaintSelector::gradient_dragged));
        new_gsel->signal_released().connect(sigc::mem_fun(this, &PaintSelector::gradient_released));
        new_gsel->signal_changed().connect(sigc::mem_fun(this, &PaintSelector::gradient_changed));
        /* Pack everything to frame */
        _frame->add(*new_gsel);
        _selector = new_gsel;
    }

    // We should now have a valid GradientSelector so we can use it
    auto gsel = dynamic_cast<GradientSelector *>(_selector);

    if (!gsel) g_warning("No GradientSelector found");

    /* Actually we have to set option menu history here */
    if (mode == PaintSelector::MODE_GRADIENT_LINEAR) {
        gsel->setMode(GradientSelector::MODE_LINEAR);
        // sp_gradient_selector_set_mode(SP_GRADIENT_SELECTOR(gsel), SP_GRADIENT_SELECTOR_MODE_LINEAR);
        _label->set_markup(_("<b>Linear gradient</b>"));
    } else if (mode == PaintSelector::MODE_GRADIENT_RADIAL) {
        gsel->setMode(GradientSelector::MODE_RADIAL);
        _label->set_markup(_("<b>Radial gradient</b>"));
    }

#ifdef SP_PS_VERBOSE
    g_print("Gradient req\n");
#endif
}

// ************************* MESH ************************
#ifdef WITH_MESH
void PaintSelector::mesh_change()
{
    _signal_changed.emit();
}

/**
 *  Returns a list of meshes in the defs of the given source document as a vector
 */
static std::vector<SPMeshGradient *> ink_mesh_list_get(SPDocument *source)
{
    std::vector<SPMeshGradient *> pl;
    if (source == nullptr)
        return pl;


    std::vector<SPObject *> meshes = source->getResourceList("gradient");
    for (auto meshe : meshes) {
        if (SP_IS_MESHGRADIENT(meshe) && SP_GRADIENT(meshe) == SP_GRADIENT(meshe)->getArray()) { // only if this is a
                                                                                                 // root mesh
            pl.push_back(SP_MESHGRADIENT(meshe));
        }
    }
    return pl;
}

/**
 * Adds menu items for mesh list.
 */
void PaintSelector::mesh_menu_build(std::vector<SPMeshGradient *> &mesh_list, SPDocument * /*source*/)
{
    for (auto i : mesh_list) {
        Inkscape::XML::Node *repr = i->getRepr();

        gchar const *meshid = repr->attribute("id");
        gchar const *label = meshid;

        // Only relevant if we supply a set of canned meshes.
        bool stockid = false;
        if (repr->attribute("inkscape:stockid")) {
            label = _(repr->attribute("inkscape:stockid"));
            stockid = true;
        }

        auto iter = _mesh_tree_model->append();
        auto row = *iter;
        row[_mesh_cols._col_label] = label;
        row[_mesh_cols._col_stock] = stockid;
        row[_mesh_cols._col_mesh]  = meshid;
        row[_mesh_cols._col_sep]   = false;
    }
}

/**
 * Pick up all meshes from source, except those that are in
 * current_doc (if non-NULL), and add items to the mesh menu.
 */
void PaintSelector::mesh_list_from_doc(SPDocument *source, SPDocument * /* mesh_doc */)
{
    auto pl = ink_mesh_list_get(source);
    mesh_menu_build(pl, source);
}


void PaintSelector::ink_mesh_menu_populate_menu(SPDocument *doc)
{
    static SPDocument *meshes_doc = nullptr;

    // If we ever add a list of canned mesh gradients, uncomment following:

    // find and load meshes.svg
    // if (meshes_doc == NULL) {
    //     char *meshes_source = g_build_filename(INKSCAPE_MESHESDIR, "meshes.svg", NULL);
    //     if (Inkscape::IO::file_test(meshes_source, G_FILE_TEST_IS_REGULAR)) {
    //         meshes_doc = SPDocument::createNewDoc(meshes_source, FALSE);
    //     }
    //     g_free(meshes_source);
    // }

    // suck in from current doc
    mesh_list_from_doc(doc, meshes_doc);

    // add separator
    // {
    //     auto iter = _mesh_tree_model->append();
    //     (*iter)[_mesh_cols._col_label] = "";
    //     (*iter)[_mesh_cols._col_stock] = false;
    //     (*iter)[_mesh_cols._col_mesh]  = "";
    //     (*iter)[_mesh_cols._col_sep]   = true;
    // }

    // suck in from meshes.svg
    // if (meshes_doc) {
    //     doc->ensureUpToDate();
    //     mesh_list_from_doc(doc, meshes_doc);
    // }
}


void PaintSelector::ink_mesh_menu()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    if (!doc) {
        auto iter = _mesh_tree_model->append();
        auto row = *iter;
        row[_mesh_cols._col_label] = _("No document selected");
        row[_mesh_cols._col_stock] = false;
        row[_mesh_cols._col_mesh]  = "";
        row[_mesh_cols._col_sep]   = false;
        _meshmenu->set_sensitive(false);
    } else {
        ink_mesh_menu_populate_menu(doc);
        _meshmenu->set_sensitive(true);
    }

    if (!_mesh_tree_model->children().empty()) {
        // Select the first item that is not a separator
        auto iter = _mesh_tree_model->children().begin();
        auto row = *iter;
        auto sep = row[_mesh_cols._col_sep];
        if (sep) {
            ++iter;
        }

        _meshmenu->set_active(iter);
    }
}


/*update mesh list*/
void PaintSelector::updateMeshList(SPMeshGradient *mesh)
{
    if (_update) {
        return;
    }

    g_assert(_meshmenu != nullptr);

    /* Clear existing menu if any */
    _mesh_tree_model->clear();

    ink_mesh_menu();

    /* Set history */

    if (mesh && !_meshmenu_update) {
        _meshmenu_update = true;
        gchar const *meshname = mesh->getRepr()->attribute("id");

        // Find this mesh and set it active in the combo_box
        auto iter = _mesh_tree_model->children().begin();
        if (!iter) {
            return;
        }
        Glib::ustring meshid = (*iter)[_mesh_cols._col_mesh];
        while (iter && strcmp(meshid.c_str(), meshname) != 0) {
            ++iter;
            meshid = (*iter)[_mesh_cols._col_mesh];
        }

        if (iter) {
            _meshmenu->set_active(iter);
        }

        _meshmenu_update = false;
    }
}

#ifdef WITH_MESH
void PaintSelector::set_mode_mesh(PaintSelector::Mode mode)
{
    if (mode == PaintSelector::MODE_GRADIENT_MESH) {
        set_style_buttons(_mesh);
    }
    _style->set_sensitive(true);

    if (_mode == PaintSelector::MODE_GRADIENT_MESH) {
        /* Already have mesh menu */
        // Do nothing - the Selector is already a Gtk::Box with the required contents
    } else {
        clear_frame();

        /* Create vbox */
        _selector = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 4));
        _selector->set_homogeneous(false);
        _selector->show();

        {
            auto hb = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 1));
            hb->set_homogeneous(false);

            _mesh_tree_model = Gtk::ListStore::create(_mesh_cols);
            _meshmenu = Gtk::make_managed<Gtk::ComboBox>();
            _meshmenu->set_model(_mesh_tree_model);
            _meshmenu->set_row_separator_func(sigc::mem_fun(*this, &PaintSelector::isSeparator));

            _mesh_cell_renderer = Gtk::make_managed<Gtk::CellRendererText>();
            _mesh_cell_renderer->set_padding(2, 0);
            _meshmenu->pack_start(*_mesh_cell_renderer, true);
            _meshmenu->add_attribute(*_mesh_cell_renderer, "text", _mesh_cols._col_label);

            ink_mesh_menu();
            _meshmenu->signal_changed().connect(sigc::mem_fun(this, &PaintSelector::mesh_change));

            hb->add(*_meshmenu);
            _selector->pack_start(*hb, false, false, AUX_BETWEEN_BUTTON_GROUPS);
        }

        {
            auto hb = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 0));
            hb->set_homogeneous(false);
            auto l = Gtk::manage(new Gtk::Label());
            l->set_markup(_("Use the <b>Mesh tool</b> to modify the mesh."));
            l->set_line_wrap(true);
            l->set_size_request(180, -1);
            hb->pack_start(*l, true, true, AUX_BETWEEN_BUTTON_GROUPS);
            _selector->pack_start(*hb, false, false, AUX_BETWEEN_BUTTON_GROUPS);
        }

        _selector->show_all();

        _frame->add(*_selector);
        _label->set_markup(_("<b>Mesh fill</b>"));
    }
#ifdef SP_PS_VERBOSE
    g_print("Mesh req\n");
#endif
}
#endif // WITH_MESH

SPMeshGradient *PaintSelector::getMeshGradient()
{
    g_return_val_if_fail((_mode == MODE_GRADIENT_MESH), NULL);

    /* no mesh menu if we were just selected */
    if (_meshmenu == nullptr) {
        return nullptr;
    }

    /* Get the selected mesh */
    auto iter = _meshmenu->get_active();
    if (!iter) {
        return nullptr;
    }

    Glib::ustring meshid = (*iter)[_mesh_cols._col_mesh];
    auto stockid = (*iter)[_mesh_cols._col_stock];

    if (meshid == nullptr) {
        return nullptr;
    }

    SPMeshGradient *mesh = nullptr;
    if (strcmp(meshid.c_str(), "none")) {
        gchar *mesh_name;
        if (stockid) {
            mesh_name = g_strconcat("urn:inkscape:mesh:", meshid.c_str(), NULL);
        } else {
            mesh_name = g_strdup(meshid.c_str());
        }

        SPObject *mesh_obj = get_stock_item(mesh_name);
        if (mesh_obj && SP_IS_MESHGRADIENT(mesh_obj)) {
            mesh = SP_MESHGRADIENT(mesh_obj);
        }
        g_free(mesh_name);
    } else {
        std::cerr << "PaintSelector::getMeshGradient: Unexpected meshid value." << std::endl;
    }

    return mesh;
}

#endif
// ************************ End Mesh ************************

void PaintSelector::set_style_buttons(Gtk::ToggleButton *active)
{
    _none->set_active(active == _none);
    _solid->set_active(active == _solid);
    _gradient->set_active(active == _gradient);
    _radial->set_active(active == _radial);
#ifdef WITH_MESH
    _mesh->set_active(active == _mesh);
#endif
    _pattern->set_active(active == _pattern);
    _swatch->set_active(active == _swatch);
    _unset->set_active(active == _unset);
}

void PaintSelector::pattern_change()
{
    _signal_changed.emit();
}


/**
 *  Returns a list of patterns in the defs of the given source document as a vector
 */
static std::vector<SPPattern *> ink_pattern_list_get(SPDocument *source)
{
    std::vector<SPPattern *> pl;
    if (source == nullptr)
        return pl;

    std::vector<SPObject *> patterns = source->getResourceList("pattern");
    for (auto pattern : patterns) {
        if (SP_PATTERN(pattern) == SP_PATTERN(pattern)->rootPattern()) { // only if this is a root pattern
            pl.push_back(SP_PATTERN(pattern));
        }
    }

    return pl;
}

/**
 * Adds menu items for pattern list - derived from marker code, left hb etc in to make addition of previews easier at
 * some point.
 */
void PaintSelector::pattern_menu_build(std::vector<SPPattern *> &pl, SPDocument * /*source*/)
{
    for (auto i = pl.rbegin(); i != pl.rend(); ++i) {

        Inkscape::XML::Node *repr = (*i)->getRepr();

        // label for combobox
        gchar const *label;
        if (repr->attribute("inkscape:stockid")) {
            label = _(repr->attribute("inkscape:stockid"));
        } else {
            label = _(repr->attribute("id"));
        }

        gchar const *patid = repr->attribute("id");

        gboolean stockid = false;
        if (repr->attribute("inkscape:stockid")) {
            stockid = true;
        }

        auto iter = _pattern_tree_model->append();
        auto row = *iter;
        row[_pattern_cols._col_label]   = label;
        row[_pattern_cols._col_stock]   = stockid;
        row[_pattern_cols._col_pattern] = patid;
        row[_pattern_cols._col_sep]     = false;
    }
}

/**
 * Pick up all patterns from source, except those that are in
 * current_doc (if non-NULL), and add items to the pattern menu.
 */
void PaintSelector::pattern_list_from_doc(SPDocument * /* current_doc */,
                                          SPDocument *source,
                                          SPDocument * /*pattern_doc */)
{
    auto pl = ink_pattern_list_get(source);
    pattern_menu_build(pl, source);
}


void PaintSelector::ink_pattern_menu_populate_menu(SPDocument *doc)
{
    static SPDocument *patterns_doc = nullptr;

    // find and load patterns.svg
    if (patterns_doc == nullptr) {
        using namespace Inkscape::IO::Resource;
        auto patterns_source = get_path_string(SYSTEM, PAINT, "patterns.svg");
        if (Glib::file_test(patterns_source, Glib::FILE_TEST_IS_REGULAR)) {
            patterns_doc = SPDocument::createNewDoc(patterns_source.c_str(), false);
        }
    }

    // suck in from current doc
    pattern_list_from_doc(nullptr, doc, patterns_doc);

    // add separator
    {
        auto iter = _pattern_tree_model->append();
        auto row = *iter;
        row[_pattern_cols._col_label]   = "";
        row[_pattern_cols._col_stock]   = false;
        row[_pattern_cols._col_pattern] = "";
        row[_pattern_cols._col_sep]     = true;
    }

    // suck in from patterns.svg
    if (patterns_doc) {
        doc->ensureUpToDate();
        pattern_list_from_doc(doc, patterns_doc, nullptr);
    }
}


void PaintSelector::ink_pattern_menu()
{
    SPDocument *doc = SP_ACTIVE_DOCUMENT;

    if (!doc) {
        auto iter = _pattern_tree_model->append();
        auto row = *iter;
        row[_pattern_cols._col_label]   = _("No document selected");
        row[_pattern_cols._col_stock]   = false;
        row[_pattern_cols._col_pattern] = "";
        row[_pattern_cols._col_sep]     = false;
        _patternmenu->set_sensitive(false);
    } else {
        ink_pattern_menu_populate_menu(doc);
        _patternmenu->set_sensitive(true);
    }

    // Select the first item that is not a separator
    if (!_pattern_tree_model->children().empty()) {
        auto iter = _pattern_tree_model->children().begin();
        auto row = *iter;
        auto sep = row[_pattern_cols._col_sep];
        if (sep) {
            ++iter;
        }
        _patternmenu->set_active(iter);
    }
}


/*update pattern list*/
void PaintSelector::updatePatternList(SPPattern *pattern)
{
    if (_update) {
        return;
    }
    g_assert(_patternmenu != nullptr);

    /* Clear existing menu if any */
    _pattern_tree_model->clear();

    ink_pattern_menu();

    /* Set history */

    if (pattern && !_patternmenu_update) {
        _patternmenu_update = true;
        gchar const *patname = pattern->getRepr()->attribute("id");

        // Find this pattern and set it active in the combo_box
        auto iter = _pattern_tree_model->children().begin();
        if (!iter) {
            return;
        }
        Glib::ustring patid = (*iter)[_pattern_cols._col_pattern];
        while (iter && strcmp(patid.c_str(), patname) != 0) {
            ++iter;
            patid = (*iter)[_pattern_cols._col_pattern];
        }

        if (iter) {
            _patternmenu->set_active(iter);
        }

        _patternmenu_update = false;
    }
}

void PaintSelector::set_mode_pattern(PaintSelector::Mode mode)
{
    if (mode == PaintSelector::MODE_PATTERN) {
        set_style_buttons(_pattern);
    }

    _style->set_sensitive(true);

    if (_mode == PaintSelector::MODE_PATTERN) {
        /* Already have pattern menu */
    } else {
        clear_frame();

        /* Create vbox */
        _selector = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 4));
        _selector->set_homogeneous(false);
        _selector->show();

        {
            auto hb = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 1));
            hb->set_homogeneous(false);

            _pattern_tree_model = Gtk::ListStore::create(_pattern_cols);
            _patternmenu = Gtk::make_managed<Gtk::ComboBox>();
            _patternmenu->set_model(_pattern_tree_model);
            _patternmenu->set_row_separator_func(sigc::mem_fun(*this, &PaintSelector::isSeparator));

            _pattern_cell_renderer = Gtk::make_managed<Gtk::CellRendererText>();
            _pattern_cell_renderer->set_padding(2, 0);
            _patternmenu->pack_start(*_pattern_cell_renderer, true);
            _patternmenu->add_attribute(*_pattern_cell_renderer, "text", _pattern_cols._col_label);

            ink_pattern_menu();
            _patternmenu->signal_changed().connect(sigc::mem_fun(this, &PaintSelector::pattern_change));

            hb->add(*_patternmenu);
            _selector->pack_start(*hb, false, false, AUX_BETWEEN_BUTTON_GROUPS);
        }

        {
            auto hb = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 0));
            hb->set_homogeneous(false);
            auto l = Gtk::manage(new Gtk::Label());
            l->set_markup(
                _("Use the <b>Node tool</b> to adjust position, scale, and rotation of the pattern on canvas. Use "
                  "<b>Object &gt; Pattern &gt; Objects to Pattern</b> to create a new pattern from selection."));
            l->set_line_wrap(true);
            l->set_size_request(180, -1);
            hb->pack_start(*l, true, true, AUX_BETWEEN_BUTTON_GROUPS);
            _selector->pack_start(*hb, false, false, AUX_BETWEEN_BUTTON_GROUPS);
        }

        _selector->show_all();

        _frame->add(*_selector);
        _label->set_markup(_("<b>Pattern fill</b>"));
    }
#ifdef SP_PS_VERBOSE
    g_print("Pattern req\n");
#endif
}

void PaintSelector::set_mode_hatch(PaintSelector::Mode mode)
{
    if (mode == PaintSelector::MODE_HATCH) {
        set_style_buttons(_unset);
    }

    _style->set_sensitive(true);

    if (_mode == PaintSelector::MODE_HATCH) {
        /* Already have hatch menu, for the moment unset */
    } else {
        clear_frame();

        _label->set_markup(_("<b>Hatch fill</b>"));
    }
#ifdef SP_PS_VERBOSE
    g_print("Hatch req\n");
#endif
}

bool PaintSelector::isSeparator(const Glib::RefPtr<Gtk::TreeModel> &model,
                                const Gtk::TreeModel::iterator     &iter)
{
    return (*iter)[_mesh_cols._col_sep];
}

SPPattern *PaintSelector::getPattern()
{
    SPPattern *pat = nullptr;
    g_return_val_if_fail(_mode == MODE_PATTERN, NULL);

    /* no pattern menu if we were just selected */
    if (!_patternmenu) {
        return nullptr;
    }

    /* Get the selected pattern */
    auto iter = _patternmenu->get_active();
    if (!iter) {
        return nullptr;
    }

    Glib::ustring patid   = (*iter)[_pattern_cols._col_pattern];
    auto stockid = (*iter)[_pattern_cols._col_stock];

    if (patid.empty()) {
        return nullptr;
    }

    if (strcmp(patid.c_str(), "none") != 0) {
        gchar *paturn;

        if (stockid) {
            paturn = g_strconcat("urn:inkscape:pattern:", patid.c_str(), NULL);
        } else {
            paturn = g_strdup(patid.c_str());
        }
        SPObject *pat_obj = get_stock_item(paturn);
        if (pat_obj) {
            pat = SP_PATTERN(pat_obj);
        }
        g_free(paturn);
    } else {
        SPDocument *doc = SP_ACTIVE_DOCUMENT;
        SPObject *pat_obj = doc->getObjectById(patid);

        if (pat_obj && SP_IS_PATTERN(pat_obj)) {
            pat = SP_PATTERN(pat_obj)->rootPattern();
        }
    }

    return pat;
}

void PaintSelector::set_mode_swatch(PaintSelector::Mode mode)
{
    if (mode == PaintSelector::MODE_SWATCH) {
        set_style_buttons(_swatch);
    }

    _style->set_sensitive(true);

    if (_mode == PaintSelector::MODE_SWATCH) {
        // Do nothing.  The selector is already a SwatchSelector
    } else {
        clear_frame();
        // Create new gradient selector
        auto swatchsel = Gtk::manage(new SwatchSelector());
        swatchsel->show();

        auto gsel = swatchsel->getGradientSelector();
        gsel->signal_grabbed().connect(sigc::mem_fun(this, &PaintSelector::gradient_grabbed));
        gsel->signal_dragged().connect(sigc::mem_fun(this, &PaintSelector::gradient_dragged));
        gsel->signal_released().connect(sigc::mem_fun(this, &PaintSelector::gradient_released));
        gsel->signal_changed().connect(sigc::mem_fun(this, &PaintSelector::gradient_changed));

        // Pack everything to frame
        _frame->add(*swatchsel);
        _selector = swatchsel;
        _label->set_markup(_("<b>Swatch fill</b>"));
    }

#ifdef SP_PS_VERBOSE
    g_print("Swatch req\n");
#endif
}

// TODO this seems very bad to be taking in a desktop pointer to muck with. Logic probably belongs elsewhere
void PaintSelector::setFlatColor(SPDesktop *desktop, gchar const *color_property, gchar const *opacity_property)
{
    SPCSSAttr *css = sp_repr_css_attr_new();

    SPColor color;
    gfloat alpha = 0;
    getColorAlpha(color, alpha);

    std::string colorStr = color.toString();

#ifdef SP_PS_VERBOSE
    guint32 rgba = color.toRGBA32(alpha);
    g_message("sp_paint_selector_set_flat_color() to '%s' from 0x%08x::%s", colorStr.c_str(), rgba,
              (color.icc ? color.icc->colorProfile.c_str() : "<null>"));
#endif // SP_PS_VERBOSE

    sp_repr_css_set_property(css, color_property, colorStr.c_str());
    Inkscape::CSSOStringStream osalpha;
    osalpha << alpha;
    sp_repr_css_set_property(css, opacity_property, osalpha.str().c_str());

    sp_desktop_set_style(desktop, css);

    sp_repr_css_attr_unref(css);
}

PaintSelector::Mode PaintSelector::getModeForStyle(SPStyle const &style, FillOrStroke kind)
{
    Mode mode = MODE_UNSET;
    SPIPaint const &target = *style.getFillOrStroke(kind == FILL);

    if (!target.set) {
        mode = MODE_UNSET;
    } else if (target.isPaintserver()) {
        SPPaintServer const *server = (kind == FILL) ? style.getFillPaintServer() : style.getStrokePaintServer();

#ifdef SP_PS_VERBOSE
        g_message("PaintSelector::getModeForStyle(%p, %d)", &style, kind);
        g_message("==== server:%p %s  grad:%s   swatch:%s", server, server->getId(),
                  (SP_IS_GRADIENT(server) ? "Y" : "n"),
                  (SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch() ? "Y" : "n"));
#endif // SP_PS_VERBOSE


        if (server && SP_IS_GRADIENT(server) && SP_GRADIENT(server)->getVector()->isSwatch()) {
            mode = MODE_SWATCH;
        } else if (SP_IS_LINEARGRADIENT(server)) {
            mode = MODE_GRADIENT_LINEAR;
        } else if (SP_IS_RADIALGRADIENT(server)) {
            mode = MODE_GRADIENT_RADIAL;
#ifdef WITH_MESH
        } else if (SP_IS_MESHGRADIENT(server)) {
            mode = MODE_GRADIENT_MESH;
#endif
        } else if (SP_IS_PATTERN(server)) {
            mode = MODE_PATTERN;
        } else if (SP_IS_HATCH(server)) {
            mode = MODE_HATCH;
        } else {
            g_warning("file %s: line %d: Unknown paintserver", __FILE__, __LINE__);
            mode = MODE_NONE;
        }
    } else if (target.isColor()) {
        // TODO this is no longer a valid assertion:
        mode = MODE_SOLID_COLOR; // so far only rgb can be read from svg
    } else if (target.isNone()) {
        mode = MODE_NONE;
    } else {
        g_warning("file %s: line %d: Unknown paint type", __FILE__, __LINE__);
        mode = MODE_NONE;
    }

    return mode;
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape
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
