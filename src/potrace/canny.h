#ifndef __CANNY_H__
#define __CANNY_H__

#include "imagemap.h"

#include <gdk-pixbuf/gdk-pixbuf.h>

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*#########################################################################
### C A N N Y    E D G E    D E T E C T I O N
#########################################################################*/



#ifdef __cplusplus
extern "C" {
#endif

GrayMap *grayMapCanny(GrayMap *gm, 
             double lowThreshold, double highThreshold);

GdkPixbuf *gdkCanny(GdkPixbuf *img,
            double lowThreshold, double highThreshold);



#ifdef __cplusplus
}
#endif


#endif /* __CANNY_H__ */

/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
