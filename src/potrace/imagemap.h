#ifndef __IMAGEMAP_H__
#define __IMAGEMAP_H__

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
    GrayMap * (*getGaussian)(GrayMap *me);

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




/*#########################################################################
### R G B   M A P
#########################################################################*/

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGB;



typedef struct RgbMap_def RgbMap;

/**
 *
 */
struct RgbMap_def
{

    /*#################
    ### METHODS
    #################*/

    /**
     *
     */
    void (*setPixel)(RgbMap *me, int x, int y, int r, int g, int b);


    /**
     *
     */
    void (*setPixelRGB)(RgbMap *me, int x, int y, RGB rgb);

    /**
     *
     */
    RGB (*getPixel)(RgbMap *me, int x, int y);

    /**
     *
     */
    RgbMap * (*getGaussian)(RgbMap *me);

    /**
     *
     */
    int (*writePPM)(RgbMap *me, char *fileName);



    /**
     *
     */
    void (*destroy)(RgbMap *me);



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
    RGB *pixels;

};



#ifdef __cplusplus
extern "C" {
#endif

RgbMap *RgbMapCreate(int width, int height);

#ifdef __cplusplus
}
#endif




#endif /* __IMAGEMAP_H__ */

/*#########################################################################
### E N D    O F    F I L E
#########################################################################*/
