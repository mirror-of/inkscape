#include "streams-zlib.h"
#include <cstring>

namespace Inkscape {

// This is the initial buffersize for the stream and
// zipbuffers (the streambuffers expand as needed).
const unsigned int BUFSIZE_STREAM = 4096; 

/**
 * ZlibBuffer (Abstract class)
 */

ZlibBuffer::ZlibBuffer(URIHandle& urih) 
    :  putsize(BUFSIZE_STREAM), getsize(BUFSIZE_STREAM), autoinflate(true), 
       _urihandle(&urih)
      
{ 
    init_inflation();
}


int ZlibBuffer::allocate_buffers()
{
    if (!eback()) {
	char *buf = new char[getsize + putsize];
	setg(buf, buf , buf);
	buf += getsize;
	setp(buf, buf + putsize);
	return 1;
    }
    return 0;
}

int ZlibBuffer::reallocate_buffers(int new_getsize, int new_putsize)
{
    char *new_buffer = new char[new_getsize + new_putsize];
    
    std::memcpy(new_buffer, eback(), getsize);
    std::memcpy(new_buffer, eback() + getsize, putsize);
    
    setg(new_buffer, new_buffer + (gptr() - eback()), 
	 new_buffer + new_getsize);
    new_buffer += new_getsize;
    setp(new_buffer, new_buffer + new_putsize);
    
    getsize = new_getsize;
    putsize = new_putsize;

    return 1;
}

int ZlibBuffer::underflow()
{
    if (eback() == 0 && allocate_buffers() == 0)
    	return EOF;

    int nbytes;

    if (!autoinflate) {
	//fixme: untested
	nbytes = get_urihandle()->read(eback(), BUFSIZE_STREAM);

	if (nbytes == EOF)
	    return EOF;
	else if (nbytes == 0)
	    return EOF;
	
    	setg (eback(), eback() + nbytes, eback() + getsize);
    } else {
	guint8 buf[BUFSIZE_STREAM];
	nbytes = get_urihandle()->read(buf, BUFSIZE_STREAM);	

	if (nbytes == EOF)
	    return EOF;
	else if (nbytes == 0)
	    return EOF;

	inflate(buf, nbytes);
    }

    return *(unsigned char *)gptr();
}

int ZlibBuffer::overflow(int c)
{
    if (c == EOF)
	return flush_output();

    if (pbase() == 0 && allocate_buffers() == 0)
	return EOF;

    if (pptr() >= epptr() && 
	flush_output() == EOF)
	return EOF;

    putchar(c);
    
    if (pptr() >= epptr() && 
	flush_output() == EOF)
	return EOF;

    return c;
}

int ZlibBuffer::flush_output()
{
    if (pptr() <= pbase())
	return 0;
    int len = pptr() - pbase();
    int nbytes = get_urihandle()->write(pbase(), len);
    setp(pbase(), pbase() + BUFSIZE_STREAM);
    if (len == nbytes)
	return 0;
    else
	return EOF;
}

int ZlibBuffer::init_inflation()
{
    memset(&_zs, 0, sizeof(z_stream));
    
    _zs.zalloc = Z_NULL;
    _zs.zfree = Z_NULL;
    _zs.opaque = Z_NULL;
    
    if(inflateInit2(&_zs, -15) != Z_OK) {
	fprintf(stderr,"error initializing inflation!\n");
	return 0;
    }

    return 1;
}

int ZlibBuffer::inflate(guint8 *in_buffer, int nbytes)
{
    //fixme: reduce the number of memcpy functions for efficiency
    GByteArray *gba = g_byte_array_new();
    guint8 out_buffer[BUFSIZE_STREAM];
        
    _zs.avail_in = 0;
    guint32 crc = crc32(crc, Z_NULL, 0);
    
    if (!_zs.avail_in) {
	_zs.avail_in = nbytes;
	_zs.next_in = (Bytef *)in_buffer;
	crc = crc32(crc, (Bytef *)in_buffer, _zs.avail_in);
    }
    do {
	_zs.next_out = out_buffer;
	_zs.avail_out = BUFSIZE_STREAM;

	int ret = ::inflate(&_zs, Z_NO_FLUSH);
	if (BUFSIZE_STREAM != _zs.avail_out) {
	    unsigned int tmp_len = BUFSIZE_STREAM - _zs.avail_out;
	    guint8 *tmp_bytes = (guint8 *)g_malloc(sizeof(guint8) 
						   * tmp_len);
	    std::memcpy(tmp_bytes, out_buffer, tmp_len);
	    g_byte_array_append(gba, tmp_bytes, tmp_len);
	}
	
	if (ret == Z_STREAM_END) {
	    break;
	}
	if (ret != Z_OK) {
	    std::fprintf(stderr, "decompression error %d\n", ret);
	    break;
	}
    } while (_zs.avail_in);

    if (gba->len + gptr() - eback() > getsize) 
	reallocate_buffers(gba->len + gptr() - eback() + BUFSIZE_STREAM, 
			   putsize);
    
    std::memcpy(gptr(), gba->data, gba->len);
    g_byte_array_free(gba, TRUE);

    return 1;
}    

} // namespace Inkscape
