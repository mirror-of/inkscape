# -*- mode: python; indnent-tabs-mode: t; tab-width: 8; c-basic-offset: 4; -*-
# vim: sw=4:ts=8:sts=4
from cpython cimport bool
from cython.operator cimport dereference as deref, address as addr
from cpython.ref cimport PyTypeObject
from libcpp.list cimport list as cpplist
from cpython.cobject cimport PyCObject_AsVoidPtr, PyCObject_Check
from cpython.cobject cimport PyCObject_FromVoidPtr
from cpython cimport PyObject, Py_INCREF, Py_DECREF
from pygobject cimport GObject, C_GObject, _PyGObject_Functions, pygobject_get
from pygobject cimport G_OBJECT_TYPE, C_PyGPointer, g_quark_to_string, g_quark_from_string, GQuark
from pybXML cimport Node, Document, ElementNode
from pybSelection cimport Selection, const_GSList
from pybNodeObserver cimport NodeObserver_proxy, NodeObserver
cimport pybXML
cimport pybSPDesktop
cimport pybInkscape
cimport pybApplication
cimport pybVerb
cimport pybgc
cimport pybindtools
import gobject


cdef class PYNodeObserver
cdef class PYNode
cdef class PYElementNode
cdef class PYDocument
cdef class PYSPDocument

cdef class PYSelection(object):
    cdef Selection *_thisptr

    def __cinit__(self, ptr_co):
        assert PyCObject_Check(ptr_co)
        sel = <Selection *>PyCObject_AsVoidPtr(ptr_co)
        self._thisptr = sel

    def toggle(self, PYSPObject node):
        spobj = <pybSPDesktop.SPObject *>pygobject_get(node)
        self._thisptr.toggle(<pybSPDesktop.SPObject *>spobj)

    def clear(self):
        self._thisptr.clear()

    def add(self, PYNode node):
        self._thisptr.add(node._thisptr)

    def remove(self, PYNode node):
        self._thisptr.remove(node._thisptr)

    def desktop(self):
        dsk = self._thisptr.desktop()
        dsk_co = PyCObject_FromVoidPtr(dsk, NULL)
        wrapper = PYSPDesktop(dsk_co)
        return wrapper

    def activeContext(self):
        ctx = <pybSPDesktop.SPObject *>self._thisptr.activeContext()
        wrapper = gobj_api.newgobj(<C_GObject *>ctx)
        return wrapper

    def list(self):
        cdef const_GSList *lst = self._thisptr.list()
        objs = []
        while lst != NULL:
            obj = <pybSPDesktop.SPObject *>(lst.data)
            wrapper = wrapobjtonode(obj)
            objs.append(wrapper)
            lst = <const_GSList *>lst.next
        return objs

