/*
 *  Path.cpp
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *
 */

#include "Path.h"
//#include "Shape.h"


Path::Path (void)
{
  descr_flags = 0;
  descr_max = descr_nb = 0;
  descr_data = NULL;
  pending_bezier = -1;
  pending_moveto = -1;

  weighted = false;
  back = false;
  nbPt = maxPt = sizePt = 0;
  pts = NULL;
}
Path::~Path (void)
{
  if (descr_data)
    {
      free (descr_data);
      descr_data = NULL;
    }
  if (pts)
    {
      free (pts);
      pts = NULL;
    }
  descr_max = descr_nb = 0;
  descr_data = NULL;
  nbPt = maxPt = sizePt = 0;
  pts = NULL;
}

/*
 * description
 */
void
Path::Affiche (void)
{
  printf ("path descr %i elems\n", descr_nb);
  for (int i = 0; i < descr_nb; i++)
    {
      printf ("  ");
      printf ("[ %i %f %f ] ", (descr_data + i)->associated,
	      (descr_data + i)->tSt, (descr_data + i)->tEn);
      if ((descr_data + i)->flags & descr_weighted)
	printf (" w ");
      int ty = (descr_data + i)->flags & descr_type_mask;
      if (ty == descr_forced)
	{
	  printf ("F\n");
	}
      else if (ty == descr_moveto)
	{
	  printf ("M %f %f %i\n", (descr_data + i)->d.m.x,
		  (descr_data + i)->d.m.y, (descr_data + i)->d.m.pathLength);
	}
      else if (ty == descr_lineto)
	{
	  printf ("L %f %f\n", (descr_data + i)->d.l.x,
		  (descr_data + i)->d.l.y);
	}
      else if (ty == descr_arcto)
	{
	  printf ("A %f %f %f %f %i %i\n", (descr_data + i)->d.a.x,
		  (descr_data + i)->d.a.y, (descr_data + i)->d.a.rx,
		  (descr_data + i)->d.a.ry,
		  ((descr_data + i)->d.a.large) ? 1 : 0,
		  ((descr_data + i)->d.a.clockwise) ? 1 : 0);
	}
      else if (ty == descr_cubicto)
	{
	  printf ("C %f %f %f %f %f %f\n", (descr_data + i)->d.c.x,
		  (descr_data + i)->d.c.y, (descr_data + i)->d.c.stDx,
		  (descr_data + i)->d.c.stDy, (descr_data + i)->d.c.enDx,
		  (descr_data + i)->d.c.enDy);
	}
      else if (ty == descr_bezierto)
	{
	  printf ("B %f %f %i\n", (descr_data + i)->d.b.x,
		  (descr_data + i)->d.b.y, (descr_data + i)->d.b.nb);
	}
      else if (ty == descr_interm_bezier)
	{
	  printf ("I %f %f\n", (descr_data + i)->d.i.x,
		  (descr_data + i)->d.i.y);
	}
      else if (ty == descr_close)
	{
	  printf ("Z\n");
	}
    }
}
void
Path::Reset (void)
{
  descr_nb = 0;
  pending_bezier = -1;
  pending_moveto = -1;
  descr_flags = 0;
}

void
Path::Copy (Path * who)
{
  ResetPoints (0);
  if (who->descr_nb > descr_max)
    {
      descr_max = who->descr_nb;
      descr_data =
	(path_descr *) realloc (descr_data, descr_max * sizeof (path_descr));
    }
  SetWeighted (who->weighted);
  descr_nb = who->descr_nb;
  memcpy (descr_data, who->descr_data, descr_nb * sizeof (path_descr));
}

void
Path::Alloue (int addSize)
{
  if (descr_nb + addSize > descr_max)
    {
      descr_max = 2 * descr_nb + addSize;
      descr_data =
	(path_descr *) realloc (descr_data, descr_max * sizeof (path_descr));
    }
}
void
Path::CloseSubpath (int add)
{
  for (int i = descr_nb - 1; i >= 0; i--)
    {
      int ty = (descr_data + i)->flags & descr_type_mask;
      if (ty == descr_moveto)
	{
	  (descr_data + i)->d.m.pathLength = descr_nb - i + add;	// il faut compter le close qui n'est pas encore ajouté
	  break;
	}
    }
  descr_flags &= ~(descr_doing_subpath);
  pending_moveto = -1;
}

int
Path::ForcePoint (void)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo ();
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return -1;
    }
  if (descr_nb <= 0)
    return -1;
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_forced;
  return descr_nb - 1;
}

