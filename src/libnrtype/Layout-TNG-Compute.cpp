/*
 * Inkscape::Text::Layout::Calculator - text layout engine meaty bits
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
#include "font-instance.h"
#include "svg/svg-types.h"
#include "Layout-TNG-Scanline-Maker.h"
#include "FontFactory.h"
#include <pango/pango.h>
#include <map>

namespace Inkscape {
namespace Text {

//#define IFDEBUG(...)     __VA_ARGS__
#define IFDEBUG(...)

#define TRACE(format, ...) IFDEBUG(g_print(format, ## __VA_ARGS__),g_print("\n"))

// ******* enum conversion tables
static const Layout::EnumConversionItem enum_convert_spstyle_direction_to_pango_direction[] = {
	{SP_CSS_WRITING_MODE_LR_TB, PANGO_DIRECTION_LTR},
	{SP_CSS_WRITING_MODE_RL_TB, PANGO_DIRECTION_RTL},
	{SP_CSS_WRITING_MODE_TB_LR, PANGO_DIRECTION_LTR}};   // this is correct

static const Layout::EnumConversionItem enum_convert_spstyle_direction_to_my_direction[] = {
	{SP_CSS_WRITING_MODE_LR_TB, Layout::LEFT_TO_RIGHT},
	{SP_CSS_WRITING_MODE_RL_TB, Layout::RIGHT_TO_LEFT},
	{SP_CSS_WRITING_MODE_TB_LR, Layout::LEFT_TO_RIGHT}};   // this is correct

/** \brief private to Layout. Does the real work of text flowing.

This class does a standard greedy paragraph wrapping algorithm.

Very high-level overview:

<pre>
foreach(paragraph) {
  call pango_itemize() (_buildPangoItemizationForPara())
  break into spans, without dealing with wrapping (_buildSpansForPara())
  foreach(line in flow shape) {
    // everything inside this loop in _outputLineThatFits()
    foreach(chunk in flow shape) {
      // this inner loop in _measureChunk()
      if the line height changed discard the line and start again
      keep adding characters until we run out of space in the chunk, then back up to the last word boundary
      (do sensible things if there is no previous word break)
    }
    push all the glyphs, chars, spans, chunks and line to output (not completely trivial because we must draw rtl in character order)
  }
  push the paragraph
}
</pre>

...and all of that needs to work vertically too, and with all the little details that make life annoying
*/
class Layout::Calculator
{
    class SpanPosition;
    friend class SpanPosition;

    Layout &_flow;

    ScanlineMaker *_scanline_maker;

    PangoContext *_pango_context;

    Direction _block_progression;

    /** for y= attributes in tspan elements et al, we do the adjustment by moving each
    glyph individually by this number. It needs to be maintained across paragraphs
    because the caller asks us what the line spacing is in order to set the correct
    values on new tspan elements with role:line. */
    double _y_offset;

    /** to stop pango from hinting its output, the font factory creates all fonts very large.
    All numbers returned from pango have to be divided by this number \em and divided by
    PANGO_SCALE. See font_factory::font_factory(). */
    double _font_factory_size_multiplier;

    /** Stuff associated with each PangoItem. */
    struct PangoItemStorage {
        PangoItem *item;
        font_instance *font;

        inline PangoItemStorage()
            : item(NULL),font(NULL) {}

        PangoItemStorage(const PangoItemStorage &other)   // the parameter is *not const*. The STL containers can't be used if it is, though.
            : item(other.item), font(other.font)
        {
            const_cast<PangoItemStorage&>(other).font = NULL;    // NB: self-assignment will break
            const_cast<PangoItemStorage&>(other).item = NULL;
        }

        ~PangoItemStorage()
        {
            if (font) font->Unref();
            if (item) pango_item_free(item);
        }
    };

    class PangoGlyphStringAutoPtr : public std::auto_ptr<PangoGlyphString> {      // evilness prize!
    public:
        PangoGlyphStringAutoPtr() {}
        PangoGlyphStringAutoPtr(PangoGlyphString *p) : std::auto_ptr<PangoGlyphString>(p) {}
        PangoGlyphStringAutoPtr(PangoGlyphStringAutoPtr const &other) : std::auto_ptr<PangoGlyphString>(const_cast<PangoGlyphStringAutoPtr&>(other)) {}
        ~PangoGlyphStringAutoPtr() {if (get()) pango_glyph_string_free(release());}
    };

    /** There's quite a lot of information about each span that we only need
    during calculation. Also, these spans are from before line breaking is
    done (actually, during). */
    struct IntermediateSpanStorage {
        PangoGlyphStringAutoPtr glyph_string;
        int pango_item_index;    /// index into _para.pango_items, or -1 if this is style only
        unsigned input_index;         /// index into Layout::_input_stream
        Glib::ustring::const_iterator input_stream_first_character;
        double font_size;
        LineHeight line_height;
        unsigned text_bytes;
        unsigned char_index_in_para;    /// the index of the first character in this span in the paragraph, for looking up char_attributes
        // these are copies of the <tspan> attributes. We change our spans when we encounter one.
        SPSVGLength x, y, dx, dy, rotate;
    };

    /** a useful little iterator for moving char-by-char across spans. */
    struct SpanPosition {
        std::vector<IntermediateSpanStorage>::iterator iter_span;
        unsigned char_byte;
        unsigned char_index;

        void increment()   /// step forward by one character
        {
            gchar const *text_base = &*iter_span->input_stream_first_character.base();
            char_byte = g_utf8_next_char(text_base + char_byte) - text_base;
            char_index++;
            if (char_byte == iter_span->text_bytes) {
                iter_span++;
                char_index = char_byte = 0;
            }
        }

        inline bool operator== (SpanPosition const &other) const
            {return char_byte == other.char_byte && iter_span == other.iter_span;}
        inline bool operator!= (SpanPosition const &other) const
            {return char_byte != other.char_byte || iter_span != other.iter_span;}
    };

    /** the same as IntermediateSpanStorage for chunks. */
    struct IntermediateChunkStorage {
        SpanPosition start_pos;
        double total_width;       /// that's the total width used by the text (excluding justification), not the total scanrun width
        double x;
        int whitespace_count;
        // these values can't just be stored in the span because they may only represent part of a span (post-breaking)
        // TODO: think of something less strange
        std::map<std::vector<IntermediateSpanStorage>::const_iterator, double> span_widths;
    };
    
    /** the same as IntermediateSpanStorage for input streams.
     * This one exists for the whole duration of the calculation, not just
     * for the current paragraph. */
    struct IntermediateInputItemStorage {
        bool in_sub_flow;
        Layout *sub_flow;    // this is only set for the first input item in a sub-flow
        IntermediateInputItemStorage() : in_sub_flow(false), sub_flow(NULL) {}
    };

    std::vector<IntermediateInputItemStorage> _input_items;

    /** Used to provide storage for anything that applies to the current
    paragraph only. Since we're only processing one paragraph at a time,
    there's only one instantiation of this struct - inline with
    Calculator. */
    struct ParagraphRelatedInfo {
        unsigned first_input_index;      /// index into Layout::_input_stream
        Direction direction;
        Alignment alignment;
        std::vector<IntermediateInputItemStorage> input_streams;
        std::vector<PangoItemStorage> pango_items;
        std::vector<IntermediateSpanStorage> spans;
        std::vector<PangoLogAttr> char_attributes;
    } _para;

    /** Storage for everything related to the current line. There's only
    ever one of these too. Most of the really interesting stuff is actually
    kept on the stack as local variables in _outputLineThatFits(). */
    struct LineRelatedInfo {
        std::vector<ScanlineMaker::ScanRun> scan_runs;
        Layout::LineHeight line_height;
        int shape_index;      /// into Layout::_input_wrap_shapes
    } _line;

