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

enum class ScrollProvider {
    PROVIDE = 0,
    NOPROVIDE
};

class DialogData {
public:
    Glib::ustring label;
    Glib::ustring icon_name;
    enum Category { Basic, Advanced, Settings, Diagnostics, Other };
    Category category;
    ScrollProvider provide_scroll;
};

// dialog categories (used to group them in a dialog submenu)
static std::map<DialogData::Category, Glib::ustring> dialog_categories = {
    { DialogData::Basic,       _("Basic") },
    { DialogData::Advanced,    _("Advanced") },
    { DialogData::Settings,    _("Settings") },
    { DialogData::Diagnostics, _("Diagnostic") },
    { DialogData::Other,       _("Other") },
};

// Note the "AttrDialog" is now part of the "XMLDialog" and the "Style" dialog is part of the "Selectors" dialog.
// Also note that the "AttrDialog" does not correspond to SP_VERB_DIALOG_ATTR!!!!! (That would be the "ObjectAttributes" dialog.)

// short-term fix for missing dialog titles; map<ustring, T> exhibits a bug where "SVGFonts" entry cannot be found
// static std::map<Glib::ustring, DialogData> dialog_data =
static std::map<std::string, DialogData> dialog_data =
{
    // clang-format off
    {"AlignDistribute",    {_("_Align and Distribute"), INKSCAPE_ICON("dialog-align-and-distribute"), DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"CloneTiler",         {_("Create Tiled Clones"),   INKSCAPE_ICON("dialog-tile-clones"),          DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"DocumentProperties", {_("_Document Properties"),  INKSCAPE_ICON("document-properties"),         DialogData::Settings,       ScrollProvider::NOPROVIDE }},
    {"Export",             {_("_Export"),               INKSCAPE_ICON("document-export"),             DialogData::Basic,          ScrollProvider::PROVIDE   }},
    {"FillStroke",         {_("_Fill and Stroke"),      INKSCAPE_ICON("dialog-fill-and-stroke"),      DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"FilterEffects",      {_("Filter _Editor"),        INKSCAPE_ICON("dialog-filters"),              DialogData::Advanced,       ScrollProvider::NOPROVIDE }},
    {"Find",               {_("_Find/Replace"),         INKSCAPE_ICON("edit-find"),                   DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"Glyphs",             {_("_Unicode Characters"),   INKSCAPE_ICON("accessories-character-map"),   DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"IconPreview",        {_("Icon Preview"),          INKSCAPE_ICON("dialog-icon-preview"),         DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"Input",              {_("_Input Devices"),        INKSCAPE_ICON("dialog-input-devices"),        DialogData::Settings,       ScrollProvider::NOPROVIDE }},
    {"LivePathEffect",     {_("Path E_ffects"),         INKSCAPE_ICON("dialog-path-effects"),         DialogData::Advanced,       ScrollProvider::NOPROVIDE }},
    {"Memory",             {_("About _Memory"),         INKSCAPE_ICON("dialog-memory"),               DialogData::Diagnostics,    ScrollProvider::NOPROVIDE }},
    {"Messages",           {_("_Messages"),             INKSCAPE_ICON("dialog-messages"),             DialogData::Diagnostics,    ScrollProvider::NOPROVIDE }},
    {"ObjectAttributes",   {_("_Object attributes"),    INKSCAPE_ICON("dialog-object-properties"),    DialogData::Settings,       ScrollProvider::NOPROVIDE }},
    {"ObjectProperties",   {_("_Object Properties"),    INKSCAPE_ICON("dialog-object-properties"),    DialogData::Settings,       ScrollProvider::NOPROVIDE }},
    {"Objects",            {_("Layers and Object_s"),   INKSCAPE_ICON("dialog-objects"),              DialogData::Basic,          ScrollProvider::PROVIDE   }},
    {"PaintServers",       {_("_Paint Servers"),        INKSCAPE_ICON("symbols"),                     DialogData::Advanced,       ScrollProvider::PROVIDE   }},
    {"Preferences",        {_("P_references"),          INKSCAPE_ICON("preferences-system"),          DialogData::Settings,       ScrollProvider::PROVIDE   }},
    {"Selectors",          {_("_Selectors and CSS"),    INKSCAPE_ICON("dialog-selectors"),            DialogData::Advanced,       ScrollProvider::PROVIDE   }},
    {"SVGFonts",           {_("SVG Font Editor"),       INKSCAPE_ICON("dialog-svg-font"),             DialogData::Advanced,       ScrollProvider::NOPROVIDE }},
    {"Swatches",           {_("S_watches"),             INKSCAPE_ICON("swatches"),                    DialogData::Basic,          ScrollProvider::PROVIDE   }},
    {"Symbols",            {_("S_ymbols"),              INKSCAPE_ICON("symbols"),                     DialogData::Basic,          ScrollProvider::PROVIDE   }},
    {"Text",               {_("_Text and Font"),        INKSCAPE_ICON("dialog-text-and-font"),        DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"Trace",              {_("_Trace Bitmap"),         INKSCAPE_ICON("bitmap-trace"),                DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"Transform",          {_("Transfor_m"),            INKSCAPE_ICON("dialog-transform"),            DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"UndoHistory",        {_("Undo _History"),         INKSCAPE_ICON("edit-undo-history"),           DialogData::Basic,          ScrollProvider::NOPROVIDE }},
    {"XMLEditor",          {_("_XML Editor"),           INKSCAPE_ICON("dialog-xml-editor"),           DialogData::Advanced,       ScrollProvider::NOPROVIDE }},
#if WITH_GSPELL
    {"Spellcheck",         {_("Check Spellin_g"),       INKSCAPE_ICON("tools-check-spelling"),        DialogData::Basic,          ScrollProvider::NOPROVIDE }},
#endif
#if DEBUG
    {"Prototype",          {_("Prototype"),             INKSCAPE_ICON("document-properties"),         DialogData::Other,          ScrollProvider::NOPROVIDE }},
#endif
    // clang-format on
};
