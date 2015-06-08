#ifndef __PYX_HAVE__pybInkscape
#define __PYX_HAVE__pybInkscape


#ifndef __PYX_HAVE_API__pybInkscape

#ifndef __PYX_EXTERN_C
  #ifdef __cplusplus
    #define __PYX_EXTERN_C extern "C"
  #else
    #define __PYX_EXTERN_C extern
  #endif
#endif

__PYX_EXTERN_C DL_IMPORT(PyObject) *wrapnode(Inkscape::XML::Node *, PyObject *);
__PYX_EXTERN_C DL_IMPORT(PyObject) *wrapnodecustom(Inkscape::XML::Node *, PyObject *);

#endif /* !__PYX_HAVE_API__pybInkscape */

#if PY_MAJOR_VERSION < 3
PyMODINIT_FUNC initpybInkscape(void);
#else
PyMODINIT_FUNC PyInit_pybInkscape(void);
#endif

#endif /* !__PYX_HAVE__pybInkscape */
