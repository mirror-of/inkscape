/**
 * Our base String stream classes.  We implement these to
 * be based on Glib::ustring
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "UriStream.h"

namespace Inkscape
{
namespace IO
{

//#########################################################################
//# U R I    I N P U T    S T R E A M
//#########################################################################


/**
 *
 */ 
UriInputStream::UriInputStream(Inkscape::URI &source)
                    throw (StreamException): uri(source) 
{
    char *cpath = (char *)uri.toNativeFilename();
    printf("path:'%s'\n", cpath);
    inf = fopen(cpath, "rb");
    if (!inf)
       {
       Glib::ustring err = "UriInputStream cannot open file ";
       err += cpath;
       throw StreamException(err);
       }
    closed = false;
}


/**
 *
 */ 
UriInputStream::~UriInputStream() throw(StreamException)
{
    close();
}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this input stream without blocking by the next caller of a method for
 * this input stream.
 */ 
int UriInputStream::available() throw(StreamException)
{
    return 0;
}

    
/**
 *  Closes this input stream and releases any system resources
 *  associated with the stream.
 */ 
void UriInputStream::close() throw(StreamException)
{
    closed = true;
    if (closed || !inf)
        return;
    fclose(inf);
    inf = NULL;
}
    
/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */ 
int UriInputStream::get() throw(StreamException)
{
    if (closed || !inf)
        return -1;
        
    if (feof(inf))
        return -1;
        
    return fgetc(inf);
}
   




//#########################################################################
//#  U R I    O U T P U T    S T R E A M
//#########################################################################

/**
 *
 */ 
UriOutputStream::UriOutputStream(Inkscape::URI &destination)
                    throw (StreamException): uri(destination) 
{
    char *cpath = (char *)uri.getPath();
    outf = fopen(cpath, "wb");
    if (!outf)
       {
       Glib::ustring err = "UriOutputStream cannot open file ";
       err += cpath;
       throw StreamException(err);
       }
    closed = false;
}


/**
 *
 */ 
UriOutputStream::~UriOutputStream() throw(StreamException)
{
    close();
}

/**
 * Closes this output stream and releases any system resources
 * associated with this stream.
 */ 
void UriOutputStream::close() throw(StreamException)
{
    if (closed || !outf)
        return;
    fflush(outf);
    fclose(outf);
    outf=NULL;
    closed = true;
}
    
/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */ 
void UriOutputStream::flush() throw(StreamException)
{
    if (closed || !outf)
        return;
    fflush(outf);
}
    
/**
 * Writes the specified byte to this output stream.
 */ 
void UriOutputStream::put(int ch) throw(StreamException)
{
    if (closed || !outf)
        return;
    unsigned char uch = (unsigned char)(ch & 0xff);
    fputc(uch, outf);
    //fwrite(uch, 1, 1, outf);
}





} // namespace IO
} // namespace Inkscape


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
