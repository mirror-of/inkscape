// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * A simple panel for objects (originally developed for Ponyscape, an Inkscape derivative)
 *
 * Authors:
 *   Martin Owens, completely rewritten
 *   Theodore Janeczko
 *   Tweaked by Liam P White for use in Inkscape
 *   Tavmjong Bah
 *
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *               Tavmjong Bah 2017
 *               Martin Owens 2020-2021
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "objects.h"

#include <gtkmm/icontheme.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <glibmm/main.h>
#include <glibmm/i18n.h>

#include "desktop-style.h"
#include "desktop.h"
#include "document-undo.h"
#include "document.h"
#include "filter-chemistry.h"
#include "inkscape.h"
#include "layer-manager.h"
#include "verbs.h"

#include "actions/actions-tools.h"

#include "helper/action.h"

#include "include/gtkmm_version.h"

#include "object/filters/blend.h"
#include "object/filters/gaussian-blur.h"
#include "object/sp-clippath.h"
#include "object/sp-mask.h"
#include "object/sp-root.h"
#include "object/sp-shape.h"
#include "style.h"

#include "ui/dialog-events.h"
#include "ui/icon-loader.h"
#include "ui/icon-names.h"
#include "ui/selected-color.h"
#include "ui/shortcuts.h"
#include "ui/desktop/menu-icon-shift.h"
#include "ui/tools/node-tool.h"

#include "ui/contextmenu.h"
#include "ui/widget/canvas.h"
#include "ui/widget/imagetoggler.h"
#include "ui/widget/shapeicon.h"

static double const SELECTED_ALPHA[8] = {0.0, 2.5, 4.0, 2.0, 8.0, 2.5, 1.0, 1.0};

//#define DUMP_LAYERS 1

namespace Inkscape {
namespace UI {
namespace Dialog {

class ObjectWatcher : public Inkscape::XML::NodeObserver
{
public:
    ObjectWatcher() = delete;
    ObjectWatcher(ObjectsPanel *panel, SPItem *, Gtk::TreeRow *row, bool layers_only);
    ~ObjectWatcher() override;

    void updateRowInfo();
    void updateRowHighlight();
    void updateRowAncestorState(bool invisible, bool locked);
    void updateRowBg(guint32 rgba = 0.0);

    ObjectWatcher *findChild(Node *node);
    void addDummyChild();
    bool addChild(SPItem *, bool dummy = true);
    void addChildren(SPItem *, bool dummy = false);
    void setSelectedBit(SelectionState mask, bool enabled);
    void setSelectedBitRecursive(SelectionState mask, bool enabled);
    void setSelectedBitChildren(SelectionState mask, bool enabled);
    void moveChild(Node &child, Node *sibling);

    Gtk::TreeNodeChildren getChildren() const;
    Gtk::TreeIter getChildIter(Node *) const;

    void notifyChildAdded(Node &, Node &, Node *) override;
    void notifyChildRemoved(Node &, Node &, Node *) override;
    void notifyChildOrderChanged(Node &, Node &child, Node *, Node *) override;
    void notifyAttributeChanged(Node &, GQuark, Util::ptr_shared, Util::ptr_shared) override;

    /// Associate this watcher with a tree row
    void setRow(const Gtk::TreeModel::Path &path)
    {
        assert(path);
        row_ref = Gtk::TreeModel::RowReference(panel->_store, path);
    }
    void setRow(const Gtk::TreeModel::Row &row)
    {
        setRow(panel->_store->get_path(row));
    }

    // Get the path out of this watcher
    Gtk::TreeModel::Path getTreePath() const {
        return row_ref.get_path();
    }

    /// True if this watchr has a valid row reference.
    bool hasRow() const { return bool(row_ref); }

    /// Transfer a child watcher to its new parent
    void transferChild(Node *childnode)
    {
        auto *target = panel->getWatcher(childnode->parent());
        assert(target != this);
        auto nh = child_watchers.extract(childnode);
        assert(nh);
        bool inserted = target->child_watchers.insert(std::move(nh)).inserted;
        assert(inserted);
    }

    /// The XML node associated with this watcher.
    Node *getRepr() const { return node; }
    Gtk::TreeModel::Row const *getRow() const {
        if (auto path = row_ref.get_path()) {
            if(auto iter = panel->_store->get_iter(path)) {
                return &*iter;
            }
        }
        return nullptr;
    }

