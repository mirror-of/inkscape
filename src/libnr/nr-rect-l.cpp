#include <libnr/nr-rect-l.h>
#include <math.h>

namespace NR{
    IRect::IRect(const Rect& r) : _min(int(floor(r.min()[X])), int(floor(r.min()[Y]))),
				  _max(int(ceil(r.min()[X])), int(ceil(r.min()[Y])))
    {
	    
    }
};
