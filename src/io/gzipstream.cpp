/**
 * Zlib-enabled input and output streams
 *
 * This is a thin wrapper of libz calls, in order
 * to provide a simple interface to our developers
 * for gzip input and output.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "gzipstream.h"

#include  <zlib.h>

namespace Inkscape
{
namespace IO
{

//#########################################################################
//# G Z I P    I N P U T    S T R E A M
//#########################################################################


/**
 *
 */ 
GzipInputStream::GzipInputStream(InputStream &sourceStream)
                    : BasicInputStream(sourceStream)
{

    outputBufPos       = 0;
    outputBufLen       = 0;
    outputBuf          = NULL;

    totalIn            = 0;
    totalOut           = 0;
    
    loaded = false;
}

/**
 *
 */ 
GzipInputStream::~GzipInputStream()
{
    close();
}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this input stream without blocking by the next caller of a method for
 * this input stream.
 */ 
int GzipInputStream::available()
{
    if (closed || !outputBuf)
        return 0;
    return outputBufLen - outputBufPos;
}

    
/**
 *  Closes this input stream and releases any system resources
 *  associated with the stream.
 */ 
void GzipInputStream::close()
{
    if (closed)
        return;
    if (outputBuf)
    {
        free(outputBuf);
        outputBuf = 0;
    }
    closed = true;
}
    
/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */ 
int GzipInputStream::get()
{
    if (closed)
        return -1;
    if (!loaded && !load())
        {
        closed=true;
        return -1;
        }

    loaded = true;

    if (outputBufPos >= outputBufLen)
        return -1;
        
    int ch = (int) outputBuf[outputBufPos++];
    return ch;
}

#define FTEXT 0x01
#define FHCRC 0x02
#define FEXTRA 0x04
#define FNAME 0x08
#define FCOMMENT 0x10

bool GzipInputStream::load()
{
    unsigned long crc = crc32(0L, Z_NULL, 0);
    
    std::vector<Byte> inputBuf;
    while (true)
        {
        int ch = source.get();
        if (ch<0)
            break;
        inputBuf.push_back((Byte)(ch & 0xff));
        }
    long inputBufLen = inputBuf.size();
    
    if (inputBufLen < 19) //header + tail + 1
        {
        return false;
        }

    uLong srcLen = inputBuf.size();
    Bytef *srcBuf = (Bytef *)malloc(srcLen * sizeof(Byte));
    if (!srcBuf)
        {
        return false;
        }
        
    std::vector<unsigned char>::iterator iter;
    Bytef *p = srcBuf;
    for (iter=inputBuf.begin() ; iter != inputBuf.end() ; iter++)
        *p++ = *iter;

    int headerLen = 10;

    //Magic
    int val = (int)srcBuf[0];
    printf("val:%x\n", val);
    val = (int)srcBuf[1];
    printf("val:%x\n", val);

    //Method
    val = (int)srcBuf[2];
    printf("val:%x\n", val);

    //flags
    int flags = (int)srcBuf[3];

    //time
    val = (int)srcBuf[4];
    val = (int)srcBuf[5];
    val = (int)srcBuf[6];
    val = (int)srcBuf[7];

    //xflags
    val = (int)srcBuf[8];
    //OS
    val = (int)srcBuf[9];

    int cur = 10;
//     if ( flags & FEXTRA ) {
//         headerLen += 2;
//         int xlen = 
//         TODO deal with optional header parts
//     }
    if ( flags & FNAME ) {
        while ( srcBuf[cur] )
        {
            cur++;
            headerLen++;
        }
        headerLen++;
    }

    
    unsigned long crc0   = (unsigned long )srcBuf[srcLen-8];
    unsigned long crc1   = (unsigned long )srcBuf[srcLen-7];
    unsigned long crc2   = (unsigned long )srcBuf[srcLen-6];
    unsigned long crc3   = (unsigned long )srcBuf[srcLen-5];
    unsigned long srcCrc = crc3 << 24 | crc2 << 16 | crc1 << 8 | crc0;
    //printf("srcCrc:%lx\n", srcCrc);
    
    unsigned long siz0   = (unsigned long )srcBuf[srcLen-4];
    unsigned long siz1   = (unsigned long )srcBuf[srcLen-3];
    unsigned long siz2   = (unsigned long )srcBuf[srcLen-2];
    unsigned long siz3   = (unsigned long )srcBuf[srcLen-1];
    unsigned long srcSiz = siz3 << 24 | siz2 << 16 | siz1 << 8 | siz0;
    //printf("srcSiz:%lx/%ld\n", srcSiz, srcSiz);
    
    if (srcSiz <=0 || srcSiz > 1000000L)
        {
        return false;
        }
        
    outputBufLen = srcSiz + srcSiz/100 + 14;
    outputBuf = (unsigned char *)malloc(outputBufLen * sizeof(unsigned char));
    if (!outputBuf)
        {
        free(srcBuf);
        return false;
        }
    
    unsigned char *data = srcBuf + headerLen;
    unsigned long dataLen = srcLen - (headerLen + 8);
    //printf("%x %x\n", data[0], data[dataLen-1]);
    
    z_stream d_stream;
    d_stream.zalloc    = (alloc_func)0;
    d_stream.zfree     = (free_func)0;
    d_stream.opaque    = (voidpf)0;
    d_stream.next_in   = data;
    d_stream.avail_in  = dataLen;
    d_stream.next_out  = outputBuf;
    d_stream.avail_out = outputBufLen;
    
    int zerr = inflateInit2(&(d_stream), -MAX_WBITS);
    zerr = inflate(&d_stream, Z_FINISH);
    
    if (zerr != Z_OK && zerr != Z_STREAM_END)
        {
        printf("inflate: Some kind of problem: %d\n", zerr);
        }
    zerr = inflateEnd(&d_stream);
    if (zerr != Z_OK)
        {
        printf("inflateEnd: Some kind of problem: %d\n", zerr);
        }
        
    outputBufLen -= d_stream.avail_out;

    crc = crc32(crc, (const Bytef *)outputBuf, outputBufLen);
   
    printf("crc:%lx\n", crc);
    
    free(srcBuf);
    
    return true;
}


