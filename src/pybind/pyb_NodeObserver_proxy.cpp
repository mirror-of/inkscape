#include "pybind/pyb_NodeObserver_proxy.h"

namespace Inkscape {
namespace pybind {

NodeObserver_proxy::~NodeObserver_proxy() {
    Py_DECREF(py_obj);
}

void
NodeObserver_proxy::notifyChildAdded(Node &node, Node &child,
				     Node *prev) {
    PyObject *_node, *_child, *_prev;
    PyObject *method, *r;
    
    _node = PyCObject_FromVoidPtr(reinterpret_cast<void *>(&node), NULL);
    _child = PyCObject_FromVoidPtr(reinterpret_cast<void *>(&child), NULL);
    if(prev)
	_prev = PyCObject_FromVoidPtr(reinterpret_cast<void *>(prev), NULL);
    else {
	Py_INCREF(Py_None);
	_prev = Py_None;
    }
    
    method = PyString_FromString("_notifyChildAdded");
    r = PyObject_CallMethodObjArgs(py_obj, method,
				   _node, _child, _prev, NULL);
    if(r != NULL) {
	Py_DECREF(r);
    } else {
	PyErr_Print();
    }

    Py_DECREF(_node);
    Py_DECREF(_child);
    Py_DECREF(_prev);
    Py_DECREF(method);
}

void
NodeObserver_proxy::notifyChildRemoved(Node &node, Node &child,
				       Node *prev) {
    PyObject *_node, *_child, *_prev;
    PyObject *method, *r;

    _node = PyCObject_FromVoidPtr(reinterpret_cast<void *>(&node), NULL);
    _child = PyCObject_FromVoidPtr(reinterpret_cast<void *>(&child), NULL);
    if(prev)
	_prev = PyCObject_FromVoidPtr(reinterpret_cast<void *>(prev), NULL);
    else {
	Py_INCREF(Py_None);
	_prev = Py_None;
    }
    
    method = PyString_FromString("_notifyChildRemoved");
    r = PyObject_CallMethodObjArgs(py_obj, method,
				   _node, _child, _prev, NULL);
    if(r != NULL) {
	Py_DECREF(r);
    } else {
	PyErr_Print();
    }

    Py_DECREF(_node);
    Py_DECREF(_child);
    Py_DECREF(_prev);
    Py_DECREF(method);
}

void
NodeObserver_proxy::notifyChildOrderChanged(Node &node, Node &child,
					    Node *old_prev,
					    Node *new_prev) {
    PyObject *_node, *_child, *_old_prev, *_new_prev;
    PyObject *method, *r;

    _node = PyCObject_FromVoidPtr(reinterpret_cast<void *>(&node), NULL);
    _child = PyCObject_FromVoidPtr(reinterpret_cast<void *>(&child), NULL);
    if(old_prev)
	_old_prev =
	    PyCObject_FromVoidPtr(reinterpret_cast<void *>(old_prev), NULL);
    else {
	Py_INCREF(Py_None);
	_old_prev = Py_None;
    }
    if(new_prev)
	_new_prev =
	    PyCObject_FromVoidPtr(reinterpret_cast<void *>(new_prev), NULL);
    else {
	Py_INCREF(Py_None);
	_new_prev = Py_None;
    }
    
    method = PyString_FromString("_notifyChildOrderChanged");
    r = PyObject_CallMethodObjArgs(py_obj, method,
				   _node, _child, _old_prev, _new_prev, NULL);
    if(r != NULL) {
	Py_DECREF(r);
    } else {
	PyErr_Print();
    }

    Py_DECREF(_node);
    Py_DECREF(_child);
    Py_DECREF(_old_prev);
    Py_DECREF(_new_prev);
    Py_DECREF(method);
}

void
NodeObserver_proxy::notifyContentChanged(Node &node,
					 Util::ptr_shared<char> old_content,
					 Util::ptr_shared<char> new_content) {
    PyObject *_node, *_old_content, *_new_content;
    PyObject *method, *r;

    _node = PyCObject_FromVoidPtr(reinterpret_cast<void *>(&node), NULL);
    if(old_content)
	_old_content = PyString_FromString(old_content);
    else {
	Py_INCREF(Py_None);
	_old_content = Py_None;
    }
    if(new_content)
	_new_content = PyString_FromString(new_content);
    else {
	Py_INCREF(Py_None);
	_new_content = Py_None;
    }
    
    method = PyString_FromString("_notifyContentChanged");
    r = PyObject_CallMethodObjArgs(py_obj, method,
				   _node, _old_content, _new_content, NULL);
    if(r != NULL) {
	Py_DECREF(r);
    } else {
	PyErr_Print();
    }

    Py_DECREF(_node);
    Py_DECREF(_old_content);
    Py_DECREF(_new_content);
    Py_DECREF(method);
}

void
NodeObserver_proxy::notifyAttributeChanged(Node &node, GQuark name,
					   Util::ptr_shared<char> old_value,
					   Util::ptr_shared<char> new_value) {
    PyObject *_node, *_name, *_old_value, *_new_value;
    PyObject *method, *r;

    _node = PyCObject_FromVoidPtr(reinterpret_cast<void *>(&node), NULL);
    _name = PyInt_FromLong(name);
    if(old_value)
	_old_value = PyString_FromString(old_value);
    else {
	Py_INCREF(Py_None);
	_old_value = Py_None;
    }
    if(new_value)
	_new_value = PyString_FromString(new_value);
    else {
	Py_INCREF(Py_None);
	_new_value = Py_None;
    }
    
    method = PyString_FromString("_notifyAttributeChanged");
    r = PyObject_CallMethodObjArgs(py_obj, method,
				   _node, _name, _old_value, _new_value, NULL);
    if(r != NULL) {
	Py_DECREF(r);
    } else {
	PyErr_Print();
    }
    
    Py_DECREF(_node);
    Py_DECREF(_old_value);
    Py_DECREF(_new_value);
    Py_DECREF(method);
}

}
}
