#ifndef INKSCAPE_LIVAROT_FLOAT_LINE_H
#define INKSCAPE_LIVAROT_FLOAT_LINE_H

#include <vector>
#include "livarot/LivarotDefs.h"

/*
 * pixel coverage classes. the goal is to salvage exact coverage info in the sweepline
 * performed by Scan() or QuickScan(), then clean up a bit, convert floating point bounds
 * to integer bounds, because pixel have integer bounds, and then raster runs of the type:
 * position on the (pixel) line:                st         en
 *                                              |          |
 * coverage value (0=empty, 1=full)            vst   ->   ven
 */

class IntLigne;
class BitLigne;

/// a coverage portion with floating point boundaries
struct float_ligne_run {
    float st;
    float en;
    float vst;
    float ven;
    float pente;   ///< (ven-vst)/(en-st)
};

/**
 * temporary data for the FloatLigne()
 * each float_ligne_bord is a boundary of some coverage
 * the Flatten() function will extract non-overlapping runs and produce an array of float_ligne_run
 * the float_ligne_bord are stored in an array, but linked like a boubly-linked list
 * the idea behind that is that a given edge produces one float_ligne_bord at the beginning
 * of Scan() and possibly another in AvanceEdge() and DestroyEdge(); but that second
 * float_ligne_bord will not be far away in the list from the first,
 * so it's faster to salvage the index of the first float_ligne_bord and try to insert the
 * second from that salvaged position.
 */

struct float_ligne_bord {
    float pos; ///< position of the boundary
    bool start; ///< is the beginning of the coverage portion?
    float val; ///< amount of coverage (ie vst if start==true, and ven if start==false)
    float pente; ///< (ven-vst)/(en-st)
//  float delta;
    int other; ///< index, in the array of float_ligne_bord, of the other boundary associated to this one
    int prev; ///< not used
    int next; ///< not used
    int s_prev; ///< index of the previous bord in the doubly-linked list
    int s_next; ///< index of the next bord in the doubly-linked list
    int pend_ind; ///< bords[i].pend_ind is the index of the float_ligne_bord that is the start of the
                  ///< coverage portion being scanned (in the Flatten() )  
    int pend_inv; ///< inverse of pend_ind, for faster handling of insertion/removal in the "pending" array
};

class FloatLigne {
public:
    std::vector<float_ligne_bord> bords; ///< vector of coverage boundaries
    std::vector<float_ligne_run> runs; ///< vector of runs

    // unused
    int firstAc;
    int lastAc;
    
    // first and last boundaries in the doubly-linked list
    int s_first;
    int s_last;

    FloatLigne();
    ~FloatLigne();

    // reset the line to  empty (boundaries and runs)
    void Reset();
    
    // add a coverage portion
    // guess is the position from where we should try to insert the first boundary, or -1 if we don't have a clue
    int AddBord(float spos, float sval, float epos, float eval, int guess = -1);
    int AddBord(float spos, float sval, float epos, float eval, float pente, int guess = -1);
    
    // minor difference: guess is the position from where we should try to insert the last boundary
    int AddBordR(float spos, float sval, float epos, float eval, float pente, int guess = -1);
    
    // simply append boundaries at the end of the list, 'cause we know they are on the right
    int AppendBord(float spos, float sval, float epos, float eval, float pente);
    
    // insertion primitive, for private use
    void InsertBord(int no, float p, int guess);

    // extract a set of non-overlapping runs from the boundaries
    // it does so by scanning the boundaries left to right, maintaining a set of coverage portions currently being scanned
    // for each such portion, the function will add the index of its first boundary in an array; but instead of allocating
    // another array, it uses a field in float_ligne_bord: pend_ind
    void Flatten();
//  void FlattenB();

    // debug dump of the instance
    void Affiche();

    // internal use only
    int AddRun(float st, float en, float vst, float ven);
    int AddRun(float st, float en, float vst, float ven, float pente);

    // operations on FloatLigne instances:
    // computes a mod b and stores the result in the present FloatLigne
    void Booleen(FloatLigne *a, FloatLigne *b, BooleanOp mod);
    
    // clips the coverage runs to tresh
    // if addIt == false, it only leaves the parts that are not entirely under tresh
    // if addIt == true, it's the coverage clamped to tresh
    void Max(FloatLigne *a, float tresh, bool addIt);
    
    // cuts the parts having less than tresh coverage
    void Min(FloatLigne *a, float tresh, bool addIt);
    
    // cuts the coverage "a" in 2 parts: "over" will receive the parts where coverage > tresh, while 
    // the present FloatLigne will receive the parts where coverage <= tresh
    void Split(FloatLigne *a, float tresh, FloatLigne *over);
    
    // extract the parts where coverage > tresh
    void Over(FloatLigne *a,float tresh);
	
    // copy the runs from another coverage structure into this one
    void Copy(IntLigne *a);
    void Copy(FloatLigne *a);

    // private use
    void Enqueue(int no);
    void Dequeue(int no);
    
    // computes the sum of the coverages of the runs currently being scanned, of which there are "pending"
    float RemainingValAt(float at, int pending);
  
    // sorting of the float_ligne_bord, for when we use a quicksort (not the case, right now)
    void SwapBords(int a,int b);
    void SwapBords(int a,int b,int c);
    void SortBords(int s,int e);

    static int CmpBord(float_ligne_bord const &d1, float_ligne_bord const &d2) {
        if ( d1.pos == d2.pos ) {
            if ( d1.start && !(d2.start) ) {
                return 1;
            }
            if ( !(d1.start) && d2.start ) {
                return -1;
            }
            return 0;
        }
        
        return (( d1.pos < d2.pos ) ? -1 : 1);
    };

    // miscanellous
    inline float ValAt(float at, float ps, float pe, float vs, float ve) {
        return ((at - ps) * ve + (pe - at) * vs) / (pe - ps);
    };
    
};

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

