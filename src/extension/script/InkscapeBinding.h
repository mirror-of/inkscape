#ifndef __INKSCAPE_BINDING_H__
#define __INKSCAPE_BINDING_H__

namespace Inkscape {
namespace Extension {
namespace Script {

class Inkscape;
class Desktop;
class Document;

Inkscape *getInkscape();

class Inkscape
{
public:
    Inkscape(){}
    
    virtual ~Inkscape(){};
    
    virtual Desktop *getDesktop() = 0;

    virtual void about() = 0;

};


class Desktop
{

public:
    Desktop() {}
    
    virtual ~Desktop(){};
    
    virtual Document *getDocument() = 0;



};


class Document
{

public:
    Document() {};
    
    virtual ~Document(){};
    
    virtual void hello() = 0;


};


}//namespace Script
}//namespace Extension
}//namespace Inkscape



#endif  /*__INKSCAPE_BINDING_H__*/

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