cdef class PYNode(object):
    cdef Node *_thisptr
    cdef public object _doc
    cdef public object pystore

    DOCUMENT_NODE = 0
    ELEMENT_NODE = 1
    TEXT_NODE = 2
    COMMENT_NODE = 3
    PI_NODE = 4
    
    def __init__(self, ptr_co, PYDocument doc, *args):
        assert PyCObject_Check(ptr_co)
        _repr = <Node *>PyCObject_AsVoidPtr(ptr_co)
        self._thisptr = _repr
        self._doc = doc
        self.pystore = {}

    #
    # Return respective SPItem of a PYNode.
    #
    cdef pybSPDesktop.SPItem *_get_spitem(self):
        pydoc = <PYDocument>self._doc
        spdoc = pydoc._pyspdoc._thisptr
        spitem = <pybSPDesktop.SPItem *>spdoc.getObjectByRepr(self._thisptr)
        return spitem

    def type(self):
        _type = self._thisptr.type()
        if _type == pybXML.DOCUMENT_NODE:
            return PYNode.DOCUMENT_NODE
        elif _type == pybXML.ELEMENT_NODE:
            return PYNode.ELEMENT_NODE
        elif _type == pybXML.TEXT_NODE:
            return PYNode.TEXT_NODE
        elif _type == pybXML.COMMENT_NODE:
            return PYNode.COMMENT_NODE
        elif _type == pybXML.PI_NODE:
            return PYNode.PI_NODE
        else:
            pass

        raise ValueError, 'unknown value'
    
    def name(self):
        _name = self._thisptr.name()
        if _name == NULL:
            return None
        return <char *>_name

    def content(self):
        _content = self._thisptr.content()
        if _content == NULL:
            return None
        return <char *>_content

    def attribute(self, bytes key):
        value = <char *>self._thisptr.attribute(<char *>key)
        if value == NULL:
            raise KeyError, 'No such key: %s' % key
        return value

    def __getitem__(self, key):
        if key.startswith("style."):
            subkey = key[len("style."):]
            style = unserattrib(self["style"])
            return style[subkey]
        else:
            return self.attribute(key)

    def get(self, key, default=None):
        if key.startswith("style."):
            subkey = key[len("style."):]
            style = unserattrib(self.get("style",""))
            return style.get(subkey, default)
        else:
            try:
                return PYNode.__getitem__(self, key)
            except KeyError:
                return default

    def __setitem__(self, key, value):
        if key.startswith("style."):
            subkey = key[len("style."):]
            style = unserattrib(self.get("style",""))
            if value:
                style[subkey] = value
            elif subkey in style:
                del style[subkey]
            self["style"] = serattrib(style)
        else:
            self.setAttribute(key, value)

    def __delitem__(self, bytes key):
        if key.startswith("style."):
            subkey = key[len("style."):]
            style = unserattrib(self.get("style",""))
            del style[subkey]
            # Should auto-delete or not?
            self["style"] = serattrib(style)
        else:
            self._thisptr.setAttribute(<char *>key, NULL, 0);

    def __contains__(self, bytes key):
        return (self.get(key) != None)

    def __richcmp__(PYNode self, PYNode other,op):
        if op == 2:
            return self._thisptr == other._thisptr
        elif op == 3:
            return self._thisptr != other._thisptr
        elif op == 0:
            return self._thisptr < other._thisptr
        elif op == 1:
            return self._thisptr <= other._thisptr
        elif op == 4:
            return self._thisptr > other._thisptr
        elif op == 5:
            return self._thisptr >= other._thisptr

    def keys(self):
        return list(self._thisptr.attributes())

    def delete(self, bytes key, bint interactive=0):
        self._thisptr.setAttribute(<char *>key, NULL, interactive);

    def setPosition(self, int pos):
        self._thisptr.setPosition(pos)
    
    def setContent(self, bytes value):
        self._thisptr.setContent(<char *>value)
    
    def setAttribute(self, bytes key, bytes value, bint interactive=0):
        self._thisptr.setAttribute(<char *>key, <char *>value, interactive)

    #http://stackoverflow.com/questions/16907168/convert-python-object-to-c-void-type
    def setwrapper(self, wrapper):
        if self._thisptr._wrapper != NULL:
            Py_DECREF(self.wrapper)
        self._thisptr._wrapper = <void *>wrapper
        Py_INCREF(wrapper)
    
    def getwrapper(self):
        if self._thisptr._wrapper == NULL:
            return None
        ptr = <PyObject *>self._thisptr._wrapper
        cdef object obj
        obj = <object>ptr
        return obj

    wrapper = property(getwrapper, setwrapper)

    @property
    def doc(self):
        return self._doc

    @property
    def root(self):
        root = self._thisptr.root()
        return wrapnode(root, self._doc)
    
    @property
    def parent(self):
        return wrapnode(self._thisptr.parent(), self._doc)
    
    def next(self):
        node = self._thisptr.next()
        return wrapnode(node, self._doc)

    def firstChild(self):
        node = self._thisptr.firstChild()
        return wrapnode(node, self._doc)

    def lastChild(self):
        return wrapnode(self._thisptr.lastChild(), self._doc)

    @property
    def children(self):
        children = []
        child = self.firstChild()
        while child:
            children.append(child)
            child = child.next()
        return children
    
    def nthChild(self, int index):
        node = self._thisptr.nthChild(index)
        return wrapnode(node, self._doc)

    def addChild(self, PYNode child, PYNode after):
        _child = child._thisptr
        _after = after._thisptr
        self._thisptr.addChild(_child, _after)
    
    def appendChild(self, PYNode child):
        _child = child._thisptr
        self._thisptr.appendChild(_child)
    
    def removeChild(self, PYNode child):
        _child = child._thisptr
        self._thisptr.removeChild(_child)
    
    def changeOrder(self, PYNode child, PYNode after):
        _child = child._thisptr
        _after = after._thisptr
        self._thisptr.changeOrder(_child, _after)
    
    def mergeFrom(self, PYNode src, bytes key):
        _src = src._thisptr
        self._thisptr.mergeFrom(_src, <char *>key)

    def overwriteWith(self, PYNode src):
        _src = src._thisptr
        self._thisptr.overwriteWith(_src)

    def addObserver(self, PYNodeObserver observer):
        if observer._proxy == NULL:
            observer._proxy = new NodeObserver_proxy(observer)
            p_ptr = <void **>addr(observer._proxy)
            core = <pybgc.Core *>NULL
            core.general_register_disappearing_link(p_ptr, observer._proxy)
        proxy = <NodeObserver *>observer._proxy
        self._thisptr.addObserver(deref(proxy))
        observer._notify_add_observer(self)
    
    def removeObserver(self, PYNodeObserver observer):
        proxy = <NodeObserver *>observer._proxy
        self._thisptr.removeObserver(deref(proxy))
    
    def addSubtreeObserver(self, PYNodeObserver observer):
        if observer._proxy == NULL:
            observer._proxy = new NodeObserver_proxy(observer)
            p_ptr = <void **>addr(observer._proxy)
            core = <pybgc.Core *>NULL
            core.general_register_disappearing_link(p_ptr, observer._proxy)
        proxy = <NodeObserver *>observer._proxy
        self._thisptr.addSubtreeObserver(deref(proxy))
        observer._notify_add_observer(self)
    
    def removeSubtreeObserver(self, PYNodeObserver observer):
        proxy = <NodeObserver *>observer._proxy
        self._thisptr.removeSubtreeObserver(deref(proxy))
    
    def duplicate(self, PYDocument doc=None):
        #Should call Inkscape::GC::release on the new node after addition...
        if doc == None:
          pydoc = <PYDocument>self._doc
          node = self._thisptr.duplicate(pydoc._back_doc)
        else:
          docptr = doc._back_doc
          node = self._thisptr.duplicate(docptr)
        node_co = PyCObject_FromVoidPtr(node, NULL)
        wrapper = PYNode(node_co, doc)
        return wrapper

    def getCenter(self):
        spitem = self._get_spitem()
        if not spitem:
            return None
        point = spitem.getCenter()
        return [point[0],point[1]]

