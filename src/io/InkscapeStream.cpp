/**
 * Our base input/output stream classes.  These are is directly
 * inherited from iostreams, and includes any extra
 * functionality that we might need.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stdarg.h>

#include "InkscapeStream.h"
#include "ftos.h"

namespace Inkscape
{
namespace IO
{

//#########################################################################
//# U T I L I T Y
//#########################################################################

void pipeStream(InputStream &source, OutputStream &dest)
{
    for (;;)
        {
        int ch = source.get();
        if (ch<0)
            break;
        dest.put(ch);
        }
    dest.flush();
}

//#########################################################################
//# B A S I C    I N P U T    S T R E A M
//#########################################################################


/**
 *
 */ 
BasicInputStream::BasicInputStream(InputStream &sourceStream)
                   : source(sourceStream)
{
    closed = false;
}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this input stream without blocking by the next caller of a method for
 * this input stream.
 */ 
int BasicInputStream::available()
{
    if (closed)
        return 0;
    return source.available();
}

    
/**
 *  Closes this input stream and releases any system resources
 *  associated with the stream.
 */ 
void BasicInputStream::close()
{
    if (closed)
        return;
    source.close();
    closed = true;
}
    
/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */ 
int BasicInputStream::get()
{
    if (closed)
        return -1;
    source.get();
}
   


//#########################################################################
//# B A S I C    O U T P U T    S T R E A M
//#########################################################################

/**
 *
 */ 
BasicOutputStream::BasicOutputStream(OutputStream &destinationStream)
                     : destination(destinationStream)
{
    closed = false;
}

/**
 * Closes this output stream and releases any system resources
 * associated with this stream.
 */ 
void BasicOutputStream::close()
{
    if (closed)
        return;
    destination.close();
    closed = true;
}
    
/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */ 
void BasicOutputStream::flush()
{
    if (closed)
        return;
    destination.flush();
}
    
/**
 * Writes the specified byte to this output stream.
 */ 
void BasicOutputStream::put(int ch)
{
    if (closed)
        return;
    destination.put(ch);
}



//#########################################################################
//# R E A D E R
//#########################################################################


/**
 *
 */ 
Reader::Reader(Reader &sourceReader)
{
    source = &sourceReader;
}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this reader without blocking by the next caller of a method for
 * this reader.
 */ 
int Reader::available()
{
    if (source)
        return source->available();
    else
        return 0;
}

    
/**
 *  Closes this reader and releases any system resources
 *  associated with the reader.
 */ 
void Reader::close()
{
    if (source)
        source->close();
}
    
/**
 * Reads the next byte of data from the reader.
 */ 
gunichar Reader::get()
{
    if (source)
        return source->get();
    else
        return (gunichar)-1;
}
   





/**
 * Reads a line of data from the reader.
 */ 
Glib::ustring Reader::readLine()
{
    Glib::ustring str;
    while (available() > 0)
        {
        gunichar ch = get();
        if (ch == '\n')
            break;
        str.push_back(ch);
        }
    return str;
}
   
/**
 * Reads a line of data from the reader.
 */ 
Glib::ustring Reader::readWord()
{
    Glib::ustring str;
    while (available() > 0)
        {
        gunichar ch = get();
        if (!g_unichar_isprint(ch))
            break;
        str.push_back(ch);
        }
    return str;
}
   

static bool getLong(Glib::ustring &str, long *val)
{
    const char *begin = str.raw().c_str();
    char *end;
    long ival = strtol(begin, &end, 10);
    if (str == end)
        return false;
    *val = ival;
    return true;
}

static bool getULong(Glib::ustring &str, unsigned long *val)
{
    const char *begin = str.raw().c_str();
    char *end;
    unsigned long ival = strtoul(begin, &end, 10);
    if (str == end)
        return false;
    *val = ival;
    return true;
}

static bool getDouble(Glib::ustring &str, double *val)
{
    const char *begin = str.raw().c_str();
    char *end;
    double ival = strtod(begin, &end);
    if (str == end)
        return false;
    *val = ival;
    return true;
}







