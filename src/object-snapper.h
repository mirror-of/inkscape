#ifndef SEEN_OBJECT_SNAPPER_H
#define SEEN_OBJECT_SNAPPER_H

/**
 *  \file object-snapper.h
 *  \brief Snapping things to objects.
 *
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2005 Authors 
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "snapper.h"

struct SPNamedView;
struct SPItem;
struct SPObject;

namespace Inkscape
{

class ObjectSnapper : public Snapper
{
public:
  ObjectSnapper(SPNamedView const *nv, NR::Coord const d);

  NR::Coord vector_snap(PointType t,
			NR::Point &req,
			NR::Point const &d,
			SPItem const *i) const;

  void setSnapToNodes(bool s) {
    _snap_to_nodes = s;
  }

  bool getSnapToNodes() const {
    return _snap_to_nodes;
  }

  void setSnapToPaths(bool s) {
    _snap_to_paths = s;
  }

  bool getSnapToPaths() const {
    return _snap_to_paths;
  }
  
private:
  NR::Coord do_vector_snap(NR::Point &req, NR::Point const &d,
			   std::list<SPItem const *> const &it) const;
  NR::Coord do_free_snap(NR::Point &req, std::list<SPItem const *> const &it) const;
  
  void _find_candidates(std::list<SPItem*>& c,
			SPObject* r,
			std::list<SPItem const *> const &it,
			NR::Point const &p) const;
  void _snap_nodes(NR::Point &snapped, NR::Coord &best, NR::Coord &upper,
		   NR::Point const &req, std::list<SPItem*> const &cand) const;
  void _snap_paths(NR::Point &snapped, NR::Coord &best, NR::Coord &upper,
		   NR::Point const &req, std::list<SPItem*> const &cand) const;
  
  bool _snap_to_nodes;
  bool _snap_to_paths;
};

}

#endif