cdef class PYElementNode(PYNode):
    def __init__(self, bytes name, PYDocument doc, *args):
        node = doc._back_doc.createElement(<char *>name)
        if node == NULL:
            raise MemoryError, 'cannot create an element'
        ptr_co = PyCObject_FromVoidPtr(node, NULL)

        assert PyCObject_Check(ptr_co)
        _repr = <Node *>PyCObject_AsVoidPtr(ptr_co)
        self._thisptr = _repr
        self._doc = doc
        self.pystore = {}

## \page how_det_layers How to detect adding and removing layers?
# 
# SPDocumentPrivate::resources is a map to track resources associated
# with a SPDocument while SPDocument::priv is a SPDocumentPrivate.  We
# can call sp_document_resources_changed_connect() of SPDocument to
# register a signal handler to get notification when the content a
# reousrce being changed.  The resources are identified by a string
# key.  Inkscape use key "layer" to track layers of a document.  We
# can register a signal handler for the key "layer". (\see
# SPGroup::setLayerMode())
# 

cdef class PYDocument(PYNode):
    cdef Document *_back_doc
    cdef PYSPDocument _pyspdoc

    def __cinit__(self, ptr_co, node, PYSPDocument pyspdoc, doc_co, *args):
        assert PyCObject_Check(doc_co)
        self._pyspdoc = pyspdoc
        self._back_doc = <Document *>PyCObject_AsVoidPtr(doc_co)
        self._doc = self    # It is assigned to None by PYNode.
                                # \see PYSPDocument.rdoc().  But, it
                                # should point back to PYDocument
                                # it-self.

    def __init__(self, ptr_co, PYDocument doc, *args):
        PYNode.__init__(self, ptr_co, self._doc, *args) 

    #
    # This function must be redefined, here, to override PYNode one.
    # _doc for PYDocument is an invalid value.  root() would use _doc
    # to create PYNode for root node.  It is fixed by use PYDocument
    # object itself.
    # 
    @property
    def root(self):
        root = self._thisptr.root()
        return wrapnode(root, self._doc)
        
    def inTransaction(self):
        r = self._back_doc.inTransaction()
        return r
    
    def beginTransaction(self):
        self._back_doc.beginTransaction()
    
    def rollback(self):
        self._back_doc.rollback()
    
    def commit(self):
        self._back_doc.commit()
    
    def createElement(self, bytes name):
        node = self._back_doc.createElement(<char *>name)
        return wrapnode(node, self._doc, True)

    def createTextNode(self, bytes content):
        node = self._back_doc.createTextNode(<char *>content)
        return wrapnode(node, self._doc, True)
    
    def createComment(self, bytes content):
        node = self._back_doc.createComment(<char *>content)
        return wrapnode(node, self._doc, True)
    
    def createPI(self, bytes target, bytes content):
        node = self._back_doc.createPI(<char *>target, <char *>content)
        return wrapnode(node, self._doc, True)
    
    @property
    def spdoc(self):
        return self._pyspdoc

