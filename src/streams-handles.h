#ifndef __STREAM_HANDLES_H_
#define __STREAM_HANDLES_H_

#include <uri.h>

namespace Inkscape {

/**
 * URIHandle (Abstract class)
 */

class URIHandle
{
public:

    virtual int read (void *buf, int buflen) = 0;
    virtual int write (void const *buf, int buflen) = 0;
    virtual void close() = 0;
    
protected:
    
    virtual int sys_read (void *buf, int buflen) = 0;
    virtual int sys_write (void const *buf, int buflen) = 0;
    virtual void sys_close() = 0;
    virtual void error(char const *errstr) = 0;
    
};

/**
 * FileHandle
 */

class IOException : public std::exception {};

class ReadException : public IOException
{
public:
    const char *what() const throw() { return "error read"; }
};

class WriteException : public IOException
{
public:
    const char *what() const throw() { return "error write"; }
};

class FileHandle : public URIHandle
{
public:
    FileHandle() : fp(0) {}
    virtual ~FileHandle() { if (fp) sys_close(); };
    virtual int open(URI const& uri, char const* mode);
    virtual void close();
    virtual int read (void *buf, int buflen);
    virtual int write (void const *buf, int buflen);
    virtual int seek (long offset, int whence);
protected:
    
    virtual FILE *sys_open(URI const& uri, char const* mode);
    virtual void sys_close();
    virtual int sys_read(void *buf, int buflen) throw(ReadException);
    virtual int sys_write(void const *buf, int buflen) throw(WriteException);
    virtual int sys_seek(long offset, int whence);
    virtual void error(char const *errstr);
    FILE *get_fp() { return fp; }
    
private:
    FILE *fp;
};

/*
  class SocketHandle : public URIHandle
  {
  // ...
  };
*/
}
#endif
