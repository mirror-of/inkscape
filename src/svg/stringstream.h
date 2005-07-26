#ifndef INKSCAPE_STRINGSTREAM_H
#define INKSCAPE_STRINGSTREAM_H

#include <glib/gtypes.h>
#include <sstream>

namespace Inkscape {

typedef std::ios_base &(*std_oct_type)(std::ios_base &);

class SVGOStringStream {
private:
    std::ostringstream ostr;

public:
    SVGOStringStream() {
        /* This is probably unnecessary now that we provide our own
         * operator<< for float and double. */
        ostr.imbue(std::locale::classic());
        ostr.setf(std::ios::showpoint);
        ostr.precision(8);
    }

#define INK_STR_OP(_t) \
    SVGOStringStream &operator<<(_t arg) {  \
        ostr << arg;    \
        return *this;   \
    }

    INK_STR_OP(char)
    INK_STR_OP(signed char)
    INK_STR_OP(unsigned char)
    INK_STR_OP(short)
    INK_STR_OP(unsigned short)
    INK_STR_OP(int)
    INK_STR_OP(unsigned int)
    INK_STR_OP(long)
    INK_STR_OP(unsigned long)
    INK_STR_OP(char const *)
    INK_STR_OP(signed char const *)
    INK_STR_OP(unsigned char const *)
    INK_STR_OP(std::string const &)
    INK_STR_OP(std_oct_type)

#undef INK_STR_OP

    gchar const *gcharp() const {
        return reinterpret_cast<gchar const *>(ostr.str().c_str());
    }

    std::string str() const {
        return ostr.str();
    }

    std::streamsize precision() const {
        return ostr.precision();
    }

    std::streamsize precision(std::streamsize p) {
        return ostr.precision(p);
    }
};

}

Inkscape::SVGOStringStream &operator<<(Inkscape::SVGOStringStream &os, float d);

Inkscape::SVGOStringStream &operator<<(Inkscape::SVGOStringStream &os, double d);


#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