cdef class PYSPObject(GObject):
    @property
    def repr(self):
        if not hasattr(self, '_repr'):
            _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
            #repr_co = PyCObject_FromVoidPtr(_thisptr.repr, NULL)
            pyspdoc = self._get_pyspdoc()
            pydoc = pyspdoc.rdoc
            #self._repr = PYNode(repr_co, pydoc)
            self._repr = wrapnode(_thisptr.repr, pydoc)
        return self._repr

    def _get_pyspdoc(self):
        spobj = <pybSPDesktop.SPObject *>pygobject_get(self)
        spdoc = spobj.document
        spdoc_co = PyCObject_FromVoidPtr(spdoc, NULL)
        pyspdoc = PYSPDocument(spdoc_co)
        return pyspdoc
    
    def firstChild(self):
        _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
        obj = _thisptr.firstChild()
        wrapobj(obj)

    def lastChild(self):
        _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
        obj = _thisptr.lastChild()
        wrapobj(obj)

    def getNext(self):
        _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
        obj = _thisptr.getNext()
        wrapobj(obj)

    def getPrev(self):
        _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
        obj = _thisptr.getPrev()
        wrapobj(obj)

    def hasChildren(self):
        _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
        return _thisptr.hasChildren()

    def getId(self):
        _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
        cdef char *_id = <char *>_thisptr.getId()
        if _id == NULL:
            return None
        return _id

    def label(self):
        _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
        cdef char *_label = <char *>_thisptr.label()
        if _label == NULL:
            return None
        return _label

    def title(self):
        _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
        cdef char *_title = <char *>_thisptr.title()
        if _title == NULL:
            return None
        return _title

    def desc(self):
        _thisptr = <pybSPDesktop.SPObject *>pygobject_get(self)
        cdef char *_desc = <char *>_thisptr.desc()
        if _desc == NULL:
            return None
        return _desc

cdef class PYSPItem(PYSPObject):
    def getCenter(self):
        _thisptr = <pybSPDesktop.SPItem *>pygobject_get(self)
        point = _thisptr.getCenter()
        return [point[0],point[1]]

cdef class PYSPRoot(PYSPItem):
    pass

cdef class PYSPRect(PYSPItem):
    pass

cdef class PYSPDocument:
    cdef pybSPDesktop.SPDocument *_thisptr
    
    def __cinit__(self, ptr_co):
        assert PyCObject_Check(ptr_co)
        self._thisptr = <pybSPDesktop.SPDocument *>PyCObject_AsVoidPtr(ptr_co)

    @property
    def rdoc(self):
        node = pybindtools.cast_Node(self._thisptr.rdoc)
        node_co = PyCObject_FromVoidPtr(node, NULL)
        # doc_co is passed for using by PYDocument, since C++ allow to
        # cast a virtual base to a drived.
        doc_co = PyCObject_FromVoidPtr(self._thisptr.rdoc, NULL)
        wrapper = PYDocument(node_co, None, self, doc_co)
        return wrapper

    @property
    def root(self):
        root = self._thisptr.getRoot()
        wrapper = wrapobj(root)
        return wrapper

    def connect_layers_changed(self, callback):
        proxy = new pybSPDesktop.slot_proxy(callback)
        slot = proxy.get_slot()
        connection = \
            self._thisptr.connectResourcesChanged("layer",
                                                  slot)

    def done(self, event_type, description):
        cdef pybSPDesktop.ustring _desc = pybSPDesktop.ustring(<char *> description)
        _verb = pybVerb.pyb_verb_getbyid(<char *>event_type)
        pybSPDesktop.done(self._thisptr, _verb.get_code(), _desc)

    def maybe_undo(self, key, event_type, description):
        cdef pybSPDesktop.ustring _desc = pybSPDesktop.ustring(<char *> description)
        cdef pybSPDesktop.gchar *_key = (<pybSPDesktop.gchar *> (<char *>key))
        _verb = pybVerb.pyb_verb_getbyid(<char *>event_type)
	
        pybSPDesktop.maybeDone(self._thisptr, _key, _verb.get_code(), _desc)

