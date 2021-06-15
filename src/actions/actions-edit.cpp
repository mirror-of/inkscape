#include <iostream>

#include <giomm.h>  // Not <gtkmm.h>! To eventually allow a headless version!
#include <glibmm/i18n.h>

#include "actions-edit.h"
#include "inkscape-application.h"
#include "inkscape-window.h"
#include "desktop.h"
#include "selection-chemistry.h"
#include "object/sp-guide.h"

void 
object_to_pattern(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->tile();
}

void 
pattern_to_object(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->untile();
}

void 
object_to_marker(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->toMarker();
}

void 
object_to_guides(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->toGuides();
}

void 
undo(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_undo(dt, dt->getDocument());
}

void 
redo(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_redo(dt, dt->getDocument());
}

void 
cut(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->cut();
}

void 
copy(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->copy();
}

void 
paste(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_selection_paste(dt, false);
}

void 
paste_in_place(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_selection_paste(dt, true);
}

void 
paste_style(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteStyle();
}

void 
paste_size(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSize(true,true);
}

void 
paste_width(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSize(true, false);
}

void 
paste_height(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSize(false, true);
}

void 
paste_size_separately(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSizeSeparately(true, true);
}

void 
paste_width_separately(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSizeSeparately(true, false);
}

void 
paste_height_separately(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pasteSizeSeparately(false, true);
}

void 
duplicate(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->duplicate();
}

void 
clone(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->clone();
}

void 
clone_unlink(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->unlink();
}

void 
clone_unlink_recursively(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->unlinkRecursive(false, true);
}

void 
clone_link(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->relink();
}

void 
select_original(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->cloneOriginal();
}

void 
clone_link_lpe(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->cloneOriginalPathLPE();
}

void 
edit_delete(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->deleteItems();
}

void 
select_all(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    Inkscape::SelectionHelper::selectAll(dt);
}

void 
select_all_layers(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    Inkscape::SelectionHelper::selectAllInAll(dt);
}

void 
select_same_fill_and_stroke(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    Inkscape::SelectionHelper::selectSameFillStroke(dt);
}

void 
select_same_fill(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    Inkscape::SelectionHelper::selectSameFillColor(dt);
}

void 
select_same_stroke_color(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    Inkscape::SelectionHelper::selectSameStrokeColor(dt);
}

void 
select_same_stroke_style(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    Inkscape::SelectionHelper::selectSameStrokeStyle(dt);
}

void 
select_same_object_type(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    Inkscape::SelectionHelper::selectSameObjectType(dt);
}

void 
select_invert(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    Inkscape::SelectionHelper::invert(dt);
}

void 
select_none(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    Inkscape::SelectionHelper::selectNone(dt);
}

void 
create_guides_around_page(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_guide_create_guides_around_page(dt);
}

void
lock_all_guides(InkscapeWindow *win)
{
    auto action = win->lookup_action("lock-all-guides");
    if (!action) {
        std::cerr << "lock_all_guides: action missing!" << std::endl;
        return;
    }

    auto saction = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(action);
    if (!saction) {
        std::cerr << "lock_all_guides: action not SimpleAction!" << std::endl;
        return;
    }

    bool state = false;
    saction->get_state(state);
    state = !state;
    saction->change_state(state);

    SPDesktop* dt = win->get_desktop();
    dt->toggleGuidesLock();
}

void
delete_all_guides(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    sp_guide_delete_all_guides(dt);
}

void
paste_path_effect(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->pastePathEffect();
}

void
remove_path_effect(InkscapeWindow* win)
{
    SPDesktop* dt = win->get_desktop();
    dt->selection->removeLPE();
}

