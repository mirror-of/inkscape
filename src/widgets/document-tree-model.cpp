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

#include "widgets/document-tree-model.h"
#include "document.h"
#include "xml/repr.h"
#include "xml/repr-private.h"

namespace Inkscape {
namespace Widgets {

Glib::RefPtr<DocumentTreeModel> DocumentTreeModel::create(SPDocument *document)
{
    return Glib::RefPtr<DocumentTreeModel>(new DocumentTreeModel(document));
}

DocumentTreeModel::DocumentTreeModel(SPDocument *document)
: Glib::ObjectBase("Inkscape_Widgets_DocumentTreeModel"),
  Glib::Object(Glib::ConstructParams(_document_tree_model_class.init(), NULL)),
  TreeModel(), _document(document)
{
    static int next_stamp=1;

    g_object_ref(_document);

    _stamp = next_stamp++;

    SPReprDoc *rdoc = sp_document_repr_doc(_document);

    rdoc->connectNodeMoved(sigc::mem_fun(this, &DocumentTreeModel::_node_moved));
    rdoc->connectAttrChanged(sigc::mem_fun(this, &DocumentTreeModel::_attr_changed));
}

DocumentTreeModel::~DocumentTreeModel() {
    g_object_unref(_document);
}

Gtk::TreeModelFlags DocumentTreeModel::get_flags_vfunc() const {
    return Gtk::TREE_MODEL_ITERS_PERSIST;
}

void DocumentTreeModel::ref_node_vfunc(Gtk::TreeIter const &iter) const {
    sp_repr_ref(reinterpret_cast<SPRepr *>(iter.gobj()->user_data));
}

void DocumentTreeModel::unref_node_vfunc(Gtk::TreeIter const &iter) const {
    sp_repr_unref(reinterpret_cast<SPRepr *>(iter.gobj()->user_data));
}

bool DocumentTreeModel::iter_next_vfunc(Gtk::TreeIter const &iter,
                                        Gtk::TreeIter &next)
                                        const
{
    SPRepr *repr=reinterpret_cast<SPRepr *>(iter.gobj()->user_data);
    repr = ( repr ? sp_repr_next(repr) : NULL );
    if (repr) {
        next.gobj()->user_data = repr;
        next.set_stamp(_stamp);
        return true;
    } else {
        return false;
    }
}

bool DocumentTreeModel::iter_parent_vfunc(Gtk::TreeIter const &child,
                                          Gtk::TreeIter &parent)
                                          const
{
    SPRepr *repr=reinterpret_cast<SPRepr *>(child.gobj()->user_data);
    repr = ( repr ? sp_repr_parent(repr) : NULL );
    if ( repr && SP_REPR_TYPE(repr) != SP_XML_DOCUMENT_NODE ) {
        parent.gobj()->user_data = repr;
        parent.set_stamp(_stamp);
        return true;
    } else {
        return false;
    }
}

bool DocumentTreeModel::iter_has_child_vfunc(Gtk::TreeIter const &iter) const {
    SPRepr *repr=reinterpret_cast<SPRepr *>(iter.gobj()->user_data);
    return repr && sp_repr_children(repr);
}

bool DocumentTreeModel::iter_children_vfunc(Gtk::TreeIter const &parent,
                                            Gtk::TreeIter &iter)
                                            const
{
    SPRepr *repr=reinterpret_cast<SPRepr *>(parent.gobj()->user_data);
    repr = ( repr ? sp_repr_children(repr) : NULL );
    if (repr) {
        iter.gobj()->user_data = repr;
        iter.set_stamp(_stamp);
        return true;
    } else {
        return false;
    }
}

int DocumentTreeModel::iter_n_children_vfunc(Gtk::TreeIter const &iter) const {
    SPRepr *repr=reinterpret_cast<SPRepr *>(iter.gobj()->user_data);
    if (repr) {
        return sp_repr_n_children(repr);
    } else {
        return 0;
    }
}

bool DocumentTreeModel::iter_nth_child_vfunc(Gtk::TreeIter const &parent,
                                             int n, Gtk::TreeIter &iter)
                                             const
{
    SPRepr *repr=reinterpret_cast<SPRepr *>(parent.gobj()->user_data);
    repr = ( repr ? sp_repr_nth_child(repr, n) : NULL );
    if (repr) {
        iter.gobj()->user_data = repr;
        iter.set_stamp(_stamp);
        return true;
    } else {
        return false;
    }
}

int DocumentTreeModel::iter_n_root_children_vfunc(void) const {
    return 1;
}

bool DocumentTreeModel::iter_nth_root_child_vfunc(int n,
                                                  Gtk::TreeIter &iter)
                                                  const
{
    if ( n == 0 ) {
        iter.gobj()->user_data = sp_document_repr_root(_document);
        iter.set_stamp(_stamp);
        return true;
    } else {
        return false;
    }
}

Gtk::TreePath DocumentTreeModel::get_path_vfunc(Gtk::TreeIter const &iter)
                                                const
{
    Gtk::TreePath path;

    // TODO: this process is intensive and needs a cache in front of it

    SPRepr *repr=reinterpret_cast<SPRepr *>(iter.gobj()->user_data);
    for ( ; repr && SP_REPR_TYPE(repr) != SP_XML_DOCUMENT_NODE
          ; repr = sp_repr_parent(repr) )
    {
        path.push_front(sp_repr_pos_of(repr));
    }

    if (!path.empty()) {
        path[0] = 0;
    }

    return path;
}

bool DocumentTreeModel::get_iter_vfunc(Gtk::TreePath const &path,
                                       Gtk::TreeIter &iter)
                                       const
{
    Gtk::TreePath::const_iterator p_iter=path.begin();
    SPRepr *repr=NULL;

    if ( *p_iter == 0 ) {
        ++p_iter;
        repr = sp_document_repr_root(_document);

        for ( ; repr && p_iter != path.end() ; ++p_iter ) {
            repr = sp_repr_nth_child(repr, *p_iter);
        }
    }

    if (repr) {
        iter.gobj()->user_data = repr;
        iter.set_stamp(_stamp);
        return true;
    } else {
        return false;
    }
}

void DocumentTreeModel::get_value_vfunc(Gtk::TreeIter const &iter,
                                        int column,
                                        Glib::ValueBase &value)
                                        const
{
    SPRepr *repr=reinterpret_cast<SPRepr *>(iter.gobj()->user_data);
    switch (column) {
        case 0: {
            Glib::Value<Glib::ustring> new_value;
            new_value.set(sp_repr_attr(repr, "id"));
            value = new_value;
        } break;
        default: {
            g_assert_not_reached();
            return;
        }
    }
}

void DocumentTreeModel::set_value_impl(Gtk::TreeIter const &iter,
                                       int column,
                                       Glib::ValueBase const &value)
{
    // TODO: call set_value_vfunc, and do the row changed dance
}

Gtk::TreeIter DocumentTreeModel::_iter_from_repr(SPRepr *repr) {
    Gtk::TreeIter iter(this);
    iter.gobj()->user_data = repr;
    iter.set_stamp(_stamp);
    return iter;
}

Gtk::TreePath DocumentTreeModel::_path_from_context(SPRepr *parent, SPRepr *ref){
    if (ref) {
        Gtk::TreePath path(_iter_from_repr(ref));
        ++(path.back());
        return path;
    } else {
        Gtk::TreePath path(_iter_from_repr(parent));
        path.push_back(0);
        return path;
    }
}

void DocumentTreeModel::_node_moved(SPRepr *node,
                                    SPRepr *old_parent, SPRepr *old_ref,
                                    SPRepr *new_parent, SPRepr *new_ref)
{
    if (old_parent) {
        if ( old_parent == new_parent ) {
            Gtk::TreeIter iter(_iter_from_repr(old_parent));

            std::vector<int> order_map;
            order_map.reserve(8);

            SPRepr *riter=sp_repr_children(old_parent);
            int old_pos=0;
            int new_pos=0;

            for ( int i=0 ; riter ; riter = sp_repr_next(riter), i++ ) {
                if ( riter == old_ref ) {
                    old_pos = i+1;
                }
                if ( riter == new_ref ) {
                    new_pos = i+1;
                }
                order_map[i] = i;
            }

            if ( old_pos > new_pos ) {
                old_pos--;
            }

            order_map.erase(order_map.begin() + old_pos);
            order_map.insert(order_map.begin() + new_pos, old_pos);

            rows_reordered(Gtk::TreePath(iter), iter, &order_map.front());
        } else if (new_parent) {
            g_assert_not_reached();
        } else {
            row_deleted(_path_from_context(old_parent, old_ref));
            if (!sp_repr_children(old_parent)) {
                Gtk::TreeIter iter(_iter_from_repr(old_parent));
                row_has_child_toggled(Gtk::TreePath(iter), iter);
            }
        }
    } else if (new_parent) {
        Gtk::TreeIter iter(_iter_from_repr(node));
        row_inserted(Gtk::TreePath(iter), iter);
        if ( !sp_repr_next(node) && sp_repr_children(new_parent) == node ) {
            iter = _iter_from_repr(new_parent);
            row_has_child_toggled(Gtk::TreePath(iter), iter);
        }
    } else {
        g_assert_not_reached();
    }
}

void DocumentTreeModel::_attr_changed(SPRepr *node,
                                      gchar const *name,
                                      gchar const *old_value,
                                      gchar const *new_value)
{
    if (!strcmp(name, "id")) {
        Gtk::TreeIter iter(_iter_from_repr(node));
        row_changed(Gtk::TreePath(iter), iter);
    }
}

Glib::Class &DocumentTreeModel_Class::init() {
    if (!gtype_) {
        class_init_func_ = &DocumentTreeModel_Class::class_init_function;

        GTypeInfo const derived_info = {
            sizeof(GObjectClass),
            NULL,
            NULL,
            class_init_func_,
            NULL,
            NULL,
            sizeof(GObject),
            0,
            0,
            NULL
        };

        gtype_ = g_type_register_static(G_TYPE_OBJECT, "Inkscape_Widgets_DocumentTreeModel_Class", &derived_info, GTypeFlags(0));

        Gtk::TreeModel::add_interface(get_type());
    }

    return *this;
}

void DocumentTreeModel_Class::class_init_function(void *g_class,
                                                  void *class_data)
{
    // (noop)
}

}
}

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
