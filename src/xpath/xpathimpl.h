#ifndef __XPATHIMPL_H__
#define __XPATHIMPL_H__

#include "xpath.h"

#include <string>

/**
 *  The actual internal imp
 */
class XPathImpl : public XPath
{
    public:

    /**
     *
     */
    XPathImpl();

    /**
     *
     */
    ~XPathImpl();

    /**
     *
     */
    bool parse(const char *inputStr);
    
    /**
     *
     */
    char *parseBuf;

    /**
     *
     */
    int parseLen;

    /**
     *
     */
    int parsePos;

    double numval;

    std::string strval;


};



#endif /* __XPATHIMPL_H__ */
