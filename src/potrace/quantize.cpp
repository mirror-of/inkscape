#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "quantize.h"


/*#########################################################################
### Q U A N T I Z A T I O N
#########################################################################*/




GrayMap *quantizeBand(RgbMap *rgbmap, int nrColors)
{

    GrayMap *gm = GrayMapCreate(rgbmap->width, rgbmap->height);

    return gm;
}





