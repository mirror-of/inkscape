from pybNodeObserver cimport NodeObserver
from pygobject cimport gchar, cgchar
from libcpp.vector cimport vector

cdef extern from "xml/document.h" namespace "Inkscape::XML":
    cppclass Document

cdef extern from "util/list.h" namespace "Inkscape::Util":
    cppclass List

cdef extern from "xml/node.h" namespace "Inkscape::XML":
    enum NodeType:
        DOCUMENT_NODE, ELEMENT_NODE, TEXT_NODE, COMMENT_NODE, PI_NODE

    cppclass Node:
        void *_wrapper
        NodeType type()
        cgchar *name()
        cgchar *content()
        cgchar *attribute(gchar *key)
        vector[cgchar *] attributes()
        void setPosition(int pos)
        void setContent(gchar *value)
        void setAttribute(gchar *key, gchar *value, bint interactive)
        
        Document *document()
        Node *root()
        Node *parent()
        Node *next()
        Node *firstChild()
        Node *lastChild()
        Node *nthChild(int index)

        void addChild(Node *child, Node *after)
        void appendChild(Node *child)
        void removeChild(Node *child)
        void changeOrder(Node *child, Node *after)
        void mergeFrom(Node *src, gchar *key)
        void overwriteWith(Node *src)

        void addObserver(NodeObserver &observer)
        void removeObserver(NodeObserver &observer)
        void addSubtreeObserver(NodeObserver &observer)
        void removeSubtreeObserver(NodeObserver &observer)
        Node *duplicate(void *doc)

#Manually added
cdef extern from "xml/element-node.h" namespace "Inkscape::XML":
    cppclass ElementNode(Node):
        ElementNode (int code, Document *doc)
