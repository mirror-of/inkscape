// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file
 * Bitmap tracing settings dialog - second implementation.
 */
/* Authors:
 *   Marc Jeanmougin <marc.jeanmougin@telecom-paristech.fr>
 *
 * Copyright (C) 2019 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "tracedialog.h"

#include <glibmm/i18n.h>
#include <gtkmm.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/notebook.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/stack.h>

#include "desktop.h"
#include "display/cairo-utils.h"
#include "inkscape.h"
#include "io/resource.h"
#include "io/sys.h"
#include "selection.h"
#include "trace/autotrace/inkscape-autotrace.h"
#include "trace/depixelize/inkscape-depixelize.h"
#include "trace/potrace/inkscape-potrace.h"
#include "ui/util.h"

// This maps the column ids in the glade file to useful enums
static const std::map<std::string, Inkscape::Trace::Potrace::TraceType> trace_types = {
    {"SS_BC", Inkscape::Trace::Potrace::TRACE_BRIGHTNESS},
    {"SS_ED", Inkscape::Trace::Potrace::TRACE_CANNY},
    {"SS_CQ", Inkscape::Trace::Potrace::TRACE_QUANT},
    {"SS_AT", Inkscape::Trace::Potrace::AUTOTRACE_SINGLE},
    {"SS_CT", Inkscape::Trace::Potrace::AUTOTRACE_CENTERLINE},

    {"MS_BS", Inkscape::Trace::Potrace::TRACE_BRIGHTNESS_MULTI},
    {"MS_C", Inkscape::Trace::Potrace::TRACE_QUANT_COLOR},
    {"MS_BW", Inkscape::Trace::Potrace::TRACE_QUANT_MONO},
    {"MS_AT", Inkscape::Trace::Potrace::AUTOTRACE_MULTI},
};

namespace Inkscape {
namespace UI {
namespace Dialog {

class TraceDialogImpl2 : public TraceDialog {
  public:
    TraceDialogImpl2();
    ~TraceDialogImpl2() override;

    void selectionModified(Selection *selection, guint flags) override;
    void selectionChanged(Inkscape::Selection *selection) override;
private:
    Inkscape::Trace::Tracer tracer;
    void traceProcess(bool do_i_trace);
    void abort();

    void previewCallback(bool force);
    bool previewResize(const Cairo::RefPtr<Cairo::Context>&);
    void traceCallback();
    void onSetDefaults();
    void show_hide_params();
    void schedule_preview_update();
    static gboolean update_cb(gpointer user_data);

    Glib::RefPtr<Gtk::Builder> builder;

    Glib::RefPtr<Gtk::Adjustment> MS_scans, PA_curves, PA_islands, PA_sparse1, PA_sparse2, SS_AT_ET_T, SS_AT_FI_T, SS_BC_T, SS_CQ_T,
        SS_ED_T, optimize, smooth, speckles;
    Gtk::ComboBoxText *CBT_SS, *CBT_MS;
    Gtk::CheckButton *CB_invert, *CB_MS_smooth, *CB_MS_stack, *CB_MS_rb, *CB_speckles, *CB_smooth, *CB_optimize, *CB_PA_optimize,
        /* *CB_live,*/ *CB_SIOX;
    Gtk::CheckButton* CB_SIOX1;
    Gtk::CheckButton* CB_speckles1;
    Gtk::CheckButton* CB_smooth1;
    Gtk::CheckButton* CB_optimize1;
    Gtk::RadioButton *RB_PA_voronoi;
    Gtk::Button *B_RESET, *B_STOP, *B_OK, *B_Update;
    Gtk::Box *mainBox;
    Gtk::Notebook *choice_tab;
    Glib::RefPtr<Gdk::Pixbuf> scaledPreview;
    Gtk::DrawingArea *previewArea;
    Gtk::Box* orient_box;
    Gtk::Frame* _preview_frame;
    Gtk::Grid* _param_grid;
    Gtk::CheckButton* _live_preview;
    guint _source = 0;
};

