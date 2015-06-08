#include "pybind/pybgc.h"

void
pyb_gc_finalizer::finalizer_callback(void *mem, void *data) {
    pyb_gc_finalizer *finalizer = reinterpret_cast<pyb_gc_finalizer *>(data);

    finalizer->destructor();    // destructor of this finalizer

    finalizer->old_fn(mem, finalizer->old_data);
                                // destructor of old finalizer

    delete finalizer;
}

void
pyb_deref_pyobj_finalizer::destructor(void) {
    Py_DECREF(_pyobj);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
