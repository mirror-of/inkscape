#ifndef __PYB_SLOT_PROXY_H_
#define __PYB_SLOT_PROXY_H_

#include <Python.h>

#include "gc-managed.h"
#include "gc-finalized.h"
#include "sigc++/sigc++.h"

namespace Inkscape {
namespace pybind {

class slot_proxy: public Inkscape::GC::Managed<>,
		  public Inkscape::GC::Finalized {
    PyObject *py_obj;
    
    void run(void);
    
public:
    slot_proxy(PyObject *obj): py_obj(obj) {
	Py_INCREF(py_obj);
    }
    ~slot_proxy() {
	Py_DECREF(py_obj);
    }
    sigc::slot0<void> get_slot(void);
};

}
}

#endif /* __PYB_SLOT_PROXY_H_ */
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
