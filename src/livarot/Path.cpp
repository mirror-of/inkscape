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
			printf ("M %f %f %i\n", (descr_data + i)->d.m.p.pt[0],
				(descr_data + i)->d.m.p.pt[1], (descr_data + i)->d.m.pathLength);
		}
		else if (ty == descr_lineto)
		{
			printf ("L %f %f\n", (descr_data + i)->d.l.p.pt[0],
				(descr_data + i)->d.l.p.pt[1]);
		}
		else if (ty == descr_arcto)
		{
			printf ("A %f %f %f %f %i %i\n", (descr_data + i)->d.a.p.pt[0],
				(descr_data + i)->d.a.p.pt[1], (descr_data + i)->d.a.rx,
				(descr_data + i)->d.a.ry,
				((descr_data + i)->d.a.large) ? 1 : 0,
				((descr_data + i)->d.a.clockwise) ? 1 : 0);
		}
		else if (ty == descr_cubicto)
		{
			printf ("C %f %f %f %f %f %f\n", (descr_data + i)->d.c.p.pt[0],
				(descr_data + i)->d.c.p.pt[1], (descr_data + i)->d.c.stD.pt[0],
				(descr_data + i)->d.c.stD.pt[1], (descr_data + i)->d.c.enD.pt[0],
				(descr_data + i)->d.c.enD.pt[1]);
		}
		else if (ty == descr_bezierto)
		{
			printf ("B %f %f %i\n", (descr_data + i)->d.b.p.pt[0],
				(descr_data + i)->d.b.p.pt[1], (descr_data + i)->d.b.nb);
		}
		else if (ty == descr_interm_bezier)
		{
			printf ("I %f %f\n", (descr_data + i)->d.i.p.pt[0],
				(descr_data + i)->d.i.p.pt[1]);
		}
		else if (ty == descr_close)
		{
			printf ("Z\n");
		}
	}
	printf("nbPt: %i\n",nbPt);
	if ( back ) {
		if ( weighted ) {
			path_lineto_wb  *tp=(path_lineto_wb*)pts;
			for (int i=0;i<nbPt;i++) {
				printf("[ %f %f  %f %i %f ] ",tp[i].p.pt[0],tp[i].p.pt[1],tp[i].w,tp[i].piece,tp[i].t);
			}
		} else {
			path_lineto_b  *tp=(path_lineto_b*)pts;
			for (int i=0;i<nbPt;i++) {
				printf("[ %f %f %i %f ] ",tp[i].p.pt[0],tp[i].p.pt[1],tp[i].piece,tp[i].t);
			}
		}
	} else {
		if ( weighted ) {
			path_lineto_w  *tp=(path_lineto_w*)pts;
			for (int i=0;i<nbPt;i++) {
				printf("[ %f %f  %f ] ",tp[i].p.pt[0],tp[i].p.pt[1],tp[i].w);
			}
		} else {
			path_lineto  *tp=(path_lineto*)pts;
			for (int i=0;i<nbPt;i++) {
				printf("[ %f %f ] ",tp[i].p.pt[0],tp[i].p.pt[1]);
			}
		}
	}
	printf("\n\n");
	fflush(stdout);
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
Path::MoveTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt);
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
	nElem->d.m.p = iPt;
	nElem->d.m.pathLength = 0;
	descr_flags |= descr_doing_subpath;
	return descr_nb - 1;
}

int
Path::MoveTo (NR::Point const &iPt, double iw)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt, iw);
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
	nElem->d.m.p = iPt;
	nElem->d.m.w = iw;
	nElem->d.m.pathLength = 0;
	descr_flags |= descr_doing_subpath;
	return descr_nb - 1;
}

int
Path::LineTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt);
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt);
	}
	Alloue (1);
	path_descr *nElem = descr_data + descr_nb;
	descr_nb++;
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_lineto;
	nElem->d.l.p = iPt;
	return descr_nb - 1;
}

int
Path::LineTo (NR::Point const &iPt, double iw)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt, iw);
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt, iw);
	}
	Alloue (1);
	path_descr *nElem = descr_data + descr_nb;
	descr_nb++;
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_lineto | descr_weighted;
	nElem->d.l.p = iPt;
	nElem->d.l.w = iw;
	return descr_nb - 1;
}

