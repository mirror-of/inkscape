#ifndef __INKSCAPE_IO_URISTREAM_H__
#define __INKSCAPE_IO_URISTREAM_H__
/**
 * This should be the only way that we provide sources/sinks
 * to any input/output stream.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "uri.h"
#include "InkscapeStream.h"


namespace Inkscape
{
namespace IO
{

//#########################################################################
//# U R I    I N P U T    S T R E A M
//#########################################################################

/**
 * This class is for receiving a stream of data from a resource
 * defined in a URI
 */
class UriInputStream : public InputStream
{

public:

    UriInputStream(Inkscape::URI &source) throw(StreamException);
    
    virtual ~UriInputStream() throw(StreamException);
    
    virtual int available() throw(StreamException);
    
    virtual void close() throw(StreamException);
    
    virtual int get() throw(StreamException);
    
private:

    bool closed;
    
    FILE *inf;
    
    Inkscape::URI &uri;

}; // class UriInputStream




//#########################################################################
//# U R I    O U T P U T    S T R E A M
//#########################################################################

/**
 * This class is for sending a stream to a destination resource
 * defined in a URI
 *
 */
class UriOutputStream : public OutputStream
{

public:

    UriOutputStream(Inkscape::URI &destination) throw(StreamException);
    
    virtual ~UriOutputStream() throw(StreamException);
    
    virtual void close() throw(StreamException);
    
    virtual void flush() throw(StreamException);
    
    virtual void put(int ch) throw(StreamException);

private:

    bool closed;
    
    FILE *outf;
    
    Inkscape::URI &uri;


}; // class UriOutputStream



} // namespace IO
} // namespace Inkscape


#endif /* __INKSCAPE_IO_URISTREAM_H__ */
