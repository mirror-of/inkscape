
set(pybind_SRC
      pybgc.cpp
      pybind.cpp
      pyb_NodeObserver_proxy.cpp
      pyb_slot_proxy.cpp
      pybInkscape.cpp
      #${CMAKE_CURRENT_BINARY_DIR}/src/pybind/pybInkscape.cpp

      # Headers
      pybgc.h
      pybind.h
      pybindtools.h
      pybInkscape.h
      pyb_NodeObserver_proxy.h
      pyb_slot_proxy.h
      pyb_verb.h
)

set(pybInkscape_cython_SRC
      pybInkscape.pyx
      pybApplication.pxd
      pybDocument.pxd
      pybElementNode.pxd
      pybgc.pxd
      pybindtools.pxd
      pybInkscape.pxd
      pybNodeObserver.pxd
      pybNode.pxd
      pybSelection.pxd
      pybSPDesktop.pxd
      pybVerb.pxd
      pybView.pxd
      pybXML.pxd
      pygobject.pxd
)

add_custom_command( OUTPUT pybInkscape.cpp
    COMMAND cython --cplus -a ${CMAKE_CURRENT_SOURCE_DIR}/pybInkscape.pyx -o pybInkscape.cpp
    COMMENT "Generating cpp file from cython..." 
    DEPENDS ${pybInkscape_cython_SRC})
#add_dependencies(pybInkscape.cpp ${pybInkscape_cython_SRC})

#set(pybind_entry_SRC
#      pybind.h
#      pybind.cpp
#)

#add_inkscape_source("${pybind_entry_SRC}")
add_inkscape_lib(pybind_LIB "${pybind_SRC}")
