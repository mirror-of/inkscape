from cpython.ref cimport PyTypeObject, PyObject

cdef extern from "pygobject.h":
    ctypedef char gchar
    ctypedef char cgchar "const gchar"
    ctypedef int GType
    ctypedef int GQuark
    ctypedef class gobject.GObject [object PyGObject]:
        pass

    struct _GSList
    
    ctypedef _GSList GSList

    struct _GSList:
        void *data
        GSList *next

    struct _GObject:
        pass

    ctypedef _GObject C_GObject "GObject"

    GType G_OBJECT_TYPE(C_GObject *object)

    ctypedef struct C_PyGPointer "PyGPointer":
        void *pointer

    struct _PyGObject_Functions:
        object (* newgobj)(C_GObject *obj)
        void (* register_class)(object dict,
                                cgchar * class_name,
                                GType gtype, PyTypeObject *type,
				PyObject *bases)

    C_GObject *pygobject_get(object)

    struct _GtkWidget:
        pass

    ctypedef _GtkWidget GtkWidget

    struct _GtkVBox:
        pass

    ctypedef _GtkVBox GtkVBox

    cgchar *g_quark_to_string(GQuark quark)
    GQuark g_quark_from_string(const gchar *string)

cdef extern from "gtkmm/box.h" namespace "Gtk":
    cppclass VBox:
        GtkVBox *gobj()
