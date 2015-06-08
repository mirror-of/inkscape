#ifndef __PYBGC_H_
#define __PYBGC_H_

#include <Python.h>

#include "gc-core.h"

/*! \brief Class to maintain reference of Python object from Inkscape.
 *
 * This class is still not completed.  it should be implmeneted with
 * Inkscape::GC::Managed and Inkscape::GC::Finalized.
 */
class pyb_gc_finalizer {
private:
    Inkscape::GC::CleanupFunc old_fn;
    void *old_data;
    void *_base;
    
    static void finalizer_callback(void *mem, void *data);
    virtual void destructor(void) = 0;
    
public:
    pyb_gc_finalizer(void *base) {
        this->_base = base;
        this->old_fn = NULL;
        this->old_data = NULL;
        Inkscape::GC::Core::
            register_finalizer_ignore_self(base, &finalizer_callback,
                                           reinterpret_cast<void *>(this),
                                           &this->old_fn, &this->old_data);
    }

    void unregister(void) {
        Inkscape::GC::Core::
            register_finalizer_ignore_self(this->_base,
                                           this->old_fn, this->old_data,
                                           NULL, NULL);
    }
};

class pyb_deref_pyobj_finalizer: public pyb_gc_finalizer {
private:
    PyObject *_pyobj;
    
public:
    pyb_deref_pyobj_finalizer(void *base, PyObject *pyobj):
        pyb_gc_finalizer(base), _pyobj(pyobj) {}

    void destructor(void);
};

#endif /* __PYBGC_H_ */
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
