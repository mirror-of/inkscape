#ifndef __INKSCAPE_PERL_H__
#define __INKSCAPE_PERL_H__


class InkscapePerl
{
public:

    /*
     *
     */
    InkscapePerl();
    

    /*
     *
     */
    virtual ~InkscapePerl();
    
    

    /*
     *
     */
    bool interpretString(char *str);
    
    

    /*
     *
     */
    bool interpretFile(char *fileName);
    
private:


};



#endif /*__INKSCAPE_PERL_H__ */

