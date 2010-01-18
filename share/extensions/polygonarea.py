#!/usr/bin/env python     

'''
Copyright (C) 2010 Helmut Eller, meller333 <at> gmail.com.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

'''

# These two lines are only needed if you don't put the script directly into
# the installation directory
import sys
sys.path.append('/usr/share/inkscape/extensions')

# We will use the inkex module with the predefined Effect base class.
import inkex, simplepath, simplestyle
from inkex import etree

def polygon_area(path):
    """Return the area of the (closed) polygon PATH.
PATH should have the form: [(x,y)...]
Example: polygonArea([(0,0),(1,1),(2,0)]) => 1.0
See: http://en.wikipedia.org/wiki/Polygon#Area_and_centroid
"""
    return 0.5 * sum((xi - xj) * (yi + yj)
                     for ((xi,yi), (xj,yj)) in zip(path, path[1:] + [path[0]]))

assert polygon_area([(0,0),(1,1),(2,0)]) == -1.0
assert polygon_area([(0,0),(2,0),(1,1)]) == +1.0

def polygon_centroid(path):
    "Return the centroid of the polygon given with PATH"
    area = polygon_area(path)
    points = zip(path, path[1:] + [path[0]])
    cx = 1/(6*area) * sum((xi + xj)*(xi*yj - xj*yi)
                          for ((xi,yi), (xj,yj)) in points)
    cy = 1/(6*area) * sum((yi + yj)*(xi*yj - xj*yi)
                          for ((xi,yi), (xj,yj)) in points)
    return (cx,cy)
    
assert polygon_centroid([(0,0),(0,2),(2,2),(2,0)]) == (1,1)
assert polygon_centroid([(0,0),(2,0),(2,2),(0,2)]) == (1,1)

def add_text(parent, x, y, string):
    "Add a text element at position (X,Y) to PARENT."
    text = etree.Element(inkex.addNS('text','svg'))
    text.text = string
    text.set('x', str(x))
    text.set('y', str(y))
    # Center text horizontally with CSS style.
    text.set('style', simplestyle.formatStyle({'text-align' : 'center', 
                                               'text-anchor': 'middle'}))
    parent.append(text)

def is_path_valid(path):
    "Return true if path looks like [['M',[x,y]],['L',[x,y]]...,['Z',[]]]"
    if len(path) < 3: return False
    first = path[0]
    last = path[-1]
    if not (first[0] == 'M' and is_valid_point(first[1])): return False
    if not (last[0] in "Zz" and last[1]==[]): return False
    for [cmd, args] in path[1:-1]:
        if not (cmd == 'L' and is_valid_point(args)):
            return False
    return True

def is_valid_point(args):
    return (len(args)==2 
            and (isinstance(args[0], int) or isinstance(args[0], float))
            and (isinstance(args[1], int) or isinstance(args[1], float)))
                
UNITS = {"px": 1,                   # px->px
         "mm": 0.2822219,           # px->mm
         "pt": 0.80,                # px->pt
         "cm": 0.02822219,          # px->cm
         "m": 0.0002822219,         # px->m
         "km": 0.0000002822219,     # px->km
         "in": 0.2822219/25.4,      # px->in
         "ft": 0.2822219/(25.4*12), # px->ft
         "yd": 0.2822219/(25.4*36)  # px->yd
         }

class PolygonAreaEffect(inkex.Effect):
    "Inkscape effect extension to compute polygon area."

    def __init__(self):
        inkex.Effect.__init__(self)
        opts =  self.OptionParser
        opts.add_option("-p", "--precision",
                        action="store", type="int", dest="precision",
                        default=2, help="Number of digits after decimal point")
        opts.add_option("-u", "--unit",
                        action="store", type="string", 
                        dest="unit", default="mm",
                        help="The unit of the measurement")
        opts.add_option("-s", "--scale",
                        action="store", type="float", 
                        dest="scale", default=1,
                        help="Scale factor of drawing/real length")

    def effect(self):
        # debug = open("/tmp/ink.tmp.xml", "w")
        # print >> debug, self.selected
        for name,node in self.selected.iteritems():
            # print >> debug, etree.tostring(node, pretty_print=True)
            path = simplepath.parsePath(node.get("d"))
            # print >> debug, path
            if not is_path_valid(path):
                sys.stderr.write("Selected object is not a valid polygon:\n"
                                 + str(path))
                return
            points = [(x,y) for [cmd,[x,y]] in path[:-1]]
            area = polygon_area(points)
            (cx,cy) = polygon_centroid(points)
            # print >> debug, points
            # print >> debug, area
            # print >> debug, (cx, cy)
            opts = self.options
            factor = opts.scale * UNITS[opts.unit]
            label = (("%." + str(opts.precision) + u"f %s\u00b2")
                     % (abs(area) * (factor**2), opts.unit))
            add_text(node.getparent(), cx, cy, label)
        # debug.flush()
        # debug.close()

    def output(self):
        "Override it to set encoding"
        self.document.write(sys.stdout, encoding="utf-8")

PolygonAreaEffect().affect()
