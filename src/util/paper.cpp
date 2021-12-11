// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Inkscape PaperSize
 *
 * Authors:
 *      Bob Jamison (2006)
 *      Martin Owens (2021)
 *
 * Copyright (C) 2021 AUTHORS
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <glibmm/i18n.h>

#include "paper.h"
#include "pages-skeleton.h"

#include "io/resource.h"

namespace Inkscape {

/**
 * Returns a list of page sizes.
 */ 
std::vector<PaperSize *> PaperSize::getPageSizes()
{   
    // Static makes us only load pages once.
    static std::vector<PaperSize *> ret;
    if (!ret.empty())
        return ret;
    
    char *path = Inkscape::IO::Resource::profile_path("pages.csv");
    if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
        if (!g_file_set_contents(path, pages_skeleton, -1, nullptr)) {
            g_warning("%s", _("Failed to create the page file."));
        }
    }   
    gchar *content = nullptr;
    if (g_file_get_contents(path, &content, nullptr, nullptr)) {
        gchar **lines = g_strsplit_set(content, "\n", 0); 
    
        for (int i = 0; lines && lines[i]; ++i) {
            gchar **line = g_strsplit_set(lines[i], ",", 5);
            if (!line[0] || !line[1] || !line[2] || !line[3] || line[0][0]=='#') 
                continue;
    
            //name, width, height, unit
            double width = g_ascii_strtod(line[1], nullptr);
            double height = g_ascii_strtod(line[2], nullptr);
            g_strstrip(line[0]);
            g_strstrip(line[3]);
            Glib::ustring name = line[0];
            ret.push_back(new PaperSize(name, width, height, Inkscape::Util::unit_table.getUnit(line[3])));
        }
        g_strfreev(lines); 
        g_free(content);
    }   
    g_free(path);
    return ret;
}   


PaperSize::PaperSize()
    : name("")
    , smaller(0.0)
    , larger(0.0)
{
    unit = Inkscape::Util::unit_table.getUnit("px");
}


PaperSize::PaperSize(std::string name, double smaller, double larger, Inkscape::Util::Unit const *unit)
    : name(std::move(name))
    , smaller(smaller)
    , larger(larger)
    , unit(unit) 
{}

std::string PaperSize::getDescription() const { return toDescription(name, smaller, larger, unit); }
std::string PaperSize::toDescription(std::string name, double x, double y, Inkscape::Util::Unit const *unit)
{
    char buf[80];
    snprintf(buf, 79, "%s (%0.1fx%0.1f %s)", name.c_str(), x, y, unit->abbr.c_str());
    return std::string(buf);
}

void PaperSize::assign(const PaperSize &other)
{
    name    = other.name;
    smaller = other.smaller;
    larger  = other.larger;
    unit    = other.unit;
}

/**
 * Returns a matching paper size, if possible.
 */
PaperSize *PaperSize::findPaperSize(double width, double height, Inkscape::Util::Unit const *unit)
{
    double smaller = width;
    double larger = height;
    if (width > height) {
        smaller = height;
        larger = width;
    }
    for (auto &page_size : Inkscape::PaperSize::getPageSizes()) {
        if (page_size->smaller == smaller && page_size->larger == larger && page_size->unit == unit) {
            return page_size;
        }
    }
    return nullptr;
}

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
