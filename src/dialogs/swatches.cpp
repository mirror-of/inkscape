/*
 * A simple panel for color swatches
 *
 * Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <vector>

#include "swatches.h"

#include <gtk/gtk.h>

#include <dialogs/dialog-events.h>
#include <gtk/gtkdialog.h> //for GTK_RESPONSE* types
#include <glibmm/i18n.h>
#include "interface.h"
#include "verbs.h"
#include "prefs-utils.h"
#include "inkscape.h"
#include "macros.h"
#include "document.h"
#include "desktop-handles.h"
#include "selection.h"
#include "display/nr-arena.h"
#include <glib.h>
#include <gtkmm/table.h>
#include "extension/db.h"
#include "desktop.h"
#include "inkscape.h"
#include "svg/svg.h"
#include "desktop-style.h"
#include "ui/previewable.h"
#include "io/sys.h"
#include "path-prefix.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {

SwatchesPanel* SwatchesPanel::instance = 0;


class ColorItem : public Inkscape::UI::Previewable
{
public:
    ColorItem( unsigned int r, unsigned int g, unsigned int b, Glib::ustring& name );
    virtual ~ColorItem();
    ColorItem(ColorItem const &other);
    virtual ColorItem &operator=(ColorItem const &other);

    virtual Gtk::Widget* getPreview(PreviewStyle style, Gtk::BuiltinIconSize size);

    void buttonClicked();

    unsigned int _r;
    unsigned int _g;
    unsigned int _b;
    Glib::ustring _name;
};

ColorItem::ColorItem( unsigned int r, unsigned int g, unsigned int b, Glib::ustring& name ) :
    _r(r),
    _g(g),
    _b(b),
    _name(name)
{
}

ColorItem::~ColorItem()
{
}

ColorItem::ColorItem(ColorItem const &other) :
    Inkscape::UI::Previewable()
{
    if ( this != &other ) {
        *this = other;
    }
}

ColorItem &ColorItem::operator=(ColorItem const &other)
{
    if ( this != &other ) {
        _r = other._r;
        _g = other._g;
        _b = other._b;
        _name = other._name;
    }
    return *this;
}


Gtk::Widget* ColorItem::getPreview(PreviewStyle style, Gtk::BuiltinIconSize size)
{
    Gtk::Widget* widget = 0;
    if ( style == PREVIEW_STYLE_BLURB ) {
        Gtk::Label *lbl = new Gtk::Label(_name);
        lbl->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);
        widget = lbl;
    } else {
        Glib::ustring blank("          ");
        if ( size == Gtk::ICON_SIZE_MENU ) {
            blank = " ";
        }

        Gtk::Button *btn = new Gtk::Button(blank);
        Gdk::Color color;
        color.set_rgb(_r << 8, _g << 8, _b << 8);
        btn->modify_bg(Gtk::STATE_NORMAL, color);
        btn->modify_bg(Gtk::STATE_ACTIVE, color);
        btn->modify_bg(Gtk::STATE_PRELIGHT, color);
        btn->modify_bg(Gtk::STATE_SELECTED, color);
        btn->signal_clicked().connect( sigc::mem_fun(*this, &ColorItem::buttonClicked) );
        widget = btn;
    }

    return widget;
}

void ColorItem::buttonClicked()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop) {
        guint32 rgba = (_r << 24) | (_g << 16) | (_b << 8) | 0xff;
        //g_object_set_data(G_OBJECT(cp), "color", GUINT_TO_POINTER(rgba));
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(desktop->namedview);

        gchar c[64];
        sp_svg_write_color(c, 64, rgba);
        if (repr)
            sp_repr_set_attr(repr, "fill", c);


        SPCSSAttr *css = sp_repr_css_attr_new();
        sp_repr_css_set_property( css, "fill", c );
        sp_desktop_set_style(desktop, css);

        sp_repr_css_attr_unref(css);
    }
}




static char* trim( char* str ) {
    char* ret = str;
    while ( *str && (*str == ' ' || *str == '\t') ) {
        str++;
    }
    ret = str;
    while ( *str ) {
        str++;
    }
    str--;
    while ( str > ret && ( *str == ' ' || *str == '\t' ) || *str == '\r' || *str == '\n' ) {
        *str-- = 0;
    }
    return ret;
}

void skipWhitespace( char*& str ) {
    while ( *str && (*str == ' ' || *str == '\t' ) ) {
        str++;
    }
}

bool parseNum( char*& str, int& val ) {
    val = 0;
    while ( '0' <= *str && *str <= '9' ) {
        val = val * 10 + (*str - '0');
        str++;
    }
    bool retval = !(*str == 0 || *str == ' ' || *str == '\t' || *str == '\r' || *str == '\n');
    return retval;
}


class JustForNow
{
public:
    Glib::ustring _name;
    std::vector<ColorItem*> _colors;
};

static std::vector<JustForNow*> possible;

static void loadPaletteFile( gchar const *filename )
{
    char block[1024];
    FILE *f = Inkscape::IO::fopen_utf8name( filename, "r" );
    if ( f ) {
        char* result = fgets( block, sizeof(block), f );
        if ( result ) {
            if ( strncmp( "GIMP Palette", block, 12 ) == 0 ) {
                bool inHeader = true;
                bool hasErr = false;

                JustForNow *onceMore = new JustForNow();

                do {
                    result = fgets( block, sizeof(block), f );
                    block[sizeof(block) - 1] = 0;
                    if ( result ) {
                        if ( block[0] == '#' ) {
                            // ignore comment
                        } else {
                            char *ptr = block;
                            // very simple check for header versus entry
                            while ( *ptr && (*ptr == ' ' || *ptr == '\t') ) {
                                ptr++;
                            }
                            if ( *ptr == 0 ) {
                                // blank line. skip it.
                            } else if ( '0' <= *ptr && *ptr <= '9' ) {
                                // should be an entry link
                                inHeader = false;
                                ptr = block;
                                char* name = 0;
                                int r = 0;
                                int g = 0;
                                int b = 0;
                                skipWhitespace(ptr);
                                if ( ptr ) {
                                    hasErr = parseNum(ptr, r);
                                    if ( !hasErr ) {
                                        skipWhitespace(ptr);
                                        hasErr = parseNum(ptr, g);
                                    }
                                    if ( !hasErr ) {
                                        skipWhitespace(ptr);
                                        hasErr = parseNum(ptr, b);
                                    }
                                    if ( !hasErr && *ptr ) {
                                        name = trim(ptr);
                                    }
                                    if ( !hasErr ) {
                                        // Add the entry now
                                        Glib::ustring nameStr(name);
                                        ColorItem* item = new ColorItem( r, g, b, nameStr );
                                        onceMore->_colors.push_back(item);
                                    }
                                } else {
                                    hasErr = true;
                                }
                            } else {
                                if ( !inHeader ) {
                                    // Hmmm... probably bad. Not quite the format we want?
                                    hasErr = true;
                                } else {
                                    char* sep = strchr(result, ':');
                                    if ( sep ) {
                                        *sep = 0;
                                        char* val = trim(sep + 1);
                                        char* name = trim(result);
                                        if ( *name ) {
                                            if ( strcmp( "Name", name ) == 0 ) {
                                                onceMore->_name = val;
                                            }
                                        } else {
                                            // error
                                            hasErr = true;
                                        }
                                    } else {
                                        // error
                                        hasErr = true;
                                    }
                                }
                            }
                        }
                    }
                } while ( result && !hasErr );
                if ( !hasErr ) {
                    possible.push_back(onceMore);
                } else {
                    delete onceMore;
                }
            }
        }

        fclose(f);
    }
}

static void loadEmUp()
{
    static bool beenHere = false;
    if ( !beenHere ) {
        beenHere = true;

        std::list<gchar *> sources;
        sources.push_back( profile_path("palettes") );
        sources.push_back( g_strdup(INKSCAPE_PALETTESDIR) );

        // Use this loop to iterate through a list of possible document locations.
        while (!sources.empty()) {
            gchar *dirname = sources.front();

            if ( Inkscape::IO::file_test( dirname, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR) ) ) {
                GError *err = 0;
                GDir *directory = g_dir_open(dirname, 0, &err);
                if (!directory) {
                    gchar *safeDir = Inkscape::IO::sanitizeString(dirname);
                    g_warning(_("Palettes directory (%s) is unavailable."), safeDir);
                    g_free(safeDir);
                } else {
                    gchar *filename = 0;
                    while ((filename = (gchar *)g_dir_read_name(directory)) != NULL) {
                        gchar* full = g_build_filename(dirname, filename, NULL);
                        if ( !Inkscape::IO::file_test( full, (GFileTest)(G_FILE_TEST_IS_DIR ) ) ) {
                            loadPaletteFile(full);
                        }
                        g_free(full);
                    }
                    g_dir_close(directory);
                }
            }

            // toss the dirname
            g_free(dirname);
            sources.pop_front();
        }
    }
}









SwatchesPanel& SwatchesPanel::getInstance()
{
    if ( !instance ) {
        instance = new SwatchesPanel();
    }

    return *instance;
}



/**
 * Constructor
 */
SwatchesPanel::SwatchesPanel() :
    _holder(0)
{
    _holder = new PreviewHolder();
    loadEmUp();

    if ( !possible.empty() ) {
        JustForNow* first = possible.front();
        for ( std::vector<ColorItem*>::iterator it = first->_colors.begin(); it != first->_colors.end(); it++ ) {
            _holder->addPreview(*it);
        }
    }

    pack_start(*_holder, Gtk::PACK_EXPAND_WIDGET);
    _setTargetFillable(_holder);

    show_all_children();
}

SwatchesPanel::~SwatchesPanel()
{
}

void SwatchesPanel::_handleAction( int setId, int itemId )
{
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
