/*
 * IO layer : gzip streambuf and streams
 *
 * Authors:
 *   Johan Ceuppens <jceuppen at easynet dot be>
 *
 * Copyright (C) 2004 Johan Ceuppens
 *
 * Released under GNU LGPL, read the file 'COPYING.LIB' for more information
 */

#include "streams-gzip.h"

namespace Inkscape {

//With some inpsiration and code from libgsf, fastjar, libpng and RFC 1952

static int const GZIP_IS_ASCII	     = 0x01; //file contains text
static int const GZIP_HEADER_CRC     = 0x02; //there is a CRC in the header
static int const GZIP_EXTRA_FIELD    = 0x04; //there is an 'extra' field
static int const GZIP_ORIGINAL_NAME  = 0x08; //the original is stored
static int const GZIP_HAS_COMMENT    = 0x10; //There is a comment in the header 
static unsigned int const GZIP_HEADER_FLAGS = (GZIP_IS_ASCII 
					       |GZIP_HEADER_CRC 
					       |GZIP_EXTRA_FIELD
					       |GZIP_ORIGINAL_NAME
					       |GZIP_HAS_COMMENT);

/**
 * GZipBuffer
 */

int GZipBuffer::consume_header() throw(GZipHeaderException)
{
    unsigned int flags;
    guint8 data[4];
    
    try {
	get_urihandle()->read(data, 4);
	check_signature(data);
	check_flags(data);
	flags = data[3];
	get_urihandle()->read(data, 4);
	//get_modification_time()
	get_urihandle()->read(data, 1);
	//check_extra_flags();
	get_urihandle()->read(data, 1);
	//check_OS();
	
	if (flags & GZIP_EXTRA_FIELD) {
	    get_extrafield();
	}
	if (flags & GZIP_ORIGINAL_NAME) {
	    get_filename();
	}
	if (flags & GZIP_HAS_COMMENT) {
	    get_comment();
	}
	if (flags & GZIP_HEADER_CRC) {
	    get_crc();
	}
    }
    catch(std::exception& e) {
	throw GZipHeaderException();
    }

    return 1;
}

void GZipBuffer::check_signature(guint8 *data) throw(GZipHeaderException)
{
    guint8 const signature[2] = {0x1f, 0x8b};
    if (memcmp(data, signature, sizeof(signature)) != 0)
	throw GZipHeaderException();
}

void GZipBuffer::check_flags(guint8 *data) throw(GZipHeaderException)
{
    unsigned int flags = data[3];
    if (data[2] != Z_DEFLATED || (flags & ~GZIP_HEADER_FLAGS) != 0)
	throw GZipHeaderException();
}

gchar *GZipBuffer::get_filename()
{
#ifdef DEBUG
    std::cout<<"Filename is ";
#endif
    return read_string();
}

gchar *GZipBuffer::get_comment()
{
#ifdef DEBUG
    std::cout<<"Comment is "<<std::endl;
#endif
    return read_string();
}

guint16 GZipBuffer::get_crc()
{
    guint16 buf;
    get_urihandle()->read(&buf, 2);
    return buf;
}

void GZipBuffer::get_extrafield()
{
    guint8 length_data[2];
    get_urihandle()->read(length_data, 2);
    unsigned int const length = length_data[0] | (length_data[1] << 8);
    guint8 *data = new guint8[length];
    get_urihandle()->read(data, length);
}

gchar *GZipBuffer::read_string() throw(GZipHeaderException)
{
    GByteArray *gba = g_byte_array_new();
    try {
	guint8 byte[1];
	do {
	    get_urihandle()->read(byte, 1);
	    g_byte_array_append(gba, byte, sizeof(byte));
#ifdef DEBUG
	    std::cout <<(char)*byte;
#endif
	} while (*byte != 0);
    } catch (std::exception& e) {
	g_byte_array_free(gba, TRUE);
	throw GZipHeaderException();
    }
#ifdef DEBUG
    std::cout<<std::endl;
#endif
    gchar *ret = (gchar *)gba->data;
    g_byte_array_free(gba, FALSE);
    return ret;
}

} // namespace Inkscape
#if 0 // testing code 
int main(int argc, char *argv[])
{
    try {
	Inkscape::FileHandle *fh = new Inkscape::FileHandle;
	Inkscape::URI uri("file:///cvs/inkscape/src/inkjar/changelog-2.gz");
	if (!fh->open(uri, "r"))
	    return 1;
	Inkscape::GZipBuffer gzb(*fh);
	Inkscape::igzipstream izs(gzb);
	
	
	char buf;
	while (izs.get(buf)) {
	    std::cout <<buf;
	}
	

	/*
	  char buf[40960];
	  while(!izs.eof())
	  {
	  izs >> buf;
	  fprintf(stdout, "%s ", buf);
	  }
	*/
	if (!izs.eof())
	    std::cerr<<"something strange happened."<<std::endl;
	fh->close();
    }
    catch (Inkscape::BadURIException& e) {
	std::cerr<<e.what()<<std::endl;
    }
    catch (Inkscape::GZipHeaderException& e) {
	std::cerr<<e.what()<<std::endl;
    }
    catch (std::exception& e) {
	std::cerr<<e.what()<<std::endl;
    }
}
#endif // testing code
