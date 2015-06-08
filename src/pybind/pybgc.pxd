cdef extern from "gc-core.h" namespace "Inkscape::GC":
    cppclass Core:
        int general_register_disappearing_link(void **p_ptr, void *base)

cdef extern from "pybind/pybgc.h" namespace "Inkscape::pybind":
    cppclass pyb_deref_pyobj_finalizer:
        pyb_deref_pyobj_finalizer(void *base, object pyobj)
        void unregister()
