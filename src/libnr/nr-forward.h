#ifndef __NR_FORWARD_H__
#define __NR_FORWARD_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

typedef struct _NRPixBlock NRPixBlock;

namespace NR {
class Matrix;
class Point;
class Rect;
class rotate;
class scale;
class translate;
}

struct NRMatrix;
struct NRPoint;
struct NRRect;
struct NRRectL;


#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
  vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
*/
