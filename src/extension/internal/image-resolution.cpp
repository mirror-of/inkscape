/*
 * Authors:
 *   Daniel Wagenaar <daw@caltech.edu>
 *
 * Copyright (C) 2012 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "image-resolution.h"

#define IR_TRY_PNG 1
#ifdef HAVE_EXIF
#define IR_TRY_EXIF 1
#endif

#if IR_TRY_PNG
#include <png.h>
#endif
#if IR_TRY_EXIF
#include <math.h>
#include <libexif/exif-data.h>
#elif IR_TRY_EXIV
#include <exiv2/exiv2.hpp>
#endif

namespace Inkscape {
namespace Extension {
namespace Internal {

ImageResolution::ImageResolution(char const *fn) {
  ok_ = false;

#if IR_TRY_PNG
  readpng(fn);
#endif
  
#if IR_TRY_EXIF  
  if (!ok_)
    readexif(fn);
#elif IR_TRY_EXIV
  if (!ok_)
    readexif(fn);
#endif
}

bool ImageResolution::ok() const {
  return ok_;
}

double ImageResolution::x() const {
  return x_;
}

double ImageResolution::y() const {
  return y_;
}

#if IR_TRY_PNG

static bool haspngheader(FILE *fp) {
  unsigned char header[8];
  if (fread(header, 1, 8, fp)!=8)
    return false;

  fseek(fp, 0, SEEK_SET);
 
  if (png_sig_cmp(header, 0, 8))
    return false;

  return true;
}

// Implementation using libpng
void ImageResolution::readpng(char const *fn) {
  FILE *fp = fopen(fn, "rb");
  if (!fp) 
    return;

  if (!haspngheader(fp)) {
    fclose(fp);
    return;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  if (!png_ptr) 
    return;
  
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!png_ptr) {
    png_destroy_read_struct(&png_ptr, 0, 0);
    return;
  }
  
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    fclose(fp);
    return;
  }

  png_init_io(png_ptr, fp);
  png_read_info(png_ptr, info_ptr);

  png_uint_32 res_x, res_y;
  int unit_type;
  png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &unit_type);

  png_destroy_read_struct(&png_ptr, &info_ptr, 0);
  fclose(fp);

  if (unit_type == PNG_RESOLUTION_METER) {
    ok_ = true;
    x_ = res_x * 2.54/100;
    y_ = res_y * 2.54/100;
  }
}
#else

// Dummy implementation
void ImageResolution::readpng(char const *fn) {
}

#endif

#if IR_TRY_EXIF

static double exifDouble(ExifEntry *entry, ExifByteOrder byte_order) {
  switch (entry->format) {
  case EXIF_FORMAT_BYTE:
    return double(entry->data[0]);
  case EXIF_FORMAT_SHORT:
    return double(exif_get_short(entry->data, byte_order));
  case EXIF_FORMAT_LONG:
    return double(exif_get_long(entry->data, byte_order));
  case EXIF_FORMAT_RATIONAL: {
    ExifRational r = exif_get_rational(entry->data, byte_order);
    return double(r.numerator)/double(r.denominator);
  }
  case EXIF_FORMAT_SBYTE:
    return double(*(signed char *)entry->data);
  case EXIF_FORMAT_SSHORT:
    return double(exif_get_sshort(entry->data, byte_order));
  case EXIF_FORMAT_SLONG:
    return double(exif_get_slong(entry->data, byte_order));
  case EXIF_FORMAT_SRATIONAL: {
    ExifSRational r = exif_get_srational(entry->data, byte_order);
    return double(r.numerator)/double(r.denominator);
  }
  case EXIF_FORMAT_FLOAT:
    return double(((float *)entry->data)[0]);
  case EXIF_FORMAT_DOUBLE:
    return ((double *)entry->data)[0];
  default:
    return nan(0);
  }
}

// Implementation using libexif
void ImageResolution::readexif(char const *fn) {
  ExifData *ed;
  ed = exif_data_new_from_file(fn);
  if (!ed)
    return;
  ExifByteOrder byte_order = exif_data_get_byte_order(ed);
  
  ExifEntry *xres = exif_content_get_entry(ed->ifd[EXIF_IFD_0],
					   EXIF_TAG_X_RESOLUTION);
  ExifEntry *yres = exif_content_get_entry(ed->ifd[EXIF_IFD_0],
					   EXIF_TAG_Y_RESOLUTION);
  ExifEntry *unit = exif_content_get_entry(ed->ifd[EXIF_IFD_0],
					   EXIF_TAG_RESOLUTION_UNIT);
  if (xres && yres) {
    x_ = exifDouble(xres, byte_order);
    y_ = exifDouble(yres, byte_order);
    if (unit) {
      double u = exifDouble(unit, byte_order);
      if (u==3) {
	x_ *= 2.54;
	y_ *= 2.54;
      }
    }
    ok_ = true;
  }
  exif_data_free(ed);
}

#elif IR_TRY_EXIV

// Implementation using libexiv2
void ImageResolution::readexif(char const *fn) {
  Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fn);
  if (!image.get())
    return;

  image->readMetadata();
  Exiv2::ExifData &exifData = image->exifData();
  if (exifData.empty())
    return;

  Exiv2::ExifData::const_iterator end = exifData.end();
  bool havex = false;
  bool havey = false;
  bool haveunit = false;
  int unit;
  for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
    if (ok_)
      break;
    if (i->tag()==0x011a) {
      // X Resolution
      x_ = i->toFloat();
      havex = true;
    } else if (i->tag()==0x011b) {
      // Y Resolution
      y_ = i->toFloat();
      havey = true;
    } else if (i->tag()==0x0128) {
      unit = i->toLong();
    }
    ok_ = havex && havey && haveunit;
  }
  if (haveunit) {
    if (unit==3) {
      x_ *= 2.54;
      y_ *= 2.54;
    }
  }
  ok_ = havex && havey;
}
#else

// Dummy implementation
void ImageResolution::readexif(char const *fn) {
}

#endif

}
}
}