int
Path::Close (void)
{
  if (descr_flags & descr_adding_bezier)
    CancelBezier ();
  if (descr_flags & descr_doing_subpath)
    {
      CloseSubpath (1);
    }
  else
    {
      // rien a fermer => byebye
      return -1;
    }
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_close;
  descr_flags &= ~(descr_doing_subpath);
  pending_moveto = -1;
  return descr_nb - 1;
}

int
Path::MoveTo (float ix, float iy)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy);
  if (descr_flags & descr_doing_subpath)
    CloseSubpath (0);

  pending_moveto = descr_nb;
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_moveto;
  nElem->d.m.x = ix;
  nElem->d.m.y = iy;
  nElem->d.m.pathLength = 0;
  descr_flags |= descr_doing_subpath;
  return descr_nb - 1;
}

int
Path::MoveTo (float ix, float iy, float iw)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy, iw);
  if (descr_flags & descr_doing_subpath)
    CloseSubpath (0);

  pending_moveto = descr_nb;
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_moveto | descr_weighted;
  nElem->d.m.x = ix;
  nElem->d.m.y = iy;
  nElem->d.m.w = iw;
  nElem->d.m.pathLength = 0;
  descr_flags |= descr_doing_subpath;
  return descr_nb - 1;
}

int
Path::LineTo (float ix, float iy)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy);
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy);
    }
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_lineto;
  nElem->d.l.x = ix;
  nElem->d.l.y = iy;
  return descr_nb - 1;
}

int
Path::LineTo (float ix, float iy, float iw)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy, iw);
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy, iw);
    }
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_lineto | descr_weighted;
  nElem->d.l.x = ix;
  nElem->d.l.y = iy;
  nElem->d.l.w = iw;
  return descr_nb - 1;
}

int
Path::CubicTo (float ix, float iy, float isDx, float isDy, float ieDx,
	       float ieDy)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy);
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy);
    }
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_cubicto;
  nElem->d.c.x = ix;
  nElem->d.c.y = iy;
  nElem->d.c.stDx = isDx;
  nElem->d.c.stDy = isDy;
  nElem->d.c.enDx = ieDx;
  nElem->d.c.enDy = ieDy;
  return descr_nb - 1;
}

int
Path::CubicTo (float ix, float iy, float isDx, float isDy, float ieDx,
	       float ieDy, float iw)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy, iw);
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy, iw);
    }
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_cubicto | descr_weighted;
  nElem->d.c.x = ix;
  nElem->d.c.y = iy;
  nElem->d.c.w = iw;
  nElem->d.c.stDx = isDx;
  nElem->d.c.stDy = isDy;
  nElem->d.c.enDx = ieDx;
  nElem->d.c.enDy = ieDy;
  return descr_nb - 1;
}

int
Path::ArcTo (float ix, float iy, float iRx, float iRy, float angle,
	     bool iLargeArc, bool iClockwise)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy);
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy);
    }
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_arcto;
  nElem->d.a.x = ix;
  nElem->d.a.y = iy;
  nElem->d.a.rx = iRx;
  nElem->d.a.ry = iRy;
  nElem->d.a.angle = angle;
  nElem->d.a.large = iLargeArc;
  nElem->d.a.clockwise = iClockwise;
  return descr_nb - 1;
}

int
Path::ArcTo (float ix, float iy, float iRx, float iRy, float angle,
	     bool iLargeArc, bool iClockwise, float iw)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy, iw);
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy, iw);
    }
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_arcto | descr_weighted;
  nElem->d.a.x = ix;
  nElem->d.a.y = iy;
  nElem->d.a.w = iw;
  nElem->d.a.rx = iRx;
  nElem->d.a.ry = iRy;
  nElem->d.a.angle = angle;
  nElem->d.a.large = iLargeArc;
  nElem->d.a.clockwise = iClockwise;
  return descr_nb - 1;
}

int
Path::TempBezierTo (void)
{
  if (descr_flags & descr_adding_bezier)
    CancelBezier ();
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      // pas de pt de départ-> pas bon
      return -1;
    }
  pending_bezier = descr_nb;
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_bezierto;
  nElem->d.b.nb = 0;
  descr_flags |= descr_adding_bezier;
  descr_flags |= descr_delayed_bezier;
  return descr_nb - 1;
}

int
Path::TempBezierToW (void)
{
  if (descr_flags & descr_adding_bezier)
    CancelBezier ();
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      // pas de pt de départ-> pas bon
      return -1;
    }
  pending_bezier = descr_nb;
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_bezierto | descr_weighted;
  nElem->d.b.nb = 0;
  descr_flags |= descr_adding_bezier;
  descr_flags |= descr_delayed_bezier;
  return descr_nb - 1;
}

