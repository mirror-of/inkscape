#ifndef INKSCAPE_STRINGSTREAM_H
#define INKSCAPE_STRINGSTREAM_H

#include <glib/gtypes.h>
#include <sstream>
#include "svg/ftos.h"

namespace Inkscape {
	class SVGOStringStream : public std::ostringstream {

	public:
		SVGOStringStream() {
			this->imbue(std::locale::classic());
			this->setf(std::ios::showpoint);
			this->precision(8);
		}

		gchar const *gcharp() const {
			return reinterpret_cast<gchar const *>(str().c_str());
		}

	};

}

#ifdef BRYCE_FLOATS
Inkscape::SVGOStringStream &operator<<(Inkscape::SVGOStringStream &os, double d)
{
	return os << ftos(d, 'g', -1, -1, 0);
}
#endif

#endif