    /** for sections of text with a block-progression different to the rest
     * of the flow, the best thing to do is to detect them in advance and
     * create child TextFlow objects with just the rotated text. In the
     * parent we then effectively use ARBITRARY_GAP fields during the
     * flowing (because we don't allow wrapping when the block-progression
     * changes) and copy the actual text in during the output phase.
     * 
     * NB: this code not enabled yet.
     */
    void _createBlockProgressionSubflows()
    {
        Direction prev_block_progression = _block_progression;
        int run_start_input_index = _para.first_input_index;
        
        for(int input_index = _para.first_input_index ; input_index < (int)_flow._input_stream.size() ; input_index++) {
            if (_flow._input_stream[input_index]->Type() == CONTROL_CODE) {
                Layout::InputStreamControlCode const *control_code = static_cast<Layout::InputStreamControlCode const *>(_flow._input_stream[input_index]);
                if (   control_code->code == SHAPE_BREAK
                    || control_code->code == PARAGRAPH_BREAK)
                    break;                                    // stop at the end of the paragraph
                // all other control codes we'll pick up later

            } else if (_flow._input_stream[input_index]->Type() == TEXT_SOURCE) {
                Layout::InputStreamTextSource *text_source = static_cast<Layout::InputStreamTextSource *>(_flow._input_stream[input_index]);
                Direction this_block_progression = text_source->styleComputeBlockProgression();
                if (this_block_progression != prev_block_progression) {
                    if (prev_block_progression != _block_progression) {
                        // need to back up so that control codes belong outside the block-progression change
                        int run_end_input_index = input_index - 1;
                        while (run_end_input_index > run_start_input_index
                               && _flow._input_stream[run_end_input_index]->Type() != TEXT_SOURCE)
                            run_end_input_index--;
                        // now create the sub-flow
                        Layout *sub_flow = new Layout;
                        for (int sub_input_index = run_start_input_index ; sub_input_index <= run_end_input_index ; sub_input_index++) {
                            _input_items[sub_input_index].in_sub_flow = true;
                            if (_flow._input_stream[sub_input_index]->Type() == CONTROL_CODE) {
                                Layout::InputStreamControlCode const *control_code = static_cast<Layout::InputStreamControlCode const *>(_flow._input_stream[sub_input_index]);
                                sub_flow->appendControlCode(control_code->code, control_code->source_cookie, control_code->width, control_code->ascent, control_code->descent);
                            } else if (_flow._input_stream[sub_input_index]->Type() == TEXT_SOURCE) {
                                Layout::InputStreamTextSource *text_source = static_cast<Layout::InputStreamTextSource *>(_flow._input_stream[sub_input_index]);
                                sub_flow->appendText(*text_source->text, text_source->style, text_source->source_cookie, NULL, 0, text_source->text_begin, text_source->text_end);
                                Layout::InputStreamTextSource *sub_flow_text_source = static_cast<Layout::InputStreamTextSource *>(sub_flow->_input_stream.back());
                                sub_flow_text_source->x = text_source->x;    // this is easier than going back to GLists for the appendText() call
                                sub_flow_text_source->y = text_source->y;    // should these actually be allowed anyway? You'll almost never get the results you expect
                                sub_flow_text_source->dx = text_source->dx;  // (not that it's very clear what you should expect, anyway)
                                sub_flow_text_source->dy = text_source->dy;
                                sub_flow_text_source->rotate = text_source->rotate;
                            }
                        }
                        sub_flow->calculateFlow();
                        _input_items[run_start_input_index].sub_flow = sub_flow;
                    }
                    run_start_input_index = input_index;
                }
                prev_block_progression = this_block_progression;
            }
        }
    }

    /** take all the text from \a _para.first_input_index to the end of the
    paragraph and stitch it together so that pango_itemize() can be called on
    the whole thing. Input: _para.first_input_index set.
    Output: _para.direction, _para.pango_items, _para.char_attributes */
    void _buildPangoItemizationForPara()
    {
        Glib::ustring para_text;
        PangoAttrList *attributes_list;
        unsigned input_index;

        _para.pango_items.clear();
        _para.char_attributes.clear();

        TRACE("itemizing para, first input %d", _para.first_input_index);

        attributes_list = pango_attr_list_new();
        for(input_index = _para.first_input_index ; input_index < _flow._input_stream.size() ; input_index++) {
            if (_flow._input_stream[input_index]->Type() == CONTROL_CODE) {
                Layout::InputStreamControlCode const *control_code = static_cast<Layout::InputStreamControlCode const *>(_flow._input_stream[input_index]);
                if (   control_code->code == SHAPE_BREAK
                    || control_code->code == PARAGRAPH_BREAK)
                    break;                                    // stop at the end of the paragraph
                // all other control codes we'll pick up later

            } else if (_flow._input_stream[input_index]->Type() == TEXT_SOURCE) {
                Layout::InputStreamTextSource *text_source = static_cast<Layout::InputStreamTextSource *>(_flow._input_stream[input_index]);

                // create the font_instance
                font_instance *font = text_source->styleGetFontInstance();
                if (font == NULL)
                    continue;  // bad news: we'll have to ignore all this text because we know of no font to render it

                PangoAttribute *attribute_font_description = pango_attr_font_desc_new(font->descr);
                attribute_font_description->start_index = para_text.bytes();
                para_text.append(&*text_source->text_begin.base(), text_source->text_length);     // build the combined text
                attribute_font_description->end_index = para_text.bytes();
                pango_attr_list_insert(attributes_list, attribute_font_description);
                  // ownership of attribute is assumed by the list
            }
        }

        TRACE("whole para: \"%s\"", para_text.data());
        TRACE("%d input sources used", input_index - _para.first_input_index);

        // do the pango_itemize()
        GList *pango_items_glist = NULL;
        if (_flow._input_stream[_para.first_input_index]->Type() == TEXT_SOURCE) {
            Layout::InputStreamTextSource const *text_source = static_cast<Layout::InputStreamTextSource *>(_flow._input_stream[_para.first_input_index]);
            if (text_source->style->direction.set) {
                PangoDirection pango_direction = (PangoDirection)_enum_converter(text_source->style->direction.computed, enum_convert_spstyle_direction_to_pango_direction, sizeof(enum_convert_spstyle_direction_to_pango_direction)/sizeof(enum_convert_spstyle_direction_to_pango_direction[0]));
                pango_items_glist = pango_itemize_with_base_dir(_pango_context, pango_direction, para_text.data(), 0, para_text.bytes(), attributes_list, NULL);
                _para.direction = (Layout::Direction)_enum_converter(text_source->style->direction.computed, enum_convert_spstyle_direction_to_my_direction, sizeof(enum_convert_spstyle_direction_to_my_direction)/sizeof(enum_convert_spstyle_direction_to_my_direction[0]));
            }
        }
        if (pango_items_glist == NULL) {
            pango_items_glist = pango_itemize(_pango_context, para_text.data(), 0, para_text.bytes(), attributes_list, NULL);

            if (pango_items_glist == NULL || pango_items_glist->data == NULL) _para.direction = LEFT_TO_RIGHT;
            else _para.direction = (((PangoItem*)pango_items_glist->data)->analysis.level & 1) ? RIGHT_TO_LEFT : LEFT_TO_RIGHT;
        }
        pango_attr_list_unref(attributes_list);

        // convert the GList to our vector<> and make the font_instance for each PangoItem at the same time
        _para.pango_items.reserve(g_list_length(pango_items_glist));
        TRACE("para itemizes to %d sections", g_list_length(pango_items_glist));
        for (GList *current_pango_item = pango_items_glist ; current_pango_item != NULL ; current_pango_item = current_pango_item->next) {
            PangoItemStorage new_item;
            new_item.item = (PangoItem*)current_pango_item->data;
            PangoFontDescription *font_description = pango_font_describe(new_item.item->analysis.font);
            new_item.font = (font_factory::Default())->Face(font_description);
            pango_font_description_free(font_description);   // Face() makes a copy
            _para.pango_items.push_back(new_item);
        }
        g_list_free(pango_items_glist);

        // and get the character attributes on everything
        _para.char_attributes.resize(para_text.length() + 1);
        pango_get_log_attrs(para_text.data(), para_text.bytes(), -1, NULL, &*_para.char_attributes.begin(), _para.char_attributes.size());

        TRACE("end para itemize, direction = %d", _para.direction);
    }