std::vector<std::vector<Glib::ustring>> raw_data_edit =
{
    // clang-format off
    {"win.object-to-pattern",           N_("Objects to Pattern"),               "Edit",  N_("Convert selection to a rectangle with tiled pattern fill")},
    {"win.pattern-to-object",           N_("Pattern to Objects"),               "Edit",  N_("Extract objects from a tiled pattern fill")},
    {"win.object-to-marker",            N_("Objects to Marker"),                "Edit",  N_("Convert selection to a line marker")},
    {"win.object-to-guides",            N_("Objects to Guides"),                "Edit",  N_("Convert selected objects to a collection of guidelines aligned with their edges")},
    {"win.undo",                        N_("Undo"),                             "Edit",  N_("Undo last action")},
    {"win.redo",                        N_("Redo"),                             "Edit",  N_("Do again the last undone action")},
    {"win.cut",                         N_("Cut"),                              "Edit",  N_("Cut selection to clipboard")},
    {"win.copy",                        N_("Copy"),                             "Edit",  N_("Copy selection to clipboard")},
    {"win.paste",                       N_("Paste"),                            "Edit",  N_("Paste objects from clipboard to mouse point, or paste text")},
    {"win.paste-in-place",              N_("Paste In Place"),                   "Edit",  N_("Paste objects from clipboard to mouse point, or paste text")},
    {"win.paste-style",                 N_("Paste Style"),                      "Edit",  N_("Apply the style of the copied object to selection")},
    {"win.paste-size",                  N_("Paste Size"),                       "Edit",  N_("Scale selection to match the size of the copied object")},
    {"win.paste-width",                 N_("Paste Width"),                      "Edit",  N_("Scale selection horizontally to match the width of the copied object")},
    {"win.paste-height",                N_("Paste Height"),                     "Edit",  N_("Scale selection vertically to match the height of the copied object")},
    {"win.paste-size-separately",       N_("Paste Size Separately"),            "Edit",  N_("Scale each selected object to match the size of the copied object")},
    {"win.paste-width-separately",      N_("Paste Width Separately"),           "Edit",  N_("Scale each selected object horizontally to match the width of the copied object")},
    {"win.paste-height-separately",     N_("Paste Height Separately"),          "Edit",  N_("Scale each selected object vertically to match the height of the copied object")},
    {"win.duplicate",                   N_("Duplicate"),                        "Edit",  N_("Duplicate Selected Objects")},
    {"win.clone",                       N_("Create Clone"),                     "Edit",  N_("Create a clone (a copy linked to the original) of selected object")},
    {"win.clone-unlink",                N_("Unlink Clone"),                     "Edit",  N_("Cut the selected clones' links to the originals, turning them into standalone objects")},
    {"win.clone-unlink-recursively",    N_("Unlink Clones recursively"),        "Edit",  N_("Unlink all clones in the selection, even if they are in groups.")},
    {"win.clone-link",                  N_("Relink to Copied"),                 "Edit",  N_("Relink the selected clones to the object currently on the clipboard")},
    {"win.select-original",             N_("Select Original"),                  "Edit",  N_("Select the object to which the selected clone is linked")},
    {"win.clone-link-lpe",              N_("Clone original path (LPE)"),        "Edit",  N_("Creates a new path, applies the Clone original LPE, and refers it to the selected path")},
    {"win.delete",                      N_("Delete"),                           "Edit",  N_("Delete selection")},
    {"win.select-all",                  N_("Select All"),                       "Edit",  N_("Select all objects or all nodes")},
    {"win.select-all-layers",           N_("Select All in All Layers"),         "Edit",  N_("Select all objects in all visible and unlocked layers")},
    {"win.select-same-fill-and-stroke", N_("Fill and Stroke"),                  "Edit",  N_("Select all objects with the same fill and stroke as the selected objects")},
    {"win.select-same-fill",            N_("Fill Color"),                       "Edit",  N_("Select all objects with the same fill as the selected objects")},
    {"win.select-same-stroke-color",    N_("Stroke Color"),                     "Edit",  N_("Select all objects with the same stroke as the selected objects")},
    {"win.select-same-stroke-style",    N_("Stroke Style"),                     "Edit",  N_("Select all objects with the same stroke style (width, dash, markers) as the selected objects")},
    {"win.select-same-object-type",     N_("Object Type"),                      "Edit",  N_("Select all objects with the same object type (rect, arc, text, path, bitmap etc) as the selected objects")},
    {"win.select-invert",               N_("Invert Selection"),                 "Edit",  N_("Invert selection (unselect what is selected and select everything else)")},
    {"win.select-none",                 N_("Deselect"),                         "Edit",  N_("Deselect any selected objects or nodes")},
    {"win.create-guides-around-page",   N_("Create Guides Around the Page"),    "Edit",  N_("Create four guides aligned with the page borders")},
    {"win.lock-all-guides",             N_("Lock All Guides"),                  "Edit",  N_("Toggle lock of all guides in the document")},
    {"win.delete-all-guides",           N_("Delete All Guides"),                "Edit",  N_("Delete all the guides in the document")},
    {"win.paste-path-effect",           N_("Paste Path Effect"),                "Edit",  N_("Apply the path effect of the copied object to selection")},
    {"win.remove-path-effect",          N_("Remove Path Effect"),               "Edit",  N_("Remove any path effects from selected objects")}
    // clang-format on
};

