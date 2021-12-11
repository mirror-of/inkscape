// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "live_effects/parameter/patharray.h"

#include <2geom/coord.h>
#include <2geom/point.h>
#include <glibmm/i18n.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/widget.h>

#include "display/curve.h"
#include "document-undo.h"
#include "document.h"
#include "inkscape.h"
#include "live_effects/effect.h"
#include "live_effects/lpe-bspline.h"
#include "live_effects/lpe-spiro.h"
#include "live_effects/lpeobject-reference.h"
#include "live_effects/lpeobject.h"
#include "live_effects/parameter/patharray.h"
#include "object/sp-shape.h"
#include "object/sp-text.h"
#include "object/uri.h"
#include "originalpath.h"
#include "svg/stringstream.h"
#include "svg/svg.h"
#include "ui/clipboard.h"
#include "ui/icon-loader.h"
#include "ui/icon-names.h"

namespace Inkscape {

namespace LivePathEffect {

class PathArrayParam::ModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

    ModelColumns()
    {
        add(_colObject);
        add(_colLabel);
        add(_colReverse);
        add(_colVisible);
    }
    ~ModelColumns() override = default;

    Gtk::TreeModelColumn<PathAndDirectionAndVisible*> _colObject;
    Gtk::TreeModelColumn<Glib::ustring> _colLabel;
    Gtk::TreeModelColumn<bool> _colReverse;
    Gtk::TreeModelColumn<bool> _colVisible;
};

PathArrayParam::PathArrayParam(const Glib::ustring &label, const Glib::ustring &tip, const Glib::ustring &key,
                               Inkscape::UI::Widget::Registry *wr, Effect *effect)
    : Parameter(label, tip, key, wr, effect)
    , _vector()
{   
    _tree = nullptr;
    _scroller = nullptr;
    _model = nullptr;
    initui();
    oncanvas_editable = true;
    _from_original_d = false;
    _allow_only_bspline_spiro = false;
}

PathArrayParam::~PathArrayParam()
{
    while (!_vector.empty()) {
        PathAndDirectionAndVisible *w = _vector.back();
        _vector.pop_back();
        unlink(w);
        delete w;
    }
    delete _model;
}

void PathArrayParam::initui()
{
    SPDesktop * desktop = SP_ACTIVE_DESKTOP;
    if (!desktop) {
        return;
    }
    if (!_tree) {
        _tree = manage(new Gtk::TreeView());
        _model = new ModelColumns();
        _store = Gtk::TreeStore::create(*_model);
        _tree->set_model(_store);

        _tree->set_reorderable(true);
        _tree->enable_model_drag_dest (Gdk::ACTION_MOVE);  
        
        Gtk::CellRendererToggle *toggle_reverse = manage(new Gtk::CellRendererToggle());
        int reverseColNum = _tree->append_column(_("Reverse"), *toggle_reverse) - 1;
        Gtk::TreeViewColumn* col_reverse = _tree->get_column(reverseColNum);
        toggle_reverse->set_activatable(true);
        toggle_reverse->signal_toggled().connect(sigc::mem_fun(*this, &PathArrayParam::on_reverse_toggled));
        col_reverse->add_attribute(toggle_reverse->property_active(), _model->_colReverse);
        

        Gtk::CellRendererToggle *toggle_visible = manage(new Gtk::CellRendererToggle());
        int visibleColNum = _tree->append_column(_("Visible"), *toggle_visible) - 1;
        Gtk::TreeViewColumn* col_visible = _tree->get_column(visibleColNum);
        toggle_visible->set_activatable(true);
        toggle_visible->signal_toggled().connect(sigc::mem_fun(*this, &PathArrayParam::on_visible_toggled));
        col_visible->add_attribute(toggle_visible->property_active(), _model->_colVisible);
        
        Gtk::CellRendererText *text_renderer = manage(new Gtk::CellRendererText());
        int nameColNum = _tree->append_column(_("Name"), *text_renderer) - 1;
        Gtk::TreeView::Column *name_column = _tree->get_column(nameColNum);
        name_column->add_attribute(text_renderer->property_text(), _model->_colLabel);

        _tree->set_expander_column(*_tree->get_column(nameColNum) );
        _tree->set_search_column(_model->_colLabel);
        _scroller = Gtk::manage(new Gtk::ScrolledWindow());
        //quick little hack -- newer versions of gtk gave the item zero space allotment
        _scroller->set_size_request(-1, 120);

        _scroller->add(*_tree);
        _scroller->set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
        //_scroller.set_shadow_type(Gtk::SHADOW_IN);
    }
    param_readSVGValue(param_getSVGValue().c_str());
}

void PathArrayParam::on_reverse_toggled(const Glib::ustring &path)
{
    Gtk::TreeModel::iterator iter = _store->get_iter(path);
    Gtk::TreeModel::Row row = *iter;
    PathAndDirectionAndVisible *w = row[_model->_colObject];
    row[_model->_colReverse] = !row[_model->_colReverse];
    w->reversed = row[_model->_colReverse];
    
    param_write_to_repr(param_getSVGValue().c_str());
    DocumentUndo::done(param_effect->getSPDoc(), _("Link path parameter to path"), INKSCAPE_ICON("dialog-path-effects"));
}

void PathArrayParam::on_visible_toggled(const Glib::ustring &path)
{
    Gtk::TreeModel::iterator iter = _store->get_iter(path);
    Gtk::TreeModel::Row row = *iter;
    PathAndDirectionAndVisible *w = row[_model->_colObject];
    row[_model->_colVisible] = !row[_model->_colVisible];
    w->visibled = row[_model->_colVisible];
    
    param_write_to_repr(param_getSVGValue().c_str());
    DocumentUndo::done(param_effect->getSPDoc(), _("Toggle path parameter visibility"), INKSCAPE_ICON("dialog-path-effects"));
}

void PathArrayParam::param_set_default() {}

Gtk::Widget *PathArrayParam::param_newWidget()
{
    
    Gtk::Box* vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
    Gtk::Box* hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    _tree = nullptr;
    _model = nullptr;
    _scroller = nullptr;
    initui();
    vbox->pack_start(*_scroller, Gtk::PACK_EXPAND_WIDGET);
    
    
    { // Paste path to link button
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("edit-clone", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathArrayParam::on_link_button_click));
        hbox->pack_start(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Link to path in clipboard"));
    }
    
    { // Remove linked path
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("list-remove", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathArrayParam::on_remove_button_click));
        hbox->pack_start(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Remove Path"));
    }
    
    { // Move Down
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("go-down", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathArrayParam::on_down_button_click));
        hbox->pack_end(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Move Down"));
    }
    
    { // Move Down
        Gtk::Image *pIcon = Gtk::manage(sp_get_icon_image("go-up", Gtk::ICON_SIZE_BUTTON));
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathArrayParam::on_up_button_click));
        hbox->pack_end(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Move Up"));
    }
    