    std::unordered_map<Node const *, std::unique_ptr<ObjectWatcher>> child_watchers;

private:
    Node *node;
    Gtk::TreeModel::RowReference row_ref;
    ObjectsPanel *panel;
    SelectionState selection_state;
    bool layers_only;
};

class ObjectsPanel::ModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    ModelColumns()
    {
        add(_colNode);
        add(_colLabel);
        add(_colType);
        add(_colIconColor);
        add(_colClipMask);
        add(_colBgColor);
        add(_colInvisible);
        add(_colLocked);
        add(_colAncestorInvisible);
        add(_colAncestorLocked);
        add(_colHover);
    }
    ~ModelColumns() override = default;
    Gtk::TreeModelColumn<Node*> _colNode;
    Gtk::TreeModelColumn<Glib::ustring> _colLabel;
    Gtk::TreeModelColumn<Glib::ustring> _colType;
    Gtk::TreeModelColumn<unsigned int> _colIconColor;
    Gtk::TreeModelColumn<unsigned int> _colClipMask;
    Gtk::TreeModelColumn<Gdk::RGBA> _colBgColor;
    Gtk::TreeModelColumn<bool> _colInvisible;
    Gtk::TreeModelColumn<bool> _colLocked;
    Gtk::TreeModelColumn<bool> _colAncestorInvisible;
    Gtk::TreeModelColumn<bool> _colAncestorLocked;
    Gtk::TreeModelColumn<bool> _colHover;
};

/**
 * Gets an instance of the Objects panel
 */
ObjectsPanel& ObjectsPanel::getInstance()
{
    return *new ObjectsPanel();
}

/**
 * Creates a new ObjectWatcher, a gtk TreeView interated watching device.
 *
 * @param panel The panel to which the object watcher belongs
 * @param obj The object to watch
 * @param iter The optional list store iter for the item, if not provided,
 *             assumes this is the root 'document' object.
 * @param layers If true, only show and watch layers, not groups or other objects.
 */
ObjectWatcher::ObjectWatcher(ObjectsPanel* panel, SPItem* obj, Gtk::TreeRow *row, bool layers)
    : panel(panel)
    , layers_only(layers)
    , row_ref()
    , selection_state(0)
    , node(obj->getRepr())
{
    if(row != nullptr) {
        assert(row->children().empty());
        setRow(*row);
        updateRowInfo();
    }
    node->addObserver(*this);

    // Only show children for groups (and their subclasses like SPAnchor or SPRoot)
    if (!dynamic_cast<SPGroup const*>(obj)) {
        return;
    }
    // Add children as a dummy row to avoid excensive execution when
    // the tree is really large, but not in layers mode.
    addChildren(obj, (bool)row && !layers);
}
ObjectWatcher::~ObjectWatcher()
{
    node->removeObserver(*this);
    Gtk::TreeModel::Path path;
    if (bool(row_ref) && (path = row_ref.get_path())) {
        auto iter = panel->_store->get_iter(path);
        if(iter) {
            panel->_store->erase(iter);
        }
    }
    child_watchers.clear();
}

/**
 * Update the information in the row from the stored node
 */
void ObjectWatcher::updateRowInfo() {
    if (auto item = dynamic_cast<SPItem *>(panel->getObject(node))) {
        assert(row_ref);
        assert(row_ref.get_path());

        auto _model = panel->_model;
        auto row = *panel->_store->get_iter(row_ref.get_path());
        row[_model->_colNode] = node;

        // show ids without "#"
        char const *id = item->getId();
        row[_model->_colLabel] = (id && !item->label()) ? id : item->defaultLabel();

        row[_model->_colType] = item->typeName();
        row[_model->_colClipMask] =
            (item->getClipObject() ? Inkscape::UI::Widget::OVERLAY_CLIP : 0) |
            (item->getMaskObject() ? Inkscape::UI::Widget::OVERLAY_MASK : 0);
        row[_model->_colInvisible] = item->isHidden();
        row[_model->_colLocked] = !item->isSensitive();

        updateRowHighlight();
        updateRowAncestorState(row[_model->_colAncestorInvisible], row[_model->_colAncestorLocked]);
    }
}

/**
 * Propegate changes to the highlight color to all children.
 */
void ObjectWatcher::updateRowHighlight() {
    if (auto item = dynamic_cast<SPItem *>(panel->getObject(node))) {
        auto row = *panel->_store->get_iter(row_ref.get_path());
        auto new_color = item->highlight_color();
        if (new_color != row[panel->_model->_colIconColor]) {
            row[panel->_model->_colIconColor] = new_color;
            updateRowBg(new_color);
            for (auto &watcher : child_watchers) {
                watcher.second->updateRowHighlight();
            }
        }
    }
}

/**
 * Propegate a change in visibility or locked state to all children
 */
void ObjectWatcher::updateRowAncestorState(bool invisible, bool locked) {
    auto _model = panel->_model;
    auto row = *panel->_store->get_iter(row_ref.get_path());
    row[_model->_colAncestorInvisible] = invisible;
    row[_model->_colAncestorLocked] = locked;
    for (auto &watcher : child_watchers) {
        watcher.second->updateRowAncestorState(
            invisible || row[_model->_colInvisible],
            locked || row[_model->_colLocked]);
    }
}

/**
 * Updates the row's background colour as indicated by it's selection.
 */
void ObjectWatcher::updateRowBg(guint32 rgba)
{
    assert(row_ref);
    if (auto row = *panel->_store->get_iter(row_ref.get_path())) {
        auto alpha = SELECTED_ALPHA[selection_state];
        if (alpha == 0.0) {
            row[panel->_model->_colBgColor] = Gdk::RGBA();
            return;
        }
        if (rgba == 0.0) {
            rgba = row[panel->_model->_colIconColor];
        }

        auto color = ColorRGBA(rgba);
        auto gdk_color = Gdk::RGBA();
        gdk_color.set_red(color[0]);
        gdk_color.set_green(color[1]);
        gdk_color.set_blue(color[2]);
        gdk_color.set_alpha(color[3] / alpha);
        row[panel->_model->_colBgColor] = gdk_color;
    }
}

/**
 * Flip the selected state bit on or off as needed, calls updateRowBg if changed.
 *
 * @param mask - The selection bit to set or unset
 * @param enabled - If the bit should be set or unset
 */
void ObjectWatcher::setSelectedBit(SelectionState mask, bool enabled) {
    if (!row_ref) return;
    SelectionState value = selection_state;
    SelectionState original = value;
    if (enabled) {
        value |= mask;
    } else {
        value &= ~mask;
    }
    if (value != original) {
        selection_state = value;
        updateRowBg();
    }
}

/**
 * Flip the selected state bit on or off as needed, on this watcher and all
 * its direct and indirect children.
 */
void ObjectWatcher::setSelectedBitRecursive(SelectionState mask, bool enabled)
{
    setSelectedBit(mask, enabled);
    setSelectedBitChildren(mask, enabled);
}
void ObjectWatcher::setSelectedBitChildren(SelectionState mask, bool enabled)
{
    for (auto &pair : child_watchers) {
        pair.second->setSelectedBitRecursive(mask, enabled);
    }
}

/**
 * Find the child watcher for the given node.
 */
ObjectWatcher *ObjectWatcher::findChild(Node *node)
{
    auto it = child_watchers.find(node);
    if (it != child_watchers.end()) {
        return it->second.get();
    }
    return nullptr;
}

/**
 * Add the child object to this node.
 *
 * @param child - SPObject to be added
 * @param dummy - Add a dummy objects (hidden) instead
 *
 * @returns true if child added was a dummy objects
 */
bool ObjectWatcher::addChild(SPItem *child, bool dummy)
{
    auto group = dynamic_cast<SPGroup *>(child);
    if (layers_only && (!group || group->layerMode() != SPGroup::LAYER)) {
        return false;
    }

    auto const children = getChildren();
    if (dummy && row_ref) {
        if (children.empty()) {
            auto const iter = panel->_store->append(children);
            assert(panel->isDummy(*iter));
            return true;
        } else if (panel->isDummy(children[0])) {
            return false;
        }
    }

    auto *node = child->getRepr();
    assert(node);
    Gtk::TreeModel::Row row = *(panel->_store->prepend(children));

    // Ancestor states are handled inside the list store (so we don't have to re-ask every update)
    auto _model = panel->_model;
    if (row_ref) {
        auto parent_row = *panel->_store->get_iter(row_ref.get_path());
        row[_model->_colAncestorInvisible] = parent_row[_model->_colAncestorInvisible] || parent_row[_model->_colInvisible];
        row[_model->_colAncestorLocked] = parent_row[_model->_colAncestorLocked] || parent_row[_model->_colLocked];
    } else {
        row[_model->_colAncestorInvisible] = false;
        row[_model->_colAncestorLocked] = false;
    }

    auto &watcher = child_watchers[node];
    assert(!watcher);
    watcher.reset(new ObjectWatcher(panel, child, &row, layers_only));

    // Make sure new children have the right focus set.
    if ((selection_state & LAYER_FOCUSED) != 0) {
        watcher->setSelectedBit(LAYER_FOCUS_CHILD, true);
    }
    return false;
}

/**
 * Add all SPItem children as child rows.
 */
void ObjectWatcher::addChildren(SPItem *obj, bool dummy)
{
    assert(child_watchers.empty());

    for (auto &child : obj->children) {
        if (auto item = dynamic_cast<SPItem *>(&child)) {
            if (addChild(item, dummy) && dummy) {
                // one dummy child is enough to make the group expandable
                break;
            }
        }
    }
}

/**
 * Move the child to just after the given sibling
 *
 * @param child - SPObject to be moved
 * @param sibling - Optional sibling Object to add next to, if nullptr the
 *                  object is moved to BEFORE the first item.
 */
void ObjectWatcher::moveChild(Node &child, Node *sibling)
{
    auto child_iter = getChildIter(&child);
    if (!child_iter)
        return; // This means the child was never added, probably not an SPItem.

    // sibling might not be an SPItem and thus not be represented in the
    // TreeView. Find the closest SPItem and use that for the reordering.
    while (sibling && !dynamic_cast<SPItem const *>(panel->getObject(sibling))) {
        sibling = sibling->prev();
    }

    auto sibling_iter = getChildIter(sibling);
    panel->_store->move(child_iter, sibling_iter);
}

/**
 * Get the TreeRow's children iterator
 *
 * @returns Gtk Tree Node Children iterator
 */
Gtk::TreeNodeChildren ObjectWatcher::getChildren() const
{
    Gtk::TreeModel::Path path;
    if (row_ref && (path = row_ref.get_path())) {
        return panel->_store->get_iter(path)->children();
    }
    assert(!row_ref);
    return panel->_store->children();
}

/**
 * Convert SPObject to TreeView Row, assuming the object is a child.
 *
 * @param child - The child object to find in this branch
 * @returns Gtk TreeRow for the child, or end() if not found
 */
Gtk::TreeIter ObjectWatcher::getChildIter(Node *node) const
{
    auto childrows = getChildren();

    if (!node) {
        return childrows.end();
    }

    // Note: TreeRow inherits from TreeIter, so this `row` variable is
    // also an iterator and a valid return value.
    for (auto &row : childrows) {
        if (panel->getRepr(row) == node) {
            return row;
        }
    }
    // In layer mode, we will come here for all non-layers
    return childrows.begin();
}

void ObjectWatcher::notifyChildAdded( Node &node, Node &child, Node *prev )
{
    assert(this->node == &node);
    // Ignore XML nodes which are not displayable items
    if (auto item = dynamic_cast<SPItem *>(panel->getObject(&child))) {
        addChild(item);
        moveChild(child, prev);
    }
}
void ObjectWatcher::notifyChildRemoved( Node &node, Node &child, Node* /*prev*/ )
{
    assert(this->node == &node);

    if (child_watchers.erase(&child) > 0) {
        return;
    }

    if (node.firstChild() == nullptr) {
        assert(row_ref);
        auto iter = panel->_store->get_iter(row_ref.get_path());
        panel->removeDummyChildren(*iter);
    }
}
void ObjectWatcher::notifyChildOrderChanged( Node &parent, Node &child, Node */*old_prev*/, Node *new_prev )
{
    assert(this->node == &parent);

    moveChild(child, new_prev);
}
void ObjectWatcher::notifyAttributeChanged( Node &node, GQuark name, Util::ptr_shared /*old_value*/, Util::ptr_shared /*new_value*/ )
{
    assert(this->node == &node);

    // The root <svg> node doesn't have a row
    if (this == panel->getRootWatcher()) {
        return;
    }

    // Almost anything could change the icon, so update upon any change, defer for lots of updates.

    // examples of not-so-obvious cases:
    // - width/height: Can change type "circle" to an "ellipse"

    static std::set<GQuark> const excluded{
        g_quark_from_static_string("transform"),
        g_quark_from_static_string("x"),
        g_quark_from_static_string("y"),
        g_quark_from_static_string("d"),
        g_quark_from_static_string("sodipodi:nodetypes"),
    };

    if (excluded.count(name)) {
        return;
    }

    updateRowInfo();
}


/**
 * Get the object from the node.
 *
 * @param node - XML Node involved in the signal.
 * @returns SPObject matching the node, returns nullptr if not found.
 */
SPObject *ObjectsPanel::getObject(Node *node) {
    if (node != nullptr && getDocument())
        return getDocument()->getObjectByRepr(node);
    return nullptr;
}

/**
 * Get the object watcher from the xml node (reverse lookup), it uses a ancesstor
 * recursive pattern to match up with the root_watcher.
 *
 * @param node - The node to look up.
 * @return the ObjectWatcher object if it's possible to find.
 */
ObjectWatcher* ObjectsPanel::getWatcher(Node *node)
{
    assert(node);
    if (root_watcher->getRepr() == node) {
        return root_watcher;
    } else if (node->parent()) {
        if (auto parent_watcher = getWatcher(node->parent())) {
            return parent_watcher->findChild(node);
        }
    }
    return nullptr;
}

/**
 * Constructor
 */
ObjectsPanel::ObjectsPanel() :
    DialogBase("/dialogs/objects", "Objects"),
    root_watcher(nullptr),
    _model(nullptr),
    _layer(nullptr),
    _is_editing(false),
    _page(Gtk::ORIENTATION_VERTICAL)
{
    //Create the tree model and store
    ModelColumns *zoop = new ModelColumns();
    _model = zoop;

    _store = Gtk::TreeStore::create( *zoop );

    //Set up the tree
    _tree.set_model( _store );
    _tree.set_headers_visible(false);
    // Reorderable means that we allow drag-and-drop, but we only allow that
    // when at least one row is selected
    _tree.set_reorderable(true);
    _tree.enable_model_drag_dest (Gdk::ACTION_MOVE);

    //Label
    _name_column = Gtk::manage(new Gtk::TreeViewColumn());
    _text_renderer = Gtk::manage(new Gtk::CellRendererText());
    _text_renderer->property_editable() = true;
    _text_renderer->property_ellipsize().set_value(Pango::ELLIPSIZE_END);

    auto icon_renderer = Gtk::manage(new Inkscape::UI::Widget::CellRendererItemIcon());
    icon_renderer->property_xpad() = 2;
    icon_renderer->property_width() = 24;
    _tree.append_column(*_name_column);
    _name_column->set_expand(true);
    _name_column->pack_start(*icon_renderer, false);
    _name_column->pack_start(*_text_renderer, true);
    _name_column->add_attribute(_text_renderer->property_text(), _model->_colLabel);
    _name_column->add_attribute(_text_renderer->property_cell_background_rgba(), _model->_colBgColor);
    _name_column->add_attribute(icon_renderer->property_shape_type(), _model->_colType);
    _name_column->add_attribute(icon_renderer->property_color(), _model->_colIconColor);
    _name_column->add_attribute(icon_renderer->property_clipmask(), _model->_colClipMask);
    _name_column->add_attribute(icon_renderer->property_cell_background_rgba(), _model->_colBgColor);

    // Visible icon
    auto *eyeRenderer = Gtk::manage( new Inkscape::UI::Widget::ImageToggler(
            INKSCAPE_ICON("object-hidden"), INKSCAPE_ICON("object-visible")));
    int visibleColNum = _tree.append_column("vis", *eyeRenderer) - 1;
    eyeRenderer->signal_toggled().connect(sigc::mem_fun(*this, &ObjectsPanel::toggleVisible));
    if (auto eye = _tree.get_column(visibleColNum)) {
        eye->add_attribute(eyeRenderer->property_active(), _model->_colInvisible);
        eye->add_attribute(eyeRenderer->property_cell_background_rgba(), _model->_colBgColor);
        eye->add_attribute(eyeRenderer->property_activatable(), _model->_colHover);
        eye->add_attribute(eyeRenderer->property_gossamer(), _model->_colAncestorInvisible);
    }

    // Unlocked icon
    Inkscape::UI::Widget::ImageToggler * lockRenderer = Gtk::manage( new Inkscape::UI::Widget::ImageToggler(
        INKSCAPE_ICON("object-locked"), INKSCAPE_ICON("object-unlocked")));
    int lockedColNum = _tree.append_column("lock", *lockRenderer) - 1;
    lockRenderer->signal_toggled().connect(sigc::mem_fun(*this, &ObjectsPanel::toggleLocked));
    if (auto lock = _tree.get_column(lockedColNum)) {
        lock->add_attribute(lockRenderer->property_active(), _model->_colLocked);
        lock->add_attribute(lockRenderer->property_cell_background_rgba(), _model->_colBgColor);
        lock->add_attribute(lockRenderer->property_activatable(), _model->_colHover);
        lock->add_attribute(lockRenderer->property_gossamer(), _model->_colAncestorLocked);
    }

    //Set the expander and search columns
    _tree.set_expander_column(*_name_column);
    // Disable search (it doesn't make much sense)
    _tree.set_search_column(-1);
    _tree.set_enable_search(false);
    _tree.get_selection()->set_mode(Gtk::SELECTION_NONE);

    //Set up tree signals
    _tree.signal_button_press_event().connect(sigc::mem_fun(*this, &ObjectsPanel::_handleButtonEvent), false);
    _tree.signal_button_release_event().connect(sigc::mem_fun(*this, &ObjectsPanel::_handleButtonEvent), false);
    _tree.signal_key_press_event().connect(sigc::mem_fun(*this, &ObjectsPanel::_handleKeyEvent), false);
    _tree.signal_motion_notify_event().connect(sigc::mem_fun(*this, &ObjectsPanel::_handleMotionEvent), false);

    // Before expanding a row, replace the dummy child with the actual children
    _tree.signal_test_expand_row().connect([this](const Gtk::TreeModel::iterator &iter, const Gtk::TreeModel::Path &) {
        if (cleanDummyChildren(*iter)) {
            if (auto selection = getSelection()) {
                selectionChanged(selection);
            }
        }
        return false;
    });

    _tree.signal_drag_motion().connect(sigc::mem_fun(*this, &ObjectsPanel::on_drag_motion), false);
    _tree.signal_drag_drop().connect(sigc::mem_fun(*this, &ObjectsPanel::on_drag_drop), false);
    _tree.signal_drag_begin().connect(sigc::mem_fun(*this, &ObjectsPanel::on_drag_start), false);
    _tree.signal_drag_end().connect(sigc::mem_fun(*this, &ObjectsPanel::on_drag_end), false);

    //Set up the label editing signals
    _text_renderer->signal_edited().connect(sigc::mem_fun(*this, &ObjectsPanel::_handleEdited));

    //Set up the scroller window and pack the page
    _scroller.add(_tree);
    _scroller.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
    _scroller.set_shadow_type(Gtk::SHADOW_IN);
    Gtk::Requisition sreq;
    Gtk::Requisition sreq_natural;
    _scroller.get_preferred_size(sreq_natural, sreq);
    int minHeight = 70;
    if (sreq.height < minHeight) {
        // Set a min height to see the layers when used with Ubuntu liboverlay-scrollbar
        _scroller.set_size_request(sreq.width, minHeight);
    }

    _page.pack_start(_buttonsRow, Gtk::PACK_SHRINK);
    _page.pack_end(_scroller, Gtk::PACK_EXPAND_WIDGET);
    pack_start(_page, Gtk::PACK_EXPAND_WIDGET);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    {
        auto child = Glib::wrap(sp_get_icon_image("layer-duplicate", GTK_ICON_SIZE_SMALL_TOOLBAR));
        child->show();
        _object_mode.add(*child);
        _object_mode.set_relief(Gtk::RELIEF_NONE);
    }
    _object_mode.set_tooltip_text(_("Switch to layers only view."));
    _object_mode.property_active() = prefs->getBool("/dialogs/objects/layers_only", false);
    _object_mode.property_active().signal_changed().connect(sigc::mem_fun(*this, &ObjectsPanel::_objects_toggle));
    _buttonsPrimary.pack_start(_object_mode, Gtk::PACK_SHRINK);

    _buttonsPrimary.pack_start(*_addBarButton(INKSCAPE_ICON("layer-new"), _("Add layer..."), (int)SP_VERB_LAYER_NEW), Gtk::PACK_SHRINK);
    _buttonsSecondary.pack_end(*_addBarButton(INKSCAPE_ICON("edit-delete"), _("Remove object"), (int)SP_VERB_EDIT_DELETE), Gtk::PACK_SHRINK);
    _buttonsSecondary.pack_end(*_addBarButton(INKSCAPE_ICON("go-down"), _("Move Down"), (int)SP_VERB_SELECTION_STACK_DOWN), Gtk::PACK_SHRINK);
    _buttonsSecondary.pack_end(*_addBarButton(INKSCAPE_ICON("go-up"), _("Move Up"), (int)SP_VERB_SELECTION_STACK_UP), Gtk::PACK_SHRINK);

    _buttonsRow.pack_start(_buttonsPrimary, Gtk::PACK_SHRINK);
    _buttonsRow.pack_end(_buttonsSecondary, Gtk::PACK_SHRINK);

    update();
    show_all_children();
}

/**
 * Destructor
 */
ObjectsPanel::~ObjectsPanel()
{
    if (root_watcher) {
        delete root_watcher;
    }
    root_watcher = nullptr;

    if (_model) {
        delete _model;
        _model = nullptr;
    }
}

void ObjectsPanel::_objects_toggle()
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setBool("/dialogs/objects/layers_only", _object_mode.get_active());
    // Clear and update entire tree (do not use this in changed/modified signals)
    setRootWatcher();
}

void ObjectsPanel::desktopReplaced()
{
    layer_changed.disconnect();

    if (auto desktop = getDesktop()) {
        layer_changed = desktop->connectCurrentLayerChanged( sigc::mem_fun(*this, &ObjectsPanel::layerChanged));
    }
}

void ObjectsPanel::documentReplaced()
{
    setRootWatcher();
}

void ObjectsPanel::setRootWatcher()
{
    if (root_watcher) {
        delete root_watcher;
    }
    root_watcher = nullptr;

    if (auto document = getDocument()) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        bool layers_only = prefs->getBool("/dialogs/objects/layers_only", true);
        root_watcher = new ObjectWatcher(this, document->getRoot(), nullptr, layers_only);
        layerChanged(getDesktop()->currentLayer());
    }
}

