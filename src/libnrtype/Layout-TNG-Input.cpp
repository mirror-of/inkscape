/*
 * Inkscape::Text::Layout - text layout engine input functions
 *
 * Authors:
 *   Richard Hughes <cyreve@users.sf.net>
 *
 * Copyright (C) 2005 Richard Hughes
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "Layout-TNG.h"
#include "style.h"
#include "svg/svg-types.h"
#include "sp-object.h"
#include "FontFactory.h"
#include <glib/glist.h>

namespace Inkscape {
namespace Text {

void Layout::_clearInputObjects()
{
    for(std::vector<InputStreamItem*>::iterator it = _input_stream.begin() ; it != _input_stream.end() ; it++)
        delete *it;
    _input_stream.clear();
    _input_wrap_shapes.clear();
}

// this function does nothing more than store all its parameters for future reference
void Layout::appendText(Glib::ustring const &text, SPStyle *style, void *source_cookie, OptionalTextTagAttrs const *optional_attributes, unsigned optional_attributes_offset, Glib::ustring::const_iterator text_begin, Glib::ustring::const_iterator text_end)
{
    if (style == NULL) return;

    InputStreamTextSource *new_source = new InputStreamTextSource;

    new_source->source_cookie = source_cookie;
    new_source->text = &text;
    new_source->text_begin = text_begin;
    new_source->text_end = text_end;
    new_source->style = style;
    sp_style_ref(style);

    new_source->text_length = 0;
    for ( ; text_begin != text_end && text_begin != text.end() ; text_begin++)
        new_source->text_length++;        // save this because calculating the length of a UTF-8 string is expensive

    if (optional_attributes) {
        // we need to fill in x and y even if the text is empty so that empty paragraphs can be positioned correctly
        _copyInputVector(optional_attributes->x, optional_attributes_offset, &new_source->x, std::max(1, new_source->text_length));
        _copyInputVector(optional_attributes->y, optional_attributes_offset, &new_source->y, std::max(1, new_source->text_length));
        _copyInputVector(optional_attributes->dx, optional_attributes_offset, &new_source->dx, new_source->text_length);
        _copyInputVector(optional_attributes->dy, optional_attributes_offset, &new_source->dy, new_source->text_length);
        _copyInputVector(optional_attributes->rotate, optional_attributes_offset, &new_source->rotate, new_source->text_length);
    }
    
    _input_stream.push_back(new_source);
}

void Layout::_copyInputVector(std::vector<SPSVGLength> const &input_vector, unsigned input_offset, std::vector<SPSVGLength> *output_vector, unsigned max_length)
{
    output_vector->clear();
    if (input_offset >= input_vector.size()) return;
    output_vector->reserve(std::min(max_length, input_vector.size() - input_offset));
    while (input_offset < input_vector.size() && max_length != 0) {
        if (!input_vector[input_offset].set)
            break;
        output_vector->push_back(input_vector[input_offset]);
        input_offset++;
        max_length--;
    }
}

// just save what we've been given, really
void Layout::appendControlCode(TextControlCode code, void *source_cookie, double width, double ascent, double descent)
{
    InputStreamControlCode *new_code = new InputStreamControlCode;

    new_code->source_cookie = source_cookie;
    new_code->code = code;
    new_code->width = width;
    new_code->ascent = ascent;
    new_code->descent = descent;
    
    _input_stream.push_back(new_code);
}

// more saving of the parameters
void Layout::appendWrapShape(Shape const *shape, DisplayAlign display_align)
{
    _input_wrap_shapes.push_back(InputWrapShape());
    _input_wrap_shapes.back().shape = shape;
    _input_wrap_shapes.back().display_align = display_align;
}

int Layout::_enum_converter(int input, EnumConversionItem const *conversion_table, unsigned conversion_table_size)
{
    for (unsigned i = 0 ; i < conversion_table_size ; i++)
        if (conversion_table[i].input == input)
            return conversion_table[i].output;
    return conversion_table[0].output;
}

// ***** the style format interface
// this doesn't include all accesses to SPStyle, only the ones that will be a pain to change when we start using libcroco

Layout::Direction Layout::InputStreamTextSource::styleComputeBlockProgression() const
{
    // SPStyle's implementation of this is all wrong. FIXME.
    if (style->writing_mode.computed == SP_CSS_WRITING_MODE_TB_LR)
        return LEFT_TO_RIGHT;
    return TOP_TO_BOTTOM;
}

static const float medium_font_size = 12.0;     // more of a default if all else fails than anything else
float Layout::InputStreamTextSource::styleComputeFontSize() const
{
    return style->font_size.computed;

    // OK, I have no idea how the style system works, so this lot is all archived and never executed:
    SPStyle const *this_style = style;
    float inherit_multiplier = 1.0;

    for ( ; ; ) {
        if (!this_style->font_size.inherit) {
            switch (this_style->font_size.type) {
                case SP_FONT_SIZE_LITERAL: {
                    switch(this_style->font_size.value) {   // these multipliers are straight out of the CSS spec
	                    case SP_CSS_FONT_SIZE_XX_SMALL: return medium_font_size * inherit_multiplier * (3.0/5.0);
	                    case SP_CSS_FONT_SIZE_X_SMALL:  return medium_font_size * inherit_multiplier * (3.0/4.0);
	                    case SP_CSS_FONT_SIZE_SMALL:    return medium_font_size * inherit_multiplier * (8.0/9.0);
                        default:
	                    case SP_CSS_FONT_SIZE_MEDIUM:   return medium_font_size * inherit_multiplier;
	                    case SP_CSS_FONT_SIZE_LARGE:    return medium_font_size * inherit_multiplier * (6.0/5.0);
	                    case SP_CSS_FONT_SIZE_X_LARGE:  return medium_font_size * inherit_multiplier * (3.0/2.0);
	                    case SP_CSS_FONT_SIZE_XX_LARGE: return medium_font_size * inherit_multiplier * 2.0;
	                    case SP_CSS_FONT_SIZE_SMALLER: inherit_multiplier *= 0.84; break;   //not exactly according to spec
	                    case SP_CSS_FONT_SIZE_LARGER:  inherit_multiplier *= 1.26; break;   //not exactly according to spec
                    }
                    break;
                }
                case SP_FONT_SIZE_PERCENTAGE: {    // 'em' units should be in here, but aren't. Fix in style.cpp.
                    inherit_multiplier *= this_style->font_size.value;
                    break;
                }
                case SP_FONT_SIZE_LENGTH: {
                    return this_style->font_size.value * inherit_multiplier;
                }
            }
        }
        if (this_style->object->parent == NULL)
            return medium_font_size * inherit_multiplier;
        this_style = this_style->object->parent->style;
    }
    // never get here
}

Layout::Alignment Layout::InputStreamTextSource::styleGetAlignment(Layout::Direction para_direction) const
{
    // all wrong again
    switch (style->text_anchor.computed) {
        case SP_CSS_TEXT_ANCHOR_START: break;  // this is especially wrong, because 'start' means left, even for rtl
        case SP_CSS_TEXT_ANCHOR_MIDDLE: return CENTER;
        case SP_CSS_TEXT_ANCHOR_END: return RIGHT;
    }
    return para_direction == LEFT_TO_RIGHT ? LEFT : RIGHT;
}

static const Layout::EnumConversionItem enum_convert_spstyle_style_to_pango_style[] = {
    {SP_CSS_FONT_STYLE_NORMAL,  PANGO_STYLE_NORMAL},
    {SP_CSS_FONT_STYLE_ITALIC,  PANGO_STYLE_ITALIC},
    {SP_CSS_FONT_STYLE_OBLIQUE, PANGO_STYLE_OBLIQUE}};

static const Layout::EnumConversionItem enum_convert_spstyle_weight_to_pango_weight[] = {
    {SP_CSS_FONT_WEIGHT_NORMAL, PANGO_WEIGHT_NORMAL},
    {SP_CSS_FONT_WEIGHT_100, PANGO_WEIGHT_ULTRALIGHT},
    {SP_CSS_FONT_WEIGHT_200, PANGO_WEIGHT_ULTRALIGHT},
    {SP_CSS_FONT_WEIGHT_300, PANGO_WEIGHT_LIGHT},
    {SP_CSS_FONT_WEIGHT_400, PANGO_WEIGHT_LIGHT},
    {SP_CSS_FONT_WEIGHT_500, PANGO_WEIGHT_NORMAL},
    {SP_CSS_FONT_WEIGHT_600, PANGO_WEIGHT_BOLD},
    {SP_CSS_FONT_WEIGHT_BOLD,PANGO_WEIGHT_BOLD},
    {SP_CSS_FONT_WEIGHT_700, PANGO_WEIGHT_BOLD},
    {SP_CSS_FONT_WEIGHT_800, PANGO_WEIGHT_ULTRABOLD},
    {SP_CSS_FONT_WEIGHT_900, PANGO_WEIGHT_HEAVY}};

static const Layout::EnumConversionItem enum_convert_spstyle_stretch_to_pango_stretch[] = {
    {SP_CSS_FONT_STRETCH_NORMAL,          PANGO_STRETCH_NORMAL},
    {SP_CSS_FONT_STRETCH_ULTRA_CONDENSED, PANGO_STRETCH_ULTRA_CONDENSED},
    {SP_CSS_FONT_STRETCH_EXTRA_CONDENSED, PANGO_STRETCH_EXTRA_CONDENSED},
    {SP_CSS_FONT_STRETCH_CONDENSED,       PANGO_STRETCH_CONDENSED},
    {SP_CSS_FONT_STRETCH_SEMI_CONDENSED,  PANGO_STRETCH_SEMI_CONDENSED},
    {SP_CSS_FONT_STRETCH_SEMI_EXPANDED,   PANGO_STRETCH_SEMI_EXPANDED},
    {SP_CSS_FONT_STRETCH_EXPANDED,        PANGO_STRETCH_EXPANDED},
    {SP_CSS_FONT_STRETCH_EXTRA_EXPANDED,  PANGO_STRETCH_EXTRA_EXPANDED},
    {SP_CSS_FONT_STRETCH_ULTRA_EXPANDED,  PANGO_STRETCH_ULTRA_EXPANDED}};

static const Layout::EnumConversionItem enum_convert_spstyle_variant_to_pango_variant[] = {
    {SP_CSS_FONT_VARIANT_NORMAL,     PANGO_VARIANT_NORMAL},
    {SP_CSS_FONT_VARIANT_SMALL_CAPS, PANGO_VARIANT_SMALL_CAPS}};

font_instance *Layout::InputStreamTextSource::styleGetFontInstance() const
{
    if (style->text == NULL) return NULL;
    return (font_factory::Default())->Face(style->text->font_family.value,
                                           _enum_converter(style->font_variant.computed, enum_convert_spstyle_variant_to_pango_variant, sizeof(enum_convert_spstyle_variant_to_pango_variant)/sizeof(enum_convert_spstyle_variant_to_pango_variant[0])),
                                           _enum_converter(style->font_style.computed,   enum_convert_spstyle_style_to_pango_style,     sizeof(enum_convert_spstyle_style_to_pango_style)/sizeof(enum_convert_spstyle_style_to_pango_style[0])),
                                           _enum_converter(style->font_weight.computed,  enum_convert_spstyle_weight_to_pango_weight,   sizeof(enum_convert_spstyle_weight_to_pango_weight)/sizeof(enum_convert_spstyle_weight_to_pango_weight[0])),
                                           _enum_converter(style->font_stretch.computed, enum_convert_spstyle_stretch_to_pango_stretch, sizeof(enum_convert_spstyle_stretch_to_pango_stretch)/sizeof(enum_convert_spstyle_stretch_to_pango_stretch[0])));
}

Layout::InputStreamTextSource::~InputStreamTextSource()
{
    sp_style_unref(style);
}

}//namespace Text
}//namespace Inkscape
