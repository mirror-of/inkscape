#ifndef __SP_TEXT_EDITING_H__
#define __SP_TEXT_EDITING_H__

/*
 * Text editing functions common for for text and flowtext
 *
 * Author:
 *   bulia byak
 *
 * Copyright (C) 2004 author
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib/gtypes.h>
struct SPItem;
namespace NR { class Point; }


Inkscape::Text::Layout const * te_get_layout (SPItem *item);
bool sp_te_is_empty (SPItem *item);

unsigned sp_text_get_length(SPObject *item);
std::vector<NR::Point> sp_te_create_selection_quads(SPItem *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, NR::Matrix const &transform);

Inkscape::Text::Layout::iterator sp_te_get_position_by_coords (SPItem *item, NR::Point &i_p);
void sp_te_get_cursor_coords (SPItem *item, Inkscape::Text::Layout::iterator const &i_position, NR::Point &p0, NR::Point &p1);

Inkscape::Text::Layout::iterator sp_te_insert(SPItem *item, Inkscape::Text::Layout::iterator const &position, gchar const *utf8);
Inkscape::Text::Layout::iterator sp_te_replace(SPItem *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end, gchar const *utf8);
Inkscape::Text::Layout::iterator sp_te_delete (SPItem *item, Inkscape::Text::Layout::iterator const &start, Inkscape::Text::Layout::iterator const &end);
Inkscape::Text::Layout::iterator sp_te_insert_line (SPItem *text, Inkscape::Text::Layout::iterator const &position);

gchar *sp_te_get_string_multiline(SPItem *text);
void sp_te_set_repr_text_multiline(SPItem *text, gchar const *str);
void sp_te_adjust_kerning_screen(SPItem *text, Inkscape::Text::Layout::iterator const &position, SPDesktop *desktop, NR::Point by);
void sp_te_adjust_tspan_letterspacing_screen(SPItem *text, Inkscape::Text::Layout::iterator const &position, SPDesktop *desktop, gdouble by);
void sp_te_adjust_linespacing_screen(SPItem *text, SPDesktop *desktop, gdouble by);

#endif
