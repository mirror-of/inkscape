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
	descr_max = descr_nb = 0;
	descr_cmd = NULL;
	ddata_max = ddata_nb = 0;
	descr_data=NULL;
	descr_flags = 0;
	pending_bezier_cmd = -1;
	pending_moveto_cmd = -1;
  
	back = false;
	nbPt = maxPt = sizePt = 0;
	pts = NULL;
}
Path::~Path (void)
{
	if (descr_cmd) {
		free (descr_cmd);
		descr_cmd = NULL;
	}
	if (descr_data) {
		free(descr_data);
		descr_data = NULL;
	}
	if (pts) {
		free (pts);
		pts = NULL;
	}
	ddata_max = ddata_nb = 0;
	descr_max = descr_nb = 0;
	nbPt = maxPt = sizePt = 0;
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
		printf ("[ %i %f %f ] ", (descr_cmd + i)->associated, (descr_cmd + i)->tSt, (descr_cmd + i)->tEn);
		int ty = (descr_cmd + i)->flags & descr_type_mask;
		if (ty == descr_forced) 	{
			printf ("F\n");
		} else if (ty == descr_moveto) {
      path_descr_moveto  *elem=(path_descr_moveto*)(descr_data+descr_cmd[i].dStart);
			printf ("M %f %f %i\n", elem->p[0], elem->p[1], elem->pathLength);
		} else if (ty == descr_lineto) {
      path_descr_lineto  *elem=(path_descr_lineto*)(descr_data+descr_cmd[i].dStart);
			printf ("L %f %f\n", elem->p[0],
              elem->p[1]);
		} else if (ty == descr_arcto) {
      path_descr_arcto  *elem=(path_descr_arcto*)(descr_data+descr_cmd[i].dStart);
			printf ("A %f %f %f %f %i %i\n", elem->p[0],elem->p[1], elem->rx,elem->ry,(elem->large) ? 1 : 0,(elem->clockwise) ? 1 : 0);
		} else if (ty == descr_cubicto) {
      path_descr_cubicto  *elem=(path_descr_cubicto*)(descr_data+descr_cmd[i].dStart);
			printf ("C %f %f %f %f %f %f\n", elem->p[0],elem->p[1], elem->stD[0],
              elem->stD[1], elem->enD[0],elem->enD[1]);
		} else if (ty == descr_bezierto) {
      path_descr_bezierto  *elem=(path_descr_bezierto*)(descr_data+descr_cmd[i].dStart);
			printf ("B %f %f %i\n", elem->p[0], elem->p[1], elem->nb);
		} else if (ty == descr_interm_bezier) {
      path_descr_intermbezierto  *elem=(path_descr_intermbezierto*)(descr_data+descr_cmd[i].dStart);
			printf ("I %f %f\n", elem->p[0], elem->p[1]);
		} else if (ty == descr_close) {
			printf ("Z\n");
		}
	}
	printf("nbPt: %i\n",nbPt);
	if ( back ) {
    path_lineto_b  *tp=(path_lineto_b*)pts;
    for (int i=0;i<nbPt;i++) {
      printf("[ %f %f %i %f ] ",tp[i].p[0],tp[i].p[1],tp[i].piece,tp[i].t);
    }
	} else {
    path_lineto  *tp=(path_lineto*)pts;
    for (int i=0;i<nbPt;i++) {
      printf("[ %f %f ] ",tp[i].p[0],tp[i].p[1]);
    }
	}
	printf("\n\n");
	fflush(stdout);
}
void
Path::Reset (void)
{
	descr_nb = 0;
	pending_bezier_cmd = -1;
	pending_moveto_cmd = -1;
	descr_flags = 0;
}

void
Path::Copy (Path * who)
{
	ResetPoints (0);
	if (who->descr_nb > descr_max)
	{
		descr_max = who->descr_nb;
		descr_cmd = (path_descr *) realloc (descr_cmd, descr_max * sizeof (path_descr));
	}
	if (who->ddata_nb > ddata_max)
	{
		ddata_max = who->ddata_nb;
		descr_data = (NR::Point *) realloc (descr_data, ddata_max * sizeof (NR::Point));
	}
	descr_nb = who->descr_nb;
	ddata_nb = who->ddata_nb;
	memcpy (descr_cmd, who->descr_cmd, descr_nb * sizeof (path_descr));
	memcpy (descr_data, who->descr_data, ddata_nb * sizeof (NR::Point));
}