enum Page {
    SingleScan, MultiScan, PixelArt
};

void TraceDialogImpl2::traceProcess(bool do_i_trace)
{
    SPDesktop* desktop = getDesktop();
    if (desktop)
        desktop->setWaitingCursor();

    auto current_page = choice_tab->get_current_page();

    auto cb_siox = current_page == SingleScan ? CB_SIOX : CB_SIOX1;
    if (cb_siox->get_active())
        tracer.enableSiox(true);
    else
        tracer.enableSiox(false);

    Glib::ustring type = current_page == SingleScan ? CBT_SS->get_active_id() : CBT_MS->get_active_id();

    bool use_autotrace = false;
    Inkscape::Trace::Autotrace::AutotraceTracingEngine ate; // TODO

    auto potraceType = trace_types.find(type);
    assert(potraceType != trace_types.end());
    switch (potraceType->second) {
        case Inkscape::Trace::Potrace::AUTOTRACE_SINGLE:
            use_autotrace = true;
            ate.opts->color_count = 2;
            break;
        case Inkscape::Trace::Potrace::AUTOTRACE_CENTERLINE:
            use_autotrace = true;
            ate.opts->color_count = 2;
            ate.opts->centerline = true;
            ate.opts->preserve_width = true;
            break;
        case Inkscape::Trace::Potrace::AUTOTRACE_MULTI:
            use_autotrace = true;
            ate.opts->color_count = (int)MS_scans->get_value() + 1;
            break;
        default:
            break;
    }

    ate.opts->filter_iterations = (int) SS_AT_FI_T->get_value();
    ate.opts->error_threshold = SS_AT_ET_T->get_value();

    Inkscape::Trace::Potrace::PotraceTracingEngine pte(
        potraceType->second, CB_invert->get_active(), (int)SS_CQ_T->get_value(), SS_BC_T->get_value(),
        0., // Brightness floor
        SS_ED_T->get_value(), (int)MS_scans->get_value(), CB_MS_stack->get_active(), CB_MS_smooth->get_active(),
        CB_MS_rb->get_active());

    auto cb_optimize = current_page == SingleScan ? CB_optimize : CB_optimize1;
    pte.potraceParams->opticurve = cb_optimize->get_active();
    pte.potraceParams->opttolerance = optimize->get_value();

    auto cb_smooth = current_page == SingleScan ? CB_smooth : CB_smooth1;
    pte.potraceParams->alphamax = cb_smooth->get_active() ? smooth->get_value() : 0;

    auto cb_speckles = current_page == SingleScan ? CB_speckles : CB_speckles1;
    pte.potraceParams->turdsize = cb_speckles->get_active() ? (int)speckles->get_value() : 0;

    //Inkscape::Trace::Autotrace::AutotraceTracingEngine ate; // TODO
    Inkscape::Trace::Depixelize::DepixelizeTracingEngine dte(
        RB_PA_voronoi->get_active() ? Inkscape::Trace::Depixelize::TraceType::TRACE_VORONOI : Inkscape::Trace::Depixelize::TraceType::TRACE_BSPLINES,
        PA_curves->get_value(), (int) PA_islands->get_value(),
        (int) PA_sparse1->get_value(), PA_sparse2->get_value(),
        CB_PA_optimize->get_active());

    //TODO: preview for multiscan: grayscale bitmap with matching number of gray levels?
    // Currently there's one created using brightness threshold, which is wrong and misleading
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = tracer.getSelectedImage();
    if (pixbuf) {
        scaledPreview = use_autotrace ? ate.preview(pixbuf) : pte.preview(pixbuf);
    }
    else {
        scaledPreview.reset();
    }

    previewArea->queue_draw();

    if (do_i_trace){
        if (current_page == PixelArt){
            tracer.trace(&dte);
            printf("dt\n");
        } else if (use_autotrace) {
	          tracer.trace(&ate);
            printf("at\n");
        } else if (current_page == SingleScan || current_page == MultiScan) {
            tracer.trace(&pte);
            printf("pt\n");
        }
    }

    if (desktop)
        desktop->clearWaitingCursor();
}

