#ifndef __GRAYMAP_GDK_H__
#define __GRAYMAP_GDK_H__

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#include "graymap.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

/*#########################################################################
### G R A Y M A P --- GDK
#########################################################################*/



#ifdef __cplusplus
extern "C" {
#endif

GrayMap *gdkPixbufToGrayMap(GdkPixbuf *buf);



GdkPixbuf *grayMapToGdkPixbuf(GrayMap *grayMap);


#ifdef __cplusplus
}
#endif


#endif /* __GRAYMAP_GDK_H__ */

/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
