#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

#include "graymap.h"
#include "rgbmap.h"

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*#########################################################################
### Q U A N T I Z E
#########################################################################*/



#ifdef __cplusplus
extern "C" {
#endif

GrayMap *quantizeBand(RgbMap *rgbmap, int nrColors);


#ifdef __cplusplus
}
#endif


#endif /* __QUANTIZE_H__ */

/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