## \page make_dock How to make a dock?
#
# SPDecktop::getDock() would return an object that is an instance of
# Dock type.  You can add dock item, of DockItem type, to the dock.  A
# dock item owns a widget that can be docked or floating.  The widget
# owns a frame that contain a vbox.  You can get the vbox through
# DockItem::get_vbox().
# 
# DocItem::gobj() returns an GtkWidget of GTK.  You can also get an
# C++ wrapper by calling DocItem::getWidget().
#
# Pybind wrap Dock and DockItem as Python type PYDock and PYDockItem
# respective.  PYDockItem.gobj() would return a Python wrapper for the
# GtkWidget.  PyDockItem.get_vbox() would return a Python wrapper for
# associated vobx.

cdef class PYDock

cdef class PYDockItem:
    cdef pybSPDesktop.DockItem *_thisptr

    UNATTACHED=0
    FLOATING_STATE=1
    DOCKED_STATE=2

    def __cinit__(self, PYDock dock, bytes name, bytes long_name,
                  bytes icon_name, int state):
        cdef pybSPDesktop.ustring _name = \
            pybSPDesktop.ustring(<char *>name)
        cdef pybSPDesktop.ustring _long_name = \
            pybSPDesktop.ustring(<char *>long_name)
        cdef pybSPDesktop.ustring _icon_name = \
            pybSPDesktop.ustring(<char *>icon_name)
        cdef pybSPDesktop.State _state
        
        _state = [pybSPDesktop.UNATTACHED,
                  pybSPDesktop.FLOATING_STATE,
                  pybSPDesktop.DOCKED_STATE][state]
        _placement = [pybSPDesktop.NONE,
                      pybSPDesktop.TOP,
                      pybSPDesktop.BOTTOM,
                      pybSPDesktop.RIGHT,
                      pybSPDesktop.LEFT,
                      pybSPDesktop.CENTER,
                      pybSPDesktop.FLOATING]

        self._thisptr = new pybSPDesktop.DockItem(deref(dock._thisptr),
                                                  _name, _long_name,
                                                  _icon_name, _state, _placement)

    def gobj(self):
        widget = self._thisptr.gobj()
        wrapper = gobj_api.newgobj(<C_GObject *>widget)
        return wrapper

    def get_vbox(self):
        vbox = self._thisptr.get_vbox()
        vbox_gobj = vbox.gobj()
        wrapper = gobj_api.newgobj(<C_GObject *>vbox_gobj)
        return wrapper

cdef class PYDock:
    cdef pybSPDesktop.Dock *_thisptr

    ITEM_ST_UNATTACHED=0
    ITEM_ST_FLOATING_STATE=1
    ITEM_ST_DOCKED_STATE=2

    def __cinit__(self, ptr):
        assert PyCObject_Check(ptr)
        self._thisptr = <pybSPDesktop.Dock *>PyCObject_AsVoidPtr(ptr)

    def new_item(self, bytes name, bytes long_name,
                 bytes icon_name, int state):
        item = PYDockItem(self, name, long_name, icon_name, state)
        return item

cdef class PYView:
    cdef pybSPDesktop.SPDesktop *_thisptr
    
    def __cinit__(self, ptr):
        assert PyCObject_Check(ptr)
        self._thisptr = <pybSPDesktop.SPDesktop *>PyCObject_AsVoidPtr(ptr)