//#########################################################################
//# G Z I P   O U T P U T    S T R E A M
//#########################################################################

/**
 *
 */ 
GzipOutputStream::GzipOutputStream(OutputStream &destinationStream)
                     : BasicOutputStream(destinationStream)
{

    totalIn         = 0;
    totalOut        = 0;
    crc             = crc32(0L, Z_NULL, 0);

    //Gzip header
    destination.put(0x1f);
    destination.put(0x8b);

    //Say it is compressed
    destination.put(Z_DEFLATED);

    //flags
    destination.put(0);

    //time
    destination.put(0);
    destination.put(0);
    destination.put(0);
    destination.put(0);

    //xflags
    destination.put(0);

    //OS code - from zutil.h
    //destination.put(OS_CODE);
    //apparently, we should not explicitly include zutil.h
    destination.put(0);

}

/**
 *
 */ 
GzipOutputStream::~GzipOutputStream()
{
    close();
}

/**
 * Closes this output stream and releases any system resources
 * associated with this stream.
 */ 
void GzipOutputStream::close()
{
    if (closed)
        return;

    flush();

    //# Send the CRC
    uLong outlong = crc;
    for (int n = 0; n < 4; n++)
        {
        destination.put((int)(outlong & 0xff));
        outlong >>= 8;
        }
    //# send the file length
    outlong = totalIn & 0xffffffffL;
    for (int n = 0; n < 4; n++)
        {
        destination.put((int)(outlong & 0xff));
        outlong >>= 8;
        }

    destination.close();
    closed = true;
}
    
/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */ 
void GzipOutputStream::flush()
{
    if (closed || inputBuf.size()<1)
        return;
    
    uLong srclen = inputBuf.size();
    Bytef *srcbuf = (Bytef *)malloc(srclen * sizeof(Byte));
    if (!srcbuf)
        {
        return;
        }
        
    uLong destlen = srclen;
    Bytef *destbuf = (Bytef *)malloc((destlen + (srclen/100) + 13) * sizeof(Byte));
    if (!destbuf)
        {
        free(srcbuf);
        return;
        }
        
    std::vector<unsigned char>::iterator iter;
    Bytef *p = srcbuf;
    for (iter=inputBuf.begin() ; iter != inputBuf.end() ; iter++)
        *p++ = *iter;
        
    crc = crc32(crc, (const Bytef *)srcbuf, srclen);
    
    int zerr = compress(destbuf, (uLongf *)&destlen, srcbuf, srclen);
    if (zerr != Z_OK)
        {
        printf("Some kind of problem\n");
        }

    totalOut += destlen;
    //skip the redundant zlib header and checksum
    for (uLong i=2; i<destlen-4 ; i++)
        {
        destination.put((int)destbuf[i]);
        }
        
    destination.flush();

    inputBuf.clear();
    free(srcbuf);
    free(destbuf);
    
    printf("done\n");
    
}



/**
 * Writes the specified byte to this output stream.
 */ 
void GzipOutputStream::put(int ch)
{
    if (closed)
        {
        //probably throw an exception here
        return;
        }


    //Add char to buffer
    inputBuf.push_back(ch);
    totalIn++;

}



} // namespace IO
} // namespace Inkscape


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
