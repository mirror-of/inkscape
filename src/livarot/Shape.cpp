/*
 *  Shape.cpp
 *  nlivarot
 *
 *  Created by fred on Thu Jun 12 2003.
 *
 */

#include <glib/gmem.h>
#include "Shape.h"
//#include "MyMath.h"
#include <libnr/nr-point.h>
#include <libnr/nr-point-fns.h>

/*
 * Shape instances handling.
 * never (i repeat: never) modify edges and points links; use Connect() and Disconnect() instead
 * the graph is stored as a set of points and edges, with edges in a doubly-linked list for each point.
 */

Shape::Shape (void)
  : _need_points_sorting(false),
    _need_edges_sorting(false),
    _has_points_data(false),
    _has_edges_data(false),
    _has_sweep_src_data(false),
    _has_sweep_dest_data(false),
    _has_sweep_data(false),
    _has_raster_data(false),
    _has_quick_raster_data(false),
    _has_back_data(false),
    _has_voronoi_data(false)
{
  leftX = topY = rightX = bottomY = 0;
  nbPt = maxPt = 0;
  nbAr = maxAr = 0;

  type = shape_polygon;

  pData = NULL;
  eData = NULL;
  ebData = NULL;
  swsData = NULL;
  swdData = NULL;
  swrData = NULL;
  qrsData = NULL;
  vorpData = NULL;
  voreData = NULL;
}
Shape::~Shape (void)
{
  nbPt = maxPt = 0;
  nbAr = maxAr = 0;
  g_free(eData);
  g_free(ebData);
  g_free(swsData);
  g_free(swdData);
  g_free(swrData);
  g_free(qrsData);
  g_free(pData);
  g_free(vorpData);
  g_free(voreData);
}

void Shape::Affiche(void)
{
  /*
  printf("sh=%p nbPt=%i nbAr=%i\n",this,nbPt,nbAr); // localizing ok
  for (int i=0;i<nbPt;i++) {
    printf("pt %i : x=(%f %f) dI=%i dO=%i\n",i,pts[i].x[0],pts[i].x[1],pts[i].dI,pts[i].dO); // localizing ok
  }
  for (int i=0;i<nbAr;i++) {
    printf("ar %i : dx=(%f %f) st=%i en=%i\n",i,aretes[i].dx[0],aretes[i].dx[1],aretes[i].st,aretes[i].en); // localizing ok
  }
  */
}

void
Shape::MakePointData (bool nVal)
{
  if (nVal)
    {
      if (_has_points_data == false)
	{
	  _has_points_data = true;
	  g_free(pData);
	  pData = (point_data *) g_malloc(maxPt * sizeof (point_data));
	}
    }
  else
    {
      if (_has_points_data)
	{
	  _has_points_data = false;
	  g_free(pData);
	  pData = NULL;
	}
    }
}
void
Shape::MakeEdgeData (bool nVal)
{
  if (nVal)
    {
      if (_has_edges_data == false)
	{
	  _has_edges_data = true;
	  g_free(eData);
	  eData = (edge_data *) g_malloc(maxAr * sizeof (edge_data));
	}
    }
  else
    {
      if (_has_edges_data)
	{
	  _has_edges_data = false;
	  g_free(eData);
	  eData = NULL;
	}
    }
}
void
Shape::MakeRasterData (bool nVal)
{
  if (nVal)
    {
      if (_has_raster_data == false)
	{
	  _has_raster_data = true;
	  g_free(swrData);
	  swrData = (raster_data *) g_malloc(maxAr * sizeof (raster_data));
	}
    }
  else
    {
      if (_has_raster_data)
	{
	  _has_raster_data = false;
	  g_free(swrData);
	  swrData = NULL;
	}
    }
}
void
Shape::MakeQuickRasterData (bool nVal)
{
  if (nVal)
    {
      if (_has_quick_raster_data == false)
	{
	  _has_quick_raster_data = true;
	  g_free(qrsData);
	  qrsData =
	    (quick_raster_data *) g_malloc(maxAr * sizeof (quick_raster_data));
	}
    }
  else
    {
      if (_has_quick_raster_data)
	{
	  _has_quick_raster_data = false;
	  g_free(qrsData);
	  qrsData = NULL;
	}
    }
}
void
Shape::MakeSweepSrcData (bool nVal)
{
  if (nVal)
    {
      if (_has_sweep_src_data == false)
	{
	  _has_sweep_src_data = true;
	  g_free(swsData);
	  swsData =
	    (sweep_src_data *) g_malloc(maxAr * sizeof (sweep_src_data));
	}
    }
  else
    {
      if (_has_sweep_src_data)
	{
	  _has_sweep_src_data = false;
	  g_free(swsData);
	  swsData = NULL;
	}
    }
}
void
Shape::MakeSweepDestData (bool nVal)
{
  if (nVal)
    {
      if (_has_sweep_dest_data == false)
	{
	  _has_sweep_dest_data = true;
	  g_free(swdData);
	  swdData =
	    (sweep_dest_data *) g_malloc(maxAr * sizeof (sweep_dest_data));
	}
    }
  else
    {
      if (_has_sweep_dest_data)
	{
	  _has_sweep_dest_data = false;
	  g_free(swdData);
	  swdData = NULL;
	}
    }
}
void
Shape::MakeBackData (bool nVal)
{
  if (nVal)
    {
      if (_has_back_data == false)
	{
	  _has_back_data = true;
	  g_free(ebData);
	  ebData = (back_data *) g_malloc(maxAr * sizeof (back_data));
	}
    }
  else
    {
      if (_has_back_data)
	{
	  _has_back_data = false;
	  g_free(ebData);
	  ebData = NULL;
	}
    }
}
void
Shape::MakeVoronoiData (bool nVal)
{
  if (nVal)
    {
      if (_has_voronoi_data == false)
	{
	  _has_voronoi_data = true;
	  g_free(vorpData);
	  g_free(voreData);
	  vorpData =
	    (voronoi_point *) g_malloc(maxPt * sizeof (voronoi_point));
	  voreData = (voronoi_edge *) g_malloc(maxAr * sizeof (voronoi_edge));
	}
    }
  else
    {
      if (_has_voronoi_data)
	{
	  _has_voronoi_data = false;
	  g_free(vorpData);
	  vorpData = NULL;
	  g_free(voreData);
	  voreData = NULL;
	}
    }
}


/**
 *  Copy point and edge data from `who' into this object, discarding
 *  any cached data that we have.
 */
void
Shape::Copy (Shape * who)
{
  if (who == NULL)
    {
      Reset (0, 0);
      return;
    }
  MakePointData (false);
  MakeEdgeData (false);
  MakeSweepSrcData (false);
  MakeSweepDestData (false);
  MakeRasterData (false);
  MakeQuickRasterData (false);
  MakeBackData (false);
  if (_has_sweep_data)
    {
      SweepTree::DestroyList (sTree);
      SweepEvent::DestroyQueue (sEvts);
      _has_sweep_data = false;
    }

  Reset (who->nbPt, who->nbAr);
  nbPt = who->nbPt;
  nbAr = who->nbAr;
  type = who->type;
  _need_points_sorting = who->_need_points_sorting;
  _need_edges_sorting = who->_need_edges_sorting;
  _has_points_data = false;
  _has_edges_data = false;
  _has_sweep_src_data = false;
  _has_sweep_dest_data = false;
  _has_sweep_data = false;
  _has_raster_data = false;
  _has_quick_raster_data = false;
  _has_back_data = false;
  _has_voronoi_data = false;

  _pts = who->_pts;
  aretes = who->aretes;
}