void       Path::AlloueDCmd (int addNb)
{
	if (descr_nb + addNb > descr_max)
	{
		descr_max = 2 * descr_nb + addNb;
		descr_cmd =(path_descr *) realloc (descr_cmd, descr_max * sizeof (path_descr));
	}
}
void       Path::AlloueDData (int addNb)
{
	if (ddata_nb + addNb > ddata_max)
	{
		ddata_max = 2 * ddata_nb + addNb;
		descr_data =(NR::Point *) realloc (descr_data, ddata_max * sizeof (NR::Point));
	}
}
int        Path::SizeForData(int typ)
{
  int res=0;
  switch ( typ ) {
    case descr_moveto:
      res=sizeof(path_descr_moveto);
      break;
    case descr_lineto:
      res=sizeof(path_descr_lineto);
      break;
    case descr_cubicto:
      res=sizeof(path_descr_cubicto);
      break;
    case descr_arcto:
      res=sizeof(path_descr_arcto);
      break;
    case descr_bezierto:
      res=sizeof(path_descr_bezierto);
      break;
    case descr_interm_bezier:
      res=sizeof(path_descr_intermbezierto);
      break;
    case descr_close:
    case descr_forced:
    default:
      res=0;
      break;
  }
  int  n=res/sizeof(NR::Point);
  int  r=res%sizeof(NR::Point);
  if ( r ) n+=1;
  return n;
}
void
Path::CloseSubpath (int add)
{
	for (int i = descr_nb - 1; i >= 0; i--)
	{
		int ty = (descr_cmd + i)->flags & descr_type_mask;
		if (ty == descr_moveto) {
      path_descr_moveto *nData=(path_descr_moveto*)(descr_data+descr_cmd[i].dStart);
			nData->pathLength = descr_nb - i + add;	// il faut compter le close qui n'est pas encore ajout√©
			break;
		}
	}
	descr_flags &= ~(descr_doing_subpath);
	pending_moveto_cmd = -1;
}

int
Path::ForcePoint (void)
{
	if (descr_flags & descr_adding_bezier)
		EndBezierTo ();
	if (descr_flags & descr_doing_subpath) {
	} else {
		return -1;
	}
	if (descr_nb <= 0) return -1;
	AlloueDCmd (1);
  AlloueDData(SizeForData(descr_forced));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_forced;
  nElem->dStart=ddata_nb;
	return descr_nb - 1;
}

int
Path::Close (void)
{
	if (descr_flags & descr_adding_bezier) CancelBezier ();
	if (descr_flags & descr_doing_subpath) {
		CloseSubpath (1);
	} else {
		// rien a fermer => byebye
		return -1;
	}
	AlloueDCmd (1);
  AlloueDData(SizeForData(descr_close));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_close;
  nElem->dStart=ddata_nb;
	descr_flags &= ~(descr_doing_subpath);
	pending_moveto_cmd = -1;
	return descr_nb - 1;
}

int
Path::MoveTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier) EndBezierTo (iPt);
	if (descr_flags & descr_doing_subpath) CloseSubpath (0);
  
	pending_moveto_cmd = descr_nb;
  pending_moveto_data = ddata_nb;
	AlloueDCmd (1);
  AlloueDData(SizeForData(descr_moveto));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
  nElem->dStart=ddata_nb;
  path_descr_moveto *nData=(path_descr_moveto*)(descr_data+ddata_nb);
  ddata_nb+=SizeForData(descr_moveto);
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_moveto;
	nData->p = iPt;
	nData->pathLength = 0;
	descr_flags |= descr_doing_subpath;
	return descr_nb - 1;
}


int
Path::LineTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier) EndBezierTo (iPt);
	if (descr_flags & descr_doing_subpath) {
	} else {
		return MoveTo (iPt);
	}
	AlloueDCmd (1);
  AlloueDData(SizeForData(descr_lineto));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
  nElem->dStart=ddata_nb;
  path_descr_lineto *nData=(path_descr_lineto*)(descr_data+ddata_nb);
  ddata_nb+=SizeForData(descr_lineto);
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_lineto;
	nData->p = iPt;
	return descr_nb - 1;
}