    /** split the paragraph into spans. Also calls pango_shape()  on them.
    Input: _para.first_input_index, _para.pango_items
    Output: _para.spans, \a next_para_input_index set */
    void _buildSpansForPara(unsigned *next_para_input_index)
    {
        unsigned pango_item_index = 0;
        unsigned char_index_in_para = 0;
        unsigned byte_index_in_para = 0;
        unsigned input_index;

        TRACE("build spans");
        _para.spans.clear();

        for(input_index = _para.first_input_index ; input_index < _flow._input_stream.size() ; input_index++) {
            if (_flow._input_stream[input_index]->Type() == CONTROL_CODE) {
                Layout::InputStreamControlCode const *control_code = static_cast<Layout::InputStreamControlCode const *>(_flow._input_stream[input_index]);
                if (   control_code->code == SHAPE_BREAK
                    || control_code->code == PARAGRAPH_BREAK)
                    break;                                    // stop at the end of the paragraph
                else if (control_code->code == ARBITRARY_GAP) {
                    IntermediateSpanStorage new_span;
                    new_span.pango_item_index = -1;
                    new_span.input_index = input_index;
                    new_span.line_height.ascent = control_code->ascent;
                    new_span.line_height.descent = control_code->descent;
                    new_span.line_height.leading = 0.0;
                    new_span.text_bytes = 0;
                    new_span.char_index_in_para = char_index_in_para;
                    _para.spans.push_back(new_span);
                    TRACE("add gap span %d", _para.spans.size() - 1);
                }
            } else if (_flow._input_stream[input_index]->Type() == TEXT_SOURCE && pango_item_index < _para.pango_items.size()) {
                Layout::InputStreamTextSource const *text_source = static_cast<Layout::InputStreamTextSource const *>(_flow._input_stream[input_index]);
                unsigned char_index_in_source = 0;

                unsigned span_start_byte_in_source = 0;
                for ( ; ; ) {
                    IntermediateSpanStorage new_span;
                    unsigned pango_item_bytes;
                    unsigned text_source_bytes;
                    /* we need to change spans at every change of PangoItem, source stream change,
                    or change in one of the attributes altering position/rotation. */

                    pango_item_bytes = pango_item_index >= _para.pango_items.size() ? 0 : _para.pango_items[pango_item_index].item->offset + _para.pango_items[pango_item_index].item->length - byte_index_in_para; 
                    text_source_bytes = text_source->text_end.base() - text_source->text_begin.base() - span_start_byte_in_source;
                    new_span.text_bytes = std::min(text_source_bytes, pango_item_bytes);
                    new_span.input_stream_first_character = Glib::ustring::const_iterator(text_source->text_begin.base() + span_start_byte_in_source);
                    new_span.char_index_in_para = char_index_in_para + char_index_in_source;
                    new_span.input_index = input_index;

                    // cut at <tspan> attribute changes as well
                    new_span.x.set = false;
                    new_span.y.set = false;
                    new_span.dx.set = false;
                    new_span.dy.set = false;
                    new_span.rotate.set = false;
                    if (text_source->x.size()  > char_index_in_source)     new_span.x  = text_source->x[char_index_in_source];
                    if (text_source->y.size()  > char_index_in_source)     new_span.y  = text_source->y[char_index_in_source];
                    if (text_source->dx.size() > char_index_in_source)     new_span.dx = text_source->dx[char_index_in_source];
                    if (text_source->dy.size() > char_index_in_source)     new_span.dy = text_source->dy[char_index_in_source];
                    if (text_source->rotate.size() > char_index_in_source) new_span.rotate = text_source->rotate[char_index_in_source];
                    Glib::ustring::const_iterator iter_text = new_span.input_stream_first_character;
                    iter_text++;
                    for (unsigned i = char_index_in_source + 1 ; ; i++, iter_text++) {
                        if (iter_text >= text_source->text_end) break;
                        if (iter_text.base() - new_span.input_stream_first_character.base() >= (int)new_span.text_bytes) break;
                        if (   i >= text_source->x.size() && i >= text_source->y.size()
                            && i >= text_source->dx.size() && i >= text_source->dy.size()
                            && i >= text_source->rotate.size()) break;
                        if (   (text_source->x.size()  > i && text_source->x[i].set)
                            || (text_source->y.size()  > i && text_source->y[i].set)
                            || (text_source->dx.size() > i && text_source->dx[i].set && text_source->dx[i].computed != 0.0)
                            || (text_source->dy.size() > i && text_source->dy[i].set && text_source->dy[i].computed != 0.0)
                            || (text_source->rotate.size() > i && text_source->rotate[i].set && text_source->rotate[i].computed != 0.0)) {
                            new_span.text_bytes = iter_text.base() - new_span.input_stream_first_character.base();
                            break;
                        }
                    }

                    // if we've still got anything to add, add it
                    PangoGlyphStringAutoPtr new_glyph_string(pango_glyph_string_new());
                    new_span.glyph_string = new_glyph_string;
                    if (new_span.text_bytes) {
                        int original_bidi_level = _para.pango_items[pango_item_index].item->analysis.level;
                        _para.pango_items[pango_item_index].item->analysis.level = 0;
                        // pango_shape() will reorder glyphs in rtl sections which messes us up because
                        // the svg spec requires us to draw glyphs in character order
                        pango_shape(text_source->text->data() + span_start_byte_in_source,
                                    new_span.text_bytes,
                                    &_para.pango_items[pango_item_index].item->analysis,
                                    new_span.glyph_string.get());
                        _para.pango_items[pango_item_index].item->analysis.level = original_bidi_level;
                        new_span.pango_item_index = pango_item_index;
                        _para.pango_items[pango_item_index].font->FontMetrics(new_span.line_height.ascent, new_span.line_height.descent, new_span.line_height.leading);
                        // TODO: metrics for vertical text
                    } else {
                        // if there's no text we still need to initialise the styles
                        new_span.pango_item_index = -1;
                        font_instance *font = text_source->styleGetFontInstance();
                        if (font) {
                            font->FontMetrics(new_span.line_height.ascent,new_span.line_height.descent,new_span.line_height.leading);
                            font->Unref();
                        } else {
                            new_span.line_height.ascent = 0.0;
                            new_span.line_height.descent = 0.0;
                            new_span.line_height.leading = 0.0;
                        }
                    }
                    new_span.font_size = text_source->styleComputeFontSize();
                    new_span.line_height.ascent *= new_span.font_size;
                    new_span.line_height.descent *= new_span.font_size;
                    new_span.line_height.leading *= new_span.font_size;
                    _para.spans.push_back(new_span);
                    byte_index_in_para += new_span.text_bytes;
                    char_index_in_source += g_utf8_strlen(&*new_span.input_stream_first_character.base(), new_span.text_bytes);
                    TRACE("add text span %d \"%s\"", _para.spans.size() - 1, text_source->text->raw().substr(span_start_byte_in_source, new_span.text_bytes).c_str());
                    TRACE("  %d glyphs", _para.spans.back().glyph_string->num_glyphs);

                    if (new_span.text_bytes >= pango_item_bytes) {   // end of pango item
                        pango_item_index++;
                        if (pango_item_index == _para.pango_items.size()) break;
                    }
                    if (new_span.text_bytes == text_source_bytes)
                        break;    // end of source
                    // else <tspan> attribute changed
                    span_start_byte_in_source += new_span.text_bytes;
                }
                char_index_in_para += char_index_in_source;
            }
        }
        *next_para_input_index = input_index;
        TRACE("end build spans");
    }