/**
 *
 */
const Reader &Reader::readBool (bool& val )
{
    Glib::ustring buf = readWord();
    if (buf == "true")
        val = true;
    else
        val = false;
    return *this;
}

/**
 *
 */
const Reader &Reader::readShort (short& val )
{
    Glib::ustring buf = readWord();
    long ival;
    if (getLong(buf, &ival))
        val = (short) ival;
    return *this;
}

/**
 *
 */
const Reader &Reader::readUnsignedShort (unsigned short& val )
{
    Glib::ustring buf = readWord();
    unsigned long ival;
    if (getULong(buf, &ival))
        val = (unsigned short) ival;
    return *this;
}

/**
 *
 */
const Reader &Reader::readInt (int& val )
{
    Glib::ustring buf = readWord();
    long ival;
    if (getLong(buf, &ival))
        val = (int) ival;
    return *this;
}

/**
 *
 */
const Reader &Reader::readUnsignedInt (unsigned int& val )
{
    Glib::ustring buf = readWord();
    unsigned long ival;
    if (getULong(buf, &ival))
        val = (unsigned int) ival;
    return *this;
}

/**
 *
 */
const Reader &Reader::readLong (long& val )
{
    Glib::ustring buf = readWord();
    long ival;
    if (getLong(buf, &ival))
        val = ival;
    return *this;
}

/**
 *
 */
const Reader &Reader::readUnsignedLong (unsigned long& val )
{
    Glib::ustring buf = readWord();
    unsigned long ival;
    if (getULong(buf, &ival))
        val = ival;
    return *this;
}

/**
 *
 */
const Reader &Reader::readFloat (float& val )
{
    Glib::ustring buf = readWord();
    double ival;
    if (getDouble(buf, &ival))
        val = (float)ival;
    return *this;
}

/**
 *
 */
const Reader &Reader::readDouble (double& val )
{
    Glib::ustring buf = readWord();
    double ival;
    if (getDouble(buf, &ival))
        val = ival;
    return *this;
}





//#########################################################################
//# W R I T E R
//#########################################################################

/**
 *
 */ 
Writer::Writer(Writer &destinationWriter)
{
    destination = &destinationWriter;
}

/**
 * Closes this writer and releases any system resources
 * associated with this writer.
 */ 
void Writer::close()
{
    if (destination)
        destination->close();
}
    
/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */ 
void Writer::flush()
{
    if (destination)
        destination->flush();
}
    
/**
 * Writes the specified byte to this output writer.
 */ 
void Writer::put(gunichar ch)
{
    if (destination)
        destination->put(ch);
}

/**
 * Provide printf()-like formatting
 */ 
Writer &Writer::printf(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    gchar *buf = g_strdup_vprintf(fmt, args);
    va_end(args);
    if (buf) {
        writeString(buf);
        g_free(buf);
    }
  return *this;
    return *this;
}
/**
 * Writes the specified character to this output writer.
 */ 
Writer &Writer::writeChar(char ch)
{
    gunichar uch = ch;
    put(uch);
    return *this;
}


/**
 * Writes the specified unicode string to this output writer.
 */ 
Writer &Writer::writeUString(Glib::ustring &str)
{
    for (int i=0; i< str.size(); i++)
        put(str[i]);
    return *this;
}

/**
 * Writes the specified standard string to this output writer.
 */ 
Writer &Writer::writeStdString(std::string &str)
{
    Glib::ustring tmp(str);
    writeUString(tmp);
    return *this;
}

/**
 * Writes the specified character string to this output writer.
 */ 
Writer &Writer::writeString(const char *str)
{
    Glib::ustring tmp;
    if (str)
        tmp = str;
    else
        tmp = "null";
    writeUString(tmp);
    return *this;
}




/**
 *
 */
Writer &Writer::writeBool (bool val )
{
    if (val)
        writeString("true");
    else
        writeString("false");
    return *this;
}


/**
 *
 */
Writer &Writer::writeShort (short val )
{
    gchar *buf = g_strdup_printf("%d", val);
    if (buf) {
        writeString(buf);
        g_free(buf);
    }
  return *this;
}



