#include <libnr/nr-rect-l.h>
#include <math.h>

namespace NR{
    IRect::IRect(const Rect& r) : _min(floor(r.min()[X]), floor(r.min()[Y])), 
				  _max(ceil(r.min()[X]), ceil(r.min()[Y])) 
    {
	    
    }
};
