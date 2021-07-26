// SPDX-License-Identifier: GPL-2.0-or-later
#include "builder-utils.h"
#include "io/resource.h"

namespace Inkscape {
namespace UI {

Glib::RefPtr<Gtk::Builder> create_builder(const char* filename) {
    auto glade = Inkscape::IO::Resource::get_filename(Inkscape::IO::Resource::UIS, filename);
    Glib::RefPtr<Gtk::Builder> builder;
    try {
        return Gtk::Builder::create_from_file(glade);
    }
    catch (Glib::Error& ex) {
        g_error("Cannot load glade file: %s", ex.what().c_str());
        throw;
    }
}

}} // namespace
