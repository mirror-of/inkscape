from pybXML cimport Node

cdef extern from "pybind/pybindtools.h":
    Node *cast_Node(void *)
