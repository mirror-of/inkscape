/*
 *  Shape.cpp
 *  nlivarot
 *
 *  Created by fred on Thu Jun 12 2003.
 *
 */

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
{
  leftX = topY = rightX = bottomY = 0;
  nbPt = maxPt = 0;
  pts = NULL;
  nbAr = maxAr = 0;
  aretes = NULL;

  flags = 0;
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
  if (maxPt > 0)
    {
      free (pts);
    }
  nbPt = maxPt = 0;
  pts = NULL;
  if (maxAr > 0)
    {
      free (aretes);
    }
  nbAr = maxAr = 0;
  aretes = NULL;
  if (eData)
    free (eData);
  if (ebData)
    free (ebData);
  if (swsData)
    free (swsData);
  if (swdData)
    free (swdData);
  if (swrData)
    free (swrData);
  if (qrsData)
    free (qrsData);
  if (pData)
    free (pData);
  if (vorpData)
    free (vorpData);
  if (voreData)
    free (voreData);
}

void
Shape::MakePointData (bool nVal)
{
  if (nVal)
    {
      if (HasPointsData ())
	{
	}
      else
	{
	  flags |= has_points_data;
	  if (pData)
	    free (pData);
	  pData = (point_data *) malloc (maxPt * sizeof (point_data));
	}
    }
  else
    {
      if (HasPointsData ())
	{
	  flags &= ~(has_points_data);
	  if (pData)
	    {
	      free (pData);
	      pData = NULL;
	    }
	}
      else
	{
	}
    }
}
void
Shape::MakeEdgeData (bool nVal)
{
  if (nVal)
    {
      if (HasEdgesData ())
	{
	}
      else
	{
	  flags |= has_edges_data;
	  if (eData)
	    free (eData);
	  eData = (edge_data *) malloc (maxAr * sizeof (edge_data));
	}
    }
  else
    {
      if (HasEdgesData ())
	{
	  flags &= ~(has_edges_data);
	  if (eData)
	    {
	      free (eData);
	      eData = NULL;
	    }
	}
      else
	{
	}
    }
}
void
Shape::MakeRasterData (bool nVal)
{
  if (nVal)
    {
      if (HasRasterData ())
	{
	}
      else
	{
	  flags |= has_raster_data;
	  if (swrData)
	    free (swrData);
	  swrData = (raster_data *) malloc (maxAr * sizeof (raster_data));
	}
    }
  else
    {
      if (HasRasterData ())
	{
	  flags &= ~(has_raster_data);
	  if (swrData)
	    {
	      free (swrData);
	      swrData = NULL;
	    }
	}
      else
	{
	}
    }
}
void
Shape::MakeQuickRasterData (bool nVal)
{
  if (nVal)
    {
      if (HasQuickRasterData ())
	{
	}
      else
	{
	  flags |= has_quick_raster_data;
	  if (qrsData)
	    free (qrsData);
	  qrsData =
	    (quick_raster_data *) malloc (maxAr * sizeof (quick_raster_data));
	}
    }
  else
    {
      if (HasQuickRasterData ())
	{
	  flags &= ~(has_quick_raster_data);
	  if (qrsData)
	    {
	      free (qrsData);
	      qrsData = NULL;
	    }
	}
      else
	{
	}
    }
}
void
Shape::MakeSweepSrcData (bool nVal)
{
  if (nVal)
    {
      if (HasSweepSrcData ())
	{
	}
      else
	{
	  flags |= has_sweep_src_data;
	  if (swsData)
	    free (swsData);
	  swsData =
	    (sweep_src_data *) malloc (maxAr * sizeof (sweep_src_data));
	}
    }
  else
    {
      if (HasSweepSrcData ())
	{
	  flags &= ~(has_sweep_src_data);
	  if (swsData)
	    {
	      free (swsData);
	      swsData = NULL;
	    }
	}
      else
	{
	}
    }
}
void
Shape::MakeSweepDestData (bool nVal)
{
  if (nVal)
    {
      if (HasSweepDestData ())
	{
	}
      else
	{
	  flags |= has_sweep_dest_data;
	  if (swdData)
	    free (swdData);
	  swdData =
	    (sweep_dest_data *) malloc (maxAr * sizeof (sweep_dest_data));
	}
    }
  else
    {
      if (HasSweepDestData ())
	{
	  flags &= ~(has_sweep_dest_data);
	  if (swdData)
	    {
	      free (swdData);
	      swdData = NULL;
	    }
	}
      else
	{
	}
    }
}
void
Shape::MakeBackData (bool nVal)
{
  if (nVal)
    {
      if (HasBackData ())
	{
	}
      else
	{
	  flags |= has_back_data;
	  if (ebData)
	    free (ebData);
	  ebData = (back_data *) malloc (maxAr * sizeof (back_data));
	}
    }
  else
    {
      if (HasBackData ())
	{
	  flags &= ~(has_back_data);
	  if (ebData)
	    {
	      free (ebData);
	      ebData = NULL;
	    }
	}
      else
	{
	}
    }
}
void
Shape::MakeVoronoiData (bool nVal)
{
  if (nVal)
    {
      if (HasVoronoiData ())
	{
	}
      else
	{
	  flags |= has_voronoi_data;
	  if (vorpData)
	    free (vorpData);
	  if (voreData)
	    free (voreData);
	  vorpData =
	    (voronoi_point *) malloc (maxPt * sizeof (voronoi_point));
	  voreData = (voronoi_edge *) malloc (maxAr * sizeof (voronoi_edge));
	}
    }
  else
    {
      if (HasVoronoiData ())
	{
	  flags &= ~(has_voronoi_data);
	  if (vorpData)
	    {
	      free (vorpData);
	      vorpData = NULL;
	    }
	  if (voreData)
	    {
	      free (voreData);
	      voreData = NULL;
	    }
	}
      else
	{
	}
    }
}

