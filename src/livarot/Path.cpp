/*
 *  Path.cpp
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *
 */

#include <glib.h>
#include "Path.h"
#include <libnr/nr-point.h>
#include <libnr/nr-point-ops.h>
#include <libnr/nr-matrix-ops.h>
#include "../svg/stringstream.h"

/*
 * manipulation of the path data: path description and polyline
 * grunt work...
 * at the end of this file, 2 utilitary functions to get the point and tangent to path associated with a (command no;abcissis)
 */


Path::Path()
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

Path::~Path()
{
	g_free(descr_cmd);
	descr_cmd = NULL;
	g_free(descr_data);
	descr_data = NULL;
	g_free(pts);
	pts = NULL;
	ddata_max = ddata_nb = 0;
	descr_max = descr_nb = 0;
	nbPt = maxPt = sizePt = 0;
}

// debug function do dump the path contents on stdout
void
Path::Affiche()
{
  printf("path: %i cmds, %i data (%i %i max)\n",descr_nb,ddata_nb,descr_max,ddata_max);
  for (int i=0;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    printf("cmd %i : t=%i s=%i ",i,typ,descr_cmd[i].dStart);
    switch ( typ ) {
      case descr_lineto:
      {
        path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[i].dStart );
        printf("  l %f %f",nData->p[0],nData->p[1]); // localizing ok
      }
        break;
      case descr_moveto:
      {
        path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + descr_cmd[i].dStart );
        printf("  m %f %f",nData->p[0],nData->p[1]); // localizing ok
      }
        break;
      case descr_arcto:
      {
        path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[i].dStart );
        printf("  a %f %f %f %f %f %i %i",nData->p[0],nData->p[1],nData->rx,nData->ry,nData->angle,((nData->clockwise)?1:0),((nData->large)?1:0)); // localizing ok
      }
        break;
      case descr_cubicto:
      {
        path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[i].dStart );
        printf("  c %f %f %f %f %f %f",nData->p[0],nData->p[1],nData->stD[0],nData->stD[1],nData->enD[0],nData->enD[1]); // localizing ok
      }
        break;
      case descr_bezierto:
      {
        path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[i].dStart );
        printf("  b %f %f %i",nData->p[0],nData->p[1],nData->nb); // localizing ok
      }
        break;
      case descr_interm_bezier:
      {
        path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[i].dStart );
        printf("  i %f %f",nData->p[0],nData->p[1]); // localizing ok
      }
        break;
    }
    printf("\n");
  }
  printf("\n");
}

void
Path::Reset()
{
  ddata_nb = 0;
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
		descr_cmd = (path_descr *) g_realloc(descr_cmd, descr_max * sizeof (path_descr));
	}
	if (who->ddata_nb > ddata_max)
	{
		ddata_max = who->ddata_nb;
		descr_data = (NR::Point *) g_realloc(descr_data, ddata_max * sizeof (NR::Point));
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
		descr_cmd =(path_descr *) g_realloc(descr_cmd, descr_max * sizeof (path_descr));
	}
}
void       Path::AlloueDData (int addNb)
{
	if (ddata_nb + addNb > ddata_max)
	{
		ddata_max = 2 * ddata_nb + addNb;
		descr_data =(NR::Point *) g_realloc(descr_data, ddata_max * sizeof (NR::Point));
	}
}
void       Path::ShiftDCmd(int at,int dec)
{
  if ( at < 0 || at > descr_nb || dec == 0 ) return;
  if ( at == descr_nb ) {
    if ( dec > 0 ) {
      AlloueDCmd(dec);
      descr_nb+=dec;
    }
    return;
  }
  if ( dec > 0 ) {
    AlloueDCmd(dec);
    if ( at < descr_nb ) memmove(descr_cmd+(at+dec),descr_cmd+at,(descr_nb-at)*sizeof(path_descr));
    descr_nb+=dec;
  } else {
    if ( (-dec) <= descr_nb-at ) {
      if ( at < descr_nb+dec ) memmove(descr_cmd+at,descr_cmd+(at-dec),(descr_nb-at+dec)*sizeof(path_descr));
      descr_nb+=dec;      
    }
  }
}
void       Path::ShiftDData(int at,int dec)
{
  if ( at < 0 || at > ddata_nb || dec == 0 ) return;
  if ( at == ddata_nb ) {
    if ( dec > 0 ) {
      AlloueDData(dec);
      ddata_nb+=dec;
    }
    return;
  }
  if ( dec > 0 ) {
    AlloueDData(dec);
    if ( at < ddata_nb ) memmove(descr_data+(at+dec),descr_data+at,(ddata_nb-at)*sizeof(NR::Point));
    ddata_nb+=dec;
    for (int i=0;i<descr_nb;i++) {
      if ( descr_cmd[i].dStart >= at ) descr_cmd[i].dStart+=dec;
    }
  } else {
    if ( (-dec) <= ddata_nb-at ) {
      if ( at < ddata_nb+dec ) memmove(descr_data+at,descr_data+(at-dec),(ddata_nb-at+dec)*sizeof(NR::Point));
      ddata_nb+=dec;      
      for (int i=0;i<descr_nb;i++) {
        if ( descr_cmd[i].dStart >= at ) descr_cmd[i].dStart+=dec;
      }
    }
  }
}

