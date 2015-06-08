from pybNodeObserver cimport NodeObserver
from pygobject cimport gchar, cgchar
from libcpp.vector cimport vector

cdef extern from "xml/element-node.h" namespace "Inkscape::XML":
    cppclass ElementNode(Node):
        pass
