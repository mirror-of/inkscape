#ifndef __RECOLOR_WHEEL_NODE_H__
#define __RECOLOR_WHEEL_NODE_H__

struct _RecolorWheelNode
{
    gint     _id; //for the object id in XML Tree.
    //SPColor color; //for the color value. (Not declared, so not used)
    gdouble      x;
    gdouble      y;
    gfloat     _color[3];
    gboolean   main;
    //gboolean   dragging;
};

struct _RecolorNodeExchangeData
{
    gchar* id;
    float h;
    float s;
    float v;
};


typedef struct _RecolorWheelNode   RecolorWheelNode;
typedef struct _RecolorNodeExchangeData RecolorNodeExchangeData;


#endif