/** Returns ceil( (double) numerator / denominator ). */
static inline unsigned roundup_div(unsigned const numerator, unsigned const denominator)
{
	g_assert( denominator != 0 );
	unsigned const ret = ( numerator == 0
			       ? 0
			       : 1 + ( numerator - 1 ) / denominator );
	g_assert( numerator == 0 && ret == 0
		  || ( ret != 0
		       && ( ret - 1 ) * denominator < numerator
		       && numerator / ret <= denominator ) );
	return ret;
}

/** Returns the number of NR::Point's worth of space required for the path_descr_blah type
    corresponding to \a typ.
*/
static inline unsigned SizeForData(int const typ)
{
	/* impl: inline because always called with a constant, so the compiler should be able to
	 * evaluate the function at compile time. */
	size_t res;

	switch (typ) {
	case descr_moveto:        res = sizeof(Path::path_descr_moveto); break;
	case descr_lineto:        res = sizeof(Path::path_descr_lineto); break;
	case descr_cubicto:       res = sizeof(Path::path_descr_cubicto); break;
	case descr_arcto:         res = sizeof(Path::path_descr_arcto); break;
	case descr_bezierto:      res = sizeof(Path::path_descr_bezierto); break;
	case descr_interm_bezier: res = sizeof(Path::path_descr_intermbezierto); break;

	case descr_close:
	case descr_forced:
	default:
		res = 0;
		break;
	}

	return roundup_div(res, sizeof(NR::Point));
}


void
Path::CloseSubpath()
{
	descr_flags &= ~(descr_doing_subpath);
	pending_moveto_cmd = -1;
}

int Path::ForcePoint()
{
    if (descr_flags & descr_adding_bezier) {
        EndBezierTo ();
    }
    
    if ( (descr_flags & descr_doing_subpath) == 0 ) {
        return -1;
    }
    
    if (descr_nb <= 0) {
        return -1;
    }
    
    AlloueDCmd(1);
    AlloueDData(SizeForData(descr_forced));
    path_descr *nElem = descr_cmd + descr_nb;
    descr_nb++;
    nElem->associated = -1;
    nElem->tSt = 0.0;
    nElem->tEn = 1.0;
    nElem->flags = descr_forced;
    nElem->dStart = ddata_nb;
    
    return descr_nb - 1;
}
void
Path::InsertForcePoint (int at)
{
  if ( at < 0 || at > descr_nb ) return;
  if ( at == descr_nb ) {
    ForcePoint();
    return;
  }
  int  dataPos=ddata_nb;
  for (int i=at;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    if ( typ == descr_lineto || typ == descr_moveto || typ == descr_arcto || typ == descr_cubicto || typ == descr_bezierto ||
         typ == descr_interm_bezier ) {
      dataPos=descr_cmd[i].dStart;
      break;
    }
  }
//	ShiftDData(dataPos,SizeForData(descr_moveto));
	ShiftDCmd (at,1);
  
	path_descr *nElem = descr_cmd + at;
  
	nElem->dStart = dataPos;
  
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_forced;
}

int
Path::Close()
{
    if ( descr_flags & descr_adding_bezier ) {
        CancelBezier();
    }
    if ( descr_flags & descr_doing_subpath ) {
        CloseSubpath();
    } else {
        // Nothing to close.
        return -1;
    }
    
    AlloueDCmd(1);
    AlloueDData(SizeForData(descr_close));
    path_descr *nElem = descr_cmd + descr_nb;
    descr_nb++;
    nElem->associated = -1;
    nElem->tSt = 0.0;
    nElem->tEn = 1.0;
    nElem->flags = descr_close;
    nElem->dStart = ddata_nb;
    descr_flags &= ~(descr_doing_subpath);
    pending_moveto_cmd = -1;
    
    return descr_nb - 1;
}

int
Path::MoveTo (NR::Point const &iPt)
{
	if ( descr_flags & descr_adding_bezier ) {
		EndBezierTo(iPt);
	}
	if ( descr_flags & descr_doing_subpath ) {
		CloseSubpath();
	}
	pending_moveto_cmd = descr_nb;
	pending_moveto_data = ddata_nb;
	AlloueDCmd (1);
	AlloueDData(SizeForData(descr_moveto));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
	nElem->dStart = ddata_nb;
	path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + ddata_nb );
	ddata_nb += SizeForData(descr_moveto);
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_moveto;
	nData->p = iPt;
	descr_flags |= descr_doing_subpath;
	return descr_nb - 1;
}

