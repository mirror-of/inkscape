from pygobject cimport GSList
from pybNode cimport Node

cdef extern from "desktop.h":
    cppclass SPDesktop
    cppclass SPObject

cdef extern from "selection.h" namespace "Inkscape":
    ctypedef GSList const_GSList "const GSList"
    cppclass Selection:
        SPDesktop *desktop()
        SPObject *activeContext()
        const_GSList *list()
        void toggle(SPObject *)
        void clear()
        #void add(SPObject *, bool)
        #void remove(SPObject *)
        void add(Node *)
        void remove(Node *)
        const_GSList *reprList()
