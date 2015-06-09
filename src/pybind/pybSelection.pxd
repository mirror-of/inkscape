from pygobject cimport GSList
from pybNode cimport Node
from libcpp.vector cimport vector

cdef extern from "desktop.h":
    cppclass SPDesktop
    cppclass SPObject

cdef extern from "selection.h" namespace "Inkscape":
    ctypedef GSList const_GSList "const GSList"
    cppclass Selection:
        SPDesktop *desktop()
        SPObject *activeContext()
        vector[SPObject *] *list()
        void toggle(SPObject *)
        void clear()
        #void add(SPObject *, bool)
        #void remove(SPObject *)
        void add(Node *)
        void remove(Node *)
        const_GSList *reprList()
