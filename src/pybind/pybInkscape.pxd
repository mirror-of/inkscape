from pybApplication cimport Application
from pybSPDesktop cimport SPDesktop, SPDocument

cdef extern from "inkscape.h":
    cdef SPDocument *SP_ACTIVE_DOCUMENT
    cdef SPDesktop *SP_ACTIVE_DESKTOP