void ObjectsPanel::selectionChanged(Selection *selected)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if(!prefs->getBool("/dialogs/objects/layers_only", true)) {
        root_watcher->setSelectedBitRecursive(SELECTED_OBJECT, false);

        for (auto item : selected->items()) {
            ObjectWatcher *watcher = nullptr;
            // This both unpacks the tree, and populates lazy loading
            for (auto &parent : item->ancestorList(true)) {
                if (parent->getRepr() == root_watcher->getRepr()) {
                    watcher = root_watcher;
                } else if (watcher) {
                    if (watcher = watcher->findChild(parent->getRepr())) {
                        if (auto row = watcher->getRow()) {
                            cleanDummyChildren(*row);
                        }
                    }
                }
            }
            if (watcher) {
                if (auto final_watcher = watcher->findChild(item->getRepr())) {
                    final_watcher->setSelectedBit(SELECTED_OBJECT, true);
                    _tree.expand_to_path(final_watcher->getTreePath());
                } else {
                    g_warning("Can't find final step in tree selection!");
                }
            } else {
                g_warning("Can't find a mid step in tree selection!");
            }
        }
    }
}

/**
 * Happens when the layer selected is changed.
 *
 * @param layer - The layer now selected
 */
void ObjectsPanel::layerChanged(SPObject *layer)
{
    root_watcher->setSelectedBitRecursive(LAYER_FOCUS_CHILD | LAYER_FOCUSED, false);

    if (!layer) return;
    auto watcher = getWatcher(layer->getRepr());
    if (watcher && watcher != root_watcher) {
        watcher->setSelectedBitChildren(LAYER_FOCUS_CHILD, true);
        watcher->setSelectedBit(LAYER_FOCUSED, true);
    }
    _layer = layer;
}