void
Shape::Reset (int n, int m)
{
  nbPt = 0;
  nbAr = 0;
  type = shape_polygon;
  if (n > maxPt)
    {
      maxPt = n;
      if (_has_points_data)
	pData = (point_data *) g_realloc(pData, maxPt * sizeof (point_data));
      if (_has_voronoi_data)
	vorpData =
	  (voronoi_point *) g_realloc(vorpData,
				     maxPt * sizeof (voronoi_point));
    }
  if (m > maxAr)
    {
      maxAr = m;
      aretes.reserve(maxAr);
      if (_has_edges_data)
	eData = (edge_data *) g_realloc(eData, maxAr * sizeof (edge_data));
      if (_has_sweep_dest_data)
	swdData =
	  (sweep_dest_data *) g_realloc(swdData,
				       maxAr * sizeof (sweep_dest_data));
      if (_has_sweep_src_data)
	swsData =
	  (sweep_src_data *) g_realloc(swsData,
				      maxAr * sizeof (sweep_src_data));
      if (_has_back_data)
	ebData = (back_data *) g_realloc(ebData, maxAr * sizeof (back_data));
      if (_has_voronoi_data)
	voreData =
	  (voronoi_edge *) g_realloc(voreData, maxAr * sizeof (voronoi_edge));
    }
  _pts.resize(n);
  aretes.resize(m);
  _need_points_sorting = false;
  _need_edges_sorting = false;
}

int
Shape::AddPoint (const NR::Point x)
{
  if (nbPt >= maxPt)
    {
      maxPt = 2 * nbPt + 1;
      if (_has_points_data)
	pData = (point_data *) g_realloc(pData, maxPt * sizeof (point_data));
      if (_has_voronoi_data)
	vorpData =
	  (voronoi_point *) g_realloc(vorpData,
				     maxPt * sizeof (voronoi_point));
    }
  int n = nbPt++;
  _pts.resize(nbPt);
  _pts[n].x = x;
  _pts[n].dI = _pts[n].dO = 0;
  _pts[n].firstA = _pts[n].lastA = -1;
  if (_has_points_data)
    {
      pData[n].pending = 0;
      pData[n].edgeOnLeft = -1;
      pData[n].nextLinkedPoint = -1;
      pData[n].askForWindingS = NULL;
      pData[n].askForWindingB = -1;
    }
  if (_has_voronoi_data)
    {
      vorpData[n].value = 0.0;
      vorpData[n].winding = -2;
    }
  _need_points_sorting = true;
  return n;
}

void
Shape::SubPoint (int p)
{
  if (p < 0 || p >= nbPt)
    return;
  _need_points_sorting = true;
  int cb;
  cb = getPoint(p).firstA;
  while (cb >= 0 && cb < nbAr)
    {
      if (aretes[cb].st == p)
	{
	  int ncb = aretes[cb].nextS;
	  aretes[cb].nextS = aretes[cb].prevS = -1;
	  aretes[cb].st = -1;
	  cb = ncb;
	}
      else if (aretes[cb].en == p)
	{
	  int ncb = aretes[cb].nextE;
	  aretes[cb].nextE = aretes[cb].prevE = -1;
	  aretes[cb].en = -1;
	  cb = ncb;
	}
      else
	{
	  break;
	}
    }
  _pts[p].firstA = _pts[p].lastA = -1;
  if (p < nbPt - 1)
    SwapPoints (p, nbPt - 1);
  nbPt--;
}

void
Shape::SwapPoints (int a, int b)
{
  if (a == b)
    return;
  if (getPoint(a).dI + getPoint(a).dO == 2 && getPoint(b).dI + getPoint(b).dO == 2)
    {
      int cb = getPoint(a).firstA;
      if (aretes[cb].st == a)
	{
	  aretes[cb].st = nbPt;
	}
      else if (aretes[cb].en == a)
	{
	  aretes[cb].en = nbPt;
	}
      cb = getPoint(a).lastA;
      if (aretes[cb].st == a)
	{
	  aretes[cb].st = nbPt;
	}
      else if (aretes[cb].en == a)
	{
	  aretes[cb].en = nbPt;
	}

      cb = getPoint(b).firstA;
      if (aretes[cb].st == b)
	{
	  aretes[cb].st = a;
	}
      else if (aretes[cb].en == b)
	{
	  aretes[cb].en = a;
	}
      cb = getPoint(b).lastA;
      if (aretes[cb].st == b)
	{
	  aretes[cb].st = a;
	}
      else if (aretes[cb].en == b)
	{
	  aretes[cb].en = a;
	}

      cb = getPoint(a).firstA;
      if (aretes[cb].st == nbPt)
	{
	  aretes[cb].st = b;
	}
      else if (aretes[cb].en == nbPt)
	{
	  aretes[cb].en = b;
	}
      cb = getPoint(a).lastA;
      if (aretes[cb].st == nbPt)
	{
	  aretes[cb].st = b;
	}
      else if (aretes[cb].en == nbPt)
	{
	  aretes[cb].en = b;
	}

    }
  else
    {
      int cb;
      cb = getPoint(a).firstA;
      while (cb >= 0)
	{
	  int ncb = NextAt (a, cb);
	  if (aretes[cb].st == a)
	    {
	      aretes[cb].st = nbPt;
	    }
	  else if (aretes[cb].en == a)
	    {
	      aretes[cb].en = nbPt;
	    }
	  cb = ncb;
	}
      cb = getPoint(b).firstA;
      while (cb >= 0)
	{
	  int ncb = NextAt (b, cb);
	  if (aretes[cb].st == b)
	    {
	      aretes[cb].st = a;
	    }
	  else if (aretes[cb].en == b)
	    {
	      aretes[cb].en = a;
	    }
	  cb = ncb;
	}
      cb = getPoint(a).firstA;
      while (cb >= 0)
	{
	  int ncb = NextAt (nbPt, cb);
	  if (aretes[cb].st == nbPt)
	    {
	      aretes[cb].st = b;
	    }
	  else if (aretes[cb].en == nbPt)
	    {
	      aretes[cb].en = b;
	    }
	  cb = ncb;
	}
    }
  {
    dg_point swap = getPoint(a);
    _pts[a] = getPoint(b);
    _pts[b] = swap;
  }
  if (_has_points_data)
    {
      point_data swad = pData[a];
      pData[a] = pData[b];
      pData[b] = swad;
      //              pData[pData[a].oldInd].newInd=a;
      //              pData[pData[b].oldInd].newInd=b;
    }
  if (_has_voronoi_data)
    {
      voronoi_point swav = vorpData[a];
      vorpData[a] = vorpData[b];
      vorpData[b] = swav;
    }
}
void
Shape::SwapPoints (int a, int b, int c)
{
  if (a == b || b == c || a == c)
    return;
  SwapPoints (a, b);
  SwapPoints (b, c);
}

