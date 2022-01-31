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
    enum Category { Basic, Advanced, Settings, Diagnostics, Other };
    Category category;
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
    {"AlignDistribute",    {_("_Align and Distribute"), INKSCAPE_ICON("dialog-align-and-distribute"), DialogData::Basic }},
    {"CloneTiler",         {_("Create Tiled Clones"),   INKSCAPE_ICON("dialog-tile-clones"),          DialogData::Basic }},
    {"DocumentProperties", {_("_Document Properties"),  INKSCAPE_ICON("document-properties"),         DialogData::Settings }},
    {"Export",             {_("_Export"),               INKSCAPE_ICON("document-export"),             DialogData::Basic }},
    {"FillStroke",         {_("_Fill and Stroke"),      INKSCAPE_ICON("dialog-fill-and-stroke"),      DialogData::Basic }},
    {"FilterEffects",      {_("Filter _Editor"),        INKSCAPE_ICON("dialog-filters"),              DialogData::Advanced }},
    {"Find",               {_("_Find/Replace"),         INKSCAPE_ICON("edit-find"),                   DialogData::Basic }},
    {"Glyphs",             {_("_Unicode Characters"),   INKSCAPE_ICON("accessories-character-map"),   DialogData::Basic }},
    {"IconPreview",        {_("Icon Preview"),          INKSCAPE_ICON("dialog-icon-preview"),         DialogData::Basic }},
    {"Input",              {_("_Input Devices"),        INKSCAPE_ICON("dialog-input-devices"),        DialogData::Settings }},
    {"LivePathEffect",     {_("Path E_ffects"),         INKSCAPE_ICON("dialog-path-effects"),         DialogData::Advanced }},
    {"Memory",             {_("About _Memory"),         INKSCAPE_ICON("dialog-memory"),               DialogData::Diagnostics }},
    {"Messages",           {_("_Messages"),             INKSCAPE_ICON("dialog-messages"),             DialogData::Diagnostics }},
    {"ObjectAttributes",   {_("_Object attributes"),    INKSCAPE_ICON("dialog-object-properties"),    DialogData::Settings }},
    {"ObjectProperties",   {_("_Object Properties"),    INKSCAPE_ICON("dialog-object-properties"),    DialogData::Settings }},
    {"Objects",            {_("Layers and Object_s"),   INKSCAPE_ICON("dialog-objects"),              DialogData::Basic }},
    {"PaintServers",       {_("_Paint Servers"),        INKSCAPE_ICON("symbols"),                     DialogData::Advanced }},
    {"Preferences",        {_("P_references"),          INKSCAPE_ICON("preferences-system"),          DialogData::Settings }},
    {"Selectors",          {_("_Selectors and CSS"),    INKSCAPE_ICON("dialog-selectors"),            DialogData::Advanced }},
    {"SVGFonts",           {_("SVG Font Editor"),       INKSCAPE_ICON("dialog-svg-font"),             DialogData::Advanced }},
    {"Swatches",           {_("S_watches"),             INKSCAPE_ICON("swatches"),                    DialogData::Basic }},
    {"Symbols",            {_("S_ymbols"),              INKSCAPE_ICON("symbols"),                     DialogData::Basic }},
    {"Text",               {_("_Text and Font"),        INKSCAPE_ICON("dialog-text-and-font"),        DialogData::Basic }},
    {"Trace",              {_("_Trace Bitmap"),         INKSCAPE_ICON("bitmap-trace"),                DialogData::Basic }},
    {"Transform",          {_("Transfor_m"),            INKSCAPE_ICON("dialog-transform"),            DialogData::Basic }},
    {"UndoHistory",        {_("Undo _History"),         INKSCAPE_ICON("edit-undo-history"),           DialogData::Basic }},
    {"XMLEditor",          {_("_XML Editor"),           INKSCAPE_ICON("dialog-xml-editor"),           DialogData::Advanced }},
#if WITH_GSPELL
    {"Spellcheck",         {_("Check Spellin_g"),       INKSCAPE_ICON("tools-check-spelling"),        DialogData::Basic }},
#endif
#if DEBUG
    {"Prototype",          {_("Prototype"),             INKSCAPE_ICON("document-properties"),         DialogData::Other }},
#endif
    // clang-format on
};
