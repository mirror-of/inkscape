#ifndef __PYB_NODEOBSERVER_PROXY_H_
#define __PYB_NODEOBSERVER_PROXY_H_

#include <Python.h>

#include "xml/node-observer.h"
#include "xml/node.h"
#include "gc-managed.h"
#include "gc-finalized.h"

namespace Inkscape {
namespace pybind {

using Inkscape::XML::Node;

/*! \brief Proxy of Python objects to deliver NodeObserver events.
 *
 * Since this class is managed and finalized, it would keep from cycling
 * whenever a subject (Node?) is refering it, and destructor would be called
 * when it is being recycled for no more reachable.  So, we maintain a
 * reference to PyObject in constructor and destructor to keep PyObject from
 * recycling whenever it is used by a subject.
 *
 * \see gc-managed.h and gc-finalized.h
 */
class NodeObserver_proxy: public Inkscape::XML::NodeObserver,
                          public Inkscape::GC::Managed<>,
                          public Inkscape::GC::Finalized {
private:
    PyObject *py_obj;
    
public:
    NodeObserver_proxy(PyObject *obj): py_obj(obj) {
        Py_INCREF(obj);
    };
    ~NodeObserver_proxy();
    void notifyChildAdded(Node &node, Node &child, Node *prev);
    void notifyChildRemoved(Node &node, Node &child, Node *prev);
    void notifyChildOrderChanged(Node &node, Node &child,
				 Node *old_prev, Node *new_prev);
    void notifyContentChanged(Node &node,
			      Util::ptr_shared<char> old_content,
			      Util::ptr_shared<char> new_content);
    void notifyAttributeChanged(Node &node, GQuark name,
				Util::ptr_shared<char> old_value,
				Util::ptr_shared<char> new_value);
};

}
}

#endif /* __PYB_NODEOBSERVER_PROXY_H_ */
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