/**
 * Stylizes a button using the given icon name and tooltip
 */
Gtk::Button* ObjectsPanel::_addBarButton(char const* iconName, char const* tooltip, int verb_id)
{
    Gtk::Button* btn = Gtk::manage(new Gtk::Button());
    auto child = Glib::wrap(sp_get_icon_image(iconName, GTK_ICON_SIZE_SMALL_TOOLBAR));
    child->show();
    btn->add(*child);
    btn->set_relief(Gtk::RELIEF_NONE);
    btn->set_tooltip_text(tooltip);
    btn->signal_clicked().connect(sigc::bind( sigc::mem_fun(*this, &ObjectsPanel::_takeAction), verb_id));
    return btn;
}

/**
 * Sets visibility of items in the tree
 * @param iter Current item in the tree
 */
void ObjectsPanel::toggleVisible(const Glib::ustring& path)
{
    Gtk::TreeModel::Row row = *_store->get_iter(path);
    if (SPItem* item = getItem(row))
        item->setHidden(!row[_model->_colInvisible]);
}

/**
 * Sets sensitivity of items in the tree
 * @param iter Current item in the tree
 * @param locked Whether the item should be locked
 */
void ObjectsPanel::toggleLocked(const Glib::ustring& path)
{
    Gtk::TreeModel::Row row = *_store->get_iter(path);
    if (SPItem* item = getItem(row))
        item->setLocked(!row[_model->_colLocked]);
}

