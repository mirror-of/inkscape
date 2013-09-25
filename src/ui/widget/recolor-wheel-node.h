#ifndef __RECOLOR_WHEEL_NODE_H__
#define __RECOLOR_WHEEL_NODE_H__

struct _RecolorWheelNode
{
    gdouble      x;
    gdouble      y;
    gfloat     _color[3];
    gboolean   unpublished;
    //sigc::signal<void,std::string,gfloat[3],_RecolorWheelNode> _node_updated_signal;
    //gboolean   dragging;
};

typedef struct _RecolorWheelNode   RecolorWheelNode;

#endif