    /** Given a starting position, keep adding stuff to the output until we
    run out of space on a line. If the line gets too big try to expand it
    without restarting and if that fails, return false to signify a restart
    is required, and \a line_height will be filled with the new size.

    If everything goes well, the glyphs for the line will be dumped to
    output and the non-const parameters will be updated to reflect where
    the next line will continue.

    Input: _para.everything
    Output: Layout::everything, _line.line_height if the line needs to
       be expanded.
    */
    bool _outputLineThatFits(SpanPosition *start_span_pos)
    {
        SpanPosition span_pos = *start_span_pos;
        std::vector<IntermediateChunkStorage> chunk_info;

        if (_flow._input_wrap_shapes.empty()) {
            TRACE("finding non-wrapped line fit y=%f", _line.scan_runs.front().y);
            // we need to create a new chunk every time the x/y/dx/dy/rotate attributes change
            chunk_info.push_back(IntermediateChunkStorage());
            chunk_info[0].start_pos = *start_span_pos;
            for (unsigned chunk_index = 0;  ; chunk_index++) {
                if (!_measureChunk(&span_pos, _line.scan_runs[0], &chunk_info[chunk_index]))
                    return false;  // never happens
                _line.scan_runs[0].x_start = chunk_info.back().x + chunk_info.back().total_width;
                chunk_info.push_back(IntermediateChunkStorage());
                chunk_info[chunk_index + 1].start_pos = span_pos;
                if (span_pos.iter_span == _para.spans.end()) break;
            }
        } else {
            TRACE("finding line fit y=%f, %d chunks", _line.scan_runs.front().y, _line.scan_runs.size());
            if (_para.direction == RIGHT_TO_LEFT) std::reverse(_line.scan_runs.begin(), _line.scan_runs.end());
            chunk_info.resize((_line.scan_runs.size() + 1));
            chunk_info[0].start_pos = *start_span_pos;
            for (unsigned chunk_index = 0; chunk_index < _line.scan_runs.size() ; chunk_index++) {
                if (!_measureChunk(&span_pos, _line.scan_runs[chunk_index], &chunk_info[chunk_index]))
                    return false;
                chunk_info[chunk_index + 1].start_pos = span_pos;
            }

            if (chunk_info.front().start_pos == chunk_info.back().start_pos) {
                TRACE("line too short to fit anything on it, go to next");
                return true;
            }
        }

        // we've finished fiddling about with ascents and descents: create the output
        TRACE("found line fit; creating output");
        Layout::Line new_line;
        new_line.in_paragraph = _flow._paragraphs.size() - 1;
        new_line.baseline_y = _line.scan_runs[0].y + _line.line_height.ascent;
        new_line.in_shape = _line.shape_index;
        _flow._lines.push_back(new_line);

        for (unsigned chunk_index = 0; chunk_index < chunk_info.size() - 1 ; chunk_index++) {

            float glyph_rotate = 0.0;

            // add the chunk to the list
            Layout::Chunk new_chunk;
            new_chunk.in_line = _flow._lines.size() - 1;
            double add_to_each_whitespace = 0.0;   // for justification
            if (_flow._input_wrap_shapes.empty()) {
                switch (_para.alignment) {
                    case FULL:
                    case LEFT:
                    default:
                        new_chunk.left_x = chunk_info[chunk_index].x;
                        break;
                    case RIGHT:
                        new_chunk.left_x = chunk_info[chunk_index].x - chunk_info[chunk_index].total_width;
                        break;
                    case CENTER:
                        new_chunk.left_x = chunk_info[chunk_index].x - chunk_info[chunk_index].total_width / 2;
                        break;
                }

                // if we're not wrapping then we will usually also have move orders
                if (chunk_info[chunk_index].start_pos != chunk_info.back().start_pos) {
                    SPSVGLength const *reoriented_y = &chunk_info[chunk_index].start_pos.iter_span->y;
                    SPSVGLength const *reoriented_dy = &chunk_info[chunk_index].start_pos.iter_span->dy;
                    if (_block_progression == LEFT_TO_RIGHT || _block_progression == RIGHT_TO_LEFT) {
                        reoriented_y = &chunk_info[chunk_index].start_pos.iter_span->x;
                        reoriented_dy = &chunk_info[chunk_index].start_pos.iter_span->dx;
                    }
                    if (reoriented_y->set) {
                        // if this is the start of a line, we should change the baseline rather than each glyph individually
                        if (_flow._characters.empty() || _flow._characters.back().chunk(&_flow).in_line != _flow._lines.size() - 1) {
                            new_line.baseline_y = reoriented_y->computed;
                            _flow._lines.back().baseline_y = reoriented_y->computed;
                            _y_offset = 0.0;
                            _scanline_maker->setNewYCoordinate(reoriented_y->computed - _line.line_height.ascent);
                        } else
                            _y_offset = reoriented_y->computed - new_line.baseline_y;
                    }
                    if (reoriented_dy->set) _y_offset += reoriented_dy->computed;
                    if (chunk_info[chunk_index].start_pos.iter_span->rotate.set)
                        glyph_rotate = chunk_info[chunk_index].start_pos.iter_span->rotate.computed;
                }

            } else {   // back to the alignment adjustment stuff: we are using wrapping this time
                switch (_para.alignment) {
                    case FULL:
                        new_chunk.left_x = chunk_info[chunk_index].x;
                        if (span_pos.iter_span != _para.spans.end())    // don't justify the last line in the para
                            add_to_each_whitespace = (_line.scan_runs[chunk_index].width() - chunk_info[chunk_index].total_width) / chunk_info[chunk_index].whitespace_count;
                        break;
                    case LEFT:
                    default:
                        new_chunk.left_x = _line.scan_runs[chunk_index].x_start;
                        break;
                    case RIGHT:
                        new_chunk.left_x = _line.scan_runs[chunk_index].x_end - chunk_info[chunk_index].total_width;
                        break;
                    case CENTER:
                        new_chunk.left_x = (_line.scan_runs[chunk_index].x_start + _line.scan_runs[chunk_index].x_end - chunk_info[chunk_index].total_width) / 2;
                        break;
                }
            }
            new_chunk.baseline_shift = _y_offset;
            _flow._chunks.push_back(new_chunk);
            // if the chunk doesn't contain any text we just added a chunk with no spans
            // this is necessary for line breaks, which get added to the chunk at the bottom of Calculate(),
            // but may prove to be a problem in future code for redundant chunks in the middle of a line

            if (chunk_info[chunk_index].start_pos == chunk_info[chunk_index + 1].start_pos)
                continue;

            // begin adding spans to the list
            Direction previous_direction = _para.direction;
            double counter_directional_width_remaining = 0.0;
            double x;
            double direction_sign;

            if (_para.direction == LEFT_TO_RIGHT) {
                direction_sign = +1.0;
                x = 0.0;
            } else {
                direction_sign = -1.0;
                if (_para.alignment == FULL && !_flow._input_wrap_shapes.empty())
                    x = _line.scan_runs[chunk_index].width();
                else
                    x = chunk_info[chunk_index].total_width;
            }

            std::vector<IntermediateSpanStorage>::const_iterator iter_span;
            bool last_glyph_was_soft_hyphen = false;
            Layout::Glyph soft_hyphen_glyph;
            for (iter_span = chunk_info[chunk_index].start_pos.iter_span ; iter_span != chunk_info[chunk_index+1].start_pos.iter_span + 1; iter_span++) {
                if (iter_span == chunk_info[chunk_index+1].start_pos.iter_span && chunk_info[chunk_index+1].start_pos.char_byte == 0)
                    break;

                if (_flow._input_stream[iter_span->input_index]->Type() == TEXT_SOURCE && iter_span->pango_item_index == -1) {
                    // style only, nothing to output
                    continue;
                }

                Layout::Span new_span;
                bool block_direction_orthogonal = false;
                double x_in_span = 0.0;

                new_span.in_chunk = _flow._chunks.size() - 1;
                new_span.line_height = iter_span->line_height;      // except for orthogonal block-progression, done below
                new_span.in_input_stream_item = iter_span->input_index;
                new_span.x_start = x;
                if (_flow._input_stream[iter_span->input_index]->Type() == TEXT_SOURCE && iter_span->pango_item_index != -1) {
                    InputStreamTextSource const *text_source = static_cast<InputStreamTextSource const *>(_flow._input_stream[iter_span->input_index]);

                    new_span.font = _para.pango_items[iter_span->pango_item_index].font;
                    new_span.font->Ref();
                    new_span.font_size = iter_span->font_size;
                    new_span.direction = _para.pango_items[iter_span->pango_item_index].item->analysis.level & 1 ? RIGHT_TO_LEFT : LEFT_TO_RIGHT;
                    new_span.block_progression = text_source->styleComputeBlockProgression();
                    if (iter_span == chunk_info[chunk_index].start_pos.iter_span)    // the first span broke across the chunk
                        new_span.input_stream_first_character = Glib::ustring::const_iterator(iter_span->input_stream_first_character.base() + chunk_info[chunk_index].start_pos.char_byte);
                    else 
                        new_span.input_stream_first_character = iter_span->input_stream_first_character;
                } else {  // a control code
                    new_span.font = NULL;
                    new_span.font_size = new_span.line_height.ascent + new_span.line_height.descent;
                    new_span.direction = _para.direction;
                    new_span.block_progression = _block_progression;
                }

                if (Layout::_directions_are_orthogonal(new_span.block_progression, _block_progression)) {
                    block_direction_orthogonal = true;
                    new_span.line_height.ascent = new_span.line_height.descent = chunk_info[chunk_index].span_widths[iter_span] * 0.5;
                    new_span.line_height.leading = 0.0;
                } else {
                    if (new_span.direction == _para.direction)
                        counter_directional_width_remaining = 0.0;
                    else if (new_span.direction != previous_direction) {
                        // measure width of spans we need to switch round
                        counter_directional_width_remaining = 0.0;
                        std::vector<IntermediateSpanStorage>::const_iterator iter_following_span;
                        for (iter_following_span = iter_span ; ; iter_following_span++) {
                            if (iter_following_span == chunk_info[chunk_index+1].start_pos.iter_span && chunk_info[chunk_index+1].start_pos.char_byte == 0) break;
                            if (iter_following_span == chunk_info[chunk_index+1].start_pos.iter_span + 1) break;
                            Layout::Direction following_span_progression = static_cast<InputStreamTextSource const *>(_flow._input_stream[iter_following_span->input_index])->styleComputeBlockProgression();
                            if (!Layout::_directions_are_orthogonal(following_span_progression, _block_progression)) {
                                if (iter_following_span->pango_item_index == -1) {   // when the span came from a control code
                                    if (new_span.direction != _para.direction) break;
                                } else
                                    if (new_span.direction != (_para.pango_items[iter_following_span->pango_item_index].item->analysis.level & 1 ? RIGHT_TO_LEFT : LEFT_TO_RIGHT)) break;
                            }
                            counter_directional_width_remaining += direction_sign * chunk_info[chunk_index].span_widths[iter_following_span];
                        }
                        x += counter_directional_width_remaining;
                        counter_directional_width_remaining = 0.0;    // we want to go increasingly negative
                    }
                }

                if (_flow._input_stream[iter_span->input_index]->Type() == TEXT_SOURCE) {
                    // the span is set up, push the glyphs and chars
                    InputStreamTextSource const *text_source = static_cast<InputStreamTextSource const *>(_flow._input_stream[iter_span->input_index]);
                    int glyph_index = 0;
                    Glib::ustring::const_iterator iter_source_text = iter_span->input_stream_first_character;
                    int char_index_in_span = 0;
                    double font_size_multiplier = new_span.font_size / (PANGO_SCALE * _font_factory_size_multiplier);

                    if (iter_span == chunk_info[chunk_index].start_pos.iter_span) {   // the first span broke across the line
                        // skip glyphs until we get to the break point
                        iter_source_text = Glib::ustring::const_iterator(iter_source_text.base() + chunk_info[chunk_index].start_pos.char_byte);
                        while (glyph_index < iter_span->glyph_string->num_glyphs
                               && iter_span->glyph_string->log_clusters[glyph_index] < (int)chunk_info[chunk_index].start_pos.char_byte)
                            glyph_index++;
                    }
                    while (glyph_index < iter_span->glyph_string->num_glyphs) {
                        unsigned char_byte = iter_source_text.base() - iter_span->input_stream_first_character.base();
                        if (iter_span == chunk_info[chunk_index+1].start_pos.iter_span) {
                            if (char_byte == chunk_info[chunk_index+1].start_pos.char_byte)
                                break;   // the last span ends early
                            Glib::ustring::const_iterator iter_source_text_next = iter_source_text;
                        }
                        Layout::Glyph new_glyph;
                        new_glyph.glyph = iter_span->glyph_string->glyphs[glyph_index].glyph;
                        new_glyph.in_character = _flow._characters.size();
                        new_glyph.rotation = glyph_rotate;
                        if (block_direction_orthogonal) {
                            new_glyph.x = x + counter_directional_width_remaining + iter_span->glyph_string->glyphs[glyph_index].geometry.x_offset * font_size_multiplier;
                            new_glyph.y = _y_offset + _line.line_height.descent - x_in_span + iter_span->glyph_string->glyphs[glyph_index].geometry.y_offset * font_size_multiplier;
                        } else {
                            new_glyph.x = x + counter_directional_width_remaining + iter_span->glyph_string->glyphs[glyph_index].geometry.x_offset * font_size_multiplier;
                            new_glyph.y = _y_offset + iter_span->glyph_string->glyphs[glyph_index].geometry.y_offset * font_size_multiplier;
                        }
                        if (new_span.block_progression == LEFT_TO_RIGHT || new_span.block_progression == RIGHT_TO_LEFT) {
                            new_glyph.x += new_span.line_height.ascent;
                            new_glyph.y -= iter_span->glyph_string->glyphs[glyph_index].geometry.width * font_size_multiplier * 0.5;
                            new_glyph.width = new_span.line_height.ascent + new_span.line_height.descent;
                        } else
                            new_glyph.width = iter_span->glyph_string->glyphs[glyph_index].geometry.width * font_size_multiplier;
                        if (new_span.direction == RIGHT_TO_LEFT) new_glyph.x -= new_glyph.width;

                        if (iter_span->glyph_string->log_clusters[glyph_index] < (int)iter_span->text_bytes
                            && *iter_source_text == UNICODE_SOFT_HYPHEN) {
                            // if we're looking at a soft hyphen we don't draw the glyph
                            //   but we still need to add to _characters
                            // we *do* need to draw it if it was the last in the chunk. We'll correct ourselves later
                            //   (at the end of the chunk loop)
                            Layout::Character new_character;
                            new_character.in_span = _flow._spans.size();     // the span hasn't been added yet, so no -1
                            new_character.char_attributes = _para.char_attributes[iter_span->char_index_in_para + char_index_in_span];
                            new_character.in_glyph = -1;
                            _flow._characters.push_back(new_character);
                            iter_source_text++;
                            char_index_in_span++;
                            while (glyph_index < iter_span->glyph_string->num_glyphs
                                   && iter_span->glyph_string->log_clusters[glyph_index] == (int)char_byte)
                                glyph_index++;
                            last_glyph_was_soft_hyphen = true;
                            soft_hyphen_glyph = new_glyph;    // in case we need to add this glyph back again
                            glyph_rotate = 0.0;
                            continue;
                        }
                        last_glyph_was_soft_hyphen = false;
                        _flow._glyphs.push_back(new_glyph);

                        double advance_width = new_glyph.width;
                        unsigned end_byte;
                        if (glyph_index == iter_span->glyph_string->num_glyphs - 1)
                            end_byte = iter_span->text_bytes;
                        else end_byte = iter_span->glyph_string->log_clusters[glyph_index + 1];
                        while (char_byte < end_byte) {
                            Layout::Character new_character;
                            new_character.in_span = _flow._spans.size();
                            new_character.x = x_in_span;
                            new_character.char_attributes = _para.char_attributes[iter_span->char_index_in_para + char_index_in_span];
                            new_character.in_glyph = _flow._glyphs.size() - 1;
                            _flow._characters.push_back(new_character);
                            if (new_character.char_attributes.is_white)
                                advance_width += text_source->style->word_spacing.computed + add_to_each_whitespace;    // justification
                            if (new_character.char_attributes.is_cursor_position)
                                advance_width += text_source->style->letter_spacing.computed;
                            iter_source_text++;
                            char_index_in_span++;
                            char_byte = iter_source_text.base() - iter_span->input_stream_first_character.base();
                            glyph_rotate = 0.0;    // only the first glyph is rotated because of how we split the spans
                        }

                        if (new_span.direction != _para.direction) {
                            counter_directional_width_remaining -= direction_sign * advance_width;
                            x_in_span -= direction_sign * advance_width;
                        } else {
                            x += direction_sign * advance_width;
                            x_in_span += direction_sign * advance_width;
                        }
                        glyph_index++;
                    }
                } else if (_flow._input_stream[iter_span->input_index]->Type() == CONTROL_CODE) {
                    x += static_cast<InputStreamControlCode const *>(_flow._input_stream[iter_span->input_index])->width;
                    last_glyph_was_soft_hyphen = false;
                }

                new_span.x_end = new_span.x_start + x_in_span;
                _flow._spans.push_back(new_span);
                previous_direction = new_span.direction;
            }
            if (last_glyph_was_soft_hyphen) {
                _flow._characters.back().in_glyph = _flow._glyphs.size();
                _flow._glyphs.push_back(soft_hyphen_glyph);
            }
            // end adding spans to the list, on to the next chunk...
        }
        TRACE("output done");
        *start_span_pos = chunk_info.back().start_pos;
        return true;
    }

