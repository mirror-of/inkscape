#ifndef __CANNY_H__
#define __CANNY_H__

#include <gdk-pixbuf/gdk-pixbuf.h>

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*#########################################################################
### G R A Y M A P --- GDK
#########################################################################*/



#ifdef __cplusplus
extern "C" {
#endif

GdkPixbuf *gdkCanny(GdkPixbuf *img);


#ifdef __cplusplus
}
#endif


#endif /* __CANNY_H__ */

/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