void
Path::CancelBezier (void)
{
  descr_flags &= ~(descr_adding_bezier);
  descr_flags &= ~(descr_delayed_bezier);
  if (pending_bezier < 0)
    return;
  descr_nb = pending_bezier;
  pending_bezier = -1;
}

int
Path::EndBezierTo (void)
{
  if (descr_flags & descr_delayed_bezier)
    {
      CancelBezier ();
    }
  else
    {
      pending_bezier = -1;
      descr_flags &= ~(descr_adding_bezier);
      descr_flags &= ~(descr_delayed_bezier);
    }
  return -1;
}

int
Path::EndBezierTo (float ix, float iy)
{
  if (descr_flags & descr_adding_bezier)
    {
    }
  else
    {
      return LineTo (ix, iy);
    }
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy);
    }
  if (descr_flags & descr_delayed_bezier)
    {
    }
  else
    {
      return EndBezierTo ();
    }
  (descr_data + pending_bezier)->d.b.x = ix;
  (descr_data + pending_bezier)->d.b.y = iy;
  if ((descr_data + pending_bezier)->flags & descr_weighted)
    (descr_data + pending_bezier)->d.b.w = 1;
  pending_bezier = -1;
  descr_flags &= ~(descr_adding_bezier);
  descr_flags &= ~(descr_delayed_bezier);
  return -1;
}

int
Path::EndBezierTo (float ix, float iy, float iw)
{
  if (descr_flags & descr_adding_bezier)
    {
    }
  else
    {
      return LineTo (ix, iy, iw);
    }
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy, iw);
    }
  if (descr_flags & descr_delayed_bezier)
    {
    }
  else
    {
      return EndBezierTo ();
    }
  (descr_data + pending_bezier)->d.b.x = ix;
  (descr_data + pending_bezier)->d.b.y = iy;
  (descr_data + pending_bezier)->d.b.w = iw;
  pending_bezier = -1;
  descr_flags &= ~(descr_adding_bezier);
  descr_flags &= ~(descr_delayed_bezier);
  return -1;
}

int
Path::IntermBezierTo (float ix, float iy)
{
  if (descr_flags & descr_adding_bezier)
    {
    }
  else
    {
      return LineTo (ix, iy);
    }
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy);
    }

  if ((descr_data + pending_bezier)->flags & descr_weighted)
    {
      return IntermBezierTo (ix, iy, 1);
    }
  else
    {
      Alloue (1);
      path_descr *nElem = descr_data + descr_nb;
      descr_nb++;
      nElem->associated = -1;
      nElem->tSt = 0.0;
      nElem->tEn = 1.0;
      nElem->flags = descr_interm_bezier;
      nElem->d.i.x = ix;
      nElem->d.i.y = iy;
      (descr_data + pending_bezier)->d.b.nb++;
      return descr_nb - 1;
    }
  return -1;
}

int
Path::IntermBezierTo (float ix, float iy, float iw)
{
  if (descr_flags & descr_adding_bezier)
    {
    }
  else
    {
      return LineTo (ix, iy, iw);
    }
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy, iw);
    }

  if ((descr_data + pending_bezier)->flags & descr_weighted)
    {
      Alloue (1);
      path_descr *nElem = descr_data + descr_nb;
      descr_nb++;
      nElem->associated = -1;
      nElem->tSt = 0.0;
      nElem->tEn = 1.0;
      nElem->flags = descr_interm_bezier | descr_weighted;
      nElem->d.i.x = ix;
      nElem->d.i.y = iy;
      nElem->d.i.w = iw;
      (descr_data + pending_bezier)->d.b.nb++;
      return descr_nb - 1;
    }
  else
    {
      return IntermBezierTo (ix, iy);
    }
  return -1;
}

int
Path::BezierTo (float ix, float iy)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy);
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy);
    }
  pending_bezier = descr_nb;
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_bezierto;
  nElem->d.b.nb = 0;
  nElem->d.b.x = ix;
  nElem->d.b.y = iy;
  descr_flags |= descr_adding_bezier;
  descr_flags &= ~(descr_delayed_bezier);
  return descr_nb - 1;
}