/**
 * Handles keyboard events
 * @param event Keyboard event passed in from GDK
 * @return Whether the event should be eaten (om nom nom)
 */
bool ObjectsPanel::_handleKeyEvent(GdkEventKey *event)
{
    auto desktop = getDesktop();
    if (!desktop)
        return false;

    Gtk::AccelKey shortcut = Inkscape::Shortcuts::get_from_event(event);
    switch (shortcut.get_key()) {
        case GDK_KEY_Escape:
            if (desktop->canvas) {
                desktop->canvas->grab_focus();
                return true;
            }
            break;
    }

    // invoke user defined shortcuts first
    if (Inkscape::Shortcuts::getInstance().invoke_verb(event, desktop))
        return true;
    return false;
}

/**
 * Handles mouse movements
 * @param event Motion event passed in from GDK
 * @returns Whether the event should be eaten.
 */
bool ObjectsPanel::_handleMotionEvent(GdkEventMotion* motion_event)
{
    if (_is_editing) return false;

    // Unhover any existing hovered row.
    if (_hovered_row_ref) {
        if (auto row = *_store->get_iter(_hovered_row_ref.get_path()))
            row[_model->_colHover] = false;
    }
    // Allow this function to be called blind.
    if (!motion_event)
        return false;

    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* col = nullptr;
    int x, y;
    if (_tree.get_path_at_pos((int)motion_event->x, (int)motion_event->y, path, col, x, y)) {
        if (auto row = *_store->get_iter(path)) {
            row[_model->_colHover] = true;
            _hovered_row_ref = Gtk::TreeModel::RowReference(_store, path);
        }
    }

    return false;
}