    inline const PangoLogAttr& _charAttributes(SpanPosition const &span_pos) const
    {
        return _para.char_attributes[span_pos.iter_span->char_index_in_para + span_pos.char_index];
    }

    /** Find the SpanPosition where the next chunk after this one should go
    if we're trying to fit on the given scan run. Also fills in the
    \a chunk_info. Returns early if it encounters a span with a x/y/dx/dy/rotate
    attribute set. Returns false if the baseline has to be moved and the
    ScanlineMaker indicates a recalculate is required.
    Input: loads of stuff
    Output: \a start_span_pos pointing to the end of the text used, all of
       \a chunk_info filled. */
    bool _measureChunk(SpanPosition *start_span_pos,
                       ScanlineMaker::ScanRun const &scan_run,
                       IntermediateChunkStorage *chunk_info)
    {
        double available_width = scan_run.width();
        IntermediateChunkStorage current_pos, last_break_pos;
        double width_at_last_span_start = 0.0;

        chunk_info->start_pos = *start_span_pos;
        chunk_info->total_width = 0.0;
        chunk_info->whitespace_count = 0;
        chunk_info->x = scan_run.x_start;
        current_pos.start_pos = *start_span_pos;
        current_pos.whitespace_count = 0;
        current_pos.total_width = 0.0;
        last_break_pos = current_pos;

        TRACE("trying chunk from %f to %g", scan_run.x_start, scan_run.x_end);
        for ( ; current_pos.start_pos.iter_span != _para.spans.end() ; ) {

            // force a chunk change at attribute change
            if (   current_pos.start_pos.iter_span->x.set || current_pos.start_pos.iter_span->y.set
                || current_pos.start_pos.iter_span->dx.set || current_pos.start_pos.iter_span->dy.set
                || current_pos.start_pos.iter_span->rotate.set) {

                if (current_pos.start_pos.iter_span != start_span_pos->iter_span) {
                    last_break_pos = current_pos;
                    break;
                }

                // beginning of the current chunk, use the applied x attributes
                SPSVGLength const *reoriented_x = &current_pos.start_pos.iter_span->x;
                SPSVGLength const *reoriented_dx = &current_pos.start_pos.iter_span->dx;
                if (_block_progression == LEFT_TO_RIGHT || _block_progression == RIGHT_TO_LEFT) {
                    reoriented_x = &current_pos.start_pos.iter_span->y;
                    reoriented_dx = &current_pos.start_pos.iter_span->dy;
                }
                if (reoriented_x->set) chunk_info->x = reoriented_x->computed;
                if (reoriented_dx->set) chunk_info->x += reoriented_dx->computed;
            }

            width_at_last_span_start = current_pos.total_width;
            // see if this span is too tall to fit on the current line
            if (   current_pos.start_pos.iter_span->line_height.ascent  > _line.line_height.ascent
                || current_pos.start_pos.iter_span->line_height.descent > _line.line_height.descent
                || current_pos.start_pos.iter_span->line_height.leading > _line.line_height.leading) {
                _line.line_height.max(current_pos.start_pos.iter_span->line_height);
                if (!_scanline_maker->canExtendCurrentScanline(_line.line_height))
                    return false;
            }

            // if this is a style-only span there's no text in it: go to next
            if (current_pos.start_pos.iter_span->pango_item_index == -1) {
                current_pos.start_pos.iter_span++;
                continue;
            }


            if (_flow._input_stream[current_pos.start_pos.iter_span->input_index]->Type() == CONTROL_CODE) {
                InputStreamControlCode const *control_code = static_cast<InputStreamControlCode const *>(_flow._input_stream[current_pos.start_pos.iter_span->input_index]);
                if (control_code->code == SHAPE_BREAK
                    || control_code->code == PARAGRAPH_BREAK) {
                    last_break_pos = current_pos;
                    break;  // heh. In this function, 'break' means line break
                }
                if (control_code->code == ARBITRARY_GAP) {
                    if (current_pos.total_width + control_code->width > available_width)
                        break;
                    TRACE("fitted control code, width = %f", control_code->width);
                    current_pos.total_width += control_code->width;
                    chunk_info->span_widths[current_pos.start_pos.iter_span] = control_code->width;
                    current_pos.start_pos.increment();
                }

            } else if (_flow._input_stream[current_pos.start_pos.iter_span->input_index]->Type() == TEXT_SOURCE) {
                InputStreamTextSource const *text_source = static_cast<InputStreamTextSource const *>(_flow._input_stream[current_pos.start_pos.iter_span->input_index]);
                double font_size_multiplier = current_pos.start_pos.iter_span->font_size / (PANGO_SCALE * _font_factory_size_multiplier);
                Direction span_block_progression = text_source->styleComputeBlockProgression();

                if (current_pos.start_pos.char_byte == 0
                    && _directions_are_orthogonal(_block_progression, span_block_progression)) {
                    // this is the supreme weirdness of block progression alteration. See the diagram in CSS3 section 3.2
                    // TODO: break this out into a subflow and use an ARBITRARY_GAP which is inserted later
                    double span_width = 0.0;
                    last_break_pos = current_pos;
                    for (int glyph_index = 0 ; glyph_index < current_pos.start_pos.iter_span->glyph_string->num_glyphs ; glyph_index++)
                        span_width += current_pos.start_pos.iter_span->glyph_string->glyphs[glyph_index].geometry.width * font_size_multiplier;
                    if (   span_width > _line.line_height.ascent * 2
                        || span_width > _line.line_height.descent * 2) {
                        _line.line_height.ascent  = std::max(_line.line_height.ascent,  span_width * 0.5);
                        _line.line_height.descent = std::max(_line.line_height.descent, span_width * 0.5);
                        if (!_scanline_maker->canExtendCurrentScanline(_line.line_height))
                            return false;
                    }
                    span_width = current_pos.start_pos.iter_span->line_height.total();
                    current_pos.total_width += span_width;
                    if (current_pos.total_width  > available_width)
                        break;
                    last_break_pos = current_pos;
                    chunk_info->span_widths[current_pos.start_pos.iter_span] = span_width;
                    TRACE("block-prog span %d width = %f", current_pos.start_pos.iter_span - _para.spans.begin(), span_width);
                } else {
                    // a normal span going with a normal block-progression
                    double soft_hyphen_glyph_width = 0.0;
                    bool soft_hyphen_in_word = false;
                    bool is_soft_hyphen = false;
                    int glyph_index = 0;
                    bool force_break_now = false;
                    IFDEBUG(int char_count = 0);

                    // if we're not at the start of the span we need to pre-init glyph_index
                    while (glyph_index < current_pos.start_pos.iter_span->glyph_string->num_glyphs
                           && current_pos.start_pos.iter_span->glyph_string->log_clusters[glyph_index] < (int)current_pos.start_pos.char_byte)
                        glyph_index++;

                    // go char-by-char summing the width, while keeping track of the previous break point
                    do {
                        PangoLogAttr const &char_attributes = _charAttributes(current_pos.start_pos);
                        if (char_attributes.is_mandatory_break) {
                            last_break_pos = current_pos;
                            force_break_now = true;
                            break;
                        }
                        if (char_attributes.is_line_break || char_attributes.is_white || is_soft_hyphen) {
                            last_break_pos = current_pos;
                            if (soft_hyphen_in_word) {
                                current_pos.total_width -= soft_hyphen_glyph_width;
                                if (!is_soft_hyphen)
                                    soft_hyphen_in_word = false;
                            }
                        }
                        // todo: break between chars if necessary (ie no word breaks present) when doing rectangular flowing

                        double char_width = 0.0;
                        if (span_block_progression == LEFT_TO_RIGHT || span_block_progression == RIGHT_TO_LEFT) {
                            char_width = current_pos.start_pos.iter_span->line_height.ascent + current_pos.start_pos.iter_span->line_height.descent;
                        } else {
                            while (glyph_index < current_pos.start_pos.iter_span->glyph_string->num_glyphs
                                   && current_pos.start_pos.iter_span->glyph_string->log_clusters[glyph_index] <= (int)current_pos.start_pos.char_byte) {
                                char_width += current_pos.start_pos.iter_span->glyph_string->glyphs[glyph_index].geometry.width;
                                glyph_index++;
                            }
                        }
                        char_width *= font_size_multiplier;
                        if (char_attributes.is_cursor_position)
                            char_width += text_source->style->letter_spacing.computed;
                        if (char_attributes.is_white)
                            char_width += text_source->style->word_spacing.computed;
                        IFDEBUG(char_count++);
                        current_pos.total_width += char_width;
                        if (current_pos.total_width > available_width) {
                            force_break_now = true;
                            break;
                        }
                        if (char_attributes.is_white)
                            current_pos.whitespace_count++;
                        is_soft_hyphen = (UNICODE_SOFT_HYPHEN == *Glib::ustring::const_iterator(current_pos.start_pos.iter_span->input_stream_first_character.base() + current_pos.start_pos.char_byte));
                        if (is_soft_hyphen)
                            soft_hyphen_glyph_width = char_width;

                        current_pos.start_pos.increment();
                    } while (current_pos.start_pos.char_byte != 0);  // while we haven't wrapped to the next span
                    TRACE("fitted span %d width = %f chars = %d", current_pos.start_pos.iter_span - _para.spans.begin() - (current_pos.start_pos.char_byte == 0 ? 1 : 0), current_pos.total_width - width_at_last_span_start, char_count);
                    chunk_info->span_widths[current_pos.start_pos.iter_span - (current_pos.start_pos.char_byte == 0 ? 1 : 0)] = current_pos.total_width - width_at_last_span_start;   // this is wrong if we're just about to break: we'll fix that case at the end
                    if (force_break_now) break;
                }
            }
        }
        if (current_pos.start_pos.iter_span == _para.spans.end())
            last_break_pos = current_pos;
        else if (!_flow._input_wrap_shapes.empty()
                 && _flow._input_stream[last_break_pos.start_pos.iter_span->input_index]->Type() == TEXT_SOURCE
                 && _charAttributes(last_break_pos.start_pos).is_white)
            last_break_pos.start_pos.increment();     // if we have one extra whitespace on the end of the line we can swallow it
        TRACE("correction: fitted span %d width = %f", last_break_pos.start_pos.iter_span - _para.spans.begin() - (last_break_pos.start_pos.char_byte == 0 ? 1 : 0), last_break_pos.total_width - width_at_last_span_start);
        chunk_info->span_widths[last_break_pos.start_pos.iter_span] = last_break_pos.total_width - width_at_last_span_start;
                       // when the last span was cut off we set its width wrongly (we didn't know there were any more spans)
        *start_span_pos = last_break_pos.start_pos;
        chunk_info->total_width = last_break_pos.total_width;
        chunk_info->whitespace_count = last_break_pos.whitespace_count;
        TRACE("chunk complete, used %f width (%d whitespaces)", chunk_info->total_width, chunk_info->whitespace_count);
        return true;
    }