bool TraceDialogImpl2::previewResize(const Cairo::RefPtr<Cairo::Context>& cr)
{
    /* Checkerboard - is not applicable here; left for reference
    if (auto wnd = dynamic_cast<Gtk::Window*>(this->get_toplevel())) {
        auto color = wnd->get_style_context()->get_background_color();
        auto background =
            gint32(0xff * color.get_red()) << 24 |
            gint32(0xff * color.get_green()) << 16 |
            gint32(0xff * color.get_blue()) << 8 |
            0xff;

        auto device_scale = get_scale_factor();
        Cairo::RefPtr<Cairo::Pattern> pattern(new Cairo::Pattern(ink_cairo_pattern_create_checkerboard(background)));
        cr->save();
        cr->scale(device_scale, device_scale);
        cr->set_operator(Cairo::OPERATOR_SOURCE);
        cr->set_source(pattern);
        cr->paint();
        cr->restore();
    } */

    if (scaledPreview) {
        int width = scaledPreview->get_width();
        int height = scaledPreview->get_height();
        const Gtk::Allocation &vboxAlloc = previewArea->get_allocation();
        double scaleFX = vboxAlloc.get_width() / (double)width;
        double scaleFY = vboxAlloc.get_height() / (double)height;
        double scaleFactor = scaleFX > scaleFY ? scaleFY : scaleFX;
        int newWidth = (int)(((double)width) * scaleFactor);
        int newHeight = (int)(((double)height) * scaleFactor);
        int offsetX = (vboxAlloc.get_width() - newWidth)/2;
        int offsetY = (vboxAlloc.get_height() - newHeight)/2;
        cr->scale(scaleFactor, scaleFactor);
        Gdk::Cairo::set_source_pixbuf(cr, scaledPreview, offsetX / scaleFactor, offsetY / scaleFactor);
        cr->paint();
    }
    else {
        cr->set_source_rgba(0, 0, 0, 0);
        cr->paint();
    }

    return false;
}

void TraceDialogImpl2::abort()
{
     SPDesktop *desktop = SP_ACTIVE_DESKTOP;
     if (desktop)
         desktop->clearWaitingCursor();
     tracer.abort();
}

void TraceDialogImpl2::selectionChanged(Inkscape::Selection *selection) {
    previewCallback(false);
}

void TraceDialogImpl2::selectionModified(Selection *selection, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG)) {
        previewCallback(false);
    }
}

void TraceDialogImpl2::onSetDefaults()
{
    MS_scans->set_value(8);
    PA_curves->set_value(1);
    PA_islands->set_value(5);
    PA_sparse1->set_value(4);
    PA_sparse2->set_value(1);
    SS_AT_FI_T->set_value(4);
    SS_AT_ET_T->set_value(2);
    SS_BC_T->set_value(0.45);
    SS_CQ_T->set_value(64);
    SS_ED_T->set_value(.65);
    optimize->set_value(0.2);
    smooth->set_value(1);
    speckles->set_value(2);
    CB_invert->set_active(false);
    CB_MS_smooth->set_active(true);
    CB_MS_stack->set_active(true);
    CB_MS_rb->set_active(false);
    CB_speckles->set_active(true);
    CB_smooth->set_active(true);
    CB_optimize->set_active(true);
    CB_speckles1->set_active(true);
    CB_smooth1->set_active(true);
    CB_optimize1->set_active(true);
    CB_PA_optimize->set_active(false);
    CB_SIOX->set_active(false);
    CB_SIOX1->set_active(false);
}

void TraceDialogImpl2::previewCallback(bool force) {
    if (force || (_live_preview->get_active() && is_widget_effectively_visible(this))) {
        traceProcess(false);
    }
}

void TraceDialogImpl2::traceCallback() { traceProcess(true); }


