from pybView cimport View
from pygobject cimport gchar

cdef extern from "verbs.h" namespace "Inkscape":
    cppclass Verb

cdef extern from "helper/action.h":
    struct SPAction:
        pass

    void sp_action_perform(SPAction *action, void *data)
    void sp_action_set_active(SPAction *action, unsigned active)
    void sp_action_set_sensitive(SPAction *action, unsigned sensitive)

cdef extern from "pybind/pyb_verb.h":
    Verb *pyb_verb_getbyid(gchar *_id)

cdef extern from "verbs.h" namespace "Inkscape":
    cppclass Verb:
        SPAction *get_action(View *view)
        unsigned int get_code()
