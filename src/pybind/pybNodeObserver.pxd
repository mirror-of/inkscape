cdef extern from "xml/node-observer.h" namespace "Inkscape::XML":
    cppclass NodeObserver:
        pass

cdef extern from "pybind/pyb_NodeObserver_proxy.h" \
    namespace "Inkscape::pybind":
    cppclass NodeObserver_proxy:
        NodeObserver_proxy(object)