    bool _goToNextWrapShape()
    {
        delete _scanline_maker;
        _scanline_maker = NULL;
        _line.shape_index++;
        if (_line.shape_index == (int)_flow._input_wrap_shapes.size()) return false;
        _scanline_maker = new ShapeScanlineMaker(_flow._input_wrap_shapes[_line.shape_index].shape, _block_progression);
        TRACE("begin wrap shape %d", _line.shape_index);
        return true;
    }

public:
    Calculator(Layout *text_flow)
        : _flow(*text_flow) {}

    /** The management function to start the whole thing off. */
    bool Calculate()
    {
        if (_flow._input_stream.empty())
            return false;
        g_assert(_flow._input_stream.front()->Type() == TEXT_SOURCE);
        if (_flow._input_stream.front()->Type() != TEXT_SOURCE)
            return false;

        TRACE("begin calculateFlow()");

        _flow._clearOutputObjects();
        
        _pango_context = (font_factory::Default())->fontContext;
        _font_factory_size_multiplier = (font_factory::Default())->fontSize;

        _line.shape_index = 0;

        _y_offset = 0.0;
        _block_progression = _flow._blockProgression();

        if (_flow._input_wrap_shapes.empty()) {
            // create the special no-wrapping infinite scanline maker
            double initial_x = 0, initial_y = 0;
            InputStreamTextSource const *text_source = static_cast<InputStreamTextSource const *>(_flow._input_stream.front());
            if (!text_source->x.empty())
                initial_x = text_source->x.front().computed;
            if (!text_source->y.empty())
                initial_y = text_source->y.front().computed;
            _scanline_maker = new InfiniteScanlineMaker(initial_x, initial_y, _block_progression);
            TRACE("  wrapping disabled");
        }
        else {
            _scanline_maker = new ShapeScanlineMaker(_flow._input_wrap_shapes[_line.shape_index].shape, _block_progression);
            TRACE("  begin wrap shape 0");
        }

        _input_items.resize(_flow._input_stream.size());    // the constructor is sufficient initialisation

        for(_para.first_input_index = 0 ; _para.first_input_index < _flow._input_stream.size() ; ) {
            // jump to the next wrap shape if this is a SHAPE_BREAK control code
            if (_flow._input_stream[_para.first_input_index]->Type() == CONTROL_CODE) {
                InputStreamControlCode const *control_code = static_cast<InputStreamControlCode const *>(_flow._input_stream[_para.first_input_index]);
                if (control_code->code == SHAPE_BREAK) {
                    TRACE("shape break control code");
                    if (!_goToNextWrapShape()) break;
                    continue;
                }
            }
            if (_scanline_maker == NULL)
                break;       // we're trying to flow past the last wrap shape

            unsigned para_end_input_index;
            _buildPangoItemizationForPara();
            _buildSpansForPara(&para_end_input_index);

            if (_flow._input_stream[_para.first_input_index]->Type() == TEXT_SOURCE)
                _para.alignment = static_cast<InputStreamTextSource*>(_flow._input_stream[_para.first_input_index])->styleGetAlignment(_para.direction);
            else
                _para.alignment = _para.direction == LEFT_TO_RIGHT ? LEFT : RIGHT;

            TRACE("para prepared, adding as #%d", _flow._paragraphs.size());
            Layout::Paragraph new_paragraph;
            new_paragraph.base_direction = _para.direction;
            _flow._paragraphs.push_back(new_paragraph);

            // start scanning lines
            SpanPosition span_pos;
            span_pos.iter_span = _para.spans.begin();
            span_pos.char_byte = 0;
            span_pos.char_index = 0;

            for ( ; ; ) {   // for each line in the paragraph
                TRACE("begin line");

                // init the initial line_height
                if (span_pos.iter_span == _para.spans.end()) {
                    if (_flow._spans.empty()) {
                        // empty first para: create a font for the sole purpose of measuring it
                        InputStreamTextSource const *text_source = static_cast<InputStreamTextSource const *>(_flow._input_stream.front());
                        font_instance *font = text_source->styleGetFontInstance();
                        if (font) {
                            double font_size = text_source->styleComputeFontSize();
                            font->FontMetrics(_line.line_height.ascent, _line.line_height.descent, _line.line_height.leading);
                            font->Unref();
                            _line.line_height.ascent *= font_size;
                            _line.line_height.descent *= font_size;
                            _line.line_height.leading *= font_size;
                            double initial_y = 0.0;
                            if (_block_progression == LEFT_TO_RIGHT || _block_progression == RIGHT_TO_LEFT) {
                                if (!text_source->x.empty())
                                    initial_y = text_source->x.front().computed;
                            } else {
                                if (!text_source->y.empty())
                                    initial_y = text_source->y.front().computed;
                            }
                            _scanline_maker->setNewYCoordinate(initial_y - _line.line_height.ascent);
                        } else {
                            _line.line_height.ascent = 0.0;
                            _line.line_height.descent = 0.0;
                            _line.line_height.leading = 0.0;
                        }
                    } else    // empty subsequent para
                        _line.line_height = _flow._spans.back().line_height;
                } else
                    _line.line_height = span_pos.iter_span->line_height;

                // keep trying to create a line until the line height stops changing
                do {
                    _line.scan_runs = _scanline_maker->makeScanline(_line.line_height);
                    while (_line.scan_runs.empty()) {
                        if (!_goToNextWrapShape()) break;
                        _line.scan_runs = _scanline_maker->makeScanline(_line.line_height);
                    }
                    if (_scanline_maker == NULL) break;
                } while (!_outputLineThatFits(&span_pos));

                if (_scanline_maker == NULL) break;
                _scanline_maker->completeLine();
                if (span_pos.iter_span == _para.spans.end()) break;
            }

            TRACE("para %d end\n", _flow._paragraphs.size() - 1);
            if (_scanline_maker != NULL) {
                bool is_empty_last_para = para_end_input_index + 1 >= _flow._input_stream.size()
                                          && (_flow._characters.empty()
                                              || _flow._characters.back().line(&_flow).in_paragraph != _flow._paragraphs.size() - 1);
                if (is_empty_last_para || para_end_input_index + 1 < _flow._input_stream.size()) {
                    // we need a span just for the para if it's either an empty last para or a break in the middle
                    Layout::Span new_span;
                    if (_flow._spans.empty()) {
                        new_span.font = NULL;
                        new_span.font_size = _line.line_height.ascent + _line.line_height.descent;
                        new_span.line_height = _line.line_height;
                        new_span.x_end = 0.0;
                    } else {
                        new_span = _flow._spans.back();
                        if (_flow._chunks[new_span.in_chunk].in_line != _flow._lines.size() - 1)
                            new_span.x_end = 0.0;
                    }
                    new_span.in_chunk = _flow._chunks.size() - 1;
                    if (new_span.font)
                        new_span.font->Ref();
                    new_span.x_start = new_span.x_end;
                    new_span.direction = _para.direction;
                    new_span.block_progression = _block_progression;
                    if (para_end_input_index == _flow._input_stream.size())
                        new_span.in_input_stream_item = _flow._input_stream.size() - 1;
                    else
                        new_span.in_input_stream_item = para_end_input_index;
                    _flow._spans.push_back(new_span);
                }
                if (para_end_input_index + 1 < _flow._input_stream.size()) {
                    // we've got to add an invisible character between paragraphs so that we can position iterators
                    // (and hence cursors) both before and after the paragraph break
                    Layout::Character new_character;
                    new_character.in_span = _flow._spans.size() - 1;
                    new_character.char_attributes.is_line_break = 1;
                    new_character.char_attributes.is_mandatory_break = 1;
                    new_character.char_attributes.is_char_break = 1;
                    new_character.char_attributes.is_white = 1;
                    new_character.char_attributes.is_cursor_position = 1;
                    new_character.char_attributes.is_word_start = 0;
                    new_character.char_attributes.is_word_end = 0;
                    new_character.char_attributes.is_sentence_start = 0;
                    new_character.char_attributes.is_sentence_end = 0;
                    new_character.char_attributes.is_sentence_boundary = 1;
                    new_character.char_attributes.backspace_deletes_character = 1;
                    new_character.x = _flow._spans.back().x_end - _flow._spans.back().x_start;
                    new_character.in_glyph = -1;
                    _flow._characters.push_back(new_character);
                }
            }
            _para.first_input_index = para_end_input_index + 1;
        }
        if (_scanline_maker)
            delete _scanline_maker;
        
        for (std::vector<IntermediateInputItemStorage>::iterator it = _input_items.begin() ; it != _input_items.end() ; it++) {
            if (it->sub_flow) delete it->sub_flow;
        }
        return true;
    }
};

bool Layout::calculateFlow()
{
    return Calculator(this).Calculate();
}

}//namespace Text
}//namespace Inkscape