TraceDialogImpl2::TraceDialogImpl2()
    : TraceDialog()
{
    const std::string req_widgets[] = { "MS_scans",    "PA_curves", "PA_islands",  "PA_sparse1", "PA_sparse2",
                                        "SS_AT_FI_T", "SS_AT_ET_T",     "SS_BC_T",   "SS_CQ_T",     "SS_ED_T",
                                        "optimize",    "smooth",    "speckles",    "CB_invert",  "CB_MS_smooth",
                                        "CB_MS_stack", "CB_MS_rb",  "CB_speckles", "CB_smooth",  "CB_optimize",
                                        "CB_speckles1", "CB_smooth1",  "CB_optimize1", "CB_SIOX1",
                                        "CB_PA_optimize", /*"CB_live",*/ "CB_SIOX", "CBT_SS",    "CBT_MS",
                                        "B_RESET",     "B_STOP",    "B_OK",         "mainBox",   "choice_tab",
                                        /*"choice_scan",*/ "previewArea", "_live_preview" };
    auto gladefile = get_filename_string(Inkscape::IO::Resource::UIS, "dialog-trace.glade");
    try {
        builder = Gtk::Builder::create_from_file(gladefile);
    } catch (const Glib::Error &ex) {
        g_warning("Glade file loading failed for filter effect dialog");
        return;
    }

    Glib::RefPtr<Glib::Object> test;
    for (std::string w : req_widgets) {
        test = builder->get_object(w);
        if (!test) {
            g_warning("Required widget %s does not exist", w.c_str());
            return;
        }
    }

#define GET_O(name)                                                                                                    \
    tmp = builder->get_object(#name);                                                                                  \
    name = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(tmp);

    Glib::RefPtr<Glib::Object> tmp;

#define GET_W(name) builder->get_widget(#name, name);
    GET_O(MS_scans)
    GET_O(PA_curves)
    GET_O(PA_islands)
    GET_O(PA_sparse1)
    GET_O(PA_sparse2)
    GET_O(SS_AT_FI_T)
    GET_O(SS_AT_ET_T)
    GET_O(SS_BC_T)
    GET_O(SS_CQ_T)
    GET_O(SS_ED_T)
    GET_O(optimize)
    GET_O(smooth)
    GET_O(speckles)

    GET_W(CB_invert)
    GET_W(CB_MS_smooth)
    GET_W(CB_MS_stack)
    GET_W(CB_MS_rb)
    GET_W(CB_speckles)
    GET_W(CB_smooth)
    GET_W(CB_optimize)
    GET_W(CB_speckles1)
    GET_W(CB_smooth1)
    GET_W(CB_optimize1)
    GET_W(CB_PA_optimize)
    GET_W(CB_SIOX)
    GET_W(CB_SIOX1)
    GET_W(RB_PA_voronoi)
    GET_W(CBT_SS)
    GET_W(CBT_MS)
    GET_W(B_RESET)
    GET_W(B_STOP)
    GET_W(B_OK)
    GET_W(B_Update)
    GET_W(mainBox)
    GET_W(choice_tab)
    GET_W(previewArea)
    GET_W(orient_box)
    GET_W(_preview_frame)
    GET_W(_param_grid)
    GET_W(_live_preview)
#undef GET_W
#undef GET_O
    add(*mainBox);

    Inkscape::Preferences* prefs = Inkscape::Preferences::get();

    _live_preview->set_active(prefs->getBool(getPrefsPath() + "liveUpdate", true));

    B_Update->signal_clicked().connect([=](){ previewCallback(true); });
    B_OK->signal_clicked().connect(sigc::mem_fun(*this, &TraceDialogImpl2::traceCallback));
    B_STOP->signal_clicked().connect(sigc::mem_fun(*this, &TraceDialogImpl2::abort));
    B_RESET->signal_clicked().connect(sigc::mem_fun(*this, &TraceDialogImpl2::onSetDefaults));
    previewArea->signal_draw().connect(sigc::mem_fun(*this, &TraceDialogImpl2::previewResize));

    // attempt at making UI responsive: relocate preview to the right or bottom of dialog depending on dialog size
    this->signal_size_allocate().connect([=](const Gtk::Allocation& alloc){
        // skip bogus sizes
        if (alloc.get_width() < 10 || alloc.get_height() < 10) return;
        // ratio: is dialog wide or is it tall?
        double ratio = alloc.get_width() / static_cast<double>(alloc.get_height());
        // g_warning("size alloc: %d x %d - %f", alloc.get_width(), alloc.get_height(), ratio);
        const auto hysteresis = 0.01;
        if (ratio < 1 - hysteresis) {
            // narrow/tall
            choice_tab->set_valign(Gtk::ALIGN_START);
            orient_box->set_orientation(Gtk::ORIENTATION_VERTICAL);
        }
        else if (ratio > 1 + hysteresis) {
            // wide/short
            orient_box->set_orientation(Gtk::ORIENTATION_HORIZONTAL);
            choice_tab->set_valign(Gtk::ALIGN_FILL);
        }
    });

    CBT_SS->signal_changed().connect([=](){ show_hide_params(); });
    show_hide_params();

    // watch for changes, but only in params that can impact preview bitmap
    for (auto adj : {SS_BC_T, SS_ED_T, SS_CQ_T, SS_AT_FI_T, SS_AT_ET_T, /* optimize, smooth, speckles,*/ MS_scans, PA_curves, PA_islands, PA_sparse1, PA_sparse2 }) {
        adj->signal_value_changed().connect([=](){ schedule_preview_update(); });
    }
    for (auto checkbtn : {CB_invert, CB_MS_rb, /* CB_MS_smooth, CB_MS_stack, CB_optimize1, CB_optimize, */ CB_PA_optimize, CB_SIOX1, CB_SIOX, /* CB_smooth1, CB_smooth, CB_speckles1, CB_speckles, */ _live_preview}) {
        checkbtn->signal_toggled().connect([=](){ schedule_preview_update(); });
    }
    for (auto combo : {CBT_SS, CBT_MS}) {
        combo->signal_changed().connect([=](){ schedule_preview_update(); });
    }
    choice_tab->signal_switch_page().connect([=](Gtk::Widget*, guint){ schedule_preview_update(); });

    signal_set_focus_child().connect([=](Gtk::Widget* w){
        if (w) schedule_preview_update();
    });
}

TraceDialogImpl2::~TraceDialogImpl2() {
    Inkscape::Preferences* prefs = Inkscape::Preferences::get();
    prefs->setBool(getPrefsPath() + "liveUpdate", _live_preview->get_active());

    if (_source) {
        g_source_destroy(g_main_context_find_source_by_id(nullptr, _source));
    }
}

gboolean TraceDialogImpl2::update_cb(gpointer user_data) {
    auto self = static_cast<TraceDialogImpl2*>(user_data);
    self->previewCallback(false);
    self->_source = 0;
    return FALSE;
}

void TraceDialogImpl2::schedule_preview_update() {
    if (!_live_preview->get_active() || _source) return;

    _source = g_idle_add(&TraceDialogImpl2::update_cb, this);
}

void TraceDialogImpl2::show_hide_params() {
    int start_row = 2;
    int option = CBT_SS->get_active_row_number();
    if (option >= 3) option = 3;
    int show1 = start_row + option;
    int show2 = start_row + option;
    if (option >= 3) ++show2;

    for (int row = start_row; row < start_row + 5; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (auto widget = _param_grid->get_child_at(col, row)) {
                if (row == show1 || row == show2) widget->show(); else widget->hide();
            }
        }
    }
}

TraceDialog &TraceDialog::getInstance()
{
    TraceDialog *dialog = new TraceDialogImpl2();
    return *dialog;
}



} // namespace Dialog
} // namespace UI
} // namespace Inkscape