/**
 * Handles mouse up events
 * @param event Mouse event from GDK
 * @return whether to eat the event (om nom nom)
 */
bool ObjectsPanel::_handleButtonEvent(GdkEventButton* event)
{
    auto selection = getSelection();
    if (!selection)
        return false;

    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn* col = nullptr;
    int x, y;
    if (_tree.get_path_at_pos((int)event->x, (int)event->y, path, col, x, y)) {
        // Only the label reacts to clicks, nothing else, only need to test horz
        Gdk::Rectangle r;
        _tree.get_cell_area(path, *_name_column, r);
        if (x < r.get_x() || x > (r.get_x() + r.get_width()))
            return false;

        // This doesn't work, it might be being eaten.
        if (event->type == GDK_2BUTTON_PRESS) {
            _tree.set_cursor(path, *col, true);
            _is_editing = true;
            return true;
        }
        _is_editing = _is_editing && event->type == GDK_BUTTON_RELEASE;
        auto row = *_store->get_iter(path);
        if (!row) return false;
        SPItem *item = getItem(row);

        if (!item) return false;
        SPGroup *group = SP_GROUP(item);

        // Load the right click menu
        if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
            ContextMenu* menu = new ContextMenu(getDesktop(), item);
            menu->show();
            menu->popup_at_pointer(nullptr);
            return true;
        }

        // Select items on button release to not confuse drag
        if (!_is_editing && event->type == GDK_BUTTON_RELEASE) {
            if (event->state & GDK_SHIFT_MASK) {
                // Select everything between this row and the already seleced item
                selection->setBetween(item);
            } else if (event->state & GDK_CONTROL_MASK) {
                selection->toggle(item);
            } else if (group && group->layerMode() == SPGroup::LAYER) {
                // Clicking on layers firstly switches to that layer.
                if(selection->includes(item)) {
                    selection->clear();
                } else if (_layer != item) {
                    selection->clear();
                    getDesktop()->setCurrentLayer(item);
                } else {
                    selection->set(item);
                }
            } else {
                selection->set(item);
            }
            return true;
        } else {
            // Remember the item for we are about to drag it!
            current_item = item;
        }
    }
    return false;
}

