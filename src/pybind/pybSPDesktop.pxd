from libcpp.list cimport list as clist
from pybApplication cimport Application
from pybXML cimport Node, Document
from pybView cimport View
from pybSelection cimport Selection
from pygobject cimport C_GObject, gchar
cimport pygobject

cdef extern from "document.h":
    cppclass SPDocument

cdef extern from "sp-object.h":
    cppclass SPObject:
        Node *repr
        SPDocument *document
        
        Node *getRepr()
        SPObject *firstChild()
        SPObject *lastChild()
        SPObject *getNext()
        SPObject *getPrev()
        bint hasChildren()
        #
        # Follow function is actually return a (const char *), but
        # Cython does not known const.  So, we should declare them
        # with (void *) returning type.  And cast them to (char *)
        # later.  If we declare them to return (char *), it causes a
        # compiling time error.  Syntax <char *> does not work since
        # Cython think they are (char *) type already and skip the
        # casting.
        #
        void *getId()
        void *label()
        void *title()
        void *desc()

    pygobject.GType SP_OBJECT

cdef extern from "2geom/coord.h" namespace "Geom":
    ctypedef double Coord

cdef extern from "2geom/point.h" namespace "Geom":
    cppclass Point:
        Coord operator[](unsigned i)
        
cdef extern from "sp-item.h":
    cppclass SPItem(SPObject):
        Point getCenter()

    pygobject.GType SP_ITEM

cdef extern from "sp-item-group.h":
    cppclass SPGroup(SPItem):
        pass

cdef extern from "sp-root.h":
    cppclass SPRoot(SPItem):
        pass

    pygobject.GType SP_ROOT

cdef extern from "sp-rect.h":
    cppclass SPRect(SPItem):
        pass

    pygobject.GType SP_RECT

cdef extern from "sigc++/sigc++.h" namespace "sigc":
    cppclass connection:
        pass

cdef extern from "document.h" namespace "SPDocument::ResourcesChangedSignal":
    cppclass slot_type:
        pass

ctypedef connection sig_connection
ctypedef slot_type res_slot_type

cdef extern from "pybind/pyb_slot_proxy.h" namespace "Inkscape::pybind":
    cppclass slot_proxy:
        slot_proxy(object obj)
        res_slot_type get_slot()

cdef extern from "glibmm/ustring.h" namespace "Glib":
    cppclass ustring:
        ustring()
        ustring(char *src)

cdef extern from "document-undo.h" namespace "Inkscape::DocumentUndo":
    void done(SPDocument *document, 
                          unsigned int event_type, 
                          ustring event_description)
    void maybeDone(SPDocument *document, 
                                gchar *keyconst, 
                                unsigned int event_type, 
                                ustring event_description)

cdef extern from "document.h":
    cppclass SPDocument(SPItem):
        Document *rdoc
        SPObject *getObjectByRepr(Node *repr)
        SPObject *getRoot()
        sig_connection connectResourcesChanged(gchar *key,
                                               res_slot_type slot)

cdef extern from "gtkmm/window.h" namespace "Gtk":
    cppclass Window:
        # Return associated GtkWindow.
        C_GObject *gobj()

cdef extern from "ui/widget/dock.h" namespace "Inkscape::UI::Widget":
    cppclass Dock:
        pass

#
# Cython does not support typedef in a C++ class.  So, we use
# namespace to workaround.
#
cdef extern from "ui/widget/dock-item.h" \
    namespace "Inkscape::UI::Widget::DockItem":
    enum State:
        UNATTACHED, FLOATING_STATE, DOCKED_STATE
    enum Placement:
        NONE, TOP, BOTTOM, RIGHT, LEFT, CENTER, FLOATING

cdef extern from "ui/widget/dock-item.h" namespace "Inkscape::UI::Widget":
    cppclass DockItem:
        DockItem(Dock &dock, ustring &name, ustring &long_name,
                 ustring &icon_name, State state, Placement placement)
        pygobject.GtkWidget *gobj()
        pygobject.VBox *get_vbox()

cdef extern from "desktop.h":
    cppclass SPDesktop(View):
        Selection *selection
        SPDocument *doc()
        # Return associated top level window.
        Window *getToplevel()
        Dock *getDock()
        SPObject *currentLayer()
        void setCurrentLayer(SPObject *object)

    ctypedef SPDesktop *SPDesktop_p