void
Shape::SortPoints (void)
{
  if (_need_points_sorting && nbPt > 0)
    SortPoints (0, nbPt - 1);
  _need_points_sorting = false;
}

void
Shape::SortPointsRounded (void)
{
  if (nbPt > 0)
    SortPointsRounded (0, nbPt - 1);
}

void
Shape::SortPoints (int s, int e)
{
  if (s >= e)
    return;
  if (e == s + 1)
    {
      if (getPoint(s).x[1] > getPoint(e).x[1]
	  || (getPoint(s).x[1] == getPoint(e).x[1] && getPoint(s).x[0] > getPoint(e).x[0]))
	SwapPoints (s, e);
      return;
    }

  int ppos = (s + e) / 2;
  int plast = ppos;
  double pvalx = getPoint(ppos).x[0];
  double pvaly = getPoint(ppos).x[1];

  int le = s, ri = e;
  while (le < ppos || ri > plast)
    {
      if (le < ppos)
	{
	  do
	    {
	      int test = 0;
	      if (getPoint(le).x[1] > pvaly)
		{
		  test = 1;
		}
	      else if (getPoint(le).x[1] == pvaly)
		{
		  if (getPoint(le).x[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (getPoint(le).x[0] == pvalx)
		    {
		      test = 0;
		    }
		  else
		    {
		      test = -1;
		    }
		}
	      else
		{
		  test = -1;
		}
	      if (test == 0)
		{
		  // on colle les valeurs egales au pivot ensemble
		  if (le < ppos - 1)
		    {
		      SwapPoints (le, ppos - 1, ppos);
		      ppos--;
		      continue;	// sans changer le
		    }
		  else if (le == ppos - 1)
		    {
		      ppos--;
		      break;
		    }
		  else
		    {
		      // oupsie
		      break;
		    }
		}
	      if (test > 0)
		{
		  break;
		}
	      le++;
	    }
	  while (le < ppos);
	}
      if (ri > plast)
	{
	  do
	    {
	      int test = 0;
	      if (getPoint(ri).x[1] > pvaly)
		{
		  test = 1;
		}
	      else if (getPoint(ri).x[1] == pvaly)
		{
		  if (getPoint(ri).x[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (getPoint(ri).x[0] == pvalx)
		    {
		      test = 0;
		    }
		  else
		    {
		      test = -1;
		    }
		}
	      else
		{
		  test = -1;
		}
	      if (test == 0)
		{
		  // on colle les valeurs egales au pivot ensemble
		  if (ri > plast + 1)
		    {
		      SwapPoints (ri, plast + 1, plast);
		      plast++;
		      continue;	// sans changer ri
		    }
		  else if (ri == plast + 1)
		    {
		      plast++;
		      break;
		    }
		  else
		    {
		      // oupsie
		      break;
		    }
		}
	      if (test < 0)
		{
		  break;
		}
	      ri--;
	    }
	  while (ri > plast);
	}
      if (le < ppos)
	{
	  if (ri > plast)
	    {
	      SwapPoints (le, ri);
	      le++;
	      ri--;
	    }
	  else
	    {
	      if (le < ppos - 1)
		{
		  SwapPoints (ppos - 1, plast, le);
		  ppos--;
		  plast--;
		}
	      else if (le == ppos - 1)
		{
		  SwapPoints (plast, le);
		  ppos--;
		  plast--;
		}
	    }
	}
      else
	{
	  if (ri > plast + 1)
	    {
	      SwapPoints (plast + 1, ppos, ri);
	      ppos++;
	      plast++;
	    }
	  else if (ri == plast + 1)
	    {
	      SwapPoints (ppos, ri);
	      ppos++;
	      plast++;
	    }
	  else
	    {
	      break;
	    }
	}
    }
  SortPoints (s, ppos - 1);
  SortPoints (plast + 1, e);
}

void
Shape::SortPointsByOldInd (int s, int e)
{
  if (s >= e)
    return;
  if (e == s + 1)
    {
      if (getPoint(s).x[1] > getPoint(e).x[1] || (getPoint(s).x[1] == getPoint(e).x[1] && getPoint(s).x[0] > getPoint(e).x[0])
	  || (getPoint(s).x[1] == getPoint(e).x[1] && getPoint(s).x[0] == getPoint(e).x[0]
	      && pData[s].oldInd > pData[e].oldInd))
	SwapPoints (s, e);
      return;
    }

  int ppos = (s + e) / 2;
  int plast = ppos;
  double pvalx = getPoint(ppos).x[0];
  double pvaly = getPoint(ppos).x[1];
  int pvali = pData[ppos].oldInd;

  int le = s, ri = e;
  while (le < ppos || ri > plast)
    {
      if (le < ppos)
	{
	  do
	    {
	      int test = 0;
	      if (getPoint(le).x[1] > pvaly)
		{
		  test = 1;
		}
	      else if (getPoint(le).x[1] == pvaly)
		{
		  if (getPoint(le).x[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (getPoint(le).x[0] == pvalx)
		    {
		      if (pData[le].oldInd > pvali)
			{
			  test = 1;
			}
		      else if (pData[le].oldInd == pvali)
			{
			  test = 0;
			}
		      else
			{
			  test = -1;
			}
		    }
		  else
		    {
		      test = -1;
		    }
		}
	      else
		{
		  test = -1;
		}
	      if (test == 0)
		{
		  // on colle les valeurs egales au pivot ensemble
		  if (le < ppos - 1)
		    {
		      SwapPoints (le, ppos - 1, ppos);
		      ppos--;
		      continue;	// sans changer le
		    }
		  else if (le == ppos - 1)
		    {
		      ppos--;
		      break;
		    }
		  else
		    {
		      // oupsie
		      break;
		    }
		}
	      if (test > 0)
		{
		  break;
		}
	      le++;
	    }
	  while (le < ppos);
	}
      if (ri > plast)
	{
	  do
	    {
	      int test = 0;
	      if (getPoint(ri).x[1] > pvaly)
		{
		  test = 1;
		}
	      else if (getPoint(ri).x[1] == pvaly)
		{
		  if (getPoint(ri).x[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (getPoint(ri).x[0] == pvalx)
		    {
		      if (pData[ri].oldInd > pvali)
			{
			  test = 1;
			}
		      else if (pData[ri].oldInd == pvali)
			{
			  test = 0;
			}
		      else
			{
			  test = -1;
			}
		    }
		  else
		    {
		      test = -1;
		    }
		}
	      else
		{
		  test = -1;
		}
	      if (test == 0)
		{
		  // on colle les valeurs egales au pivot ensemble
		  if (ri > plast + 1)
		    {
		      SwapPoints (ri, plast + 1, plast);
		      plast++;
		      continue;	// sans changer ri
		    }
		  else if (ri == plast + 1)
		    {
		      plast++;
		      break;
		    }
		  else
		    {
		      // oupsie
		      break;
		    }
		}
	      if (test < 0)
		{
		  break;
		}
	      ri--;
	    }
	  while (ri > plast);
	}
      if (le < ppos)
	{
	  if (ri > plast)
	    {
	      SwapPoints (le, ri);
	      le++;
	      ri--;
	    }
	  else
	    {
	      if (le < ppos - 1)
		{
		  SwapPoints (ppos - 1, plast, le);
		  ppos--;
		  plast--;
		}
	      else if (le == ppos - 1)
		{
		  SwapPoints (plast, le);
		  ppos--;
		  plast--;
		}
	    }
	}
      else
	{
	  if (ri > plast + 1)
	    {
	      SwapPoints (plast + 1, ppos, ri);
	      ppos++;
	      plast++;
	    }
	  else if (ri == plast + 1)
	    {
	      SwapPoints (ppos, ri);
	      ppos++;
	      plast++;
	    }
	  else
	    {
	      break;
	    }
	}
    }
  SortPointsByOldInd (s, ppos - 1);
  SortPointsByOldInd (plast + 1, e);
}

void
Shape::SortPointsRounded (int s, int e)
{
  if (s >= e)
    return;
  if (e == s + 1)
    {
      if (pData[s].rx[1] > pData[e].rx[1]
	  || (pData[s].rx[1] == pData[e].rx[1] && pData[s].rx[0] > pData[e].rx[0]))
	SwapPoints (s, e);
      return;
    }

  int ppos = (s + e) / 2;
  int plast = ppos;
  double pvalx = pData[ppos].rx[0];
  double pvaly = pData[ppos].rx[1];

  int le = s, ri = e;
  while (le < ppos || ri > plast)
    {
      if (le < ppos)
	{
	  do
	    {
	      int test = 0;
	      if (pData[le].rx[1] > pvaly)
		{
		  test = 1;
		}
	      else if (pData[le].rx[1] == pvaly)
		{
		  if (pData[le].rx[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (pData[le].rx[0] == pvalx)
		    {
		      test = 0;
		    }
		  else
		    {
		      test = -1;
		    }
		}
	      else
		{
		  test = -1;
		}
	      if (test == 0)
		{
		  // on colle les valeurs egales au pivot ensemble
		  if (le < ppos - 1)
		    {
		      SwapPoints (le, ppos - 1, ppos);
		      ppos--;
		      continue;	// sans changer le
		    }
		  else if (le == ppos - 1)
		    {
		      ppos--;
		      break;
		    }
		  else
		    {
		      // oupsie
		      break;
		    }
		}
	      if (test > 0)
		{
		  break;
		}
	      le++;
	    }
	  while (le < ppos);
	}
      if (ri > plast)
	{
	  do
	    {
	      int test = 0;
	      if (pData[ri].rx[1] > pvaly)
		{
		  test = 1;
		}
	      else if (pData[ri].rx[1] == pvaly)
		{
		  if (pData[ri].rx[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (pData[ri].rx[0] == pvalx)
		    {
		      test = 0;
		    }
		  else
		    {
		      test = -1;
		    }
		}
	      else
		{
		  test = -1;
		}
	      if (test == 0)
		{
		  // on colle les valeurs egales au pivot ensemble
		  if (ri > plast + 1)
		    {
		      SwapPoints (ri, plast + 1, plast);
		      plast++;
		      continue;	// sans changer ri
		    }
		  else if (ri == plast + 1)
		    {
		      plast++;
		      break;
		    }
		  else
		    {
		      // oupsie
		      break;
		    }
		}
	      if (test < 0)
		{
		  break;
		}
	      ri--;
	    }
	  while (ri > plast);
	}
      if (le < ppos)
	{
	  if (ri > plast)
	    {
	      SwapPoints (le, ri);
	      le++;
	      ri--;
	    }
	  else
	    {
	      if (le < ppos - 1)
		{
		  SwapPoints (ppos - 1, plast, le);
		  ppos--;
		  plast--;
		}
	      else if (le == ppos - 1)
		{
		  SwapPoints (plast, le);
		  ppos--;
		  plast--;
		}
	    }
	}
      else
	{
	  if (ri > plast + 1)
	    {
	      SwapPoints (plast + 1, ppos, ri);
	      ppos++;
	      plast++;
	    }
	  else if (ri == plast + 1)
	    {
	      SwapPoints (ppos, ri);
	      ppos++;
	      plast++;
	    }
	  else
	    {
	      break;
	    }
	}
    }
  SortPointsRounded (s, ppos - 1);
  SortPointsRounded (plast + 1, e);
}

/*
 *
 */
int
Shape::AddEdge (int st, int en)
{
  if (st == en)
    return -1;
  if (st < 0 || en < 0)
    return -1;
  type = shape_graph;
  if (nbAr >= maxAr)
    {
      maxAr = 2 * nbAr + 1;
      aretes.reserve(maxAr);
      if (_has_edges_data)
	eData = (edge_data *) g_realloc(eData, maxAr * sizeof (edge_data));
      if (_has_sweep_src_data)
	swsData =
	  (sweep_src_data *) g_realloc(swsData,
				      maxAr * sizeof (sweep_src_data));
      if (_has_sweep_dest_data)
	swdData =
	  (sweep_dest_data *) g_realloc(swdData,
				       maxAr * sizeof (sweep_dest_data));
      if (_has_raster_data)
	swrData =
	  (raster_data *) g_realloc(swrData, maxAr * sizeof (raster_data));
      if (_has_back_data)
	ebData = (back_data *) g_realloc(ebData, maxAr * sizeof (back_data));
      if (_has_voronoi_data)
	voreData =
	  (voronoi_edge *) g_realloc(voreData, maxAr * sizeof (voronoi_edge));
    }
  int n = nbAr++;
  aretes.resize(nbAr);
  aretes[n].st = aretes[n].en = -1;
  aretes[n].prevS = aretes[n].nextS = -1;
  aretes[n].prevE = aretes[n].nextE = -1;
  if (st >= 0 && en >= 0)
    {
      aretes[n].dx = getPoint(en).x - getPoint(st).x;
    }
  else
    {
      aretes[n].dx[0] = aretes[n].dx[1] = 0;
    }
  ConnectStart (st, n);
  ConnectEnd (en, n);
  if (_has_edges_data)
    {
      eData[n].weight = 1;
      eData[n].rdx = aretes[n].dx;
    }
  if (_has_sweep_src_data)
    {
      swsData[n].misc = NULL;
      swsData[n].firstLinkedPoint = -1;
    }
  if (_has_back_data)
    {
      ebData[n].pathID = -1;
      ebData[n].pieceID = -1;
      ebData[n].tSt = ebData[n].tEn = 0;
    }
  if (_has_voronoi_data)
    {
      voreData[n].leF = -1;
      voreData[n].riF = -1;
    }
  _need_edges_sorting = true;
  return n;
}

int
Shape::AddEdge (int st, int en, int leF, int riF)
{
  if (st == en)
    return -1;
  if (st < 0 || en < 0)
    return -1;
  {
    int cb = getPoint(st).firstA;
    while (cb >= 0)
      {
	if (aretes[cb].st == st && aretes[cb].en == en)
	  return -1;		// doublon
	if (aretes[cb].st == en && aretes[cb].en == st)
	  return -1;		// doublon
	cb = NextAt (st, cb);
      }
  }
  type = shape_graph;
  if (nbAr >= maxAr)
    {
      maxAr = 2 * nbAr + 1;
      aretes.reserve(maxAr);
      if (_has_edges_data)
	eData = (edge_data *) g_realloc(eData, maxAr * sizeof (edge_data));
      if (_has_sweep_src_data)
	swsData =
	  (sweep_src_data *) g_realloc(swsData,
				      maxAr * sizeof (sweep_src_data));
      if (_has_sweep_dest_data)
	swdData =
	  (sweep_dest_data *) g_realloc(swdData,
				       maxAr * sizeof (sweep_dest_data));
      if (_has_raster_data)
	swrData =
	  (raster_data *) g_realloc(swrData, maxAr * sizeof (raster_data));
      if (_has_back_data)
	ebData = (back_data *) g_realloc(ebData, maxAr * sizeof (back_data));
      if (_has_voronoi_data)
	voreData =
	  (voronoi_edge *) g_realloc(voreData, maxAr * sizeof (voronoi_edge));
    }
  int n = nbAr++;
  aretes.resize(nbAr);
  aretes[n].st = aretes[n].en = -1;
  aretes[n].prevS = aretes[n].nextS = -1;
  aretes[n].prevE = aretes[n].nextE = -1;
  if (st >= 0 && en >= 0)
    {
      aretes[n].dx = getPoint(en).x - getPoint(st).x;
    }
  else
    {
      aretes[n].dx[0] = aretes[n].dx[1] = 0;
    }
  ConnectStart (st, n);
  ConnectEnd (en, n);
  if (_has_edges_data)
    {
      eData[n].weight = 1;
      eData[n].rdx = aretes[n].dx;
    }
  if (_has_sweep_src_data)
    {
      swsData[n].misc = NULL;
      swsData[n].firstLinkedPoint = -1;
    }
  if (_has_back_data)
    {
      ebData[n].pathID = -1;
      ebData[n].pieceID = -1;
      ebData[n].tSt = ebData[n].tEn = 0;
    }
  if (_has_voronoi_data)
    {
      voreData[n].leF = leF;
      voreData[n].riF = riF;
    }
  _need_edges_sorting = true;
  return n;
}

void
Shape::SubEdge (int e)
{
  if (e < 0 || e >= nbAr)
    return;
  type = shape_graph;
  DisconnectStart (e);
  DisconnectEnd (e);
  if (e < nbAr - 1)
    SwapEdges (e, nbAr - 1);
  nbAr--;
  _need_edges_sorting = true;
}

void
Shape::SwapEdges (int a, int b)
{
  if (a == b)
    return;
  if (aretes[a].prevS >= 0 && aretes[a].prevS != b)
    {
      if (aretes[aretes[a].prevS].st == aretes[a].st)
	{
	  aretes[aretes[a].prevS].nextS = b;
	}
      else if (aretes[aretes[a].prevS].en == aretes[a].st)
	{
	  aretes[aretes[a].prevS].nextE = b;
	}
    }
  if (aretes[a].nextS >= 0 && aretes[a].nextS != b)
    {
      if (aretes[aretes[a].nextS].st == aretes[a].st)
	{
	  aretes[aretes[a].nextS].prevS = b;
	}
      else if (aretes[aretes[a].nextS].en == aretes[a].st)
	{
	  aretes[aretes[a].nextS].prevE = b;
	}
    }
  if (aretes[a].prevE >= 0 && aretes[a].prevE != b)
    {
      if (aretes[aretes[a].prevE].st == aretes[a].en)
	{
	  aretes[aretes[a].prevE].nextS = b;
	}
      else if (aretes[aretes[a].prevE].en == aretes[a].en)
	{
	  aretes[aretes[a].prevE].nextE = b;
	}
    }
  if (aretes[a].nextE >= 0 && aretes[a].nextE != b)
    {
      if (aretes[aretes[a].nextE].st == aretes[a].en)
	{
	  aretes[aretes[a].nextE].prevS = b;
	}
      else if (aretes[aretes[a].nextE].en == aretes[a].en)
	{
	  aretes[aretes[a].nextE].prevE = b;
	}
    }
  if (aretes[a].st >= 0)
    {
      if (getPoint(aretes[a].st).firstA == a)
	_pts[aretes[a].st].firstA = nbAr;
      if (getPoint(aretes[a].st).lastA == a)
	_pts[aretes[a].st].lastA = nbAr;
    }
  if (aretes[a].en >= 0)
    {
      if (getPoint(aretes[a].en).firstA == a)
	_pts[aretes[a].en].firstA = nbAr;
      if (getPoint(aretes[a].en).lastA == a)
	_pts[aretes[a].en].lastA = nbAr;
    }


  if (aretes[b].prevS >= 0 && aretes[b].prevS != a)
    {
      if (aretes[aretes[b].prevS].st == aretes[b].st)
	{
	  aretes[aretes[b].prevS].nextS = a;
	}
      else if (aretes[aretes[b].prevS].en == aretes[b].st)
	{
	  aretes[aretes[b].prevS].nextE = a;
	}
    }
  if (aretes[b].nextS >= 0 && aretes[b].nextS != a)
    {
      if (aretes[aretes[b].nextS].st == aretes[b].st)
	{
	  aretes[aretes[b].nextS].prevS = a;
	}
      else if (aretes[aretes[b].nextS].en == aretes[b].st)
	{
	  aretes[aretes[b].nextS].prevE = a;
	}
    }
  if (aretes[b].prevE >= 0 && aretes[b].prevE != a)
    {
      if (aretes[aretes[b].prevE].st == aretes[b].en)
	{
	  aretes[aretes[b].prevE].nextS = a;
	}
      else if (aretes[aretes[b].prevE].en == aretes[b].en)
	{
	  aretes[aretes[b].prevE].nextE = a;
	}
    }
  if (aretes[b].nextE >= 0 && aretes[b].nextE != a)
    {
      if (aretes[aretes[b].nextE].st == aretes[b].en)
	{
	  aretes[aretes[b].nextE].prevS = a;
	}
      else if (aretes[aretes[b].nextE].en == aretes[b].en)
	{
	  aretes[aretes[b].nextE].prevE = a;
	}
    }
  if (aretes[b].st >= 0)
    {
      if (getPoint(aretes[b].st).firstA == b)
	_pts[aretes[b].st].firstA = a;
      if (getPoint(aretes[b].st).lastA == b)
	_pts[aretes[b].st].lastA = a;
    }
  if (aretes[b].en >= 0)
    {
      if (getPoint(aretes[b].en).firstA == b)
	_pts[aretes[b].en].firstA = a;
      if (getPoint(aretes[b].en).lastA == b)
	_pts[aretes[b].en].lastA = a;
    }

  if (aretes[a].st >= 0)
    {
      if (getPoint(aretes[a].st).firstA == nbAr)
	_pts[aretes[a].st].firstA = b;
      if (getPoint(aretes[a].st).lastA == nbAr)
	_pts[aretes[a].st].lastA = b;
    }
  if (aretes[a].en >= 0)
    {
      if (getPoint(aretes[a].en).firstA == nbAr)
	_pts[aretes[a].en].firstA = b;
      if (getPoint(aretes[a].en).lastA == nbAr)
	_pts[aretes[a].en].lastA = b;
    }

  if (aretes[a].prevS == b)
    aretes[a].prevS = a;
  if (aretes[a].prevE == b)
    aretes[a].prevE = a;
  if (aretes[a].nextS == b)
    aretes[a].nextS = a;
  if (aretes[a].nextE == b)
    aretes[a].nextE = a;
  if (aretes[b].prevS == a)
    aretes[a].prevS = b;
  if (aretes[b].prevE == a)
    aretes[a].prevE = b;
  if (aretes[b].nextS == a)
    aretes[a].nextS = b;
  if (aretes[b].nextE == a)
    aretes[a].nextE = b;

  dg_arete swap = aretes[a];
  aretes[a] = aretes[b];
  aretes[b] = swap;
  if (_has_edges_data)
    {
      edge_data swae = eData[a];
      eData[a] = eData[b];
      eData[b] = swae;
    }
  if (_has_sweep_src_data)
    {
      sweep_src_data swae = swsData[a];
      swsData[a] = swsData[b];
      swsData[b] = swae;
    }
  if (_has_sweep_dest_data)
    {
      sweep_dest_data swae = swdData[a];
      swdData[a] = swdData[b];
      swdData[b] = swae;
    }
  if (_has_raster_data)
    {
      raster_data swae = swrData[a];
      swrData[a] = swrData[b];
      swrData[b] = swae;
    }
  if (_has_back_data)
    {
      back_data swae = ebData[a];
      ebData[a] = ebData[b];
      ebData[b] = swae;
    }
  if (_has_voronoi_data)
    {
      voronoi_edge swav = voreData[a];
      voreData[a] = voreData[b];
      voreData[b] = swav;
    }
}
void
Shape::SwapEdges (int a, int b, int c)
{
  if (a == b || b == c || a == c)
    return;
  SwapEdges (a, b);
  SwapEdges (b, c);
}

void
Shape::SortEdges (void)
{
  if (_need_edges_sorting == false) {
    return;
  }
  _need_edges_sorting = false;

  edge_list *list = (edge_list *) g_malloc(nbAr * sizeof (edge_list));
  for (int p = 0; p < nbPt; p++)
    {
      int d = getPoint(p).dI + getPoint(p).dO;
      if (d > 1)
	{
	  int cb;
	  cb = getPoint(p).firstA;
	  int nb = 0;
	  while (cb >= 0)
	    {
	      int n = nb++;
	      list[n].no = cb;
	      if (aretes[cb].st == p)
		{
		  list[n].x = aretes[cb].dx;
		  list[n].starting = true;
		}
	      else
		{
		  list[n].x = -aretes[cb].dx;
		  list[n].starting = false;
		}
	      cb = NextAt (p, cb);
	    }
	  SortEdgesList (list, 0, nb - 1);
	  _pts[p].firstA = list[0].no;
	  _pts[p].lastA = list[nb - 1].no;
	  for (int i = 0; i < nb; i++)
	    {
	      if (list[i].starting)
		{
		  if (i > 0)
		    {
		      aretes[list[i].no].prevS = list[i - 1].no;
		    }
		  else
		    {
		      aretes[list[i].no].prevS = -1;
		    }
		  if (i < nb - 1)
		    {
		      aretes[list[i].no].nextS = list[i + 1].no;
		    }
		  else
		    {
		      aretes[list[i].no].nextS = -1;
		    }
		}
	      else
		{
		  if (i > 0)
		    {
		      aretes[list[i].no].prevE = list[i - 1].no;
		    }
		  else
		    {
		      aretes[list[i].no].prevE = -1;
		    }
		  if (i < nb - 1)
		    {
		      aretes[list[i].no].nextE = list[i + 1].no;
		    }
		  else
		    {
		      aretes[list[i].no].nextE = -1;
		    }
		}
	    }
	}
    }
  g_free(list);
}

int
Shape::CmpToVert (NR::Point ax, NR::Point bx,bool as,bool bs)
{
  int tstAX = 0;
  int tstAY = 0;
  int tstBX = 0;
  int tstBY = 0;
  if (ax[0] > 0)
    tstAX = 1;
  if (ax[0] < 0)
    tstAX = -1;
  if (ax[1] > 0)
    tstAY = 1;
  if (ax[1] < 0)
    tstAY = -1;
  if (bx[0] > 0)
    tstBX = 1;
  if (bx[0] < 0)
    tstBX = -1;
  if (bx[1] > 0)
    tstBY = 1;
  if (bx[1] < 0)
    tstBY = -1;

  int quadA = 0, quadB = 0;
  if (tstAX < 0)
    {
      if (tstAY < 0)
	{
	  quadA = 7;
	}
      else if (tstAY == 0)
	{
	  quadA = 6;
	}
      else if (tstAY > 0)
	{
	  quadA = 5;
	}
    }
  else if (tstAX == 0)
    {
      if (tstAY < 0)
	{
	  quadA = 0;
	}
      else if (tstAY == 0)
	{
	  quadA = -1;
	}
      else if (tstAY > 0)
	{
	  quadA = 4;
	}
    }
  else if (tstAX > 0)
    {
      if (tstAY < 0)
	{
	  quadA = 1;
	}
      else if (tstAY == 0)
	{
	  quadA = 2;
	}
      else if (tstAY > 0)
	{
	  quadA = 3;
	}
    }
  if (tstBX < 0)
    {
      if (tstBY < 0)
	{
	  quadB = 7;
	}
      else if (tstBY == 0)
	{
	  quadB = 6;
	}
      else if (tstBY > 0)
	{
	  quadB = 5;
	}
    }
  else if (tstBX == 0)
    {
      if (tstBY < 0)
	{
	  quadB = 0;
	}
      else if (tstBY == 0)
	{
	  quadB = -1;
	}
      else if (tstBY > 0)
	{
	  quadB = 4;
	}
    }
  else if (tstBX > 0)
    {
      if (tstBY < 0)
	{
	  quadB = 1;
	}
      else if (tstBY == 0)
	{
	  quadB = 2;
	}
      else if (tstBY > 0)
	{
	  quadB = 3;
	}
    }
  if (quadA < quadB)
    return 1;
  if (quadA > quadB)
    return -1;

  NR::Point av, bv;
  av = ax;
  bv = bx;
  double si = cross (bv, av);
  int tstSi = 0;
  if (si > 0.000001) tstSi = 1;
  if (si < -0.000001) tstSi = -1;
  if ( tstSi == 0 ) {
    if ( as == true && bs == false ) return -1;
    if ( as == false && bs == true ) return 1;
  }
  return tstSi;
}

void
Shape::SortEdgesList (edge_list * list, int s, int e)
{
  if (s >= e)
    return;
  if (e == s + 1) {
    int cmpval=CmpToVert (list[e].x, list[s].x,list[e].starting,list[s].starting);
    if ( cmpval > 0 )  { // priorite aux sortants
      edge_list swap = list[s];
      list[s] = list[e];
      list[e] = swap;
    }
    return;
 }

  int ppos = (s + e) / 2;
  int plast = ppos;
  NR::Point pvalx = list[ppos].x;
  bool      pvals = list[ppos].starting;
  
  int le = s, ri = e;
  while (le < ppos || ri > plast)
    {
      if (le < ppos)
	{
	  do
	    {
        int test = CmpToVert (pvalx, list[le].x,pvals,list[le].starting);
	      if (test == 0)
		{
		  // on colle les valeurs egales au pivot ensemble
		  if (le < ppos - 1)
		    {
		      edge_list swap = list[le];
		      list[le] = list[ppos - 1];
		      list[ppos - 1] = list[ppos];
		      list[ppos] = swap;
		      ppos--;
		      continue;	// sans changer le
		    }
		  else if (le == ppos - 1)
		    {
		      ppos--;
		      break;
		    }
		  else
		    {
		      // oupsie
		      break;
		    }
		}
	      if (test > 0)
		{
		  break;
		}
	      le++;
	    }
	  while (le < ppos);
	}
      if (ri > plast)
	{
	  do
	    {
        int test = CmpToVert (pvalx, list[ri].x,pvals,list[ri].starting);
	      if (test == 0)
		{
		  // on colle les valeurs egales au pivot ensemble
		  if (ri > plast + 1)
		    {
		      edge_list swap = list[ri];
		      list[ri] = list[plast + 1];
		      list[plast + 1] = list[plast];
		      list[plast] = swap;
		      plast++;
		      continue;	// sans changer ri
		    }
		  else if (ri == plast + 1)
		    {
		      plast++;
		      break;
		    }
		  else
		    {
		      // oupsie
		      break;
		    }
		}
	      if (test < 0)
		{
		  break;
		}
	      ri--;
	    }
	  while (ri > plast);
	}

      if (le < ppos)
	{
	  if (ri > plast)
	    {
	      edge_list swap = list[le];
	      list[le] = list[ri];
	      list[ri] = swap;
	      le++;
	      ri--;
	    }
	  else if (le < ppos - 1)
	    {
	      edge_list swap = list[ppos - 1];
	      list[ppos - 1] = list[plast];
	      list[plast] = list[le];
	      list[le] = swap;
	      ppos--;
	      plast--;
	    }
	  else if (le == ppos - 1)
	    {
	      edge_list swap = list[plast];
	      list[plast] = list[le];
	      list[le] = swap;
	      ppos--;
	      plast--;
	    }
	  else
	    {
	      break;
	    }
	}
      else
	{
	  if (ri > plast + 1)
	    {
	      edge_list swap = list[plast + 1];
	      list[plast + 1] = list[ppos];
	      list[ppos] = list[ri];
	      list[ri] = swap;
	      ppos++;
	      plast++;
	    }
	  else if (ri == plast + 1)
	    {
	      edge_list swap = list[ppos];
	      list[ppos] = list[ri];
	      list[ri] = swap;
	      ppos++;
	      plast++;
	    }
	  else
	    {
	      break;
	    }
	}
    }
  SortEdgesList (list, s, ppos - 1);
  SortEdgesList (list, plast + 1, e);

}



/*
 *
 */
void
Shape::ConnectStart (int p, int b)
{
  if (aretes[b].st >= 0)
    DisconnectStart (b);
  aretes[b].st = p;
  _pts[p].dO++;
  aretes[b].nextS = -1;
  aretes[b].prevS = getPoint(p).lastA;
  if (getPoint(p).lastA >= 0)
    {
      if (aretes[getPoint(p).lastA].st == p)
	{
	  aretes[getPoint(p).lastA].nextS = b;
	}
      else if (aretes[getPoint(p).lastA].en == p)
	{
	  aretes[getPoint(p).lastA].nextE = b;
	}
    }
  _pts[p].lastA = b;
  if (getPoint(p).firstA < 0)
    _pts[p].firstA = b;
}

void
Shape::ConnectEnd (int p, int b)
{
  if (aretes[b].en >= 0)
    DisconnectEnd (b);
  aretes[b].en = p;
  _pts[p].dI++;
  aretes[b].nextE = -1;
  aretes[b].prevE = getPoint(p).lastA;
  if (getPoint(p).lastA >= 0)
    {
      if (aretes[getPoint(p).lastA].st == p)
	{
	  aretes[getPoint(p).lastA].nextS = b;
	}
      else if (aretes[getPoint(p).lastA].en == p)
	{
	  aretes[getPoint(p).lastA].nextE = b;
	}
    }
  _pts[p].lastA = b;
  if (getPoint(p).firstA < 0)
    _pts[p].firstA = b;
}

void
Shape::DisconnectStart (int b)
{
  if (aretes[b].st < 0)
    return;
  _pts[aretes[b].st].dO--;
  if (aretes[b].prevS >= 0)
    {
      if (aretes[aretes[b].prevS].st == aretes[b].st)
	{
	  aretes[aretes[b].prevS].nextS = aretes[b].nextS;
	}
      else if (aretes[aretes[b].prevS].en == aretes[b].st)
	{
	  aretes[aretes[b].prevS].nextE = aretes[b].nextS;
	}
    }
  if (aretes[b].nextS >= 0)
    {
      if (aretes[aretes[b].nextS].st == aretes[b].st)
	{
	  aretes[aretes[b].nextS].prevS = aretes[b].prevS;
	}
      else if (aretes[aretes[b].nextS].en == aretes[b].st)
	{
	  aretes[aretes[b].nextS].prevE = aretes[b].prevS;
	}
    }
  if (getPoint(aretes[b].st).firstA == b)
    _pts[aretes[b].st].firstA = aretes[b].nextS;
  if (getPoint(aretes[b].st).lastA == b)
    _pts[aretes[b].st].lastA = aretes[b].prevS;
  aretes[b].st = -1;
}

void
Shape::DisconnectEnd (int b)
{
  if (aretes[b].en < 0)
    return;
  _pts[aretes[b].en].dI--;
  if (aretes[b].prevE >= 0)
    {
      if (aretes[aretes[b].prevE].st == aretes[b].en)
	{
	  aretes[aretes[b].prevE].nextS = aretes[b].nextE;
	}
      else if (aretes[aretes[b].prevE].en == aretes[b].en)
	{
	  aretes[aretes[b].prevE].nextE = aretes[b].nextE;
	}
    }
  if (aretes[b].nextE >= 0)
    {
      if (aretes[aretes[b].nextE].st == aretes[b].en)
	{
	  aretes[aretes[b].nextE].prevS = aretes[b].prevE;
	}
      else if (aretes[aretes[b].nextE].en == aretes[b].en)
	{
	  aretes[aretes[b].nextE].prevE = aretes[b].prevE;
	}
    }
  if (getPoint(aretes[b].en).firstA == b)
    _pts[aretes[b].en].firstA = aretes[b].nextE;
  if (getPoint(aretes[b].en).lastA == b)
    _pts[aretes[b].en].lastA = aretes[b].prevE;
  aretes[b].en = -1;
}

bool
Shape::Eulerian (bool directed)
{
  if (directed)
    {
      for (int i = 0; i < nbPt; i++)
	{
	  if (getPoint(i).dI != getPoint(i).dO)
	    {
	      return false;
	    }
	}
    }
  else
    {
      for (int i = 0; i < nbPt; i++)
	{
	  int d = getPoint(i).dI + getPoint(i).dO;
	  if (d % 2 == 1)
	    {
	      return false;
	    }
	}
    }
  return true;
}

void
Shape::Inverse (int b)
{
  int swap;
  swap = aretes[b].st;
  aretes[b].st = aretes[b].en;
  aretes[b].en = swap;
  swap = aretes[b].prevE;
  aretes[b].prevE = aretes[b].prevS;
  aretes[b].prevS = swap;
  swap = aretes[b].nextE;
  aretes[b].nextE = aretes[b].nextS;
  aretes[b].nextS = swap;
  aretes[b].dx = -aretes[b].dx;
  if (aretes[b].st >= 0)
    {
      _pts[aretes[b].st].dO++;
      _pts[aretes[b].st].dI--;
    }
  if (aretes[b].en >= 0)
    {
      _pts[aretes[b].en].dO--;
      _pts[aretes[b].en].dI++;
    }
  if (_has_edges_data)
    eData[b].weight = -eData[b].weight;
  if (_has_sweep_dest_data)
    {
      int swap = swdData[b].leW;
      swdData[b].leW = swdData[b].riW;
      swdData[b].riW = swap;
    }
  if (_has_back_data)
    {
      double swat = ebData[b].tSt;
      ebData[b].tSt = ebData[b].tEn;
      ebData[b].tEn = swat;
    }
  if (_has_voronoi_data)
    {
      int swai = voreData[b].leF;
      voreData[b].leF = voreData[b].riF;
      voreData[b].riF = swai;
    }
}
void
Shape::CalcBBox (bool strict_degree)
{
  if (nbPt <= 0)
  {
    leftX = rightX = topY = bottomY = 0;
    return;
  }
  leftX = rightX = getPoint(0).x[0];
  topY = bottomY = getPoint(0).x[1];
  bool not_set=true;
  for (int i = 0; i < nbPt; i++)
  {
    if ( strict_degree == false || getPoint(i).dI > 0 || getPoint(i).dO > 0 ) {
      if ( not_set ) {
        leftX = rightX = getPoint(i).x[0];
        topY = bottomY = getPoint(i).x[1];
        not_set=false;
      } else {
        if (  getPoint(i).x[0] < leftX) leftX = getPoint(i).x[0];
        if (  getPoint(i).x[0] > rightX) rightX = getPoint(i).x[0];
        if (  getPoint(i).x[1] < topY) topY = getPoint(i).x[1];
        if (  getPoint(i).x[1] > bottomY) bottomY = getPoint(i).x[1];
      }
    }
  }
}

/** Returns true iff the L2 distance from \a thePt to this shape is <= \a max_l2.
*  Distance = the min of distance to its points and distance to its edges.
*  Points without edges are considered, which is maybe unwanted...
*/
bool Shape::DistanceLE(NR::Point const thePt, double const max_l2)
{
  if ( nbPt <= 0 ) {
    return false;
  }
  
  /* TODO: Consider using bbox to return early, perhaps conditional on nbPt or nbAr. */
  
  /* Test thePt against pts[i].x for all i. */
  {
    /* effic: In one test case (scribbling with the freehand tool to create a small number of long
    * path elements), changing from a Distance method to a DistanceLE method reduced this
* function's CPU time from about 21% of total inkscape CPU time to 14-15% of total inkscape
* CPU time, due to allowing early termination.  I don't know how much the L1 test helps, it
* may well be a case of premature optimization.  Consider testing dot(offset, offset)
* instead.
*/
    double const max_l1 = max_l2 * M_SQRT2;
    for (int i = 0; i < nbPt; i++) {
      NR::Point const offset( thePt - getPoint(i).x );
      double const l1 = NR::L1(offset);
      if ( ( l1 <= max_l2 )
           || ( ( l1 <= max_l1 )
                && ( NR::L2(offset) <= max_l2 ) ) )
      {
        return true;
      }
    }
  }
  
  for (int i = 0; i < nbAr; i++) {
    if ( aretes[i].st >= 0 &&
         aretes[i].en >= 0 ) {
      NR::Point const st(getPoint(aretes[i].st).x);
      NR::Point const en(getPoint(aretes[i].en).x);
      NR::Point const d( thePt - st );
      NR::Point const e( en - st );
      double const el = NR::L2(e);
      if ( el > 0.001 ) {
        NR::Point const e_unit( e / el );
        double const npr = NR::dot(d, e_unit);
        if ( npr > 0 && npr < el ) {
          double const nl = fabs( NR::cross(d, e_unit) );
          if ( nl <= max_l2 ) {
            return true;
          }
        }
      }
    }
  }
  return false;
}
/** Returns true iff the L2 distance from \a thePt to this shape is <= \a max_l2.
*  Distance = the min of distance to its points and distance to its edges.
*  Points without edges are considered, which is maybe unwanted...
*/
double Shape::Distance(NR::Point const thePt)
{
  if ( nbPt <= 0 ) {
    return 0.0;
  }
  
  double bdot=NR::dot(thePt-getPoint(0).x,thePt-getPoint(0).x);
  {
    for (int i = 0; i < nbPt; i++) {
      NR::Point const offset( thePt - getPoint(i).x );
      double ndot=NR::dot(offset,offset);
      if ( ndot < bdot ) {
        bdot=ndot;
      }
    }
  }
  
  for (int i = 0; i < nbAr; i++) {
    if ( aretes[i].st >= 0 &&
         aretes[i].en >= 0 ) {
      NR::Point const st(getPoint(aretes[i].st).x);
      NR::Point const en(getPoint(aretes[i].en).x);
      NR::Point const d( thePt - st );
      NR::Point const e( en - st );
      double const el = NR::dot(e,e);
      if ( el > 0.001 ) {
        double const npr = NR::dot(d, e);
        if ( npr > 0 && npr < el ) {
          double const nl = fabs( NR::cross(d, e) );
          double ndot=nl*nl/el;
          if ( ndot < bdot ) {
            bdot=ndot;
          }
        }
      }
    }
  }
  return sqrt(bdot);
}

// winding of a point with respect to the Shape
// 0= outside
// 1= inside (or -1, that usually the same)
// other=depends on your fill rule
// if the polygon is uncrossed, it's all the same, usually
int
Shape::PtWinding (const NR::Point px) const 
{
  int lr = 0, ll = 0, rr = 0;
  
  for (int i = 0; i < nbAr; i++)
  {
    NR::Point const adir = aretes[i].dx;

    NR::Point const ast = getPoint(aretes[i].st).x;
    NR::Point const aen = getPoint(aretes[i].en).x;
    
    //int const nWeight = eData[i].weight;
    int const nWeight = 1;

    if (ast[0] < aen[0]) {
      if (ast[0] > px[0]) continue;
      if (aen[0] < px[0]) continue;
    } else {
      if (ast[0] < px[0]) continue;
      if (aen[0] > px[0]) continue;
    }
    if (ast[0] == px[0]) {
      if (ast[1] >= px[1]) continue;
      if (aen[0] == px[0]) continue;
      if (aen[0] < px[0]) ll += nWeight;  else rr -= nWeight;
      continue;
    }
    if (aen[0] == px[0]) {
      if (aen[1] >= px[1]) continue;
      if (ast[0] == px[0]) continue;
      if (ast[0] < px[0]) ll -= nWeight; else rr += nWeight;
      continue;
    }
    
    if (ast[1] < aen[1]) {
      if (ast[1] >= px[1])  continue;
    } else {
      if (aen[1] >= px[1]) continue;
    }

    NR::Point const diff = px - ast;
    double const cote = cross(diff, adir);
    if (cote == 0) continue;
    if (cote < 0) {
      if (ast[0] > px[0]) lr += nWeight;
    } else {
      if (ast[0] < px[0]) lr -= nWeight;
    }
  }
  return lr + (ll + rr) / 2;
}

//};
