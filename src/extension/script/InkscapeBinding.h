#ifndef __INKSCAPE_BINDING_H__
#define __INKSCAPE_BINDING_H__

namespace Inkscape
{
namespace Extension
{
namespace Script
{

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
