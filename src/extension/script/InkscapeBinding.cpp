

#include <stdio.h>
#include "InkscapeBinding.h"

namespace Inkscape {
namespace Extension {
namespace Script {


class InkscapeImpl;
class DesktopImpl;
class DocumentImpl;


//#########################################################################
//# D O C U M E N T
//#########################################################################
class DocumentImpl : public Document
{
public:
    DocumentImpl();
    
    virtual ~DocumentImpl();
    
    virtual void hello();
    
private:


};


DocumentImpl::DocumentImpl()
{


}
    
DocumentImpl::~DocumentImpl()
{


}
    
void DocumentImpl::hello()
{
    printf("######## HELLO, WORLD! #######\n");
}



//#########################################################################
//# D E S K T O P
//#########################################################################
class DesktopImpl : public Desktop
{
public:
    DesktopImpl();
    
    virtual ~DesktopImpl();
    
    virtual Document *getDocument();
    
private:

    DocumentImpl document;

};


DesktopImpl::DesktopImpl()
{


}
    
DesktopImpl::~DesktopImpl()
{


}
    

Document *DesktopImpl::getDocument()
{
    return &document;
}



//#########################################################################
//# I N K S C A P E
//#########################################################################

class InkscapeImpl : public Inkscape
{
public:
    InkscapeImpl();
    
    virtual ~InkscapeImpl();
    
    virtual Desktop *getDesktop();
    
private:

    DesktopImpl desktop;

};

Inkscape *getInkscape()
{
    Inkscape *inkscape = new InkscapeImpl();
    return inkscape;
}


InkscapeImpl::InkscapeImpl()
{

}

    
InkscapeImpl::~InkscapeImpl()
{

}
    

Desktop *InkscapeImpl::getDesktop()
{
    return &desktop;

}




}//namespace Script
}//namespace Extension
}//namespace Inkscape


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