cdef class PYSPDesktop(PYView):
    @property
    def selection(self):
        sel = self._thisptr.selection
        return wrapsel(sel)

    @property
    def doc(self):
        doc = self._thisptr.doc()
        return wrapdoc(doc)

    def getToplevel(self):
        win = self._thisptr.getToplevel()
        if win == NULL:
            return
        win_C_gobject = <C_GObject *>win.gobj()
        wrapper = gobj_api.newgobj(win_C_gobject)
        return wrapper

    def getDock(self):
        dock = self._thisptr.getDock()
        dock_co = PyCObject_FromVoidPtr(dock, NULL)
        wrapper = PYDock(dock_co)
        return wrapper

    def setCurrentLayer(self, PYNode node):
        spobj = node._get_spitem()
        self._thisptr.setCurrentLayer(spobj)
    
    def currentLayer(self):
        layer = self._thisptr.currentLayer()
        wrapper = gobj_api.newgobj(<C_GObject *>layer)
        return wrapper

    def __dealloc__(PYSPDesktop self):
        self._thisptr = NULL

    def __richcmp__(PYSPDesktop self, PYSPDesktop other,op):
        if op == 2:
            return self._thisptr == other._thisptr
        elif op == 3:
            return self._thisptr != other._thisptr
        elif op == 0:
            return self._thisptr < other._thisptr
        elif op == 1:
            return self._thisptr <= other._thisptr
        elif op == 4:
            return self._thisptr > other._thisptr
        elif op == 5:
            return self._thisptr >= other._thisptr


cdef class Inkscape(GObject):
    #
    # Inkscape support these glib signals.
    #
    signal_names = ['modify_selection', 'change_selection',
                    'change_subselection', 'set_selection',
                    'set_eventcontext', 'activate_desktop',
                    'deactivate_desktop', 'shut_down',
                    'dialogs_hide', 'dialogs_unhide',
                    'external_change']

    def get_all_desktops(self):
        cdef cpplist[SPDesktop_p] listbuf
        pybInkscape.inkscape_get_all_desktops(listbuf)
        
        ret = []
        itr = listbuf.begin()
        while itr == listbuf.end():
            dsk_co = PyCObject_FromVoidPtr(deref(itr), NULL)
            dsk = PYSPDesktop(dsk_co)
            ret.append(dsk)
        
        return ret

    @property
    def active_document(self):
        spdoc_co = PyCObject_FromVoidPtr(SP_ACTIVE_DOCUMENT, NULL)
        pyspdoc = PYSPDocument(spdoc_co)
        return pyspdoc

    @property
    def active_desktop(self):
        dsk_co = PyCObject_FromVoidPtr(SP_ACTIVE_DESKTOP, NULL)
        dsk = PYSPDesktop(dsk_co)
        return dsk

    cdef Application *_get_C_inkscape(self):
        return <Application *>pygobject_get(self)
    
    def refresh_display(self):
        _inkscape = self._get_C_inkscape()
        pybInkscape.inkscape_refresh_display(_inkscape)
    
    def exit(self):
        _inkscape = self._get_C_inkscape()
        pybInkscape.inkscape_exit(_inkscape)

cdef class PYSPAction:
    cdef pybVerb.SPAction *_thisptr
    
    def __cinit__(self, ptr):
        _verb = <pybVerb.SPAction *>PyCObject_AsVoidPtr(ptr)
        self._thisptr = _verb

    def perform(self):
        _action = self._thisptr
        pybVerb.sp_action_perform(_action, NULL)

    def set_active(self, int active):
        _action = self._thisptr
        pybVerb.sp_action_set_active(_action, active)
        
    def set_sensitive(self, int sensitive):
        _action = self._thisptr
        pybVerb.sp_action_set_sensitive(_action, sensitive)

cdef class PYVerb:
    cdef pybVerb.Verb *_thisptr
    
    def __cinit__(self, ptr):
        _verb = <pybVerb.Verb *>PyCObject_AsVoidPtr(ptr)
        self._thisptr = _verb
    
    def get_action(self, PYView view):
        _view = view._thisptr
        _verb = self._thisptr
        _action = _verb.get_action(_view)
        _action_co = PyCObject_FromVoidPtr(_action, NULL)
        action_wrapper = PYSPAction(_action_co)
        return action_wrapper

## \brief Return the verb specified by an ID.
#
# This is here
def verb_getbyid(bytes _id):
    _verb = pybVerb.pyb_verb_getbyid(<char *>_id)
    _verb_co = PyCObject_FromVoidPtr(_verb, NULL)
    wrapper = PYVerb(_verb_co)
    return wrapper

