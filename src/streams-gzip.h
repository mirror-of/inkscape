#ifndef __STREAMS_GZIP_H_
#define __STREAMS_GZIP_H_

#include "streams-zlib.h"

namespace Inkscape {

class GZipBufferException : public std::exception {};

class GZipHeaderException : public GZipBufferException
{
public:
    const char *what() const throw() { return "Invalid gzip file"; }
};

/**
 * GZipBuffer
 */

class GZipBuffer : public ZlibBuffer
{
public:
    
    GZipBuffer(URIHandle& urih) //throws GZipHeaderException
	: ZlibBuffer(urih)
    { consume_header(); } 
    ~GZipBuffer() {}
    
protected:

    int consume_header() throw(GZipHeaderException);
    void check_signature(guint8 *data) throw(GZipHeaderException);
    void check_flags(guint8 *data) throw(GZipHeaderException);
    gchar *get_filename();
    gchar *get_comment();
    guint16 get_crc();
    void get_extrafield();
    gchar *read_string() throw(GZipHeaderException);
 
private:
    
    GZipBuffer& operator=(GZipBuffer const& rhs);
    GZipBuffer(GZipBuffer const& rhs);

};

class igzipstream : public std::istream {
public:

    igzipstream(std::streambuf& sb) : std::istream(&sb) {}
    ~igzipstream() { std::ios::init(0); }
    
    GZipBuffer *rdbuf() { return (GZipBuffer *)std::ios::rdbuf(); }
    GZipBuffer *operator ->() { return rdbuf(); }

};

} // namespace Inkscape
#endif
