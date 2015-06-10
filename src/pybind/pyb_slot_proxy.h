#ifndef __PYB_SLOT_PROXY_H_
#define __PYB_SLOT_PROXY_H_

#include <Python.h>

#include "inkgc/gc-managed.h"
#include "gc-finalized.h"
#include "sigc++/sigc++.h"

namespace Inkscape {
namespace pybind {

class slot_proxy{
    PyObject *py_obj;
    
    void run0(void);
    void run1(void*);
    void run2(void*, int);
    
public:
    slot_proxy(PyObject *obj): py_obj(obj) {
	Py_INCREF(py_obj);
    }
    ~slot_proxy() {
	Py_DECREF(py_obj);
    }
    sigc::slot0<void> get_slot_0(void);
    sigc::slot1<void,void*> get_slot_1(void);
    void connect(char *);
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