int
Path::CubicTo (NR::Point const &iPt, NR::Point const &iStD, NR::Point const &iEnD)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt);
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt);
	}
	Alloue (1);
	path_descr *nElem = descr_data + descr_nb;
	descr_nb++;
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_cubicto;
	nElem->d.c.p = iPt;
	nElem->d.c.stD = iStD;
	nElem->d.c.enD = iEnD;
	return descr_nb - 1;
}

int
Path::CubicTo (NR::Point const &iPt, NR::Point const &iStD, NR::Point const &iEnD, double iw)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt, iw);
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt, iw);
	}
	Alloue (1);
	path_descr *nElem = descr_data + descr_nb;
	descr_nb++;
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_cubicto | descr_weighted;
	nElem->d.c.p = iPt;
	nElem->d.c.w = iw;
	nElem->d.c.stD = iStD;
	nElem->d.c.enD = iEnD;
	return descr_nb - 1;
}

int
Path::ArcTo (NR::Point const &iPt, double iRx, double iRy, double angle,
             bool iLargeArc, bool iClockwise)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt);
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt);
	}
	Alloue (1);
	path_descr *nElem = descr_data + descr_nb;
	descr_nb++;
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_arcto;
	nElem->d.a.p = iPt;
	nElem->d.a.rx = iRx;
	nElem->d.a.ry = iRy;
	nElem->d.a.angle = angle;
	nElem->d.a.large = iLargeArc;
	nElem->d.a.clockwise = iClockwise;
	return descr_nb - 1;
}

int
Path::ArcTo (NR::Point const &iPt, double iRx, double iRy, double angle,
             bool iLargeArc, bool iClockwise, double iw)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt, iw);
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt, iw);
	}
	Alloue (1);
	path_descr *nElem = descr_data + descr_nb;
	descr_nb++;
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_arcto | descr_weighted;
	nElem->d.a.p = iPt;
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
Path::EndBezierTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier)
	{
	}
	else
	{
		return LineTo (iPt);
	}
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt);
	}
	if (descr_flags & descr_delayed_bezier)
	{
	}
	else
	{
		return EndBezierTo ();
	}
	(descr_data + pending_bezier)->d.b.p = iPt;
	if ((descr_data + pending_bezier)->flags & descr_weighted)
		(descr_data + pending_bezier)->d.b.w = 1;
	pending_bezier = -1;
	descr_flags &= ~(descr_adding_bezier);
	descr_flags &= ~(descr_delayed_bezier);
	return -1;
}

int
Path::EndBezierTo (NR::Point const &iPt, double iw)
{
	if (descr_flags & descr_adding_bezier)
	{
	}
	else
	{
		return LineTo (iPt, iw);
	}
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt, iw);
	}
	if (descr_flags & descr_delayed_bezier)
	{
	}
	else
	{
		return EndBezierTo ();
	}
	(descr_data + pending_bezier)->d.b.p = iPt;
	(descr_data + pending_bezier)->d.b.w = iw;
	pending_bezier = -1;
	descr_flags &= ~(descr_adding_bezier);
	descr_flags &= ~(descr_delayed_bezier);
	return -1;
}

int
Path::IntermBezierTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier)
	{
	}
	else
	{
		return LineTo (iPt);
	}
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt);
	}
  
	if ((descr_data + pending_bezier)->flags & descr_weighted)
	{
		return IntermBezierTo (iPt, 1);
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
		nElem->d.i.p = iPt;
		(descr_data + pending_bezier)->d.b.nb++;
		return descr_nb - 1;
	}
	return -1;
}

int
Path::IntermBezierTo (NR::Point const &iPt, double iw)
{
	if (descr_flags & descr_adding_bezier)
	{
	}
	else
	{
		return LineTo (iPt, iw);
	}
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt, iw);
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
		nElem->d.i.p = iPt;
		nElem->d.i.w = iw;
		(descr_data + pending_bezier)->d.b.nb++;
		return descr_nb - 1;
	}
	else
	{
		return IntermBezierTo (iPt);
	}
	return -1;
}

