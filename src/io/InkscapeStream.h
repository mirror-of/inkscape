#ifndef __INKSCAPE_IO_INKSCAPESTREAM_H__
#define __INKSCAPE_IO_INKSCAPESTREAM_H__
/**
 * Our base basic stream classes.  
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <glibmm.h>

namespace Inkscape
{
namespace IO
{

class StreamException : public std::exception
{
public:

    StreamException(const char *theReason) throw()
        { reason = theReason; }
    StreamException(Glib::ustring &theReason) throw()
        { reason = theReason; }
    ~StreamException() throw()
        {  }
    char const *what() const throw()
        { return reason.c_str(); }
        
private:
    Glib::ustring reason;

};

//#########################################################################
//# I N P U T    S T R E A M
//#########################################################################

/**
 * This interface is the base of all input stream classes.  Users who wish
 * to make an InputStream that is part of a chain should inherit from
 * BasicInputStream.  Inherit from this class to make a source endpoint,
 * such as a URI or buffer.
 *
 */
class InputStream
{

public:

    /**
     * Return the number of bytes that are currently available
     * to be read
     */
    virtual int available() = 0;
    
    /**
     * Do whatever it takes to 'close' this input stream
     * The most likely implementation of this method will be
     * for endpoints that use a resource for their data.
     */
    virtual void close() = 0;
    
    /**
     * Read one byte from this input stream.  This is a blocking
     * call.  If no data is currently available, this call will
     * not return until it exists.  If the user does not want
     * their code to block,  then the usual solution is:
     *     if (available() > 0)
     *         myChar = get();
     * This call returns -1 on end-of-file.
     */
    virtual int get() = 0;
    
}; // class InputStream




/**
 * This is the class that most users should inherit, to provide
 * their own streams.
 *
 */
class BasicInputStream : public InputStream
{

public:

    BasicInputStream(InputStream &sourceStream);
    
    virtual ~BasicInputStream() {}
    
    virtual int available();
    
    virtual void close();
    
    virtual int get();
    
protected:

    bool closed;

    InputStream &source;
    
private:


}; // class BasicInputStream



/**
 * Convenience class for reading from standard input
 */
class StdInputStream : public InputStream
{
public:

    int available()
        { return 0; }
    
    void close()
        { /* do nothing */ }
    
    int get()
        {  return getchar(); }

};






//#########################################################################
//# O U T P U T    S T R E A M
//#########################################################################

/**
 * This interface is the base of all input stream classes.  Users who wish
 * to make an OutputStream that is part of a chain should inherit from
 * BasicOutputStream.  Inherit from this class to make a destination endpoint,
 * such as a URI or buffer.
 */
class OutputStream
{

public:

    /**
     * This call should
     *  1.  flush itself
     *  2.  close itself
     *  3.  close the destination stream
     */
    virtual void close() = 0;
    
    /**
     * This call should push any pending data it might have to
     * the destination stream.  It should NOT call flush() on
     * the destination stream.
     */
    virtual void flush() = 0;
    
    /**
     * Send one byte to the destination stream.
     */
    virtual void put(int ch) = 0;


}; // class OutputStream


/**
 * This is the class that most users should inherit, to provide
 * their own output streams.
 */
class BasicOutputStream : public OutputStream
{

public:

    BasicOutputStream(OutputStream &destinationStream);
    
    virtual ~BasicOutputStream() {}

    virtual void close();
    
    virtual void flush();
    
    virtual void put(int ch);

protected:

    bool closed;

    OutputStream &destination;


}; // class BasicOutputStream



/**
 * Convenience class for writing to standard output
 */
class StdOutputStream : public OutputStream
{
public:

    void close()
        { }
    
    void flush()
        { }
    
    void put(int ch)
        {  putchar(ch); }

};




//#########################################################################
//# R E A D E R
//#########################################################################


/**
 * This class and its descendants are for unicode character-oriented input
 *
 */
class Reader
{

public:

    Reader(Reader &sourceStream);
    
    virtual ~Reader() {}

    virtual int available();
    