    vbox->pack_end(*hbox, Gtk::PACK_SHRINK);
    
    vbox->show_all_children(true);
    
    return vbox;
}

bool PathArrayParam::_selectIndex(const Gtk::TreeIter &iter, int *i)
{
    if ((*i)-- <= 0) {
        _tree->get_selection()->select(iter);
        return true;
    }
    return false;
}

void PathArrayParam::on_up_button_click()
{
    Gtk::TreeModel::iterator iter = _tree->get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        
        int i = -1;
        std::vector<PathAndDirectionAndVisible*>::iterator piter = _vector.begin();
        for (std::vector<PathAndDirectionAndVisible*>::iterator iter = _vector.begin(); iter != _vector.end(); piter = iter, i++, ++iter) {
            if (*iter == row[_model->_colObject]) {
                _vector.erase(iter);
                _vector.insert(piter, row[_model->_colObject]);
                break;
            }
        }
        
        param_write_to_repr(param_getSVGValue().c_str());

        DocumentUndo::done(param_effect->getSPDoc(), _("Move path up"), INKSCAPE_ICON("dialog-path-effects"));

        _store->foreach_iter(sigc::bind<int *>(sigc::mem_fun(*this, &PathArrayParam::_selectIndex), &i));
    }
}

void PathArrayParam::on_down_button_click()
{
    Gtk::TreeModel::iterator iter = _tree->get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;

        int i = 0;
        for (std::vector<PathAndDirectionAndVisible*>::iterator iter = _vector.begin(); iter != _vector.end(); i++, ++iter) {
            if (*iter == row[_model->_colObject]) {
                std::vector<PathAndDirectionAndVisible*>::iterator niter = _vector.erase(iter);
                if (niter != _vector.end()) {
                    ++niter;
                    i++;
                }
                _vector.insert(niter, row[_model->_colObject]);
                break;
            }
        }
        
        param_write_to_repr(param_getSVGValue().c_str());
        
        DocumentUndo::done(param_effect->getSPDoc(), _("Move path down"), INKSCAPE_ICON("dialog-path-effects"));

        _store->foreach_iter(sigc::bind<int *>(sigc::mem_fun(*this, &PathArrayParam::_selectIndex), &i));
    }
}

void PathArrayParam::on_remove_button_click()
{
    Gtk::TreeModel::iterator iter = _tree->get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        unlink(row[_model->_colObject]);

        param_write_to_repr(param_getSVGValue().c_str());
        
        DocumentUndo::done(param_effect->getSPDoc(), _("Remove path"), INKSCAPE_ICON("dialog-path-effects"));
    }
}

