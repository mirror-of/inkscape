#include "pybind/pyb_slot_proxy.h"
#include "inkscape.h"
#include "string.h"

void
Inkscape::pybind::slot_proxy::run0(void) {
    PyObject_CallFunctionObjArgs(py_obj, NULL);
}

sigc::slot0<void>
Inkscape::pybind::slot_proxy::get_slot_0(void) {
    return sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run0);
}

sigc::slot1<void, void*>
Inkscape::pybind::slot_proxy::get_slot_1(void) {
    return sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run1);
}

void
Inkscape::pybind::slot_proxy::run1(void* arg1) {
    PyObject_CallFunctionObjArgs(py_obj, PyCObject_FromVoidPtr(arg1, NULL), NULL);
}

void
Inkscape::pybind::slot_proxy::run2(void* arg1, int arg2) {
    PyObject_CallFunctionObjArgs(py_obj, PyCObject_FromVoidPtr(arg1, NULL), arg2, NULL);
}

void
Inkscape::pybind::slot_proxy::connect(char *signal_name) {
    if (strcmp(signal_name, "activate_desktop") == 0) {
      INKSCAPE.signal_activate_desktop.connect(sigc::retype(sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run1)));
    } else if (strcmp(signal_name, "deactivate_desktop") == 0) {
      INKSCAPE.signal_deactivate_desktop.connect(sigc::retype(sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run1)));
    } else if (strcmp(signal_name, "selection_changed") == 0) {
      INKSCAPE.signal_selection_changed.connect(sigc::retype(sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run1)));
    } else if (strcmp(signal_name, "subselection_changed") == 0) {
      INKSCAPE.signal_subselection_changed.connect(sigc::retype(sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run1)));
    } else if (strcmp(signal_name, "selection_modified") == 0) {
      INKSCAPE.signal_selection_modified.connect(sigc::retype(sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run2)));
    } else if (strcmp(signal_name, "selection_set") == 0) {
      INKSCAPE.signal_selection_set.connect(sigc::retype(sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run1)));
    } else if (strcmp(signal_name, "shut_down") == 0) {
      INKSCAPE.signal_shut_down.connect(sigc::retype(sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run0)));
    } else {
      printf("Error: unknown signal name"); //Should throw an error or display differently probably.
    }
}
