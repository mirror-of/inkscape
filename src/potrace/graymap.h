#ifndef __GRAYMAP_H__
#define __GRAYMAP_H__

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/*#########################################################################
### G R A Y M A P
#########################################################################*/


typedef struct GrayMap_def GrayMap;

/**
 *
 */
struct GrayMap_def
{

    /*#################
    ### METHODS
    #################*/

    /**
     *
     */
    void (*setPixel)(GrayMap *me, int x, int y, unsigned long val);

    /**
     *
     */
    unsigned long (*getPixel)(GrayMap *me, int x, int y);

    /**
     *
     */
    int (*writePPM)(GrayMap *me, char *fileName);



    /**
     *
     */
    void (*destroy)(GrayMap *me);



    /*#################
    ### FIELDS
    #################*/

    /**
     *
     */
    int width;

    /**
     *
     */
    int height;

    /**
     *
     */
    unsigned long *pixels;

};

#ifdef __cplusplus
extern "C" {
#endif

GrayMap *GrayMapCreate(int width, int height);

#ifdef __cplusplus
}
#endif


#endif /* __GRAYMAP_H__ */

/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