void
add_actions_edit(InkscapeWindow* win)
{
    Glib::VariantType String(Glib::VARIANT_TYPE_STRING);

    // clang-format off
    win->add_action( "object-to-pattern",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&object_to_pattern), win));
    win->add_action( "pattern-to-object",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&pattern_to_object), win));
    win->add_action( "object-to-marker",                sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&object_to_marker), win));
    win->add_action( "object-to-guides",                sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&object_to_guides), win));
    win->add_action( "undo",                            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&undo), win));
    win->add_action( "redo",                            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&redo), win));
    win->add_action( "cut",                             sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&cut), win));
    win->add_action( "copy",                            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&copy), win));
    win->add_action( "paste",                           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste), win));
    win->add_action( "paste-in-place",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_in_place), win));
    win->add_action( "paste-style",                     sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_style), win));
    win->add_action( "paste-size",                      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_size), win));
    win->add_action( "paste-width",                     sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_width), win));
    win->add_action( "paste-height",                    sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_height), win));
    win->add_action( "paste-size-separately",           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_size_separately), win));
    win->add_action( "paste-width-separately",          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_width_separately), win));
    win->add_action( "paste-height-separately",         sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_height_separately), win));
    win->add_action( "duplicate",                       sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&duplicate), win));
    win->add_action( "clone",                           sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&clone), win));
    win->add_action( "clone-unlink",                    sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&clone_unlink), win));
    win->add_action( "clone-unlink-recursively",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&clone_unlink_recursively), win));
    win->add_action( "clone-link",                      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&clone_link), win));
    win->add_action( "select-original",                 sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_original), win));
    win->add_action( "clone-link-lpe",                  sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&clone_link_lpe), win));
    win->add_action( "delete",                          sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&edit_delete), win));
    win->add_action( "select-all",                      sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_all), win));
    win->add_action( "select-all-layers",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_all_layers), win));
    win->add_action( "select-same-fill-and-stroke",     sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_fill_and_stroke), win));
    win->add_action( "select-same-fill",                sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_fill), win));
    win->add_action( "select-same-stroke-color",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_stroke_color), win));
    win->add_action( "select-same-stroke-style",        sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_stroke_style), win));
    win->add_action( "select-same-object-type",         sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_same_object_type), win));
    win->add_action( "select-invert",                   sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_invert), win));
    win->add_action( "select-none",                     sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&select_none), win));
    win->add_action( "create-guides-around-page",       sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&create_guides_around_page), win));
    win->add_action_bool( "lock-all-guides",            sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&lock_all_guides),   win));
    win->add_action( "delete-all-guides",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&delete_all_guides), win));
    win->add_action( "paste-path-effect",               sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&paste_path_effect), win));
    win->add_action( "remove-path-effect",              sigc::bind<InkscapeWindow*>(sigc::ptr_fun(&remove_path_effect), win));
    // clang-format on

    auto app = InkscapeApplication::instance();
    if (!app) {
        std::cerr << "add_actions_edit: no app!" << std::endl;
        return;
    }
    app->get_action_extra_data().add_data(raw_data_edit);
}