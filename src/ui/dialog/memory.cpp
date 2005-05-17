/**
 * \brief Memory statistics dialog
 *
 * Copyright 2005 MenTaLguY <mental@rydia.net>
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "ui/dialog/memory.h"
#include "debug/heap.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

struct Memory::Private {
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<std::size_t> used;
        Gtk::TreeModelColumn<std::size_t> slack;
        Gtk::TreeModelColumn<std::size_t> total;

        ModelColumns() { add(name); add(used); add(slack); add(total); }
    };

    Private() {
        model = Gtk::ListStore::create(columns);
        view.set_model(model);
        view.append_column(_("Heap"), columns.name);
        view.append_column(_("In Use"), columns.used);
        view.append_column(_("Slack"), columns.slack);
        view.append_column(_("Total"), columns.total);
    }

    void update();

    void start_update_task();
    void stop_update_task();

    ModelColumns columns;
    Glib::RefPtr<Gtk::ListStore> model;
    Gtk::TreeView view;

    sigc::connection update_task;
};

void Memory::Private::update() {
    Debug::Heap::Stats total(0, 0);
    Gtk::ListStore::iterator row;

    row = model->children().begin();

    for ( unsigned i = 0 ; i < Debug::heap_count() ; i++ ) {
        Debug::Heap *heap=Debug::get_heap(i);
        if (heap) {
            Debug::Heap::Stats stats=heap->stats();

            if ( row == model->children().end() ) {
                row = model->append();
            }

            row->set_value(columns.name, Glib::ustring(heap->name()));
            row->set_value(columns.used, stats.size - stats.bytes_free);
            row->set_value(columns.slack, stats.bytes_free);
            row->set_value(columns.total, stats.size);

            total.size += stats.size;
            total.bytes_free += stats.bytes_free;

            ++row;
        }
    }

    if ( row == model->children().end() ) {
        row = model->append();
    }

    row->set_value(columns.name, Glib::ustring(_("Combined")));
    row->set_value(columns.used, total.size - total.bytes_free);
    row->set_value(columns.slack, total.bytes_free);
    row->set_value(columns.total, total.size);

    ++row;

    while ( row != model->children().end() ) {
        row = model->erase(row);
    }
}

void Memory::Private::start_update_task() {
    update_task.disconnect();
    update_task = Glib::signal_timeout().connect(
        sigc::bind_return(sigc::mem_fun(*this, &Private::update), true),
        500
    );
}

void Memory::Private::stop_update_task() {
    update_task.disconnect();
}

Memory::Memory() : _private(*(new Memory::Private())) {
    set_title(_("Memory Info"));
    set_default_size(200, 200);

    get_vbox()->add(_private.view);

    _private.update();

    transientize();

    show_all_children();

    signal_show().connect(sigc::mem_fun(_private, &Private::start_update_task));
    signal_hide().connect(sigc::mem_fun(_private, &Private::stop_update_task));
}

Memory::~Memory() {
    delete &_private;
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