/**
 * Executes the given button action during the idle time
 */
void ObjectsPanel::_takeAction(int val)
{
    if (auto desktop = getDesktop()) {
        if (auto verb = Verb::get(val)) {
            SPAction *action = verb->get_action(desktop);
            if (action) {
                sp_action_perform( action, nullptr );
            }
        }
    }
}

/**
 * Handle a successful item label edit
 * @param path Tree path of the item currently being edited
 * @param new_text New label text
 */
void ObjectsPanel::_handleEdited(const Glib::ustring& path, const Glib::ustring& new_text)
{
    _is_editing = false;
    if (auto row = *_store->get_iter(path)) {
        if (auto item = getItem(row)) {
            if (!new_text.empty() && (!item->label() || new_text != item->label())) {
                item->setLabel(new_text.c_str());
                DocumentUndo::done(getDocument(), SP_VERB_NONE, _("Rename object"));
            }
        }
    }
}

/**
 * Take over the select row functionality from the TreeView, this is because
 * we have two selections (layer and object selection) and require a custom
 * method of rendering the result to the treeview.
 */
bool ObjectsPanel::select_row( Glib::RefPtr<Gtk::TreeModel> const & /*model*/, Gtk::TreeModel::Path const &path, bool /*sel*/ )
{
    return true;
}

/**
 * Get the XML node which is associated with a row. Can be NULL for dummy children.
 */