int
Path::BezierTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt);
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt);
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
	nElem->d.b.p = iPt;
	descr_flags |= descr_adding_bezier;
	descr_flags &= ~(descr_delayed_bezier);
	return descr_nb - 1;
}

int
Path::BezierTo (NR::Point const &iPt, double iw)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo (iPt, iw);
	if (descr_flags & descr_doing_subpath)
	{
	}
	else
	{
		return MoveTo (iPt, iw);
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
	nElem->d.b.p = iPt;
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
Path::AddPoint (NR::Point const &iPt, bool mvto)
{
	if (back)
	{
		return AddPoint (iPt, -1, 0.0, mvto);
	}
	else
	{
		if (weighted)
		{
			return AddPoint (iPt, 1.0, mvto);
		}
	}
	int nextSize = sizePt + sizeof (path_lineto);
	if (nextSize > maxPt)
	{
		maxPt = 2 * sizePt + sizeof (path_lineto);
		pts = (char *) realloc (pts, maxPt);
	}
	if (mvto == false && nbPt > 0 && ((path_lineto *) pts)[nbPt - 1].p == iPt)
		return -1;
	int n = nbPt++;
	sizePt = nextSize;
	if (mvto)
		((path_lineto *) pts)[n].isMoveTo = polyline_moveto;
	else
		((path_lineto *) pts)[n].isMoveTo = polyline_lineto;
	((path_lineto *) pts)[n].p = iPt;
	return n;
}

int
Path::AddPoint (NR::Point const &iPt, double iw, bool mvto)
{
	if (back)
	{
		return AddPoint (iPt, iw, -1, 0.0, mvto);
	}
	else
	{
		if (weighted)
		{
		}
		else
		{
			return AddPoint (iPt, mvto);
		}
	}
	int nextSize = sizePt + sizeof (path_lineto_w);
	if (nextSize > maxPt)
	{
		maxPt = 2 * sizePt + sizeof (path_lineto_w);
		pts = (char *) realloc (pts, maxPt);
	}
	if (mvto == false && nbPt > 0 && ((path_lineto_w *) pts)[nbPt - 1].p == iPt)
		return -1;
	int n = nbPt++;
	sizePt = nextSize;
	if (mvto)
		((path_lineto_w *) pts)[n].isMoveTo = polyline_moveto;
	else
		((path_lineto_w *) pts)[n].isMoveTo = polyline_lineto;
	((path_lineto_w *) pts)[n].p = iPt;
	((path_lineto_w *) pts)[n].w = iw;
	return n;
}

int
Path::AddPoint (NR::Point const &iPt, int ip, double it, bool mvto)
{
	if (back)
	{
		if (weighted)
		{
			return AddPoint (iPt, 1.0, ip, it, mvto);
		}
	}
	else
	{
		return AddPoint (iPt, mvto);
	}
	int nextSize = sizePt + sizeof (path_lineto_b);
	if (nextSize > maxPt)
	{
		maxPt = 2 * sizePt + sizeof (path_lineto_b);
		pts = (char *) realloc (pts, maxPt);
	}
	if (mvto == false && nbPt > 0 && ((path_lineto_b *) pts)[nbPt - 1].p == iPt)
		return -1;
	int n = nbPt++;
	sizePt = nextSize;
	if (mvto)
		((path_lineto_b *) pts)[n].isMoveTo = polyline_moveto;
	else
		((path_lineto_b *) pts)[n].isMoveTo = polyline_lineto;
	((path_lineto_b *) pts)[n].p = iPt;
	((path_lineto_b *) pts)[n].piece = ip;
	((path_lineto_b *) pts)[n].t = it;
	return n;
}

int
Path::AddPoint (NR::Point const &iPt, double iw, int ip, double it, bool mvto)
{
	if (back)
	{
		if (weighted)
		{
		}
		else
		{
			return AddPoint (iPt, ip, it, mvto);
		}
	}
	else
	{
		return AddPoint (iPt, iw, mvto);
	}
	int nextSize = sizePt + sizeof (path_lineto_wb);
	if (nextSize > maxPt)
	{
		maxPt = 2 * sizePt + sizeof (path_lineto_wb);
		pts = (char *) realloc (pts, maxPt);
	}
	if (mvto == false && nbPt > 0 && ((path_lineto_wb *) pts)[nbPt - 1].p == iPt )
		return -1;
	int n = nbPt++;
	sizePt = nextSize;
	if (mvto)
		((path_lineto_wb *) pts)[n].isMoveTo = polyline_moveto;
	else
		((path_lineto_wb *) pts)[n].isMoveTo = polyline_lineto;
	((path_lineto_wb *) pts)[n].p = iPt;
	((path_lineto_wb *) pts)[n].w = iw;
	((path_lineto_wb *) pts)[n].piece = ip;
	((path_lineto_wb *) pts)[n].t = it;
	return n;
}

int
Path::AddForcedPoint (NR::Point const &iPt)
{
	if (back)
	{
		return AddForcedPoint (iPt, -1, 0.0);
	}
	else
	{
		if (weighted)
		{
			return AddForcedPoint (iPt, 1.0);
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
	((path_lineto *) pts)[n].p = ((path_lineto *) pts)[n - 1].p;
	return n;
}

int
Path::AddForcedPoint (NR::Point const &iPt, double iw)
{
	if (back)
	{
		return AddForcedPoint (iPt, iw, -1, 0.0);
	}
	else
	{
		if (weighted)
		{
		}
		else
		{
			return AddForcedPoint (iPt);
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
	((path_lineto_w *) pts)[n].p = ((path_lineto_w *) pts)[n - 1].p;
	((path_lineto_w *) pts)[n].w = ((path_lineto_w *) pts)[n - 1].w;
	return n;
}

int
Path::AddForcedPoint (NR::Point const &iPt, int ip, double it)
{
	if (back)
	{
		if (weighted)
		{
			return AddForcedPoint (iPt, 1.0, ip, it);
		}
	}
	else
	{
		return AddForcedPoint (iPt);
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
	((path_lineto_b *) pts)[n].p = ((path_lineto_b *) pts)[n - 1].p;
	((path_lineto_b *) pts)[n].piece = ((path_lineto_b *) pts)[n - 1].piece;
	((path_lineto_b *) pts)[n].t = ((path_lineto_b *) pts)[n - 1].t;
	return n;
}

int
Path::AddForcedPoint (NR::Point const &iPt, double iw, int ip, double it)
{
	if (back)
	{
		if (weighted)
		{
		}
		else
		{
			return AddForcedPoint (iPt, ip, it);
		}
	}
	else
	{
		return AddForcedPoint (iPt, iw);
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
	((path_lineto_wb *) pts)[n].p = ((path_lineto_wb *) pts)[n - 1].p;
	((path_lineto_wb *) pts)[n].w = ((path_lineto_wb *) pts)[n - 1].w;
	((path_lineto_wb *) pts)[n].piece = ((path_lineto_wb *) pts)[n - 1].piece;
	((path_lineto_wb *) pts)[n].t = ((path_lineto_wb *) pts)[n - 1].t;
	return n;
}

int
Path::Winding (void)
{
	// dead code; was used to compute the winding number of a path (a path without self-intersections)
	return 0;
	/*  if (nbPt <= 1)
	    return 0;
	    double sum = 0;
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
	    return (sum > 0) ? 1 : -1;*/
}

// utilities
void
Path::PointAt (int piece, double at, NR::Point & pos)
{
	if (piece < 0 || piece >= descr_nb)
	{
		// this shouldn't happen: the piece we are asked for doesn't exist in the path
		pos.pt[0] = pos.pt[1] = 0;
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
		NR::Point tgt;
		double len;
		TangentOnSegAt (at, PrevPoint (piece - 1), theD.d.l, pos, tgt, len);
	}
	else if (typ == descr_arcto)
	{
		NR::Point tgt;
		double len, rad;
		TangentOnArcAt (at,PrevPoint (piece - 1), theD.d.a, pos, tgt, len, rad);
	}
	else if (typ == descr_cubicto)
	{
		NR::Point tgt;
		double len, rad;
		TangentOnCubAt (at, PrevPoint (piece - 1), theD.d.c, false, pos, tgt, len, rad);
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
			NR::Point tgt;
			double len, rad;
			TangentOnBezAt (at, PrevPoint (bez_st - 1), descr_data[bez_st + 1].d.i,
					descr_data[bez_st].d.b, false, pos, tgt, len, rad);
		}
		else
		{
			// forcement plus grand que 1
			if (k == 1)
			{
				NR::Point tgt;
				double len, rad;
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5*(descr_data[bez_st + 1].d.i.p +
					     descr_data[bez_st + 2].d.i.p) ;
				TangentOnBezAt (at, PrevPoint (bez_st - 1), descr_data[bez_st + 1].d.i,
						fin, false, pos, tgt, len, rad);
			}
			else if (k == stB.nb)
			{
				NR::Point tgt;
				double len, rad;
				NR::Point stP = 0.5*(descr_data[bez_st + k].d.i.p +
					   descr_data[bez_st + k - 1].d.i.p) ;
				TangentOnBezAt (at, stP, descr_data[bez_st + k].d.i,
						descr_data[bez_st].d.b, false, pos, tgt, len,
						rad);
			}
			else
			{
				NR::Point tgt;
				double len, rad;
				NR::Point stP = 0.5* (descr_data[bez_st + k].d.i.p +
					    descr_data[bez_st + k - 1].d.i.p);
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5* (descr_data[bez_st + k].d.i.p +
					      descr_data[bez_st + k + 1].d.i.p);
				TangentOnBezAt (at, stP, descr_data[bez_st + k].d.i,
						fin, false, pos, tgt, len, rad);
			}
		}
	}
}
void
Path::PointAndTangentAt (int piece, double at, NR::Point & pos, NR::Point & tgt)
{
	if (piece < 0 || piece >= descr_nb)
	{
		pos.pt[0] = pos.pt[1] = 0;
		tgt.pt[0] = tgt.pt[1] = 0;
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
		double len;
		TangentOnSegAt (at, PrevPoint (piece - 1), theD.d.l, pos, tgt, len);
	}
	else if (typ == descr_arcto)
	{
		double len, rad;
		TangentOnArcAt (at,PrevPoint (piece - 1), theD.d.a, pos, tgt, len, rad);
	}
	else if (typ == descr_cubicto)
	{
		double len, rad;
		TangentOnCubAt (at,PrevPoint (piece - 1), theD.d.c, false, pos, tgt, len, rad);
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
			double len, rad;
			TangentOnBezAt (at,PrevPoint (bez_st - 1), descr_data[bez_st + 1].d.i,
					descr_data[bez_st].d.b, false, pos, tgt, len, rad);
		}
		else
		{
			// forcement plus grand que 1
			if (k == 1)
			{
				double len, rad;
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5*(descr_data[bez_st + 1].d.i.p +
					     descr_data[bez_st + 2].d.i.p);
				TangentOnBezAt (at,PrevPoint (bez_st - 1), descr_data[bez_st + 1].d.i,
						fin, false, pos, tgt, len, rad);
			}
			else if (k == stB.nb)
			{
				NR::Point stP;
				double len, rad;
				stP = 0.5*(descr_data[bez_st + k].d.i.p +
					   descr_data[bez_st + k - 1].d.i.p) ;
				TangentOnBezAt (at,stP, descr_data[bez_st + k].d.i,
						descr_data[bez_st].d.b, false, pos, tgt, len,
						rad);
			}
			else
			{
				NR::Point stP;
				double len, rad;
				stP = 0.5* (descr_data[bez_st + k].d.i.p +
					    descr_data[bez_st + k - 1].d.i.p);
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5* (descr_data[bez_st + k].d.i.p +
					      descr_data[bez_st + k + 1].d.i.p) ;
				TangentOnBezAt (at,stP, descr_data[bez_st + k].d.i,
						fin, false, pos, tgt, len, rad);
			}
		}
	}
}
