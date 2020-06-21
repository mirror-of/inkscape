// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Generic paint selector widget
 *//*
 * Authors:
 *   Lauris
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#ifndef SEEN_SP_PAINT_SELECTOR_H
#define SEEN_SP_PAINT_SELECTOR_H

#include "color.h"
#include "fill-or-stroke.h"
#include <glib.h>
#include <gtkmm/box.h>
#include <gtkmm/treemodel.h>

#include "object/sp-gradient-spread.h"
#include "object/sp-gradient-units.h"

#include "ui/selected-color.h"

class SPGradient;
#ifdef WITH_MESH
class SPMeshGradient;
#endif
class SPDesktop;
class SPPattern;
class SPStyle;

namespace Gtk {
class CellRendererText;
class ComboBox;
class Label;
class ListStore;
class RadioButton;
class ToggleButton;
} // namespace Gtk

namespace Inkscape {
namespace UI {
namespace Widget {

class FillRuleRadioButton;
class StyleToggleButton;
class GradientSelector;

/**
 * Generic paint selector widget.
 */
class PaintSelector : public Gtk::Box {
  public:
    enum Mode {
        MODE_EMPTY,
        MODE_MULTIPLE,
        MODE_NONE,
        MODE_SOLID_COLOR,
        MODE_GRADIENT_LINEAR,
        MODE_GRADIENT_RADIAL,
#ifdef WITH_MESH
        MODE_GRADIENT_MESH,
#endif
        MODE_PATTERN,
        MODE_HATCH,
        MODE_SWATCH,
        MODE_UNSET
    };

    enum FillRule { FILLRULE_NONZERO, FILLRULE_EVENODD };

  private:
    bool _update = false;

    Mode _mode;

    Gtk::Box *_style;
    StyleToggleButton *_none;
    StyleToggleButton *_solid;
    StyleToggleButton *_gradient;
    StyleToggleButton *_radial;
#ifdef WITH_MESH
    StyleToggleButton *_mesh;
#endif
    StyleToggleButton *_pattern;
    StyleToggleButton *_swatch;
    StyleToggleButton *_unset;

    Gtk::Box *_fillrulebox;
    FillRuleRadioButton *_evenodd;
    FillRuleRadioButton *_nonzero;

    Gtk::Box *_frame;
    Gtk::Box *_selector = nullptr;
    Gtk::Label *_label;
    Gtk::ComboBox *_patternmenu = nullptr;
    bool _patternmenu_update = false;

    class PatternModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        PatternModelColumns()
        {
            add(_col_label);
            add(_col_stock);
            add(_col_pattern);
            add(_col_sep);
        }

        Gtk::TreeModelColumn<Glib::ustring> _col_label;   ///< Label for the mesh
        Gtk::TreeModelColumn<bool>          _col_stock;   ///< Stock-ID or not
        Gtk::TreeModelColumn<Glib::ustring> _col_pattern; ///< Pointer to the pattern
        Gtk::TreeModelColumn<bool>          _col_sep;     ///< Separator or not
    };

    PatternModelColumns _pattern_cols;
    Glib::RefPtr<Gtk::ListStore> _pattern_tree_model;
    Gtk::CellRendererText *_pattern_cell_renderer;

#ifdef WITH_MESH
    Gtk::ComboBox *_meshmenu = nullptr;
    bool _meshmenu_update = false;

    class MeshModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        MeshModelColumns()
        {
            add(_col_label);
            add(_col_stock);
            add(_col_mesh);
            add(_col_sep);
        }

        Gtk::TreeModelColumn<Glib::ustring> _col_label; ///< Label for the mesh
        Gtk::TreeModelColumn<bool>          _col_stock; ///< Stock-ID or not
        Gtk::TreeModelColumn<Glib::ustring> _col_mesh;  ///< Pointer to the mesh
        Gtk::TreeModelColumn<bool>          _col_sep;   ///< Separator or not
    };

    MeshModelColumns _mesh_cols;
    Glib::RefPtr<Gtk::ListStore> _mesh_tree_model;
    Gtk::CellRendererText *_mesh_cell_renderer;
#endif

    Inkscape::UI::SelectedColor *_selected_color;
    bool _updating_color;

    void getColorAlpha(SPColor &color, gfloat &alpha) const;

