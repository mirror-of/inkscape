#!/usr/bin/env python 
'''
Copyright (C) 2005 Aaron Spike, aaron@ekips.org

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
'''
import random, math, inkex, simplepath

def randomize((x, y), r):
	r = random.uniform(0.0,r)
	a = random.uniform(0.0,2*math.pi)
	x += math.cos(a)*r
	y += math.sin(a)*r
	return [x, y]

class RadiusRandomize(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("-r", "--radius",
						action="store", type="float", 
						dest="radius", default=10.0,
						help="Randomly move control and end points in this radius")
		self.OptionParser.add_option("-c", "--ctrl",
						action="store", type="inkbool", 
						dest="ctrl", default=True,
						help="Randomize control points")
		self.OptionParser.add_option("-e", "--end",
						action="store", type="inkbool", 
						dest="end", default=True,
						help="Randomize end points")
	def effect(self):
		for id, node in self.selected.iteritems():
			if node.tagName == 'path':
				d = node.attributes.getNamedItem('d')
				p = simplepath.parsePath(d.value)
				for cmd,params in p:
					if cmd == 'Z':
						continue
					if self.options.end:
						params[-2:]=randomize(params[-2:], self.options.radius)
					if self.options.ctrl:
						if cmd in ('C','Q'):
							params[:2]=randomize(params[:2], self.options.radius)
						if cmd == 'C':
							params[2:4]=randomize(params[2:4], self.options.radius)
				d.value = simplepath.formatPath(p)

e = RadiusRandomize()
e.affect()
