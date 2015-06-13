
set(pybind_SRC
      pybgc.cpp
      pybind.cpp
      pyb_NodeObserver_proxy.cpp
      pyb_slot_proxy.cpp
      pybInkscape.cpp

      # Headers
      pybgc.h
      pybind.h
      pybindtools.h
      pybInkscape.h
      pyb_NodeObserver_proxy.h
      pyb_slot_proxy.h
      pyb_verb.h
)

add_inkscape_lib(pybind_LIB "${pybind_SRC}")