## \brief The type to implement NodeObserver in Python.
#
# This type will create an C++ proxy object for each instance.
cdef class PYNodeObserver:
    cdef NodeObserver_proxy *_proxy
    cdef PYDocument _pydoc

    def __cinit__(self):
        self._proxy = NULL
        self._pydoc = None

    def __dealloc__(self):
        del self._proxy

    #
    # When an observer is added for a node, the observer would be notified.
    # The node use the notification to get some information associated with
    # the node.  For example, the associated document object.
    #
    # \see PYNode::addObserver() and PYNode::addSubtreeObserver() 
    #
    def _notify_add_observer(self, PYNode node):
        pydoc = <PYDocument>node.doc
        if self._pydoc != pydoc and self._pydoc != None:
            raise RuntimeError, "an PYNodeObserver can not be added for nodes of different documents"
        self._pydoc = pydoc
    
    def _notifyChildAdded(self, node, child, prev):
        _node = wrapnode(<Node *>PyCObject_AsVoidPtr(node), self._pydoc)
        _child = wrapnode(<Node *>PyCObject_AsVoidPtr(child), self._pydoc)
        if prev:
            _prev = wrapnode(<Node *>PyCObject_AsVoidPtr(prev), self._pydoc)
        else:
            _prev = None
        
        self.child_added(_node, _child, _prev)

    def _notifyChildRemoved(self, node, child, prev):
        _node = wrapnode(<Node *>PyCObject_AsVoidPtr(node), self._pydoc)
        _child = wrapnode(<Node *>PyCObject_AsVoidPtr(child), self._pydoc)
        if prev:
            _prev = wrapnode(<Node *>PyCObject_AsVoidPtr(prev), self._pydoc)
        else:
            _prev = None
        
        self.child_removed(_node, _child, _prev)

    def _notifyChildOrderChanged(self, node, child, old_prev, new_prev):
        _node = wrapnode(<Node *>PyCObject_AsVoidPtr(node), self._pydoc)
        _child = wrapnode(<Node *>PyCObject_AsVoidPtr(child), self._pydoc)
        if old_prev:
            _old_prev = wrapnode(<Node *>PyCObject_AsVoidPtr(old_prev), self._pydoc)
        else:
            _old_prev = None
        if new_prev:
            _new_prev = wrapnode(<Node *>PyCObject_AsVoidPtr(new_prev), self._pydoc)
        else:
            _new_prev = None
        
        self.child_order_changed(_node, _child, _old_prev, _new_prev)

    def _notifyContentChanged(self, node, old_content, new_content):
        _node = wrapnode(<Node *>PyCObject_AsVoidPtr(node), self._pydoc)
        self.content_changed(_node, old_content, new_content)

    def _notifyAttributeChanged(self, node, name, old_value, new_value):
        _node = wrapnode(<Node *>PyCObject_AsVoidPtr(node), self._pydoc)
        name_str = <char *>g_quark_to_string(name)
        self.attribute_changed(_node, name_str, old_value, new_value)
    
    def notifyChildAdded(self, node, child, prev):
        raise NotImplementedError, "notifyChildAdded is not implemented"
    
    def notifyChildRemoved(self, node, child, prev):
        raise NotImplementedError, "notifyChildRemoved is not implemented"

    def notifyChildOrderChanged(self, node, child, old_prev, new_prev):
        raise NotImplementedError, "notifyChildOrderChanged is not implemented"

    def notifyContentChanged(self, node, old_content, new_content):
        raise NotImplementedError, "notifyContentChanged is not implemented"
    
    def notifyAttributeChanged(self, node, name, old_value, new_value):
        raise NotImplementedError, "notifyAttributeChanged is not implemented"

## \brief Get Python wrapper of the SPDesktop pointed by an PyGPointer.
#
# \param ptr is an PyGPointer.
#
def GPointer_2_PYSPDesktop(ptr):
    c_desktop = (<C_PyGPointer *>ptr).pointer
    ptr_co = PyCObject_FromVoidPtr(c_desktop, NULL)
    desktop = PYSPDesktop(ptr_co)
    return desktop

def GObjfromptr(ptr):
    co = PyCObject_FromVoidPtr(<C_PyGPointer *>ptr, NULL)
    return co