void
Path::InsertMoveTo (NR::Point const &iPt,int at)
{
  if ( at < 0 || at > descr_nb ) return;
  if ( at == descr_nb ) {
    MoveTo(iPt);
    return;
  }
  int  dataPos=ddata_nb;
  for (int i=at;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    if ( typ == descr_lineto || typ == descr_moveto || typ == descr_arcto || typ == descr_cubicto || typ == descr_bezierto ||
         typ == descr_interm_bezier ) {
      dataPos=descr_cmd[i].dStart;
      break;
    }
  }
	ShiftDData(dataPos,SizeForData(descr_moveto));
	ShiftDCmd (at,1);
  
	path_descr *nElem = descr_cmd + at;
  
	nElem->dStart = dataPos;
	path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + dataPos );
  
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_moveto;
	nData->p = iPt;
}

int
Path::LineTo (NR::Point const &iPt)
{
	if (descr_flags & descr_adding_bezier) {
		EndBezierTo (iPt);
	}
	if (!( descr_flags & descr_doing_subpath )) {
		return MoveTo (iPt);
	}
	AlloueDCmd (1);
	AlloueDData(SizeForData(descr_lineto));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
	nElem->dStart = ddata_nb;
	path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + ddata_nb );
	ddata_nb += SizeForData(descr_lineto);
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_lineto;
	nData->p = iPt;
	return descr_nb - 1;
}

void Path::InsertLineTo(NR::Point const &iPt, int at)
{
    if ( at < 0 || at > descr_nb ) {
        return;
    }
    
    if ( at == descr_nb ) {
        LineTo(iPt);
        return;
    }
    
    int dataPos = ddata_nb;
    
    for (int i = at; i < descr_nb; i++) {
        int const typ = descr_cmd[i].flags & descr_type_mask;
        if ( typ == descr_lineto || typ == descr_moveto || typ == descr_arcto ||
             typ == descr_cubicto || typ == descr_bezierto || typ == descr_interm_bezier ) {
            
            dataPos = descr_cmd[i].dStart;
            break;
        }
    }
    
    ShiftDData(dataPos, SizeForData(descr_lineto));
    ShiftDCmd(at, 1);
  
    path_descr *nElem = descr_cmd + at;
  
    nElem->dStart = dataPos;
    path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + dataPos );
  
    nElem->associated = -1;
    nElem->tSt = 0.0;
    nElem->tEn = 1.0;
    nElem->flags = descr_lineto;
    nData->p = iPt;
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
	nElem->dStart = ddata_nb;
	path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + ddata_nb );
	ddata_nb += SizeForData(descr_cubicto);
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_cubicto;
	nData->p = iPt;
	nData->stD = iStD;
	nData->enD = iEnD;
	return descr_nb - 1;
}
void
Path::InsertCubicTo (NR::Point const &iPt, NR::Point const &iStD, NR::Point const &iEnD,int at)
{
  if ( at < 0 || at > descr_nb ) return;
  if ( at == descr_nb ) {
    CubicTo(iPt,iStD,iEnD);
    return;
  }
  int  dataPos=ddata_nb;
  for (int i=at;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    if ( typ == descr_lineto || typ == descr_moveto || typ == descr_arcto || typ == descr_cubicto || typ == descr_bezierto ||
         typ == descr_interm_bezier ) {
      dataPos=descr_cmd[i].dStart;
      break;
    }
  }
	ShiftDData(dataPos,SizeForData(descr_cubicto));
	ShiftDCmd (at,1);
  
	path_descr *nElem = descr_cmd + at;
  
	nElem->dStart = dataPos;
	path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + dataPos );
  
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_cubicto;
	nData->p = iPt;
	nData->stD = iStD;
	nData->enD = iEnD;
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
	nElem->dStart = ddata_nb;
	path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + ddata_nb );
	ddata_nb += SizeForData(descr_arcto);
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
void
Path::InsertArcTo (NR::Point const &iPt, double iRx, double iRy, double angle,
                   bool iLargeArc, bool iClockwise,int at)
{
  if ( at < 0 || at > descr_nb ) return;
  if ( at == descr_nb ) {
    ArcTo(iPt,iRx,iRy,angle,iLargeArc,iClockwise);
    return;
  }
  int  dataPos=ddata_nb;
  for (int i=at;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    if ( typ == descr_lineto || typ == descr_moveto || typ == descr_arcto || typ == descr_cubicto || typ == descr_bezierto ||
         typ == descr_interm_bezier ) {
      dataPos=descr_cmd[i].dStart;
      break;
    }
  }
	ShiftDData(dataPos,SizeForData(descr_arcto));
	ShiftDCmd (at,1);
  
	path_descr *nElem = descr_cmd + at;
  
	nElem->dStart = dataPos;
	path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + dataPos );
  
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
}