int
Path::CubicTo (NR::Point const &iPt, NR::Point const &iStD, NR::Point const &iEnD)
{
	if (descr_flags & descr_adding_bezier) EndBezierTo (iPt);
	if (descr_flags & descr_doing_subpath) {
	} else {
		return MoveTo (iPt);
	}
	AlloueDCmd (1);
  AlloueDData(SizeForData(descr_cubicto));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
  nElem->dStart=ddata_nb;
  path_descr_cubicto *nData=(path_descr_cubicto*)(descr_data+ddata_nb);
  ddata_nb+=SizeForData(descr_cubicto);
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_cubicto;
	nData->p = iPt;
	nData->stD = iStD;
	nData->enD = iEnD;
	return descr_nb - 1;
}

int
Path::ArcTo (NR::Point const &iPt, double iRx, double iRy, double angle,
             bool iLargeArc, bool iClockwise)
{
	if (descr_flags & descr_adding_bezier) EndBezierTo (iPt);
	if (descr_flags & descr_doing_subpath) {
	} else {
		return MoveTo (iPt);
	}
	AlloueDCmd (1);
  AlloueDData(SizeForData(descr_arcto));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
  nElem->dStart=ddata_nb;
  path_descr_arcto *nData=(path_descr_arcto*)(descr_data+ddata_nb);
  ddata_nb+=SizeForData(descr_arcto);
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_arcto;
	nData->p = iPt;
	nData->rx = iRx;
	nData->ry = iRy;
	nData->angle = angle;
	nData->large = iLargeArc;
	nData->clockwise = iClockwise;
	return descr_nb - 1;
}

int
Path::TempBezierTo (void)
{
	if (descr_flags & descr_adding_bezier) CancelBezier ();
	if (descr_flags & descr_doing_subpath) {
	} else {
		// pas de pt de d√©part-> pas bon
		return -1;
	}
	pending_bezier_cmd = descr_nb;
  pending_bezier_data = ddata_nb;
	AlloueDCmd (1);
  AlloueDData(SizeForData(descr_bezierto));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
  nElem->dStart=ddata_nb;
  path_descr_bezierto *nData=(path_descr_bezierto*)(descr_data+ddata_nb);
  ddata_nb+=SizeForData(descr_bezierto);
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_bezierto;
	nData->nb = 0;
	descr_flags |= descr_adding_bezier;
	descr_flags |= descr_delayed_bezier;
	return descr_nb - 1;
}

void
Path::CancelBezier (void)
{
	descr_flags &= ~(descr_adding_bezier);
	descr_flags &= ~(descr_delayed_bezier);
	if (pending_bezier_cmd < 0) return;
	descr_nb = pending_bezier_cmd;
  ddata_nb = pending_bezier_data;
	pending_bezier_cmd = -1;
}

int
Path::EndBezierTo (void)
{
	if (descr_flags & descr_delayed_bezier) {
		CancelBezier ();
	} else {
		pending_bezier_cmd = -1;
		descr_flags &= ~(descr_adding_bezier);
		descr_flags &= ~(descr_delayed_bezier);
	}
	return -1;
}

int
Path::EndBezierTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier) {
	} else {
		return LineTo (iPt);
	}
	if (descr_flags & descr_doing_subpath) {
	} else {
		return MoveTo (iPt);
	}
	if (descr_flags & descr_delayed_bezier) {
	} else {
		return EndBezierTo ();
	}
  path_descr_bezierto *nData=(path_descr_bezierto*)(descr_data+pending_bezier_data);
  nData->p = iPt;
	pending_bezier_cmd = -1;
	descr_flags &= ~(descr_adding_bezier);
	descr_flags &= ~(descr_delayed_bezier);
	return -1;
}


int
Path::IntermBezierTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier) {
	} else {
		return LineTo (iPt);
	}
	if (descr_flags & descr_doing_subpath) {
	} else {
		return MoveTo (iPt);
	}
  {
    AlloueDCmd (1);
    AlloueDData(SizeForData(descr_interm_bezier));
		path_descr *nElem = descr_cmd + descr_nb;
		descr_nb++;
    nElem->dStart=ddata_nb;
    path_descr_intermbezierto *nData=(path_descr_intermbezierto*)(descr_data+ddata_nb);
    ddata_nb+=SizeForData(descr_interm_bezier);
		nElem->associated = -1;
		nElem->tSt = 0.0;
		nElem->tEn = 1.0;
		nElem->flags = descr_interm_bezier;
		nData->p = iPt;
    path_descr_bezierto *nBData=(path_descr_bezierto*)(descr_data+pending_bezier_data);
		nBData->nb++;
		return descr_nb - 1;
	}
	return -1;
}

