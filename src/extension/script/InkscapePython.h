#ifndef __INKSCAPE_PYTHON_H__
#define __INKSCAPE_PYTHON_H__


class InkscapePython
{
public:

    /*
     *
     */
    InkscapePython();
    

    /*
     *
     */
    virtual ~InkscapePython();
    
    

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












#endif /*__INKSCAPE_PYTHON_H__ */