int
Path::TempBezierTo()
{
	if (descr_flags & descr_adding_bezier) CancelBezier ();
	if (descr_flags & descr_doing_subpath) {
	} else {
		// No starting point -> bad.
		return -1;
	}
	pending_bezier_cmd = descr_nb;
	pending_bezier_data = ddata_nb;
	AlloueDCmd (1);
	AlloueDData(SizeForData(descr_bezierto));
	path_descr *nElem = descr_cmd + descr_nb;
	descr_nb++;
	nElem->dStart = ddata_nb;
	path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + ddata_nb );
	ddata_nb += SizeForData(descr_bezierto);
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
Path::CancelBezier()
{
	descr_flags &= ~(descr_adding_bezier);
	descr_flags &= ~(descr_delayed_bezier);
	if (pending_bezier_cmd < 0) return;
	descr_nb = pending_bezier_cmd;
	ddata_nb = pending_bezier_data;
	pending_bezier_cmd = -1;
}

int
Path::EndBezierTo()
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
	path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + pending_bezier_data );
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
		AlloueDCmd(1);
		AlloueDData(SizeForData(descr_interm_bezier));
		path_descr *nElem = descr_cmd + descr_nb;
		descr_nb++;
		nElem->dStart = ddata_nb;
		path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + ddata_nb );
		ddata_nb += SizeForData(descr_interm_bezier);
		nElem->associated = -1;
		nElem->tSt = 0.0;
		nElem->tEn = 1.0;
		nElem->flags = descr_interm_bezier;
		nData->p = iPt;
		path_descr_bezierto *nBData = reinterpret_cast<path_descr_bezierto *>( descr_data + pending_bezier_data );
		nBData->nb++;
		return descr_nb - 1;
	}
	return -1;
}
void
Path::InsertIntermBezierTo (NR::Point const &iPt,int at)
{
  if ( at < 0 || at > descr_nb ) return;
  if ( at == descr_nb ) {
    IntermBezierTo(iPt);
    return;
  }
  int  dataPos=ddata_nb;
  for (int i=at;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    if ( typ == descr_lineto || typ == descr_moveto || typ == descr_arcto || typ == descr_cubicto || typ == descr_bezierto ||
         typ == descr_interm_bezier ) {
      dataPos=descr_cmd[i].dStart;
      break;
    }
  }
	ShiftDData(dataPos,SizeForData(descr_interm_bezier));
	ShiftDCmd (at,1);
  
	path_descr *nElem = descr_cmd + at;
  
	nElem->dStart = dataPos;
	path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + dataPos );
  
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_interm_bezier;
	nData->p = iPt;
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
	nElem->dStart = ddata_nb;
	path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + ddata_nb );
	ddata_nb += SizeForData(descr_bezierto);
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

void
Path::InsertBezierTo (NR::Point const &iPt,int iNb,int at)
{
  if ( at < 0 || at > descr_nb ) return;
  if ( at == descr_nb ) {
    BezierTo(iPt);
    return;
  }
  int  dataPos=ddata_nb;
  for (int i=at;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    if ( typ == descr_lineto || typ == descr_moveto || typ == descr_arcto || typ == descr_cubicto || typ == descr_bezierto ||
         typ == descr_interm_bezier ) {
      dataPos=descr_cmd[i].dStart;
      break;
    }
  }
	ShiftDData(dataPos,SizeForData(descr_bezierto));
	ShiftDCmd (at,1);
  
	path_descr *nElem = descr_cmd + at;
  
	nElem->dStart = dataPos;
	path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + dataPos );
  
	nElem->associated = -1;
	nElem->tSt = 0.0;
	nElem->tEn = 1.0;
	nElem->flags = descr_bezierto;
	nData->p = iPt;
	nData->nb = iNb;
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
		pts = (char *) g_realloc(pts, maxPt);
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
		pts = (char *) g_realloc(pts, maxPt);
	}
	if ( !mvto
	     && ( nbPt > 0 )
	     && ( ((path_lineto *) pts)[nbPt - 1].p == iPt ) )
	{
		return -1;
	}
	int n = nbPt++;
	sizePt = nextSize;
	reinterpret_cast<path_lineto *>(pts)[n].isMoveTo = ( mvto
							     ? polyline_moveto
							     : polyline_lineto );
	reinterpret_cast<path_lineto *>(pts)[n].p = iPt;
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
		pts = (char *) g_realloc(pts, maxPt);
	}
	if ( !mvto
	     && ( nbPt > 0 )
	     && ( ((path_lineto_b *) pts)[nbPt - 1].p == iPt ) )
	{
		return -1;
	}
	int n = nbPt++;
	sizePt = nextSize;
	reinterpret_cast<path_lineto_b *>(pts)[n].isMoveTo = ( mvto
							       ? polyline_moveto
							       : polyline_lineto );
	reinterpret_cast<path_lineto_b *>(pts)[n].p = iPt;
	reinterpret_cast<path_lineto_b *>(pts)[n].piece = ip;
	reinterpret_cast<path_lineto_b *>(pts)[n].t = it;
	return n;
}

