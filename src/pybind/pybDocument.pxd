from pybNode cimport Node
from pygobject cimport gchar, cgchar

cdef extern from "xml/document.h" namespace "Inkscape::XML":
    cppclass Document(Node):
        bint inTransaction()
        void beginTransaction()
        void rollback()
        void commit()

        Node *createElement(cgchar *name)
        Node *createTextNode(cgchar *content)
        Node *createComment(cgchar *content)
        Node *createPI(cgchar *target, cgchar *content)
