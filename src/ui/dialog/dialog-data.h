// SPDX-License-Identifier: GPL-2.0-or-later

/** @file
 * @brief Basic dialog info.
 *
 * Authors: see git history
 *   Tavmjong Bah
 *
 * Copyright (c) 2021 Tavmjong Bah, Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

/*
 * In an ideal world, this information would be in .ui files for each
 * dialog (the .ui file would describe a dialog wrapped by a notebook
 * tab). At the moment we create each dialog notebook tab on the fly
 * so we need a place to keep this information.
 */

#include <map>
#include <glibmm/i18n.h>
#include "ui/icon-names.h"  // INKSCAPE_ICON macro

class DialogData {
public:
    Glib::ustring label;
    Glib::ustring icon_name;
};

// short-term fix for missing dialog titles; map<ustring, T> exhibits a bug where "SVGFonts" entry cannot be found
// static std::map<Glib::ustring, DialogData> dialog_data =
static std::map<std::string, DialogData> dialog_data =
{
    // clang-format off
    {"AlignDistribute",    {N_("_Align and Distribute..."), INKSCAPE_ICON("dialog-align-and-distribute")}},
    // {"Arrange",            {N_("_Arrange..."),              INKSCAPE_ICON("dialog-rows-and-columns")    }},
    {"AttrDialog",         {N_("_Object attributes..."),    INKSCAPE_ICON("dialog-object-properties")   }},
    {"Clonetiler",         {N_("Create Tiled Clones..."),   INKSCAPE_ICON("dialog-tile-clones")         }},
    {"DocumentProperties", {N_("_Document Properties..."),  INKSCAPE_ICON("document-properties")        }},
    {"Export",             {N_("_Export PNG Image..."),     INKSCAPE_ICON("document-export")            }},
    {"FillStroke",         {N_("_Fill and Stroke..."),      INKSCAPE_ICON("dialog-fill-and-stroke")     }},
    {"FilterEffects",      {N_("Filter _Editor..."),        INKSCAPE_ICON("dialog-filters")             }},
    {"Find",               {N_("_Find/Replace..."),         INKSCAPE_ICON("edit-find")                  }},
    {"Glyphs",             {N_("_Unicode Characters..."),   INKSCAPE_ICON("accessories-character-map")  }},
    {"IconPreview",        {N_("Icon Preview"),             INKSCAPE_ICON("dialog-icon-preview")        }},
    {"Input",              {N_("_Input Devices..."),        INKSCAPE_ICON("dialog-input-devices")       }},
    {"Layers",             {N_("Layer_s..."),               INKSCAPE_ICON("dialog-layers")              }},
    {"LivePathEffect",     {N_("Path E_ffects..."),         INKSCAPE_ICON("dialog-path-effects")        }},
    {"Memory",             {N_("About _Memory..."),         INKSCAPE_ICON("dialog-memory")              }},
    {"Messages",           {N_("_Messages..."),             INKSCAPE_ICON("dialog-messages")            }},
    {"ObjectAttributes",   {N_("_Object attributes..."),    INKSCAPE_ICON("dialog-object-properties")   }},
    {"ObjectProperties",   {N_("_Object Properties..."),    INKSCAPE_ICON("dialog-object-properties")   }},
    {"Objects",            {N_("Layers and Object_s..."),   INKSCAPE_ICON("dialog-objects")             }},
    {"PaintServers",       {N_("_Paint Servers..."),        INKSCAPE_ICON("symbols")                    }},
    {"Preferences",        {N_("P_references"),             INKSCAPE_ICON("preferences-system")         }},
    {"Selectors",          {N_("_Selectors and CSS..."),    INKSCAPE_ICON("dialog-selectors")           }},
    {"Style",              {N_("Style Dialog..."),          ""                                          }},
    {"SVGFonts",           {N_("SVG Font Editor..."),       INKSCAPE_ICON("dialog-svg-font")            }},
    {"Swatches",           {N_("S_watches..."),             INKSCAPE_ICON("swatches")                   }},
    {"Symbols",            {N_("S_ymbols..."),              INKSCAPE_ICON("symbols")                    }},
    {"Text",               {N_("_Text and Font..."),        INKSCAPE_ICON("dialog-text-and-font")       }},
    {"Trace",              {N_("_Trace Bitmap..."),         INKSCAPE_ICON("bitmap-trace")               }},
    {"Transform",          {N_("Transfor_m..."),            INKSCAPE_ICON("dialog-transform")           }},
    {"UndoHistory",        {N_("Undo _History..."),         INKSCAPE_ICON("edit-undo-history")          }},
    {"XMLEditor",          {N_("_XML Editor..."),           INKSCAPE_ICON("dialog-xml-editor")          }},
#if WITH_GSPELL
    {"Spellcheck",         {N_("Check Spellin_g..."),       INKSCAPE_ICON("tools-check-spelling")       }},
#endif
#if DEBUG
    {"Prototype",          {N_("Prototype..."),             INKSCAPE_ICON("document-properties")        }},
#endif
    // clang-format on
};