cdef wrapnode(Node* node, doc, memoryerror=False):
    if node == NULL:
        if memoryerror:
            raise MemoryError, 'can not create node'
        return None
    if node._wrapper != NULL:
        ptr = <PyObject *>node._wrapper
        obj = <object>ptr
        return obj
    node_co = PyCObject_FromVoidPtr(node, NULL)
    wrapper = PYNode(node_co, doc)
    node._wrapper = NULL
    return wrapper

cdef wrapobj(pybSPDesktop.SPObject *obj):
    if obj == NULL:
        return None
    wrapper = gobj_api.newgobj(<C_GObject *>obj)
    return wrapper

#SPObj to Node
cdef wrapobjtonode(pybSPDesktop.SPObject *obj):
    if obj == NULL:
        return None
    spdoc = wrapdoc(obj.document)
    if spdoc == None:
        return None
    wrapper = wrapnode(obj.repr, spdoc.rdoc)
    return wrapper

cdef wrapdoc(pybSPDesktop.SPDocument *doc):
    doc_co = PyCObject_FromVoidPtr(doc, NULL)
    pyspdoc = PYSPDocument(doc_co)
    return pyspdoc

cdef wrapsel(Selection *sel):
    sel_co = PyCObject_FromVoidPtr(sel, NULL)
    wrapper = PYSelection(sel_co)
    return wrapper

def unserattrib(attrib):
    return dict(subattrib.split(":") for subattrib in attrib.split(";") if subattrib)

def serattrib(d):
    return ";".join(":".join(item) for item in d.items())

def test():
    print "Testing 1, 2, 3"

def version():
    print "Wed Apr 29 20:10 version"

## \page reg_gobject_class How PyGObject works?
#
# If you want to register a gobject type with PyGObject to integrated
# it with PyGObject, you can call
# gobject._PyGObject_API.register_class() to register a PyTypeObject
# as the wrapper of the type specified by a GType value.  And, when
# you have an GObject with specified type, you can call
# gobject._PyGObject_API.newgobj() to generate a wrapper from the
# associated PyTypeObject.  The PyTypeObject must inherit
# PyGObjectType, it means struct of instances of the specified have
# with PyGObject as first field.
#
# gobject._PyGObject_API is actually an CObject, its content is with
# _PyGObject_Functions type. (see pygobject.h) So, it can only used by
# native module by casting it to _PyGObject_Functions.  It is a set of
# functions that PyGObject provided to other modules to cowork with
# PyGObject.
#
# PyGObject would use inheritance of GObject to create inheritance
# tree of wrappers.  So, inheritance specified in your PyTypeObject is
# useless.  PyGObject would also generate a dummy type for wrappers of
# GObjects that the programmer does not specified a wrapper type for
# them.
#
# The C function gobject._PyGObject_API.gobject_get() is used to
# return associated GObject of a wrapper.  So, you wrapper code can
# use it to get associated GObject.
#
# PyGObject will try to keep one-to-one mapping between any GObject
# and associated wrapper.  It also keep consistent of reference
# counter.  So, you don't have to care about it.
#
# PyGObject does not call tp_new and tp_init of PyTypeObject of a
# wrapper type.  Calling tp_new and tp_init is defined by object
# protocol of Python.  It is implemented by type 'type' of Python.
# So, when you call on a type object, tp_call of PyTypeObject of type
# 'type' would be called to run the protocol and return an object.
# But, native code does not works like this, especially PyGObject.
# So, you can not relys on tp_new and tp_init.
#
# Cython rely on tp_new and tp_init.  And, __cinit__ used by Cython is
# called in tp_new.  So, avoid to use __init__, __new__, and __cinit__
# for wrapper types.

cdef _create_inkscape_wrapper():
    global inkscape, gobj_api
    
    #
    # gobject._PyGObject_API is a set of C functions exposed by pygtk.
    # It provides capabilities to integrate other GObjects provided by
    # other library binding.
    #
    _gobj_api = PyCObject_AsVoidPtr(gobject._PyGObject_API)
    gobj_api = <_PyGObject_Functions *>_gobj_api
    g_inkscape = pybInkscape.inkscape_get_instance()

    #
    # Register GObject classes for Inkscape
    #
    wrapper = gobj_api.newgobj(<C_GObject *>g_inkscape)
    return wrapper

cdef _PyGObject_Functions *gobj_api

inkscape = _create_inkscape_wrapper()