int
Path::AddForcedPoint (NR::Point const &iPt)
{
	if (back) {
		return AddForcedPoint (iPt, -1, 0.0);
	}
	int nextSize = sizePt + sizeof (path_lineto);
	if (nextSize > maxPt) {
		maxPt = 2 * sizePt + sizeof (path_lineto);
		pts = (char *) g_realloc(pts, maxPt);
	}
	if ( ( nbPt <= 0 )
	     || ( ((path_lineto *) pts)[nbPt - 1].isMoveTo != polyline_lineto ) )
	{
		return -1;
	}
	int n = nbPt++;
	sizePt = nextSize;
	((path_lineto *) pts)[n].isMoveTo = polyline_forced;
	((path_lineto *) pts)[n].p = ((path_lineto *) pts)[n - 1].p;
	return n;
}
int
Path::AddForcedPoint (NR::Point const &iPt, int /*ip*/, double /*it*/)
{
	/* FIXME: ip & it aren't used.  Is this deliberate? */
	if (!back) {
		return AddForcedPoint (iPt);
	}
	int nextSize = sizePt + sizeof (path_lineto_b);
	if (nextSize > maxPt) {
		maxPt = 2 * sizePt + sizeof (path_lineto_b);
		pts = (char *) g_realloc(pts, maxPt);
	}
	if ( ( nbPt <= 0 )
	     || ( ((path_lineto_b *) pts)[nbPt - 1].isMoveTo != polyline_lineto ) )
	{
		return -1;
	}
	int n = nbPt++;
	sizePt = nextSize;
	((path_lineto_b *) pts)[n].isMoveTo = polyline_forced;
	((path_lineto_b *) pts)[n].p = ((path_lineto_b *) pts)[n - 1].p;
	((path_lineto_b *) pts)[n].piece = ((path_lineto_b *) pts)[n - 1].piece;
	((path_lineto_b *) pts)[n].t = ((path_lineto_b *) pts)[n - 1].t;
	return n;
}

void        
Path::PolylineBoundingBox(double &l,double &t,double &r,double &b)
{
  l=t=r=b=0.0;
  if ( nbPt <= 0 ) return;
  if ( back ) {
    path_lineto_b*  tp=(path_lineto_b*)pts;
    l=r=tp[0].p[0];
    t=b=tp[0].p[1];
    if ( nbPt <= 1 ) return;
    for (int i=1;i<nbPt;i++) {
      if ( tp[i].p[0] > r ) r=tp[i].p[0];
      if ( tp[i].p[0] < l ) l=tp[i].p[0];
      if ( tp[i].p[1] > b ) b=tp[i].p[1];
      if ( tp[i].p[1] < t ) t=tp[i].p[1];
    }
  } else {
    path_lineto*  tp=(path_lineto_b*)pts;
    l=r=tp[0].p[0];
    t=b=tp[0].p[1];
    if ( nbPt <= 1 ) return;
    for (int i=1;i<nbPt;i++) {
      if ( tp[i].p[0] > r ) r=tp[i].p[0];
      if ( tp[i].p[0] < l ) l=tp[i].p[0];
      if ( tp[i].p[1] > b ) b=tp[i].p[1];
      if ( tp[i].p[1] < t ) t=tp[i].p[1];
    }
  }
}