int
Path::BezierTo (float ix, float iy, float iw)
{
  if (descr_flags & descr_adding_bezier)
    EndBezierTo (ix, iy, iw);
  if (descr_flags & descr_doing_subpath)
    {
    }
  else
    {
      return MoveTo (ix, iy, iw);
    }
  pending_bezier = descr_nb;
  Alloue (1);
  path_descr *nElem = descr_data + descr_nb;
  descr_nb++;
  nElem->associated = -1;
  nElem->tSt = 0.0;
  nElem->tEn = 1.0;
  nElem->flags = descr_bezierto | descr_weighted;
  nElem->d.b.nb = 0;
  nElem->d.b.x = ix;
  nElem->d.b.y = iy;
  nElem->d.b.w = iw;
  descr_flags |= descr_adding_bezier;
  descr_flags &= ~(descr_delayed_bezier);
  return descr_nb - 1;
}


/*
 * points de la polyligne
 */
void
Path::SetWeighted (bool nVal)
{
  if (back == false)
    {
      if (nVal == true && weighted == false)
	{
	  weighted = true;
	  ResetPoints (nbPt);
	}
      else if (nVal == false && weighted == true)
	{
	  weighted = false;
	  ResetPoints (nbPt);
	}
    }
  else
    {
      if (nVal == true && weighted == false)
	{
	  weighted = true;
	  ResetPoints (nbPt);
	}
      else if (nVal == false && weighted == true)
	{
	  weighted = false;
	  ResetPoints (nbPt);
	}
    }
}
void
Path::SetBackData (bool nVal)
{
  if (back == false)
    {
      if (nVal == true && back == false)
	{
	  back = true;
	  ResetPoints (nbPt);
	}
      else if (nVal == false && back == true)
	{
	  back = false;
	  ResetPoints (nbPt);
	}
    }
  else
    {
      if (nVal == true && back == false)
	{
	  back = true;
	  ResetPoints (nbPt);
	}
      else if (nVal == false && back == true)
	{
	  back = false;
	  ResetPoints (nbPt);
	}
    }
}
void
Path::ResetPoints (int expected)
{
  nbPt = 0;
  if (back)
    {
      if (weighted)
	{
	  sizePt = expected * sizeof (path_lineto_wb);
	}
      else
	{
	  sizePt = expected * sizeof (path_lineto_b);
	}
    }
  else
    {
      if (weighted)
	{
	  sizePt = expected * sizeof (path_lineto_w);
	}
      else
	{
	  sizePt = expected * sizeof (path_lineto);
	}
    }
  if (sizePt > maxPt)
    {
      maxPt = sizePt;
      pts = (char *) realloc (pts, maxPt);
    }
}
int
Path::AddPoint (float ix, float iy, bool mvto)
{
  if (back)
    {
      return AddPoint (ix, iy, -1, 0.0, mvto);
    }
  else
    {
      if (weighted)
	{
	  return AddPoint (ix, iy, 1.0, mvto);
	}
    }
  int nextSize = sizePt + sizeof (path_lineto);
  if (nextSize > maxPt)
    {
      maxPt = 2 * sizePt + sizeof (path_lineto);
      pts = (char *) realloc (pts, maxPt);
    }
  if (mvto == false && nbPt > 0 && ((path_lineto *) pts)[nbPt - 1].x == ix
      && ((path_lineto *) pts)[nbPt - 1].y == iy)
    return -1;
  int n = nbPt++;
  sizePt = nextSize;
  if (mvto)
    ((path_lineto *) pts)[n].isMoveTo = polyline_moveto;
  else
    ((path_lineto *) pts)[n].isMoveTo = polyline_lineto;
  ((path_lineto *) pts)[n].x = ix;
  ((path_lineto *) pts)[n].y = iy;
  return n;
}

int
Path::AddPoint (float ix, float iy, float iw, bool mvto)
{
  if (back)
    {
      return AddPoint (ix, iy, iw, -1, 0.0, mvto);
    }
  else
    {
      if (weighted)
	{
	}
      else
	{
	  return AddPoint (ix, iy, mvto);
	}
    }
  int nextSize = sizePt + sizeof (path_lineto_w);
  if (nextSize > maxPt)
    {
      maxPt = 2 * sizePt + sizeof (path_lineto_w);
      pts = (char *) realloc (pts, maxPt);
    }
  if (mvto == false && nbPt > 0 && ((path_lineto_w *) pts)[nbPt - 1].x == ix
      && ((path_lineto_w *) pts)[nbPt - 1].y == iy)
    return -1;
  int n = nbPt++;
  sizePt = nextSize;
  if (mvto)
    ((path_lineto_w *) pts)[n].isMoveTo = polyline_moveto;
  else
    ((path_lineto_w *) pts)[n].isMoveTo = polyline_lineto;
  ((path_lineto_w *) pts)[n].x = ix;
  ((path_lineto_w *) pts)[n].y = iy;
  ((path_lineto_w *) pts)[n].w = iw;
  return n;
}

