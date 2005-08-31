/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libavoid - Fast, Incremental, Object-avoiding Line Router
 * Copyright (C) 2004-2005  Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
*/

#include "libavoid/connector.h"
#include "libavoid/graph.h"
#include "libavoid/makepath.h"
#include "libavoid/visibility.h"
#include "libavoid/debug.h"


namespace Avoid {

    
ConnRefList connRefs;


ConnRef::ConnRef(const uint id)
    : _id(id)
    , _needs_reroute_flag(false)
    , _false_path(false)
    , _active(false)
    , _route_dist(0)
    , _srcVert(NULL)
    , _dstVert(NULL)
{
    // TODO: Store endpoints and details.
    _route.pn = 0;
    _route.ps = NULL;

    makeActive();
}


ConnRef::ConnRef(const uint id, const Point& src, const Point& dst)
    : _id(id)
    , _needs_reroute_flag(false)
    , _active(false)
    , _route_dist(0)
    , _srcVert(NULL)
    , _dstVert(NULL)
{
    _route.pn = 0;
    _route.ps = NULL;

    if (IncludeEndpoints)
    {
        bool isShape = false;
        _srcVert = new VertInf(VertID(id, isShape, 1), src);
        _dstVert = new VertInf(VertID(id, isShape, 2), dst);
        vertices.addVertex(_srcVert);
        vertices.addVertex(_dstVert);
        makeActive();
    }
}


ConnRef::~ConnRef()
{
    freeRoute();

    if (_srcVert)
    {
        vertices.removeVertex(_srcVert);
        delete _srcVert;
        _srcVert = NULL;
    }

    if (_dstVert)
    {
        vertices.removeVertex(_dstVert);
        delete _dstVert;
        _dstVert = NULL;
    }

    if (_active)
    {
        makeInactive();
    }
}

void ConnRef::updateEndPoint(const uint type, const Point& point)
{
    assert((type == (uint) VertID::src) || (type == (uint) VertID::tar));
    //assert(IncludeEndpoints);

    VertInf *altered = NULL;
    VertInf *partner = NULL;
    bool isShape = false;

    if (type == (uint) VertID::src)
    {
        if (_srcVert)
        {
            _srcVert->Reset(point);
        }
        else
        {
            _srcVert = new VertInf(VertID(_id, isShape, type), point);
            vertices.addVertex(_srcVert);
        }
        
        altered = _srcVert;
        partner = _dstVert;
    }
    else // if (type == (uint) VertID::dst)
    {
        if (_dstVert)
        {
            _dstVert->Reset(point);
        }
        else
        {
            _dstVert = new VertInf(VertID(_id, isShape, type), point);
            vertices.addVertex(_dstVert);
        }
        
        altered = _dstVert;
        partner = _srcVert;
    }

    bool knownNew = false;
    vertexVisibility(altered, partner, knownNew, true);
}


void ConnRef::makeActive(void)
{
    assert(!_active);
    
    // Add to connRefs list.
    _pos = connRefs.insert(connRefs.begin(), this);
    _active = true;
}


void ConnRef::makeInactive(void)
{
    assert(_active);
    
    // Remove from connRefs list.
    connRefs.erase(_pos);
    _active = false;
}


void ConnRef::freeRoute(void)
{
    if (_route.ps)
    {
        _route.pn = 0;
        std::free(_route.ps);
        _route.ps = NULL;
    }
}
    

PolyLine& ConnRef::route(void)
{
    return _route;
}


void ConnRef::calcRouteDist(void)
{
    _route_dist = 0;
    for (int i = 1; i < _route.pn; i++)
    {
        _route_dist += dist(_route.ps[i], _route.ps[i - 1]);
    }
}


bool& ConnRef::needs_reroute(void)
{
    return _needs_reroute_flag;
}


void ConnRef::needs_reroute(const bool value)
{
    if (_false_path)
    {   
        // Override cancelling the needs_reroute flag
        _needs_reroute_flag = true;
        _false_path = false;
    }
    else
    {
        _needs_reroute_flag = value;
    }
}


void ConnRef::moveRoute(const int& diff_x, const int& diff_y)
{
    for (int i = 0; i < _route.pn; i++)
    {
        _route.ps[i].x += diff_x;
        _route.ps[i].y += diff_y;
    }
}


void ConnRef::lateSetup(const Point& src, const Point& dst)
{
    bool isShape = false;
    _srcVert = new VertInf(VertID(_id, isShape, 1), src);
    _dstVert = new VertInf(VertID(_id, isShape, 2), dst);
    vertices.addVertex(_srcVert);
    vertices.addVertex(_dstVert);
    makeActive();
}


VertInf *ConnRef::src(void)
{
    return _srcVert;
}

    
VertInf *ConnRef::dst(void)
{
    return _dstVert;
}


void ConnRef::removeFromGraph(void)
{
    for (VertInf *iter = _srcVert; iter != NULL; )
    {
        VertInf *tmp = iter;
        iter = (iter == _srcVert) ? _dstVert : NULL;
        
        // For each vertex.
        EdgeInfList& visList = tmp->visList;
        EdgeInfList::iterator finish = visList.end();
        EdgeInfList::iterator edge;
        while ((edge = visList.begin()) != finish)
        {
            // Remove each visibility edge
            delete (*edge);
        }

        EdgeInfList& invisList = tmp->invisList;
        finish = invisList.end();
        while ((edge = invisList.begin()) != finish)
        {
            // Remove each invisibility edge
            delete (*edge);
        }
    }
}

void ConnRef::markAsFalsePath(void)
{
    _false_path = true;
}

//============================================================================

int ObstaclePath(Point p0, Point p1, ConnRef *lineRef)
{
    VertInf *src = lineRef->src();
    VertInf *tar = lineRef->dst();
   
    if (!IncludeEndpoints)
    {
        lineRef->lateSetup(p0, p1);
        
        // Update as they have just been set by lateSetup.
        src = lineRef->src();
        tar = lineRef->dst();
   
        bool knownNew = true;
        vertexVisibility(src, tar, knownNew);
        vertexVisibility(tar, src, knownNew);
    }

    bool *flag = &(lineRef->needs_reroute());
    
    makePath(lineRef, flag);
    
    bool result = true;
    
    int pathlen = 1;
    for (VertInf *i = tar; i != src; i = i->pathNext)
    {
        pathlen++;
        if (i == NULL)
        {
            db_printf("Warning: Path not found...\n");
            pathlen = 2;
            tar->pathNext = src;
            if (InvisibilityGrph)
            {
                // TODO:  Could we know this edge already?
                EdgeInf *edge = EdgeInf::existingEdge(src, tar);
                assert(edge != NULL);
                edge->addCycleBlocker();
            }
            result = false;
            break;
        }
        if (pathlen > 100)
        {
            fprintf(stderr, "ERROR: Should never be here...\n");
            exit(1);
        }
    }
    Point *path = (Point *) malloc(pathlen * sizeof(Point));

    int j = pathlen - 1;
    for (VertInf *i = tar; i != src; i = i->pathNext)
    {
        if (InvisibilityGrph)
        {
            // TODO: Again, we could know this edge without searching.
            EdgeInf *edge = EdgeInf::existingEdge(i, i->pathNext);
            edge->addConn(flag);
        }
        else
        {
            lineRef->markAsFalsePath();
        }
        path[j--] = i->point;
    }
    path[0] = src->point;


    // Would clear visibility for endpoints here if required.

    PolyLine& output_route = lineRef->route();
    output_route.pn = pathlen;
    output_route.ps = path;
   
    return (int) result;
}


}