/*
 *
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
  if (GetFlag (has_sweep_data))
    {
      SweepTree::DestroyList (sTree);
      SweepEvent::DestroyQueue (sEvts);
      SetFlag (has_sweep_data, false);
    }

  Reset (who->nbPt, who->nbAr);
  nbPt = who->nbPt;
  nbAr = who->nbAr;
  type = who->type;
  flags = who->flags & (need_points_sorting + need_edges_sorting);

  memcpy (pts, who->pts, nbPt * sizeof (dg_point));
  memcpy (aretes, who->aretes, nbAr * sizeof (dg_arete));
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
      pts = (dg_point *) realloc (pts, maxPt * sizeof (dg_point));
      if (HasPointsData ())
	pData = (point_data *) realloc (pData, maxPt * sizeof (point_data));
      if (HasVoronoiData ())
	vorpData =
	  (voronoi_point *) realloc (vorpData,
				     maxPt * sizeof (voronoi_point));
    }
  if (m > maxAr)
    {
      maxAr = m;
      aretes = (dg_arete *) realloc (aretes, maxAr * sizeof (dg_arete));
      if (HasEdgesData ())
	eData = (edge_data *) realloc (eData, maxAr * sizeof (edge_data));
      if (HasSweepDestData ())
	swdData =
	  (sweep_dest_data *) realloc (swdData,
				       maxAr * sizeof (sweep_dest_data));
      if (HasSweepSrcData ())
	swsData =
	  (sweep_src_data *) realloc (swsData,
				      maxAr * sizeof (sweep_src_data));
      if (HasBackData ())
	ebData = (back_data *) realloc (ebData, maxAr * sizeof (back_data));
      if (HasVoronoiData ())
	voreData =
	  (voronoi_edge *) realloc (voreData, maxAr * sizeof (voronoi_edge));
    }
  SetFlag (need_points_sorting, false);
  SetFlag (need_edges_sorting, false);
}

int
Shape::AddPoint (const NR::Point x)
{
  if (nbPt >= maxPt)
    {
      maxPt = 2 * nbPt + 1;
      pts = (dg_point *) realloc (pts, maxPt * sizeof (dg_point));
      if (HasPointsData ())
	pData = (point_data *) realloc (pData, maxPt * sizeof (point_data));
      if (HasVoronoiData ())
	vorpData =
	  (voronoi_point *) realloc (vorpData,
				     maxPt * sizeof (voronoi_point));
    }
  int n = nbPt++;
  pts[n].x = x;
  pts[n].dI = pts[n].dO = 0;
  pts[n].firstA = pts[n].lastA = -1;
  if (HasPointsData ())
    {
      pData[n].pending = 0;
      pData[n].edgeOnLeft = -1;
      pData[n].nextLinkedPoint = -1;
      pData[n].askForWindingS = NULL;
      pData[n].askForWindingB = -1;
    }
  if (HasVoronoiData ())
    {
      vorpData[n].value = 0.0;
      vorpData[n].winding = -2;
    }
  SetFlag (need_points_sorting, true);
  return n;
}

void
Shape::SubPoint (int p)
{
  if (p < 0 || p >= nbPt)
    return;
  SetFlag (need_points_sorting, true);
  int cb;
  cb = pts[p].firstA;
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
  pts[p].firstA = pts[p].lastA = -1;
  if (p < nbPt - 1)
    SwapPoints (p, nbPt - 1);
  nbPt--;
}

void
Shape::SwapPoints (int a, int b)
{
  if (a == b)
    return;
  if (pts[a].dI + pts[a].dO == 2 && pts[b].dI + pts[b].dO == 2)
    {
      int cb = pts[a].firstA;
      if (aretes[cb].st == a)
	{
	  aretes[cb].st = nbPt;
	}
      else if (aretes[cb].en == a)
	{
	  aretes[cb].en = nbPt;
	}
      cb = pts[a].lastA;
      if (aretes[cb].st == a)
	{
	  aretes[cb].st = nbPt;
	}
      else if (aretes[cb].en == a)
	{
	  aretes[cb].en = nbPt;
	}

      cb = pts[b].firstA;
      if (aretes[cb].st == b)
	{
	  aretes[cb].st = a;
	}
      else if (aretes[cb].en == b)
	{
	  aretes[cb].en = a;
	}
      cb = pts[b].lastA;
      if (aretes[cb].st == b)
	{
	  aretes[cb].st = a;
	}
      else if (aretes[cb].en == b)
	{
	  aretes[cb].en = a;
	}

      cb = pts[a].firstA;
      if (aretes[cb].st == nbPt)
	{
	  aretes[cb].st = b;
	}
      else if (aretes[cb].en == nbPt)
	{
	  aretes[cb].en = b;
	}
      cb = pts[a].lastA;
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
      cb = pts[a].firstA;
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
      cb = pts[b].firstA;
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
      cb = pts[a].firstA;
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
    dg_point swap = pts[a];
    pts[a] = pts[b];
    pts[b] = swap;
  }
  if (HasPointsData ())
    {
      point_data swad = pData[a];
      pData[a] = pData[b];
      pData[b] = swad;
      //              pData[pData[a].oldInd].newInd=a;
      //              pData[pData[b].oldInd].newInd=b;
    }
  if (HasVoronoiData ())
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
  if (GetFlag (need_points_sorting) && nbPt > 0)
    SortPoints (0, nbPt - 1);
  SetFlag (need_points_sorting, false);
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
      if (pts[s].x[1] > pts[e].x[1]
	  || (pts[s].x[1] == pts[e].x[1] && pts[s].x[0] > pts[e].x[0]))
	SwapPoints (s, e);
      return;
    }

  int ppos = (s + e) / 2;
  int plast = ppos;
  double pvalx = pts[ppos].x[0];
  double pvaly = pts[ppos].x[1];

  int le = s, ri = e;
  while (le < ppos || ri > plast)
    {
      if (le < ppos)
	{
	  do
	    {
	      int test = 0;
	      if (pts[le].x[1] > pvaly)
		{
		  test = 1;
		}
	      else if (pts[le].x[1] == pvaly)
		{
		  if (pts[le].x[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (pts[le].x[0] == pvalx)
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
	      if (pts[ri].x[1] > pvaly)
		{
		  test = 1;
		}
	      else if (pts[ri].x[1] == pvaly)
		{
		  if (pts[ri].x[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (pts[ri].x[0] == pvalx)
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
      if (pts[s].x[1] > pts[e].x[1] || (pts[s].x[1] == pts[e].x[1] && pts[s].x[0] > pts[e].x[0])
	  || (pts[s].x[1] == pts[e].x[1] && pts[s].x[0] == pts[e].x[0]
	      && pData[s].oldInd > pData[e].oldInd))
	SwapPoints (s, e);
      return;
    }

  int ppos = (s + e) / 2;
  int plast = ppos;
  double pvalx = pts[ppos].x[0];
  double pvaly = pts[ppos].x[1];
  int pvali = pData[ppos].oldInd;

  int le = s, ri = e;
  while (le < ppos || ri > plast)
    {
      if (le < ppos)
	{
	  do
	    {
	      int test = 0;
	      if (pts[le].x[1] > pvaly)
		{
		  test = 1;
		}
	      else if (pts[le].x[1] == pvaly)
		{
		  if (pts[le].x[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (pts[le].x[0] == pvalx)
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
	      if (pts[ri].x[1] > pvaly)
		{
		  test = 1;
		}
	      else if (pts[ri].x[1] == pvaly)
		{
		  if (pts[ri].x[0] > pvalx)
		    {
		      test = 1;
		    }
		  else if (pts[ri].x[0] == pvalx)
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
      aretes = (dg_arete *) realloc (aretes, maxAr * sizeof (dg_arete));
      if (HasEdgesData ())
	eData = (edge_data *) realloc (eData, maxAr * sizeof (edge_data));
      if (HasSweepSrcData ())
	swsData =
	  (sweep_src_data *) realloc (swsData,
				      maxAr * sizeof (sweep_src_data));
      if (HasSweepDestData ())
	swdData =
	  (sweep_dest_data *) realloc (swdData,
				       maxAr * sizeof (sweep_dest_data));
      if (HasRasterData ())
	swrData =
	  (raster_data *) realloc (swrData, maxAr * sizeof (raster_data));
      if (HasBackData ())
	ebData = (back_data *) realloc (ebData, maxAr * sizeof (back_data));
      if (HasVoronoiData ())
	voreData =
	  (voronoi_edge *) realloc (voreData, maxAr * sizeof (voronoi_edge));
    }
  int n = nbAr++;
  aretes[n].st = aretes[n].en = -1;
  aretes[n].prevS = aretes[n].nextS = -1;
  aretes[n].prevE = aretes[n].nextE = -1;
  if (st >= 0 && en >= 0)
    {
      aretes[n].dx = pts[en].x - pts[st].x;
    }
  else
    {
      aretes[n].dx[0] = aretes[n].dx[1] = 0;
    }
  ConnectStart (st, n);
  ConnectEnd (en, n);
  if (HasEdgesData ())
    {
      eData[n].weight = 1;
      eData[n].rdx = aretes[n].dx;
    }
  if (HasSweepSrcData ())
    {
      swsData[n].misc = NULL;
      swsData[n].firstLinkedPoint = -1;
    }
  if (HasSweepDestData ())
    {
    }
  if (HasBackData ())
    {
      ebData[n].pathID = -1;
      ebData[n].pieceID = -1;
      ebData[n].tSt = ebData[n].tEn = 0;
    }
  if (HasVoronoiData ())
    {
      voreData[n].leF = -1;
      voreData[n].riF = -1;
    }
  SetFlag (need_edges_sorting, true);
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
    int cb = pts[st].firstA;
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
      aretes = (dg_arete *) realloc (aretes, maxAr * sizeof (dg_arete));
      if (HasEdgesData ())
	eData = (edge_data *) realloc (eData, maxAr * sizeof (edge_data));
      if (HasSweepSrcData ())
	swsData =
	  (sweep_src_data *) realloc (swsData,
				      maxAr * sizeof (sweep_src_data));
      if (HasSweepDestData ())
	swdData =
	  (sweep_dest_data *) realloc (swdData,
				       maxAr * sizeof (sweep_dest_data));
      if (HasRasterData ())
	swrData =
	  (raster_data *) realloc (swrData, maxAr * sizeof (raster_data));
      if (HasBackData ())
	ebData = (back_data *) realloc (ebData, maxAr * sizeof (back_data));
      if (HasVoronoiData ())
	voreData =
	  (voronoi_edge *) realloc (voreData, maxAr * sizeof (voronoi_edge));
    }
  int n = nbAr++;
  aretes[n].st = aretes[n].en = -1;
  aretes[n].prevS = aretes[n].nextS = -1;
  aretes[n].prevE = aretes[n].nextE = -1;
  if (st >= 0 && en >= 0)
    {
      aretes[n].dx = pts[en].x - pts[st].x;
    }
  else
    {
      aretes[n].dx[0] = aretes[n].dx[1] = 0;
    }
  ConnectStart (st, n);
  ConnectEnd (en, n);
  if (HasEdgesData ())
    {
      eData[n].weight = 1;
      eData[n].rdx = aretes[n].dx;
    }
  if (HasSweepSrcData ())
    {
      swsData[n].misc = NULL;
      swsData[n].firstLinkedPoint = -1;
    }
  if (HasSweepDestData ())
    {
    }
  if (HasBackData ())
    {
      ebData[n].pathID = -1;
      ebData[n].pieceID = -1;
      ebData[n].tSt = ebData[n].tEn = 0;
    }
  if (HasVoronoiData ())
    {
      voreData[n].leF = leF;
      voreData[n].riF = riF;
    }
  SetFlag (need_edges_sorting, true);
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
  SetFlag (need_edges_sorting, true);
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
      if (pts[aretes[a].st].firstA == a)
	pts[aretes[a].st].firstA = nbAr;
      if (pts[aretes[a].st].lastA == a)
	pts[aretes[a].st].lastA = nbAr;
    }
  if (aretes[a].en >= 0)
    {
      if (pts[aretes[a].en].firstA == a)
	pts[aretes[a].en].firstA = nbAr;
      if (pts[aretes[a].en].lastA == a)
	pts[aretes[a].en].lastA = nbAr;
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
      if (pts[aretes[b].st].firstA == b)
	pts[aretes[b].st].firstA = a;
      if (pts[aretes[b].st].lastA == b)
	pts[aretes[b].st].lastA = a;
    }
  if (aretes[b].en >= 0)
    {
      if (pts[aretes[b].en].firstA == b)
	pts[aretes[b].en].firstA = a;
      if (pts[aretes[b].en].lastA == b)
	pts[aretes[b].en].lastA = a;
    }

  if (aretes[a].st >= 0)
    {
      if (pts[aretes[a].st].firstA == nbAr)
	pts[aretes[a].st].firstA = b;
      if (pts[aretes[a].st].lastA == nbAr)
	pts[aretes[a].st].lastA = b;
    }
  if (aretes[a].en >= 0)
    {
      if (pts[aretes[a].en].firstA == nbAr)
	pts[aretes[a].en].firstA = b;
      if (pts[aretes[a].en].lastA == nbAr)
	pts[aretes[a].en].lastA = b;
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
  if (HasEdgesData ())
    {
      edge_data swae = eData[a];
      eData[a] = eData[b];
      eData[b] = swae;
    }
  if (HasSweepSrcData ())
    {
      sweep_src_data swae = swsData[a];
      swsData[a] = swsData[b];
      swsData[b] = swae;
    }
  if (HasSweepDestData ())
    {
      sweep_dest_data swae = swdData[a];
      swdData[a] = swdData[b];
      swdData[b] = swae;
    }
  if (HasRasterData ())
    {
      raster_data swae = swrData[a];
      swrData[a] = swrData[b];
      swrData[b] = swae;
    }
  if (HasBackData ())
    {
      back_data swae = ebData[a];
      ebData[a] = ebData[b];
      ebData[b] = swae;
    }
  if (HasVoronoiData ())
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
  if (GetFlag (need_edges_sorting))
    {
    }
  else
    {
      return;
    }
  SetFlag (need_edges_sorting, false);

  edge_list *list = (edge_list *) malloc (nbAr * sizeof (edge_list));
  for (int p = 0; p < nbPt; p++)
    {
      int d = pts[p].dI + pts[p].dO;
      if (d > 1)
	{
	  int cb;
	  cb = pts[p].firstA;
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
	  pts[p].firstA = list[0].no;
	  pts[p].lastA = list[nb - 1].no;
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
  free (list);
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
  pts[p].dO++;
  aretes[b].nextS = -1;
  aretes[b].prevS = pts[p].lastA;
  if (pts[p].lastA >= 0)
    {
      if (aretes[pts[p].lastA].st == p)
	{
	  aretes[pts[p].lastA].nextS = b;
	}
      else if (aretes[pts[p].lastA].en == p)
	{
	  aretes[pts[p].lastA].nextE = b;
	}
    }
  pts[p].lastA = b;
  if (pts[p].firstA < 0)
    pts[p].firstA = b;
}

void
Shape::ConnectEnd (int p, int b)
{
  if (aretes[b].en >= 0)
    DisconnectEnd (b);
  aretes[b].en = p;
  pts[p].dI++;
  aretes[b].nextE = -1;
  aretes[b].prevE = pts[p].lastA;
  if (pts[p].lastA >= 0)
    {
      if (aretes[pts[p].lastA].st == p)
	{
	  aretes[pts[p].lastA].nextS = b;
	}
      else if (aretes[pts[p].lastA].en == p)
	{
	  aretes[pts[p].lastA].nextE = b;
	}
    }
  pts[p].lastA = b;
  if (pts[p].firstA < 0)
    pts[p].firstA = b;
}

void
Shape::DisconnectStart (int b)
{
  if (aretes[b].st < 0)
    return;
  pts[aretes[b].st].dO--;
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
  if (pts[aretes[b].st].firstA == b)
    pts[aretes[b].st].firstA = aretes[b].nextS;
  if (pts[aretes[b].st].lastA == b)
    pts[aretes[b].st].lastA = aretes[b].prevS;
  aretes[b].st = -1;
}

void
Shape::DisconnectEnd (int b)
{
  if (aretes[b].en < 0)
    return;
  pts[aretes[b].en].dI--;
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
  if (pts[aretes[b].en].firstA == b)
    pts[aretes[b].en].firstA = aretes[b].nextE;
  if (pts[aretes[b].en].lastA == b)
    pts[aretes[b].en].lastA = aretes[b].prevE;
  aretes[b].en = -1;
}

bool
Shape::Eulerian (bool directed)
{
  if (directed)
    {
      for (int i = 0; i < nbPt; i++)
	{
	  if (pts[i].dI != pts[i].dO)
	    {
	      return false;
	    }
	}
    }
  else
    {
      for (int i = 0; i < nbPt; i++)
	{
	  int d = pts[i].dI + pts[i].dO;
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
      pts[aretes[b].st].dO++;
      pts[aretes[b].st].dI--;
    }
  if (aretes[b].en >= 0)
    {
      pts[aretes[b].en].dO--;
      pts[aretes[b].en].dI++;
    }
  if (HasEdgesData ())
    eData[b].weight = -eData[b].weight;
  if (HasSweepDestData ())
    {
      int swap = swdData[b].leW;
      swdData[b].leW = swdData[b].riW;
      swdData[b].riW = swap;
    }
  if (HasBackData ())
    {
      double swat = ebData[b].tSt;
      ebData[b].tSt = ebData[b].tEn;
      ebData[b].tEn = swat;
    }
  if (HasVoronoiData ())
    {
      int swai = voreData[b].leF;
      voreData[b].leF = voreData[b].riF;
      voreData[b].riF = swai;
    }
}
void
Shape::CalcBBox (void)
{
  if (nbPt <= 0)
    {
      leftX = rightX = topY = bottomY = 0;
      return;
    }
  leftX = rightX = pts[0].x[0];
  topY = bottomY = pts[0].x[1];
  for (int i = 1; i < nbPt; i++)
    {
      if (pts[i].x[0] < leftX) leftX = pts[i].x[0];
      if (pts[i].x[0] > rightX) rightX = pts[i].x[0];
      if (pts[i].x[1] < topY) topY = pts[i].x[1];
      if (pts[i].x[1] > bottomY) bottomY = pts[i].x[1];
    }
}

bool
Shape::SetFlag (int nFlag, bool nval)
{
  if (nval)
    {
      if (flags & nFlag)
	{
	  return false;
	}
      else
	{
	  flags |= nFlag;
	  return true;
	}
    }
  else
    {
      if (flags & nFlag)
	{
	  flags &= ~(nFlag);
	  return true;
	}
      else
	{
	  return false;
	}
    }
  return false;
}

bool
Shape::GetFlag (int nFlag)
{
  return (flags & nFlag);
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
      NR::Point const offset( thePt - pts[i].x );
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
      NR::Point const st(pts[aretes[i].st].x);
      NR::Point const en(pts[aretes[i].en].x);
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

    NR::Point const ast = pts[aretes[i].st].x;
    NR::Point const aen = pts[aretes[i].en].x;
    
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