int
Path::AddPoint (float ix, float iy, int ip, float it, bool mvto)
{
  if (back)
    {
      if (weighted)
	{
	  return AddPoint (ix, iy, 1.0, ip, it, mvto);
	}
    }
  else
    {
      return AddPoint (ix, iy, mvto);
    }
  int nextSize = sizePt + sizeof (path_lineto_b);
  if (nextSize > maxPt)
    {
      maxPt = 2 * sizePt + sizeof (path_lineto_b);
      pts = (char *) realloc (pts, maxPt);
    }
  if (mvto == false && nbPt > 0 && ((path_lineto_b *) pts)[nbPt - 1].x == ix
      && ((path_lineto_b *) pts)[nbPt - 1].y == iy)
    return -1;
  int n = nbPt++;
  sizePt = nextSize;
  if (mvto)
    ((path_lineto_b *) pts)[n].isMoveTo = polyline_moveto;
  else
    ((path_lineto_b *) pts)[n].isMoveTo = polyline_lineto;
  ((path_lineto_b *) pts)[n].x = ix;
  ((path_lineto_b *) pts)[n].y = iy;
  ((path_lineto_b *) pts)[n].piece = ip;
  ((path_lineto_b *) pts)[n].t = it;
  return n;
}

int
Path::AddPoint (float ix, float iy, float iw, int ip, float it, bool mvto)
{
  if (back)
    {
      if (weighted)
	{
	}
      else
	{
	  return AddPoint (ix, iy, ip, it, mvto);
	}
    }
  else
    {
      return AddPoint (ix, iy, iw, mvto);
    }
  int nextSize = sizePt + sizeof (path_lineto_wb);
  if (nextSize > maxPt)
    {
      maxPt = 2 * sizePt + sizeof (path_lineto_wb);
      pts = (char *) realloc (pts, maxPt);
    }
  if (mvto == false && nbPt > 0 && ((path_lineto_wb *) pts)[nbPt - 1].x == ix
      && ((path_lineto_wb *) pts)[nbPt - 1].y == iy)
    return -1;
  int n = nbPt++;
  sizePt = nextSize;
  if (mvto)
    ((path_lineto_wb *) pts)[n].isMoveTo = polyline_moveto;
  else
    ((path_lineto_wb *) pts)[n].isMoveTo = polyline_lineto;
  ((path_lineto_wb *) pts)[n].x = ix;
  ((path_lineto_wb *) pts)[n].y = iy;
  ((path_lineto_wb *) pts)[n].w = iw;
  ((path_lineto_wb *) pts)[n].piece = ip;
  ((path_lineto_wb *) pts)[n].t = it;
  return n;
}

int
Path::AddForcedPoint (float ix, float iy)
{
  if (back)
    {
      return AddForcedPoint (ix, iy, -1, 0.0);
    }
  else
    {
      if (weighted)
	{
	  return AddForcedPoint (ix, iy, 1.0);
	}
    }
  int nextSize = sizePt + sizeof (path_lineto);
  if (nextSize > maxPt)
    {
      maxPt = 2 * sizePt + sizeof (path_lineto);
      pts = (char *) realloc (pts, maxPt);
    }
  if (nbPt <= 0
      || ((path_lineto *) pts)[nbPt - 1].isMoveTo != polyline_lineto)
    return -1;
  int n = nbPt++;
  sizePt = nextSize;
  ((path_lineto *) pts)[n].isMoveTo = polyline_forced;
  ((path_lineto *) pts)[n].x = ((path_lineto *) pts)[n - 1].x;
  ((path_lineto *) pts)[n].y = ((path_lineto *) pts)[n - 1].y;
  return n;
}

int
Path::AddForcedPoint (float ix, float iy, float iw)
{
  if (back)
    {
      return AddForcedPoint (ix, iy, iw, -1, 0.0);
    }
  else
    {
      if (weighted)
	{
	}
      else
	{
	  return AddForcedPoint (ix, iy);
	}
    }
  int nextSize = sizePt + sizeof (path_lineto_w);
  if (nextSize > maxPt)
    {
      maxPt = 2 * sizePt + sizeof (path_lineto_w);
      pts = (char *) realloc (pts, maxPt);
    }
  if (nbPt <= 0
      || ((path_lineto_w *) pts)[nbPt - 1].isMoveTo != polyline_lineto)
    return -1;
  int n = nbPt++;
  sizePt = nextSize;
  ((path_lineto_w *) pts)[n].isMoveTo = polyline_forced;
  ((path_lineto_w *) pts)[n].x = ((path_lineto_w *) pts)[n - 1].x;
  ((path_lineto_w *) pts)[n].y = ((path_lineto_w *) pts)[n - 1].y;
  ((path_lineto_w *) pts)[n].w = ((path_lineto_w *) pts)[n - 1].w;
  return n;
}

