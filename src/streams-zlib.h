#ifndef __STREAMS_ZLIB_H_
#define __STREAMS_ZLIB_H_

#include "streams-handles.h"

#include <zlib.h>
#include <iostream>

namespace Inkscape {
/**
 * ZlibBuffer (Abstract class)
 */

//TODO: add uflow, oflow for unbuffered IO
class ZlibBuffer : public std::streambuf
{
public:

    ZlibBuffer(URIHandle& urih);
    virtual ~ZlibBuffer() {}
    
    virtual int allocate_buffers();
    virtual int reallocate_buffers(int new_getsize, int new_putsize);
    virtual int underflow();
    virtual int overflow(int c = EOF);
    virtual int flush_output();

protected:

    virtual int init_inflation();
    virtual int consume_header() = 0;
    virtual int inflate(guint8 *in_buffer, int nbytes);
    URIHandle *get_urihandle() { return _urihandle; }

private:

    ZlibBuffer& operator=(ZlibBuffer const& rhs);
    ZlibBuffer(ZlibBuffer const& rhs);

    z_stream _zs;
    int putsize, getsize;//sizes of in and out buffers
    bool autoinflate;
    URIHandle *_urihandle;

};

}

#endif