int
Path::BezierTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier) EndBezierTo (iPt);
	if (descr_flags & descr_doing_subpath) {
	} else {
		return MoveTo (iPt);
	}
	pending_bezier_cmd = descr_nb;
  pending_bezier_data = ddata_nb;

	AlloueDCmd (1);
  AlloueDData(SizeForData(descr_bezierto));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
  nElem->dStart=ddata_nb;
  path_descr_bezierto *nData=(path_descr_bezierto*)(descr_data+ddata_nb);
  ddata_nb+=SizeForData(descr_bezierto);
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_bezierto;
	nData->nb = 0;
	nData->p = iPt;
	descr_flags |= descr_adding_bezier;
	descr_flags &= ~(descr_delayed_bezier);
	return descr_nb - 1;
}


/*
 * points de la polyligne
 */
void
Path::SetBackData (bool nVal)
{
	if (back == false) {
		if (nVal == true && back == false) {
			back = true;
			ResetPoints (nbPt);
		} else if (nVal == false && back == true) {
			back = false;
			ResetPoints (nbPt);
		}
	} else {
		if (nVal == true && back == false) {
			back = true;
			ResetPoints (nbPt);
		} else if (nVal == false && back == true) {
			back = false;
			ResetPoints (nbPt);
		}
	}
}
void
Path::ResetPoints (int expected)
{
	nbPt = 0;
	if (back) {
    sizePt = expected * sizeof (path_lineto_b);
	} else {
    sizePt = expected * sizeof (path_lineto);
	}
	if (sizePt > maxPt) {
		maxPt = sizePt;
		pts = (char *) realloc (pts, maxPt);
	}
}
int
Path::AddPoint (NR::Point const &iPt, bool mvto)
{
	if (back) {
		return AddPoint (iPt, -1, 0.0, mvto);
	}
  
	int nextSize = sizePt + sizeof (path_lineto);
	if (nextSize > maxPt) {
		maxPt = 2 * sizePt + sizeof (path_lineto);
		pts = (char *) realloc (pts, maxPt);
	}
	if (mvto == false && nbPt > 0 && ((path_lineto *) pts)[nbPt - 1].p == iPt) return -1;
	int n = nbPt++;
	sizePt = nextSize;
	if (mvto) ((path_lineto *) pts)[n].isMoveTo = polyline_moveto; else ((path_lineto *) pts)[n].isMoveTo = polyline_lineto;
	((path_lineto *) pts)[n].p = iPt;
	return n;
}


