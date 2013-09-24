#ifndef __RECOLOR_WHEEL_NODE_H__
#define __RECOLOR_WHEEL_NODE_H__

struct _RecolorWheelNode
{
    gdouble      x;
    gdouble      y;
    gfloat     _color[3];
    gboolean   unpublished;
    //gboolean   dragging;
};

typedef struct _RecolorWheelNode   RecolorWheelNode;

#endif