// utilities
void
Path::PointAt (int piece, double at, NR::Point & pos)
{
	if (piece < 0 || piece >= descr_nb)
	{
		// this shouldn't happen: the piece we are asked for doesn't
		// exist in the path
		pos = NR::Point(0,0);
		return;
	}
	path_descr theD = descr_cmd[piece];
	int typ = theD.flags & descr_type_mask;
	NR::Point tgt;
	double len, rad;
	if (typ == descr_moveto) {
		return PointAt (piece + 1, 0.0, pos);
	} else if (typ == descr_close || typ == descr_forced) {
		return PointAt (piece - 1, 1.0, pos);
	} else if (typ == descr_lineto) {
		path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + theD.dStart );
		TangentOnSegAt (at, PrevPoint (piece - 1), *nData, pos, tgt, len);
	} else if (typ == descr_arcto) {
		path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + theD.dStart );
		TangentOnArcAt (at,PrevPoint (piece - 1), *nData, pos, tgt, len, rad);
	} else if (typ == descr_cubicto) {
		path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + theD.dStart );
		TangentOnCubAt (at, PrevPoint (piece - 1), *nData, false, pos, tgt, len, rad);
	} else if (typ == descr_bezierto || typ == descr_interm_bezier) {
		int bez_st = piece;
		while (bez_st >= 0) {
			int nt = descr_cmd[bez_st].flags & descr_type_mask;
			if (nt == descr_bezierto)
				break;
			bez_st--;
		}
		if ( bez_st < 0 ) {
			// Didn't find the beginning of the spline (bad).
			// [pas trouvé le dubut de la spline (mauvais)]
			return PointAt(piece - 1, 1.0, pos);
		}
    
		path_descr_bezierto *stB = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[bez_st].dStart );
		if ( piece > bez_st + stB->nb ) {
			// The spline goes past the authorized number of commands (bad).
			// [la spline sort du nombre de commandes autorisé (mauvais)]
			return PointAt(piece - 1, 1.0, pos);
		}

		int k = piece - bez_st;
		NR::Point const bStPt = PrevPoint(bez_st - 1);
		if (stB->nb == 1 || k <= 0) {
			path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + 1].dStart );
			TangentOnBezAt(at, bStPt, *nData, *stB, false, pos, tgt, len, rad);
		} else {
			// forcement plus grand que 1
			if (k == 1) {
				path_descr_intermbezierto *nextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + 1].dStart );
				path_descr_intermbezierto *nnextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + 2].dStart );
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5 * ( nextI->p + nnextI->p );
				TangentOnBezAt(at, bStPt, *nextI,  fin, false, pos, tgt, len, rad);
			} else if (k == stB->nb) {
				path_descr_intermbezierto *nextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k].dStart );
				path_descr_intermbezierto *prevI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k - 1].dStart );
				NR::Point stP = 0.5 * ( prevI->p + nextI->p );
				TangentOnBezAt(at, stP, *nextI, *stB, false, pos, tgt, len, rad);
			} else {
				path_descr_intermbezierto *nextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k].dStart );
				path_descr_intermbezierto *prevI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k - 1].dStart );
				path_descr_intermbezierto *nnextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k + 1].dStart );
				NR::Point stP = 0.5 * ( prevI->p + nextI->p );
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5 * ( nextI->p + nnextI->p );
				TangentOnBezAt(at, stP, *nextI, fin, false, pos, tgt, len, rad);
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
		pos = NR::Point(0, 0);
		return;
	}
	path_descr theD = descr_cmd[piece];
	int typ = theD.flags & descr_type_mask;
	double len, rad;
	if (typ == descr_moveto) {
		return PointAndTangentAt (piece + 1, 0.0, pos,tgt);
	} else if (typ == descr_close ) {
    int cp=piece-1;
    while ( cp >= 0 && (descr_cmd[cp].flags&descr_type_mask) != descr_moveto ) cp--;
    if ( cp < 0 ) {
    } else {
      path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + descr_cmd[cp].dStart );
      path_descr_lineto  dst;
      dst.p=nData->p;
      TangentOnSegAt (at, PrevPoint (piece - 1), dst, pos, tgt, len);
    }
  } else if ( typ == descr_forced) {
		return PointAndTangentAt (piece - 1, 1.0, pos,tgt);
	} else if (typ == descr_lineto) {
		path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + theD.dStart );
		TangentOnSegAt (at, PrevPoint (piece - 1), *nData, pos, tgt, len);
	} else if (typ == descr_arcto) {
		path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + theD.dStart );
		TangentOnArcAt (at,PrevPoint (piece - 1), *nData, pos, tgt, len, rad);
	} else if (typ == descr_cubicto) {
		path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + theD.dStart );
		TangentOnCubAt (at, PrevPoint (piece - 1), *nData, false, pos, tgt, len, rad);
	} else if (typ == descr_bezierto || typ == descr_interm_bezier) {
		int bez_st = piece;
		while (bez_st >= 0) {
			int nt = descr_cmd[bez_st].flags & descr_type_mask;
			if (nt == descr_bezierto) break;
			bez_st--;
		}
		if ( bez_st < 0 ) {
			return PointAndTangentAt(piece - 1, 1.0, pos, tgt);
			// Didn't find the beginning of the spline (bad).
			// [pas trouvé le dubut de la spline (mauvais)]
		}
    
		path_descr_bezierto* stB = (path_descr_bezierto*)( descr_data + descr_cmd[bez_st].dStart );
		if ( piece > bez_st + stB->nb ) {
			return PointAndTangentAt(piece - 1, 1.0, pos, tgt);
			// The spline goes past the number of authorized commands (bad).
			// [la spline sort du nombre de commandes autorisé (mauvais)]
		}
    
		int k = piece - bez_st;
		NR::Point const bStPt(PrevPoint( bez_st - 1 ));
		if (stB->nb == 1 || k <= 0) {
			path_descr_intermbezierto* nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + 1].dStart );
			TangentOnBezAt (at, bStPt, *nData, *stB, false, pos, tgt, len, rad);
		} else {
			// forcement plus grand que 1
			if (k == 1) {
				path_descr_intermbezierto *nextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + 1].dStart );
				path_descr_intermbezierto *nnextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + 2].dStart );
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5 * ( nextI->p + nnextI->p );
				TangentOnBezAt(at, bStPt, *nextI, fin, false, pos, tgt, len, rad);
			} else if (k == stB->nb) {
				path_descr_intermbezierto *prevI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k - 1].dStart );
				path_descr_intermbezierto *nextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k].dStart );
				NR::Point stP = 0.5 * ( prevI->p + nextI->p );
				TangentOnBezAt(at, stP, *nextI, *stB, false, pos, tgt, len, rad);
			} else {
				path_descr_intermbezierto *prevI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k - 1].dStart );
				path_descr_intermbezierto *nextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k].dStart );
				path_descr_intermbezierto *nnextI = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[bez_st + k + 1].dStart );
				NR::Point stP = 0.5 * ( prevI->p + nextI->p );
				path_descr_bezierto fin;
				fin.nb = 1;
				fin.p = 0.5 * ( nextI->p + nnextI->p );
				TangentOnBezAt(at, stP, *nextI, fin, false, pos, tgt, len, rad);
			}
		}
	}
}