    virtual void close();
    
    virtual gunichar get();
    
    virtual Glib::ustring readLine();
    
    virtual Glib::ustring readWord();
    
    /* Input formatting */
    const Reader& readBool (bool& val );
    const Reader& operator>> (bool& val )
        { return readBool(val); }
        
    const Reader& readShort (short &val);
    const Reader& operator>> (short &val)
        { return readShort(val); }
        
    const Reader& readUnsignedShort (unsigned short &val);
    const Reader& operator>> (unsigned short &val)
        { return readUnsignedShort(val); }
        
    const Reader& readInt (int &val);
    const Reader& operator>> (int &val)
        { return readInt(val); }
        
    const Reader& readUnsignedInt (unsigned int &val);
    const Reader& operator>> (unsigned int &val)
        { return readUnsignedInt(val); }
        
    const Reader& readLong (long &val);
    const Reader& operator>> (long &val)
        { return readLong(val); }
        
    const Reader& readUnsignedLong (unsigned long &val);
    const Reader& operator>> (unsigned long &val)
        { return readUnsignedLong(val); }
        
    const Reader& readFloat (float &val);
    const Reader& operator>> (float &val)
        { return readFloat(val); }
        
    const Reader& readDouble (double &val);
    const Reader& operator>> (double &val)
        { return readDouble(val); }
 

protected:

    Reader *source;

    Reader()
        { source = NULL; }

private:

}; // class Reader




//#########################################################################
//# W R I T E R
//#########################################################################

/**
 * This class and its descendants are for unicode character-oriented output
 *
 */
class Writer
{

public:

    Writer(Writer &destinationWriter);

    virtual ~Writer() {}

    /*Overload these 3 for your implementation*/
    virtual void close();
    
    virtual void flush();
    
    virtual void put(gunichar ch);
    
    
    
    /* Formatted output */
    Writer &printf(char *fmt, ...);

    Writer& writeChar(char val);

    Writer& writeUString(Glib::ustring &val);

    Writer& writeStdString(std::string &val);

    Writer& writeString(const char *str);

    Writer& writeBool (bool val );

    Writer& writeShort (short val );

    Writer& writeUnsignedShort (unsigned short val );

    Writer& writeInt (int val );

    Writer& writeUnsignedInt (unsigned int val );

    Writer& writeLong (long val );

    Writer& writeUnsignedLong (unsigned long val );

    Writer& writeFloat (float val );

    Writer& writeDouble (double val );

 
protected:

    Writer *destination;

    Writer()
        { destination = NULL; }
    
private:

}; // class Writer



Writer& operator<< (Writer &writer, char val);

Writer& operator<< (Writer &writer, Glib::ustring &val);

Writer& operator<< (Writer &writer, std::string &val);

Writer& operator<< (Writer &writer, char *val);

Writer& operator<< (Writer &writer, bool val);

Writer& operator<< (Writer &writer, short val);

Writer& operator<< (Writer &writer, unsigned short val);

Writer& operator<< (Writer &writer, int val);

Writer& operator<< (Writer &writer, unsigned int val);

Writer& operator<< (Writer &writer, long val);

Writer& operator<< (Writer &writer, unsigned long val);

Writer& operator<< (Writer &writer, float val);

Writer& operator<< (Writer &writer, double val);


//#########################################################################
//# O U T P U T    S T R E A M    W R I T E R
//#########################################################################

/**
 * Class for placing a Writer on an open OutputStream
 *
 */
class OutputStreamWriter : public Writer
{
public:

    OutputStreamWriter(OutputStream &outputStreamDest);
    
    /*Overload these 3 for your implementation*/
    virtual void close();
    
    virtual void flush();
    
    virtual void put(gunichar ch);


private:

    OutputStream &outputStream;


};


//#########################################################################
//# U T I L I T Y
//#########################################################################

void pipeStream(InputStream &source, OutputStream &dest);



} // namespace IO
} // namespace Inkscape


#endif /* __INKSCAPE_IO_INKSCAPESTREAM_H__ */
