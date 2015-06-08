#include "pybind/pyb_slot_proxy.h"

void
Inkscape::pybind::slot_proxy::run(void) {
    PyObject_CallFunctionObjArgs(py_obj, NULL);
}

sigc::slot0<void>
Inkscape::pybind::slot_proxy::get_slot(void) {
    return sigc::mem_fun(this, &Inkscape::pybind::slot_proxy::run);
}