int
Path::AddPoint (NR::Point const &iPt, int ip, double it, bool mvto)
{
	if (back) {
	} else {
		return AddPoint (iPt, mvto);
	}
	int nextSize = sizePt + sizeof (path_lineto_b);
	if (nextSize > maxPt) {
		maxPt = 2 * sizePt + sizeof (path_lineto_b);
		pts = (char *) realloc (pts, maxPt);
	}
	if (mvto == false && nbPt > 0 && ((path_lineto_b *) pts)[nbPt - 1].p == iPt) return -1;
	int n = nbPt++;
	sizePt = nextSize;
	if (mvto) ((path_lineto_b *) pts)[n].isMoveTo = polyline_moveto; else ((path_lineto_b *) pts)[n].isMoveTo = polyline_lineto;
	((path_lineto_b *) pts)[n].p = iPt;
	((path_lineto_b *) pts)[n].piece = ip;
	((path_lineto_b *) pts)[n].t = it;
	return n;
}
int
Path::AddForcedPoint (NR::Point const &iPt)
{
	if (back) {
		return AddForcedPoint (iPt, -1, 0.0);
	} else {
	}
	int nextSize = sizePt + sizeof (path_lineto);
	if (nextSize > maxPt) {
		maxPt = 2 * sizePt + sizeof (path_lineto);
		pts = (char *) realloc (pts, maxPt);
	}
	if (nbPt <= 0  || ((path_lineto *) pts)[nbPt - 1].isMoveTo != polyline_lineto) return -1;
	int n = nbPt++;
	sizePt = nextSize;
	((path_lineto *) pts)[n].isMoveTo = polyline_forced;
	((path_lineto *) pts)[n].p = ((path_lineto *) pts)[n - 1].p;
	return n;
}
int
Path::AddForcedPoint (NR::Point const &iPt, int ip, double it)
{
	if (back) {
	} else {
		return AddForcedPoint (iPt);
	}
	int nextSize = sizePt + sizeof (path_lineto_b);
	if (nextSize > maxPt) {
		maxPt = 2 * sizePt + sizeof (path_lineto_b);
		pts = (char *) realloc (pts, maxPt);
	}
	if (nbPt <= 0  || ((path_lineto_b *) pts)[nbPt - 1].isMoveTo != polyline_lineto) return -1;
	int n = nbPt++;
	sizePt = nextSize;
	((path_lineto_b *) pts)[n].isMoveTo = polyline_forced;
	((path_lineto_b *) pts)[n].p = ((path_lineto_b *) pts)[n - 1].p;
	((path_lineto_b *) pts)[n].piece = ((path_lineto_b *) pts)[n - 1].piece;
	((path_lineto_b *) pts)[n].t = ((path_lineto_b *) pts)[n - 1].t;
	return n;
}


