/*
 * Inkscape::Widgets::DocumentTreeModel - Gtk::TreeModel of a document
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_WIDGETS_DOCUMENT_TREE_MODEL
#define SEEN_INKSCAPE_WIDGETS_DOCUMENT_TREE_MODEL

#include <gtkmm/treemodel.h>
#include <gtkmm/treeiter.h>
#include <gtkmm/treepath.h>

class SPDesktop;
class SPDocument;
class SPRepr;

namespace Inkscape {
namespace Widgets {

class DocumentTreeModel_Class;

class DocumentTreeModel : public Glib::Object, public Gtk::TreeModel {
public:
    struct ModelColumns : public Gtk::TreeModelColumnRecord {
        Gtk::TreeModelColumn<Glib::ustring> id;

        ModelColumns() { add(id); }
    };

    const ModelColumns columns;

    static Glib::RefPtr<DocumentTreeModel> create(SPDocument *document);

    ~DocumentTreeModel();

protected:
    DocumentTreeModel(SPDocument *document);

    void set_value_impl(Gtk::TreeIter const &row, int column, Glib::ValueBase const &value);
    void get_value_impl(Gtk::TreeIter const &row, int column, Glib::ValueBase &value) const;

    Gtk::TreeModelFlags get_flags_vfunc() const;

    void ref_node_vfunc(Gtk::TreeIter const &iter) const;
    void unref_node_vfunc(Gtk::TreeIter const &iter) const;

    bool iter_next_vfunc(Gtk::TreeIter const &iter, Gtk::TreeIter &next) const;
    bool iter_parent_vfunc(Gtk::TreeIter const &child, Gtk::TreeIter &iter) const;

    bool iter_has_child_vfunc(Gtk::TreeIter const &iter) const;
    bool iter_children_vfunc(Gtk::TreeIter const &parent, Gtk::TreeIter &iter) const;
    int iter_n_children_vfunc(Gtk::TreeIter const &iter) const;
    bool iter_nth_child_vfunc(Gtk::TreeIter const &parent, int n, Gtk::TreeIter &iter) const;

    int iter_n_root_children_vfunc(void) const;
    bool iter_nth_root_child_vfunc(int n, Gtk::TreeIter &iter) const;

    Gtk::TreePath get_path_vfunc(Gtk::TreeIter const &iter) const;
    bool get_iter_vfunc(Gtk::TreePath const &path, Gtk::TreeIter &iter) const;

    void get_value_vfunc(Gtk::TreeIter const &iter, int column, Glib::ValueBase &value) const;
    void set_value_impl(Gtk::TreeIter const &iter, int column, Glib::ValueBase const &value) const;
    // TODO: void set_value_vfunc(Gtk::TreeIter const &iter, int column, Glib::ValueBase const &value) const;

private:
    friend class DocumentTreeModel_Class;

    SPDocument *_document;
    int _stamp;

    static DocumentTreeModel_Class _document_tree_model_class;

    Gtk::TreeIter _iter_from_repr(SPRepr *repr);
    Gtk::TreePath _path_from_context(SPRepr *parent, SPRepr *ref);

    void _node_moved(SPRepr *repr, SPRepr *old_parent, SPRepr *old_ref, SPRepr *new_parent, SPRepr *new_ref);
    void _attr_changed(SPRepr *repr, gchar const *name, gchar const *old_value, gchar const *new_value);
};

class DocumentTreeModel_Class : public Glib::Class {
public:
    struct DocumentTreeModelClass { GObjectClass parent_class; };

    friend class DocumentTreeModel;

    Glib::Class &init();

    static void class_init_function(void *g_class, void *class_data);
};

}
}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
