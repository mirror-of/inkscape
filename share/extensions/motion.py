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
import math, inkex, simplestyle, simplepath, bezmisc

class Motion(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("-a", "--angle",
						action="store", type="float", 
						dest="angle", default=45.0,
						help="direction of the motion vector")
		self.OptionParser.add_option("-m", "--magnitude",
						action="store", type="float", 
						dest="magnitude", default=100.0,
						help="magnitude of the motion vector")	
		self.OptionParser.add_option("-e", "--ends",
						action="store", type="inkbool", 
						dest="ends", default=True,
						help="connect segment end points with motion vector")	
		self.OptionParser.add_option("-c", "--splits",
						action="store", type="inkbool", 
						dest="splits", default=True,
						help="connect split points with motion vector")	
		self.OptionParser.add_option("-s", "--split",
						action="store", type="inkbool", 
						dest="split", default=True,
						help="split curves where slope matches vectors")	
	def effect(self):
		vx = math.cos(math.radians(self.options.angle))*self.options.magnitude
		vy = math.sin(math.radians(self.options.angle))*self.options.magnitude
		for id, node in self.selected.iteritems():
			if node.tagName == 'path':
				group = self.document.createElement('svg:g')
				node.parentNode.appendChild(group)
				copy = self.document.createElement('svg:path')
				lines = self.document.createElement('svg:path')
				
				try:
					t = node.attributes.getNamedItem('transform').value
					group.setAttribute('transform', t)
					node.attributes.getNamedItem('transform').value=""
				except AttributeError:
					pass

				s = node.attributes.getNamedItem('style').value
				copy.setAttribute('style', s)
				lines.setAttribute('style', s)

				a = []
				l = []
				p = simplepath.parsePath(node.attributes.getNamedItem('d').value)
				for cmd,params in p:
					if cmd == 'C':
						bez = (last,params[:2],params[2:4],params[-2:])
						tees = [t for t in bezmisc.beziertatslope(bez,(vy,vx)) if 0<t<1]
						tees.sort()
						if self.options.splits:
							for t in tees:
								tx,ty = bezmisc.bezierpointatt(bez,t)
								l.append(['M',[tx,ty]])
								l.append(['L',[tx+vx,ty+vy]])					
						if len(tees) == 0 or not self.options.split:
							a.append([cmd,params])
						elif len(tees) == 1:
							one,two = bezmisc.beziersplitatt(bez,tees[0])
							a.append([cmd,list(one[1]+one[2]+one[3])])
							a.append([cmd,list(two[1]+two[2]+two[3])])
						elif len(tees) == 2:
							one,two = bezmisc.beziersplitatt(bez,tees[0])
							two,three = bezmisc.beziersplitatt(two,tees[1])
							a.append([cmd,list(one[1]+one[2]+one[3])])
							a.append([cmd,list(two[1]+two[2]+two[3])])
							a.append([cmd,list(three[1]+three[2]+three[3])])
					else:
						a.append([cmd,params])	
					
					if cmd == 'M':
						subPathStart = params[-2:]
					if cmd == 'Z':
						last = subPathStart
					else:
						last = params[-2:]
						if self.options.ends:
							l.append(['M',params[-2:]])
							l.append(['L',[params[-2]+vx,params[-1]+vy]])

				node.setAttribute('d', simplepath.formatPath(a))
				simplepath.translatePath(a, vx, vy)
				copy.setAttribute('d', simplepath.formatPath(a))
				lines.setAttribute('d', simplepath.formatPath(l))
				group.appendChild(copy)
				group.appendChild(lines)
				group.appendChild(node)

				

e = Motion()
e.affect()