int
Path::AddForcedPoint (float ix, float iy, int ip, float it)
{
  if (back)
    {
      if (weighted)
	{
	  return AddForcedPoint (ix, iy, 1.0, ip, it);
	}
    }
  else
    {
      return AddForcedPoint (ix, iy);
    }
  int nextSize = sizePt + sizeof (path_lineto_b);
  if (nextSize > maxPt)
    {
      maxPt = 2 * sizePt + sizeof (path_lineto_b);
      pts = (char *) realloc (pts, maxPt);
    }
  if (nbPt <= 0
      || ((path_lineto_b *) pts)[nbPt - 1].isMoveTo != polyline_lineto)
    return -1;
  int n = nbPt++;
  sizePt = nextSize;
  ((path_lineto_b *) pts)[n].isMoveTo = polyline_forced;
  ((path_lineto_b *) pts)[n].x = ((path_lineto_b *) pts)[n - 1].x;
  ((path_lineto_b *) pts)[n].y = ((path_lineto_b *) pts)[n - 1].y;
  ((path_lineto_b *) pts)[n].piece = ((path_lineto_b *) pts)[n - 1].piece;
  ((path_lineto_b *) pts)[n].t = ((path_lineto_b *) pts)[n - 1].t;
  return n;
}

int
Path::AddForcedPoint (float ix, float iy, float iw, int ip, float it)
{
  if (back)
    {
      if (weighted)
	{
	}
      else
	{
	  return AddForcedPoint (ix, iy, ip, it);
	}
    }
  else
    {
      return AddForcedPoint (ix, iy, iw);
    }
  int nextSize = sizePt + sizeof (path_lineto_wb);
  if (nextSize > maxPt)
    {
      maxPt = 2 * sizePt + sizeof (path_lineto_wb);
      pts = (char *) realloc (pts, maxPt);
    }
  if (nbPt <= 0
      || ((path_lineto *) pts)[nbPt - 1].isMoveTo != polyline_lineto)
    return -1;
  int n = nbPt++;
  sizePt = nextSize;
  ((path_lineto_wb *) pts)[n].isMoveTo = polyline_forced;
  ((path_lineto_wb *) pts)[n].x = ((path_lineto_wb *) pts)[n - 1].x;
  ((path_lineto_wb *) pts)[n].y = ((path_lineto_wb *) pts)[n - 1].y;
  ((path_lineto_wb *) pts)[n].w = ((path_lineto_wb *) pts)[n - 1].w;
  ((path_lineto_wb *) pts)[n].piece = ((path_lineto_wb *) pts)[n - 1].piece;
  ((path_lineto_wb *) pts)[n].t = ((path_lineto_wb *) pts)[n - 1].t;
  return n;
}

int
Path::Winding (void)
{
  if (nbPt <= 1)
    return 0;
  float sum = 0;
  if (weighted)
    {
      for (int i = 0; i < nbPt - 1; i++)
	{
	  sum +=
	    (((path_lineto_w *) pts)[i].x +
	     ((path_lineto_w *) pts)[i + 1].x) * (((path_lineto_w *) pts)[i +
									  1].
						  y -
						  ((path_lineto_w *) pts)[i].
						  y);
	}
      sum +=
	(((path_lineto_w *) pts)[nbPt - 1].x +
	 ((path_lineto_w *) pts)[0].x) * (((path_lineto_w *) pts)[0].y -
					  ((path_lineto_w *) pts)[nbPt -
								  1].y);
    }
  else
    {
      for (int i = 0; i < nbPt - 1; i++)
	{
	  sum +=
	    (((path_lineto *) pts)[i].x +
	     ((path_lineto *) pts)[i + 1].x) * (((path_lineto *) pts)[i +
								      1].y -
						((path_lineto *) pts)[i].y);
	}
      sum +=
	(((path_lineto *) pts)[nbPt - 1].x +
	 ((path_lineto *) pts)[0].x) * (((path_lineto *) pts)[0].y -
					((path_lineto *) pts)[nbPt - 1].y);
    }
  return (sum > 0) ? 1 : -1;
}