// utilities
void
Path::PointAt (int piece, double at, NR::Point & pos)
{
	if (piece < 0 || piece >= descr_nb)
	{
		// this shouldn't happen: the piece we are asked for doesn't exist in the path
		pos[0] = pos[1] = 0;
		return;
	}
	path_descr theD = descr_cmd[piece];
	int typ = theD.flags & descr_type_mask;
  NR::Point tgt;
  double len,rad;
	if (typ == descr_moveto) {
		return PointAt (piece + 1, 0.0, pos);
	} else if (typ == descr_close || typ == descr_forced) {
		return PointAt (piece - 1, 1.0, pos);
	} else if (typ == descr_lineto) {
    path_descr_lineto* nData=(path_descr_lineto*)(descr_data+theD.dStart);
		TangentOnSegAt (at, PrevPoint (piece - 1), *nData, pos, tgt, len);
	} else if (typ == descr_arcto) {
    path_descr_arcto* nData=(path_descr_arcto*)(descr_data+theD.dStart);
		TangentOnArcAt (at,PrevPoint (piece - 1), *nData, pos, tgt, len, rad);
	} else if (typ == descr_cubicto) {
    path_descr_cubicto* nData=(path_descr_cubicto*)(descr_data+theD.dStart);
		TangentOnCubAt (at, PrevPoint (piece - 1), *nData, false, pos, tgt, len, rad);
	} else if (typ == descr_bezierto || typ == descr_interm_bezier) {
		int bez_st = piece;
		while (bez_st >= 0) {
			int nt = descr_cmd[bez_st].flags & descr_type_mask;
			if (nt == descr_bezierto) break;
			bez_st--;
		}
		if (bez_st < 0) return PointAt (piece - 1, 1.0, pos); // pas trouvé le dubut de la spline (mauvais)
    
		path_descr_bezierto* stB = (path_descr_bezierto*)(descr_data + descr_cmd[bez_st].dStart);
		if (piece > bez_st + stB->nb) return PointAt (piece - 1, 1.0, pos); // la spline sort du nombre de commandes autorisé (mauvais)
    
		int         k = piece - bez_st;
    NR::Point   bStPt=PrevPoint (bez_st - 1);
		if (stB->nb == 1 || k <= 0) {
      path_descr_intermbezierto* nData=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + 1].dStart);
			TangentOnBezAt (at, bStPt, *nData, *stB, false, pos, tgt, len, rad);
		} else {
			// forcement plus grand que 1
      path_descr_intermbezierto  *prevI;
      path_descr_intermbezierto  *nextI;
			if (k == 1) {
        prevI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + 1].dStart);
        nextI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + 2].dStart);
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5*(prevI->p + nextI->p) ;
				TangentOnBezAt (at, bStPt, *prevI,  fin, false, pos, tgt, len, rad);
			} else if (k == stB->nb) {
        nextI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k].dStart);
        prevI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k - 1].dStart);
				NR::Point stP = 0.5*(prevI->p +nextI->p) ;
				TangentOnBezAt (at, stP, *nextI, *stB, false, pos, tgt, len, rad);
			} else {
        nextI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k].dStart);
        prevI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k - 1].dStart);
        path_descr_intermbezierto  *nnextI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k + 1].dStart);
				NR::Point stP = 0.5* (prevI->p + nextI->p);
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5* (nextI->p + nnextI->p);
				TangentOnBezAt (at, stP, *nextI,fin, false, pos, tgt, len, rad);
			}
		}
	}
}
void
Path::PointAndTangentAt (int piece, double at, NR::Point & pos, NR::Point & tgt)
{
	if (piece < 0 || piece >= descr_nb)
	{
		// this shouldn't happen: the piece we are asked for doesn't exist in the path
		pos[0] = pos[1] = 0;
		return;
	}
	path_descr theD = descr_cmd[piece];
	int typ = theD.flags & descr_type_mask;
  double len,rad;
	if (typ == descr_moveto) {
		return PointAndTangentAt (piece + 1, 0.0, pos,tgt);
	} else if (typ == descr_close || typ == descr_forced) {
		return PointAndTangentAt (piece - 1, 1.0, pos,tgt);
	} else if (typ == descr_lineto) {
    path_descr_lineto* nData=(path_descr_lineto*)(descr_data+theD.dStart);
		TangentOnSegAt (at, PrevPoint (piece - 1), *nData, pos, tgt, len);
	} else if (typ == descr_arcto) {
    path_descr_arcto* nData=(path_descr_arcto*)(descr_data+theD.dStart);
		TangentOnArcAt (at,PrevPoint (piece - 1), *nData, pos, tgt, len, rad);
	} else if (typ == descr_cubicto) {
    path_descr_cubicto* nData=(path_descr_cubicto*)(descr_data+theD.dStart);
		TangentOnCubAt (at, PrevPoint (piece - 1), *nData, false, pos, tgt, len, rad);
	} else if (typ == descr_bezierto || typ == descr_interm_bezier) {
		int bez_st = piece;
		while (bez_st >= 0) {
			int nt = descr_cmd[bez_st].flags & descr_type_mask;
			if (nt == descr_bezierto) break;
			bez_st--;
		}
		if (bez_st < 0) return PointAndTangentAt (piece - 1, 1.0, pos,tgt); // pas trouvé le dubut de la spline (mauvais)
    
		path_descr_bezierto* stB = (path_descr_bezierto*)(descr_data + descr_cmd[bez_st].dStart);
		if (piece > bez_st + stB->nb) return PointAndTangentAt (piece - 1, 1.0, pos,tgt); // la spline sort du nombre de commandes autorisé (mauvais)
    
		int         k = piece - bez_st;
    NR::Point   bStPt=PrevPoint (bez_st - 1);
		if (stB->nb == 1 || k <= 0) {
      path_descr_intermbezierto* nData=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + 1].dStart);
			TangentOnBezAt (at, bStPt, *nData, *stB, false, pos, tgt, len, rad);
		} else {
			// forcement plus grand que 1
      path_descr_intermbezierto  *prevI;
      path_descr_intermbezierto  *nextI;
			if (k == 1) {
        prevI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + 1].dStart);
        nextI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + 2].dStart);
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5*(prevI->p + nextI->p) ;
				TangentOnBezAt (at, bStPt, *prevI,  fin, false, pos, tgt, len, rad);
			} else if (k == stB->nb) {
        nextI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k].dStart);
        prevI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k - 1].dStart);
				NR::Point stP = 0.5*(prevI->p +nextI->p) ;
				TangentOnBezAt (at, stP, *nextI, *stB, false, pos, tgt, len, rad);
			} else {
        nextI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k].dStart);
        prevI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k - 1].dStart);
        path_descr_intermbezierto  *nnextI=(path_descr_intermbezierto*)(descr_data+descr_cmd[bez_st + k + 1].dStart);
				NR::Point stP = 0.5* (prevI->p + nextI->p);
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5* (nextI->p + nnextI->p);
				TangentOnBezAt (at, stP, *nextI,fin, false, pos, tgt, len, rad);
			}
		}
	}
}