void Path::Transform(const NR::Matrix &trans)
{
    NR::Matrix trTrans = trans;
    trTrans[4] = trTrans[5] = 0;

    for (int i = 0; i < descr_nb; i++) {
        
        g_assert(descr_cmd[i].dStart < ddata_max);
        NR::Point *p = descr_data + descr_cmd[i].dStart;
        
        switch ( descr_cmd[i].flags & descr_type_mask ) {
      
            case descr_lineto:
            {
                path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>(p);
                nData->p = nData->p * trans;
            }
            break;
            
            case descr_moveto:
            {
                path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>(p);
                nData->p = nData->p * trans;
            }
            break;
            
            case descr_arcto:
            {
                path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>(p);
                nData->p = nData-> p * trans;
            }
            break;
            
            case descr_cubicto:
            {
                path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>(p);
                nData->stD = nData->stD * trTrans;
                nData->enD = nData->enD * trTrans;
                nData->p = nData->p * trans;
            }
            break;

            case descr_bezierto:
            {
                path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>(p);
                nData->p = nData->p * trans;
            }
            break;

            case descr_interm_bezier:
            {
                path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>(p);
                nData->p = nData->p * trans;
            }
            break;
        }
    }
}

void        Path::FastBBox(double &l,double &t,double &r,double &b)
{
	l=t=r=b=0;
	bool empty=true;
	NR::Point   lastP(0,0);
  for (int i=0;i<descr_nb;i++) {
    int typ=descr_cmd[i].flags&descr_type_mask;
    switch ( typ ) {
      case descr_lineto:
      {
        path_descr_lineto *nData = reinterpret_cast<path_descr_lineto *>( descr_data + descr_cmd[i].dStart );
				if ( empty ) {
					l=r=nData->p[0];
					t=b=nData->p[1];
          empty=false;
				} else {
					if ( nData->p[0] < l ) l=nData->p[0];
					if ( nData->p[0] > r ) r=nData->p[0];
					if ( nData->p[1] < t ) t=nData->p[1];
					if ( nData->p[1] > b ) b=nData->p[1];
				}
				lastP=nData->p;
      }
        break;
      case descr_moveto:
      {
        path_descr_moveto *nData = reinterpret_cast<path_descr_moveto *>( descr_data + descr_cmd[i].dStart );
				if ( empty ) {
					l=r=nData->p[0];
					t=b=nData->p[1];
         empty=false;
				} else {
					if ( nData->p[0] < l ) l=nData->p[0];
					if ( nData->p[0] > r ) r=nData->p[0];
					if ( nData->p[1] < t ) t=nData->p[1];
					if ( nData->p[1] > b ) b=nData->p[1];
				}
				lastP=nData->p;
      }
        break;
      case descr_arcto:
      {
        path_descr_arcto *nData = reinterpret_cast<path_descr_arcto *>( descr_data + descr_cmd[i].dStart );
				if ( empty ) {
					l=r=nData->p[0];
					t=b=nData->p[1];
         empty=false;
				} else {
					if ( nData->p[0] < l ) l=nData->p[0];
					if ( nData->p[0] > r ) r=nData->p[0];
					if ( nData->p[1] < t ) t=nData->p[1];
					if ( nData->p[1] > b ) b=nData->p[1];
				}
				lastP=nData->p;
      }
        break;
      case descr_cubicto:
      {
        path_descr_cubicto *nData = reinterpret_cast<path_descr_cubicto *>( descr_data + descr_cmd[i].dStart );
				if ( empty ) {
					l=r=nData->p[0];
					t=b=nData->p[1];
         empty=false;
				} else {
					if ( nData->p[0] < l ) l=nData->p[0];
					if ( nData->p[0] > r ) r=nData->p[0];
					if ( nData->p[1] < t ) t=nData->p[1];
					if ( nData->p[1] > b ) b=nData->p[1];
				}
				NR::Point np=nData->p-nData->enD;
				if ( np[0] < l ) l=np[0];
				if ( np[0] > r ) r=np[0];
				if ( np[1] < t ) t=np[1];
				if ( np[1] > b ) b=np[1];
				np=lastP+nData->stD;
				if ( np[0] < l ) l=np[0];
				if ( np[0] > r ) r=np[0];
				if ( np[1] < t ) t=np[1];
				if ( np[1] > b ) b=np[1];
				lastP=nData->p;
      }
        break;
      case descr_bezierto:
      {
        path_descr_bezierto *nData = reinterpret_cast<path_descr_bezierto *>( descr_data + descr_cmd[i].dStart );
				if ( empty ) {
					l=r=nData->p[0];
					t=b=nData->p[1];
         empty=false;
				} else {
					if ( nData->p[0] < l ) l=nData->p[0];
					if ( nData->p[0] > r ) r=nData->p[0];
					if ( nData->p[1] < t ) t=nData->p[1];
					if ( nData->p[1] > b ) b=nData->p[1];
				}
				lastP=nData->p;
      }
        break;
      case descr_interm_bezier:
      {
        path_descr_intermbezierto *nData = reinterpret_cast<path_descr_intermbezierto *>( descr_data + descr_cmd[i].dStart );
				if ( empty ) {
					l=r=nData->p[0];
					t=b=nData->p[1];
         empty=false;
				} else {
					if ( nData->p[0] < l ) l=nData->p[0];
					if ( nData->p[0] > r ) r=nData->p[0];
					if ( nData->p[1] < t ) t=nData->p[1];
					if ( nData->p[1] > b ) b=nData->p[1];
				}
      }
        break;
    }
  }
}