// utilities
void
Path::PointAt (int piece, float at, vec2 & pos)
{
  if (piece < 0 || piece >= descr_nb)
    {
      pos.x = pos.y = 0;
      return;
    }
  path_descr theD = descr_data[piece];
  int typ = theD.flags & descr_type_mask;
  if (typ == descr_moveto)
    {
      return PointAt (piece + 1, 0.0, pos);
    }
  else if (typ == descr_close || typ == descr_forced)
    {
      return PointAt (piece - 1, 1.0, pos);
    }
  else if (typ == descr_lineto)
    {
      vec2 stP;
      PrevPoint (piece - 1, stP.x, stP.y);
      vec2 tgt;
      float len;
      TangentOnSegAt (at, stP.x, stP.y, theD.d.l, pos, tgt, len);
    }
  else if (typ == descr_arcto)
    {
      vec2 stP;
      PrevPoint (piece - 1, stP.x, stP.y);
      vec2 tgt;
      float len, rad;
      TangentOnArcAt (at, stP.x, stP.y, theD.d.a, pos, tgt, len, rad);
    }
  else if (typ == descr_cubicto)
    {
      vec2 stP;
      PrevPoint (piece - 1, stP.x, stP.y);
      vec2 tgt;
      float len, rad;
      TangentOnCubAt (at, stP.x, stP.y, theD.d.c, false, pos, tgt, len, rad);
    }
  else if (typ == descr_bezierto || typ == descr_interm_bezier)
    {
      int bez_st = piece;
      while (bez_st >= 0)
	{
	  int nt = descr_data[bez_st].flags & descr_type_mask;
	  if (nt == descr_bezierto)
	    break;
	  bez_st--;
	}
      if (bez_st < 0)
	return PointAt (piece - 1, 1.0, pos);

      path_descr_bezierto_w stB = descr_data[bez_st].d.b;
      if (piece > bez_st + stB.nb)
	return PointAt (piece - 1, 1.0, pos);
      int k = piece - bez_st;
      if (stB.nb == 1 || k <= 0)
	{
	  vec2 stP;
	  PrevPoint (bez_st - 1, stP.x, stP.y);
	  vec2 tgt;
	  float len, rad;
	  TangentOnBezAt (at, stP.x, stP.y, descr_data[bez_st + 1].d.i,
			  descr_data[bez_st].d.b, false, pos, tgt, len, rad);
	}
      else
	{
	  // forcement plus grand que 1
	  if (k == 1)
	    {
	      vec2 stP;
	      PrevPoint (bez_st - 1, stP.x, stP.y);
	      vec2 tgt;
	      float len, rad;
	      path_descr_bezierto fin;
	      fin.nb = 1;
	      fin.x =
		(descr_data[bez_st + 1].d.i.x +
		 descr_data[bez_st + 2].d.i.x) / 2;
	      fin.y =
		(descr_data[bez_st + 1].d.i.y +
		 descr_data[bez_st + 2].d.i.y) / 2;
	      TangentOnBezAt (at, stP.x, stP.y, descr_data[bez_st + 1].d.i,
			      fin, false, pos, tgt, len, rad);
	    }
	  else if (k == stB.nb)
	    {
	      vec2 stP;
	      vec2 tgt;
	      float len, rad;
	      stP.x =
		(descr_data[bez_st + k].d.i.x +
		 descr_data[bez_st + k - 1].d.i.x) / 2;
	      stP.y =
		(descr_data[bez_st + k].d.i.y +
		 descr_data[bez_st + k - 1].d.i.y) / 2;
	      TangentOnBezAt (at, stP.x, stP.y, descr_data[bez_st + k].d.i,
			      descr_data[bez_st].d.b, false, pos, tgt, len,
			      rad);
	    }
	  else
	    {
	      vec2 stP;
	      vec2 tgt;
	      float len, rad;
	      stP.x =
		(descr_data[bez_st + k].d.i.x +
		 descr_data[bez_st + k - 1].d.i.x) / 2;
	      stP.y =
		(descr_data[bez_st + k].d.i.y +
		 descr_data[bez_st + k - 1].d.i.y) / 2;
	      path_descr_bezierto fin;
	      fin.nb = 1;
	      fin.x =
		(descr_data[bez_st + k].d.i.x +
		 descr_data[bez_st + k + 1].d.i.x) / 2;
	      fin.y =
		(descr_data[bez_st + k].d.i.y +
		 descr_data[bez_st + k + 1].d.i.y) / 2;
	      TangentOnBezAt (at, stP.x, stP.y, descr_data[bez_st + k].d.i,
			      fin, false, pos, tgt, len, rad);
	    }
	}
    }
}
void
Path::PointAndTangentAt (int piece, float at, vec2 & pos, vec2 & tgt)
{
  if (piece < 0 || piece >= descr_nb)
    {
      pos.x = pos.y = 0;
      tgt.x = tgt.y = 0;
      return;
    }
  path_descr theD = descr_data[piece];
  int typ = theD.flags & descr_type_mask;
  if (typ == descr_moveto)
    {
      return PointAndTangentAt (piece + 1, 0.0, pos, tgt);
    }
  else if (typ == descr_close || typ == descr_forced)
    {
      return PointAndTangentAt (piece - 1, 1.0, pos, tgt);
    }
  else if (typ == descr_lineto)
    {
      vec2 stP;
      PrevPoint (piece - 1, stP.x, stP.y);
      float len;
      TangentOnSegAt (at, stP.x, stP.y, theD.d.l, pos, tgt, len);
    }
  else if (typ == descr_arcto)
    {
      vec2 stP;
      PrevPoint (piece - 1, stP.x, stP.y);
      float len, rad;
      TangentOnArcAt (at, stP.x, stP.y, theD.d.a, pos, tgt, len, rad);
    }
  else if (typ == descr_cubicto)
    {
      vec2 stP;
      PrevPoint (piece - 1, stP.x, stP.y);
      float len, rad;
      TangentOnCubAt (at, stP.x, stP.y, theD.d.c, false, pos, tgt, len, rad);
    }
  else if (typ == descr_bezierto || typ == descr_interm_bezier)
    {
      int bez_st = piece;
      while (bez_st >= 0)
	{
	  int nt = descr_data[bez_st].flags & descr_type_mask;
	  if (nt == descr_bezierto)
	    break;
	  bez_st--;
	}
      if (bez_st < 0)
	return PointAndTangentAt (piece - 1, 1.0, pos, tgt);

      path_descr_bezierto_w stB = descr_data[bez_st].d.b;
      if (piece > bez_st + stB.nb)
	return PointAndTangentAt (piece - 1, 1.0, pos, tgt);
      int k = piece - bez_st;
      if (stB.nb == 1 || k <= 0)
	{
	  vec2 stP;
	  PrevPoint (bez_st - 1, stP.x, stP.y);
	  float len, rad;
	  TangentOnBezAt (at, stP.x, stP.y, descr_data[bez_st + 1].d.i,
			  descr_data[bez_st].d.b, false, pos, tgt, len, rad);
	}
      else
	{
	  // forcement plus grand que 1
	  if (k == 1)
	    {
	      vec2 stP;
	      PrevPoint (bez_st - 1, stP.x, stP.y);
	      float len, rad;
	      path_descr_bezierto fin;
	      fin.nb = 1;
	      fin.x =
		(descr_data[bez_st + 1].d.i.x +
		 descr_data[bez_st + 2].d.i.x) / 2;
	      fin.y =
		(descr_data[bez_st + 1].d.i.y +
		 descr_data[bez_st + 2].d.i.y) / 2;
	      TangentOnBezAt (at, stP.x, stP.y, descr_data[bez_st + 1].d.i,
			      fin, false, pos, tgt, len, rad);
	    }
	  else if (k == stB.nb)
	    {
	      vec2 stP;
	      float len, rad;
	      stP.x =
		(descr_data[bez_st + k].d.i.x +
		 descr_data[bez_st + k - 1].d.i.x) / 2;
	      stP.y =
		(descr_data[bez_st + k].d.i.y +
		 descr_data[bez_st + k - 1].d.i.y) / 2;
	      TangentOnBezAt (at, stP.x, stP.y, descr_data[bez_st + k].d.i,
			      descr_data[bez_st].d.b, false, pos, tgt, len,
			      rad);
	    }
	  else
	    {
	      vec2 stP;
	      float len, rad;
	      stP.x =
		(descr_data[bez_st + k].d.i.x +
		 descr_data[bez_st + k - 1].d.i.x) / 2;
	      stP.y =
		(descr_data[bez_st + k].d.i.y +
		 descr_data[bez_st + k - 1].d.i.y) / 2;
	      path_descr_bezierto fin;
	      fin.nb = 1;
	      fin.x =
		(descr_data[bez_st + k].d.i.x +
		 descr_data[bez_st + k + 1].d.i.x) / 2;
	      fin.y =
		(descr_data[bez_st + k].d.i.y +
		 descr_data[bez_st + k + 1].d.i.y) / 2;
	      TangentOnBezAt (at, stP.x, stP.y, descr_data[bez_st + k].d.i,
			      fin, false, pos, tgt, len, rad);
	    }
	}
    }
}