    static gboolean isSeparator(GtkTreeModel *model, GtkTreeIter *iter, gpointer data);

  private:
    sigc::signal<void, FillRule> _signal_fillrule_changed;
    sigc::signal<void> _signal_dragged;
    sigc::signal<void, Mode> _signal_mode_changed;
    sigc::signal<void> _signal_grabbed;
    sigc::signal<void> _signal_released;
    sigc::signal<void> _signal_changed;

    StyleToggleButton *style_button_add(gchar const *px, PaintSelector::Mode mode, gchar const *tip);
    void style_button_toggled(StyleToggleButton *tb);
    void fillrule_toggled(FillRuleRadioButton *tb);
    void onSelectedColorGrabbed();
    void onSelectedColorDragged();
    void onSelectedColorReleased();
    void onSelectedColorChanged();
    void set_mode_empty();
    void set_style_buttons(Gtk::ToggleButton *active);
    void set_mode_multiple();
    void set_mode_none();
    GradientSelector *getGradientFromData() const;
    void clear_frame();
    void set_mode_unset();
    void set_mode_color(PaintSelector::Mode mode);
    void set_mode_gradient(PaintSelector::Mode mode);
    void ink_pattern_menu();
    void ink_pattern_menu_populate_menu(SPDocument *doc);
    void pattern_list_from_doc(SPDocument *current_doc,
                               SPDocument *source,
                               SPDocument *pattern_doc);
    void pattern_menu_build(std::vector<SPPattern *> &pl, SPDocument *source);
#ifdef WITH_MESH
    void set_mode_mesh(PaintSelector::Mode mode);
    void ink_mesh_menu();
    void ink_mesh_menu_populate_menu(SPDocument *doc);
    void mesh_list_from_doc(SPDocument *source,
                            SPDocument *mesh_doc);
    void mesh_menu_build(std::vector<SPMeshGradient *> &mesh_list, SPDocument *source);
    bool isSeparator(const Glib::RefPtr<Gtk::TreeModel> &model,
                     const Gtk::TreeModel::iterator     &iter);

    void mesh_change();
#endif
    void set_mode_pattern(PaintSelector::Mode mode);
    void set_mode_hatch(PaintSelector::Mode mode);
    void set_mode_swatch(PaintSelector::Mode mode);

    void gradient_grabbed();
    void gradient_dragged();
    void gradient_released();
    void gradient_changed(SPGradient *gr);

    void pattern_change();

  public:
    PaintSelector(FillOrStroke kind);
    ~PaintSelector() override;

    inline decltype(_signal_fillrule_changed) signal_fillrule_changed() const { return _signal_fillrule_changed; }
    inline decltype(_signal_dragged) signal_dragged() const { return _signal_dragged; }
    inline decltype(_signal_mode_changed) signal_mode_changed() const { return _signal_mode_changed; }
    inline decltype(_signal_grabbed) signal_grabbed() const { return _signal_grabbed; }
    inline decltype(_signal_released) signal_released() const { return _signal_released; }
    inline decltype(_signal_changed) signal_changed() const { return _signal_changed; }

    void setMode(Mode mode);
    static Mode getModeForStyle(SPStyle const &style, FillOrStroke kind);
    void setFillrule(FillRule fillrule);
    void setColorAlpha(SPColor const &color, float alpha);
    void setSwatch(SPGradient *vector);
    void setGradientLinear(SPGradient *vector);
    void setGradientRadial(SPGradient *vector);
#ifdef WITH_MESH
    void setGradientMesh(SPMeshGradient *array);
#endif
    void setGradientProperties(SPGradientUnits units, SPGradientSpread spread);
    void getGradientProperties(SPGradientUnits &units, SPGradientSpread &spread) const;

#ifdef WITH_MESH
    SPMeshGradient *getMeshGradient();
    void updateMeshList(SPMeshGradient *pat);
#endif

    void updatePatternList(SPPattern *pat);
    inline decltype(_mode) get_mode() const { return _mode; }

    // TODO move this elsewhere:
    void setFlatColor(SPDesktop *desktop, const gchar *color_property, const gchar *opacity_property);

    SPGradient *getGradientVector();
    void pushAttrsToGradient(SPGradient *gr) const;
    SPPattern *getPattern();
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape
#endif // SEEN_SP_PAINT_SELECTOR_H

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