Node *ObjectsPanel::getRepr(Gtk::TreeModel::Row const &row) const
{
    return row[_model->_colNode];
}

/**
 * Get the item which is associated with a row. If getRepr(row) is not NULL,
 * then this call is expected to also not be NULL.
 */
SPItem *ObjectsPanel::getItem(Gtk::TreeModel::Row const &row) const
{
    auto const this_const = const_cast<ObjectsPanel *>(this);
    return dynamic_cast<SPItem *>(this_const->getObject(getRepr(row)));
}

/**
 * Return true if this row has dummy children.
 */
bool ObjectsPanel::hasDummyChildren(Gtk::TreeModel::Row const &row) const
{
    for (auto &c : row.children()) {
        if (isDummy(c)) {
            return true;
        }
    }
    return false;
}

/**
 * If the given row has dummy children, remove them.
 * @pre Eiter all, or no children are dummies
 * @post If the function returns true, the row has no children
 * @return False if there are children and they are not dummies
 */
bool ObjectsPanel::removeDummyChildren(Gtk::TreeModel::Row const &row)
{
    auto &children = row.children();
    if (!children.empty()) {
        Gtk::TreeStore::iterator child = children[0];
        if (!isDummy(*child)) {
            assert(!hasDummyChildren(row));
            return false;
        }

        do {
            assert(child->parent() == row);
            assert(isDummy(*child));
            child = _store->erase(child);
        } while (child && child->parent() == row);
    }
    return true;
}

bool ObjectsPanel::cleanDummyChildren(Gtk::TreeModel::Row const &row)
{
    if (removeDummyChildren(row)) {
        assert(row);
        getWatcher(getRepr(row))->addChildren(getItem(row));
        return true;
    }
    return false;
}

/**
 * Signal handler for "drag-motion"
 *
 * Refuses drops into non-group items.
 */
bool ObjectsPanel::on_drag_motion(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time)
{
    Gtk::TreeModel::Path path;
    Gtk::TreeViewDropPosition pos;

    auto selection = getSelection();
    auto document = getDocument();

    if (!selection || !document)
        goto finally;

    _tree.get_dest_row_at_pos(x, y, path, pos);

    if (path) {
        auto iter = _store->get_iter(path);
        auto repr = getRepr(*iter);
        auto obj = document->getObjectByRepr(repr);

        bool const drop_into = pos != Gtk::TREE_VIEW_DROP_BEFORE && //
                               pos != Gtk::TREE_VIEW_DROP_AFTER;

        // don't drop on self
        if (selection->includes(obj)) {
            goto finally;
        }

        auto item = getItem(*iter);

        // only groups can have children
        if (drop_into && !dynamic_cast<SPGroup const *>(item)) {
            goto finally;
        }

        context->drag_status(Gdk::ACTION_MOVE, time);
        return false;
    }

finally:
    // remove drop highlight
    _tree.unset_drag_dest_row();
    context->drag_refuse(time);
    return true;
}

/**
 * Signal handler for "drag-drop".
 *
 * Do the actual work of drag-and-drop.
 */
bool ObjectsPanel::on_drag_drop(const Glib::RefPtr<Gdk::DragContext> &context, int x, int y, guint time)
{
    Gtk::TreeModel::Path path;
    Gtk::TreeViewDropPosition pos;
    _tree.get_dest_row_at_pos(x, y, path, pos);

    if (!path) {
        return true;
    }

    auto drop_repr = getRepr(*_store->get_iter(path));
    bool const drop_into = pos != Gtk::TREE_VIEW_DROP_BEFORE && //
                           pos != Gtk::TREE_VIEW_DROP_AFTER;

    auto selection = getSelection();
    auto document = getDocument();
    if (selection && document) {
        if (drop_into) {
            selection->toLayer(document->getObjectByRepr(drop_repr));
        } else {
            Node *after = (pos == Gtk::TREE_VIEW_DROP_BEFORE) ? drop_repr : drop_repr->prev();
            selection->toLayer(document->getObjectByRepr(drop_repr->parent()), false, after);
        }
    }

    on_drag_end(context);
    return true;
}

void ObjectsPanel::on_drag_start(const Glib::RefPtr<Gdk::DragContext> &context)
{
    auto selection = _tree.get_selection();
    selection->set_mode(Gtk::SELECTION_MULTIPLE);
    selection->unselect_all();

    auto obj_selection = getSelection();
    if (!obj_selection)
        return;

    if (current_item && !obj_selection->includes(current_item)) {
        // This means the item the user started to drag is not one that is selected
        // So we'll deselect everything and start draging this item instead.
        auto watcher = getWatcher(current_item->getRepr());
        if (watcher) {
            auto path = watcher->getTreePath();
            selection->select(path);
            obj_selection->set(current_item);
        }
    } else {
        // Drag all the items currently selected (multi-row)
        for (auto item : obj_selection->items()) {
            auto watcher = getWatcher(item->getRepr());
            if (watcher) {
                auto path = watcher->getTreePath();
                selection->select(path);
            }
        }
    }
}

void ObjectsPanel::on_drag_end(const Glib::RefPtr<Gdk::DragContext> &context)
{
    auto selection = _tree.get_selection();
    selection->unselect_all();
    selection->set_mode(Gtk::SELECTION_NONE);
}

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

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