void PathArrayParam::on_link_button_click()
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    std::vector<Glib::ustring> pathsid = cm->getElementsOfType(SP_ACTIVE_DESKTOP, "svg:path");
    std::vector<Glib::ustring> textsid = cm->getElementsOfType(SP_ACTIVE_DESKTOP, "svg:text");
    pathsid.insert(pathsid.end(), textsid.begin(), textsid.end());
    if (pathsid.empty()) {
        return;
    }
    bool foundOne = false;
    Inkscape::SVGOStringStream os;
    for (auto iter : _vector) {
        if (foundOne) {
            os << "|";
        } else {
            foundOne = true;
        }
        os << iter->href << "," << (iter->reversed ? "1" : "0") << "," << (iter->visibled ? "1" : "0");
    }
    for (auto pathid : pathsid) {
        // add '#' at start to make it an uri.
        pathid.insert(pathid.begin(), '#');

        if (foundOne) {
            os << "|";
        } else {
            foundOne = true;
        }
        os << pathid.c_str() << ",0,1";
    }
    param_write_to_repr(os.str().c_str());
    DocumentUndo::done(param_effect->getSPDoc(), _("Link patharray parameter to path"), INKSCAPE_ICON("dialog-path-effects"));
}

void PathArrayParam::unlink(PathAndDirectionAndVisible *to)
{
    to->linked_modified_connection.disconnect();
    to->linked_delete_connection.disconnect();
    to->ref.detach();
    to->_pathvector = Geom::PathVector();
    if (to->href) {
        g_free(to->href);
        to->href = nullptr;
    }
    for (std::vector<PathAndDirectionAndVisible*>::iterator iter = _vector.begin(); iter != _vector.end(); ++iter) {
        if (*iter == to) {
            PathAndDirectionAndVisible *w = *iter;
            _vector.erase(iter);
            delete w;
            return;
        }
    }
}

void PathArrayParam::linked_delete(SPObject * /*deleted*/, PathAndDirectionAndVisible * /*to*/)
{
    // unlink(to);

    param_write_to_repr(param_getSVGValue().c_str());
}

bool PathArrayParam::_updateLink(const Gtk::TreeIter &iter, PathAndDirectionAndVisible *pd)
{
    Gtk::TreeModel::Row row = *iter;
    if (row[_model->_colObject] == pd) {
        SPObject *obj = pd->ref.getObject();
        row[_model->_colLabel] = obj && obj->getId() ? ( obj->label() ? obj->label() : obj->getId() ) : pd->href;
        return true;
    }
    return false;
}

void PathArrayParam::linked_changed(SPObject * /*old_obj*/, SPObject *new_obj, PathAndDirectionAndVisible *to)
{
    to->linked_delete_connection.disconnect();
    to->linked_modified_connection.disconnect();
    to->linked_transformed_connection.disconnect();
    
    if (new_obj && SP_IS_ITEM(new_obj)) {
        to->linked_delete_connection = new_obj->connectDelete(
            sigc::bind<PathAndDirectionAndVisible *>(sigc::mem_fun(*this, &PathArrayParam::linked_delete), to));
        to->linked_modified_connection = new_obj->connectModified(
            sigc::bind<PathAndDirectionAndVisible *>(sigc::mem_fun(*this, &PathArrayParam::linked_modified), to));
        to->linked_transformed_connection = SP_ITEM(new_obj)->connectTransformed(
            sigc::bind<PathAndDirectionAndVisible *>(sigc::mem_fun(*this, &PathArrayParam::linked_transformed), to));

        linked_modified(new_obj, SP_OBJECT_MODIFIED_FLAG, to);
    } else {
        to->_pathvector = Geom::PathVector();
        param_effect->getLPEObj()->requestModified(SP_OBJECT_MODIFIED_FLAG);
        if (_store.get()) {
            _store->foreach_iter(
                sigc::bind<PathAndDirectionAndVisible *>(sigc::mem_fun(*this, &PathArrayParam::_updateLink), to));
        }
    }
}

