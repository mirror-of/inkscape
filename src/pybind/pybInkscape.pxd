from libcpp.list cimport list as cpplist
from pybApplication cimport Application
from pybSPDesktop cimport SPDesktop, SPDocument

ctypedef SPDesktop *SPDesktop_p

cdef extern from "inkscape.h":
    Application *inkscape_get_instance()
    
    cdef SPDocument *SP_ACTIVE_DOCUMENT
    cdef SPDesktop *SP_ACTIVE_DESKTOP

    void inkscape_activate_desktop (SPDesktop * desktop)
    void inkscape_switch_desktops_next ()
    void inkscape_switch_desktops_prev ()
    void inkscape_get_all_desktops (cpplist[SPDesktop_p] & listbuf)
    
    void inkscape_dialogs_hide ()
    void inkscape_dialogs_unhide ()
    void inkscape_dialogs_toggle ()
    
    void inkscape_external_change ()
    void inkscape_subselection_changed (SPDesktop *desktop)
    
    void inkscape_refresh_display (Application *inkscape)
    
    void inkscape_exit (Application *inkscape)
    pass