char *
Path::svg_dump_path ()
{
	Inkscape::SVGOStringStream os;
  
	for (int i = 0; i < descr_nb; i++) {

		Path::path_descr theD = descr_cmd[i];
		int typ = theD.flags & descr_type_mask;

		if (typ == descr_moveto) {
			Path::path_descr_moveto*  nData=(Path::path_descr_moveto*)(descr_data+theD.dStart);
			os << "M " << nData->p[0] << " " << nData->p[1] << " ";
		}

		else if (typ == descr_lineto) {
			Path::path_descr_lineto*  nData=(Path::path_descr_lineto*)(descr_data+theD.dStart);
			os << "L " << nData->p[0] << " " << nData->p[1] << " ";
		}

		else if (typ == descr_cubicto) {
			Path::path_descr_cubicto*  nData=(Path::path_descr_cubicto*)(descr_data+theD.dStart);
			float lastX, lastY;
			{
				NR::Point tmp = PrevPoint (i - 1);
				lastX=tmp[0];
				lastY=tmp[1];
			}
			os << "C " << lastX + nData->stD[0] / 3 << " "
				 << lastY + nData->stD[1] / 3 << " "
				 << nData->p[0] - nData->enD[0] / 3 << " "
				 << nData->p[1] - nData->enD[1] / 3 << " "
				 << nData->p[0] << " "
				 << nData->p[1] << " ";
		}

		else if (typ == descr_arcto) {
			Path::path_descr_arcto*  nData=(Path::path_descr_arcto*)(descr_data+theD.dStart);
			os << "A " << nData->rx << " "
				 << nData->ry << " "
				 << nData->angle << " "
				 << ((nData->large) ? "1" : "0") << " "
				 << ((nData->clockwise) ? "0" : "1") << " "
				 << nData->p[NR::X] << " "
				 << nData->p[NR::Y] << " ";
		}

		else if (typ == descr_close) {
			os << "z ";
		}
	}
  
	return g_strdup (os.str().c_str());
}

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
