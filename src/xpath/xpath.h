#ifndef __XPATH_H__
#define __XPATH_H__

class XPath
{
    public:

    /**
     *
     */
    XPath(){};

    /**
     *
     */
    virtual ~XPath() {};

    /**
     *
     */
    static XPath *create();


    /**
     *
     */
    virtual bool parse(const char *inputStr) = 0;

};




#endif /*__XPATH_H__*/