/**
 *
 */
Writer &Writer::writeUnsignedShort (unsigned short val )
{
    gchar *buf = g_strdup_printf("%u", val);
    if (buf) {
        writeString(buf);
        g_free(buf);
    }
    return *this;
}

/**
 *
 */
Writer &Writer::writeInt (int val)
{
    gchar *buf = g_strdup_printf("%d", val);
    if (buf) {
        writeString(buf);
        g_free(buf);
    }
    return *this;
}

/**
 *
 */
Writer &Writer::writeUnsignedInt (unsigned int val)
{
    gchar *buf = g_strdup_printf("%u", val);
    if (buf) {
        writeString(buf);
        g_free(buf);
    }
    return *this;
}

/**
 *
 */
Writer &Writer::writeLong (long val)
{
    gchar *buf = g_strdup_printf("%ld", val);
    if (buf) {
        writeString(buf);
        g_free(buf);
    }
    return *this;
}

/**
 *
 */
Writer &Writer::writeUnsignedLong(unsigned long val)
{
    gchar *buf = g_strdup_printf("%lu", val);
    if (buf) {
        writeString(buf);
        g_free(buf);
    }
    return *this;
}

/**
 *
 */
Writer &Writer::writeFloat(float val)
{
#if 1
    gchar *buf = g_strdup_printf("%8.3f", val);
    if (buf) {
        writeString(buf);
        g_free(buf);
    }
#else
    std::string tmp = ftos(val, 'g', 8, 3, 0);
    writeStdString(tmp);
#endif
    return *this;
}

/**
 *
 */
Writer &Writer::writeDouble(double val)
{
#if 1
    gchar *buf = g_strdup_printf("%8.3f", val);
    if (buf) {
        writeString(buf);
        g_free(buf);
    }
#else
    std::string tmp = ftos(val, 'g', 8, 3, 0);
    writeStdString(tmp);
#endif
    return *this;
}


Writer& operator<< (Writer &writer, char val)
    { return writer.writeChar(val); }

Writer& operator<< (Writer &writer, Glib::ustring &val)
    { return writer.writeUString(val); }

Writer& operator<< (Writer &writer, std::string &val)
    { return writer.writeStdString(val); }

Writer& operator<< (Writer &writer, char *val)
    { return writer.writeString(val); }

Writer& operator<< (Writer &writer, bool val)
    { return writer.writeBool(val); }

Writer& operator<< (Writer &writer, short val)
    { return writer.writeShort(val); }

Writer& operator<< (Writer &writer, unsigned short val)
    { return writer.writeUnsignedShort(val); }

Writer& operator<< (Writer &writer, int val)
    { return writer.writeInt(val); }

Writer& operator<< (Writer &writer, unsigned int val)
    { return writer.writeUnsignedInt(val); }

Writer& operator<< (Writer &writer, long val)
    { return writer.writeLong(val); }

Writer& operator<< (Writer &writer, unsigned long val)
    { return writer.writeUnsignedLong(val); }

Writer& operator<< (Writer &writer, float val)
    { return writer.writeFloat(val); }

Writer& operator<< (Writer &writer, double val)
    { return writer.writeDouble(val); }



//#########################################################################
//# O U T P U T    S T R E A M    W R I T E R
//#########################################################################


OutputStreamWriter::OutputStreamWriter(OutputStream &outputStreamDest)
                     : outputStream(outputStreamDest)
{
}

    

/**
 *  Close the underlying OutputStream
 */
void OutputStreamWriter::close()
{
    flush();
    outputStream.close();
}
    
/**
 *  Flush the underlying OutputStream
 */
void OutputStreamWriter::flush()
{
      outputStream.flush();
}
    
/**
 *  Overloaded to redirect the output chars from the next Writer
 *  in the chain to an OutputStream instead.
 */
void OutputStreamWriter::put(gunichar ch)
{
    //Do we need conversions here?
    int intCh = (int) ch;
    outputStream.put(intCh);
}


} // namespace IO
} // namespace Inkscape


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