void PathArrayParam::setPathVector(SPObject *linked_obj, guint /*flags*/, PathAndDirectionAndVisible *to)
{
    if (!to) {
        return;
    }
    std::unique_ptr<SPCurve> curve;
    SPText *text = dynamic_cast<SPText *>(linked_obj);
    if (auto shape = dynamic_cast<SPShape const *>(linked_obj)) {
        SPLPEItem * lpe_item = SP_LPE_ITEM(linked_obj);
        if (_from_original_d) {
            curve = SPCurve::copy(shape->curveForEdit());
        } else if (_allow_only_bspline_spiro && lpe_item && lpe_item->hasPathEffect()){
            curve = SPCurve::copy(shape->curveForEdit());
            PathEffectList lpelist = lpe_item->getEffectList();
            PathEffectList::iterator i;
            for (i = lpelist.begin(); i != lpelist.end(); ++i) {
                LivePathEffectObject *lpeobj = (*i)->lpeobject;
                if (lpeobj) {
                    Inkscape::LivePathEffect::Effect *lpe = lpeobj->get_lpe();
                    if (dynamic_cast<Inkscape::LivePathEffect::LPEBSpline *>(lpe)) {
                        Geom::PathVector hp;
                        LivePathEffect::sp_bspline_do_effect(curve.get(), 0, hp);
                    } else if (dynamic_cast<Inkscape::LivePathEffect::LPESpiro *>(lpe)) {
                        LivePathEffect::sp_spiro_do_effect(curve.get());
                    }
                }
            }
        } else {
            curve = SPCurve::copy(shape->curve());
        }
    } else if (text) {
        bool hidden = text->isHidden();
        if (hidden) {
            if (to->_pathvector.empty()) {
                text->setHidden(false);
                curve = text->getNormalizedBpath();
                text->setHidden(true);
            } else {
                if (curve == nullptr) {
                    curve = std::make_unique<SPCurve>();
                }
                curve->set_pathvector(to->_pathvector);
            }
        } else {
            curve = text->getNormalizedBpath();
        }
    }

    if (curve == nullptr) {
        // curve invalid, set empty pathvector
        to->_pathvector = Geom::PathVector();
    } else {
        to->_pathvector = curve->get_pathvector();
    }
    
}

void PathArrayParam::linked_modified(SPObject *linked_obj, guint flags, PathAndDirectionAndVisible *to)
{
    if (!to) {
        return;
    }
    setPathVector(linked_obj, flags, to);
    param_effect->getLPEObj()->requestModified(SP_OBJECT_MODIFIED_FLAG);
    if (_store.get()) {
        _store->foreach_iter(
            sigc::bind<PathAndDirectionAndVisible *>(sigc::mem_fun(*this, &PathArrayParam::_updateLink), to));
    }
}

bool PathArrayParam::param_readSVGValue(const gchar *strvalue)
{
    if (strvalue) {
        while (!_vector.empty()) {
            PathAndDirectionAndVisible *w = _vector.back();
            unlink(w);
            _vector.pop_back();
            delete w;
        }
        
        if (_store.get()) {
            _store->clear();
        }

        gchar ** strarray = g_strsplit(strvalue, "|", 0);
        for (gchar ** iter = strarray; *iter != nullptr; iter++) {
            if ((*iter)[0] == '#') {
                gchar ** substrarray = g_strsplit(*iter, ",", 0);
                PathAndDirectionAndVisible* w = new PathAndDirectionAndVisible((SPObject *)param_effect->getLPEObj());
                w->href = g_strdup(*substrarray);
                w->reversed = *(substrarray+1) != nullptr && (*(substrarray+1))[0] == '1';
                //Like this to make backwards compatible, new value added in 0.93
                w->visibled = *(substrarray+2) == nullptr || (*(substrarray+2))[0] == '1';
                w->linked_changed_connection = w->ref.changedSignal().connect(
                    sigc::bind<PathAndDirectionAndVisible *>(sigc::mem_fun(*this, &PathArrayParam::linked_changed), w));
                w->ref.attach(URI(w->href));

                _vector.push_back(w);
                if (_store.get()) {
                    Gtk::TreeModel::iterator iter = _store->append();
                    Gtk::TreeModel::Row row = *iter;
                    SPObject *obj = w->ref.getObject();

                    row[_model->_colObject] = w;
                    row[_model->_colLabel] = obj ? ( obj->label() ? obj->label() : obj->getId() ) : w->href;
                    row[_model->_colReverse] = w->reversed;
                    row[_model->_colVisible] = w->visibled;
                }
                g_strfreev (substrarray);
            }
        }
        g_strfreev (strarray);
        return true;
    }
    return false;
}

Glib::ustring PathArrayParam::param_getSVGValue() const
{
    Inkscape::SVGOStringStream os;
    bool foundOne = false;
    for (auto iter : _vector) {
        if (foundOne) {
            os << "|";
        } else {
            foundOne = true;
        }
        os << iter->href << "," << (iter->reversed ? "1" : "0") << "," << (iter->visibled ? "1" : "0");
    }
    return os.str();
}

Glib::ustring PathArrayParam::param_getDefaultSVGValue() const
{
    return "";
}

void PathArrayParam::update()
{
    for (auto & iter : _vector) {
        SPObject *linked_obj = iter->ref.getObject();
        linked_modified(linked_obj, SP_OBJECT_MODIFIED_FLAG, iter);
    }
}

} /* namespace LivePathEffect */

} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
