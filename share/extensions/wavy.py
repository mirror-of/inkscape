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



drawwave() was translated into python from the postscript version
described at http://www.ghostscript.com/person/toby/ and located
at http://www.telegraphics.com.au/sw/sine.ps . 
http://www.tinaja.com/glib/bezsine.pdf shows another method for
approximating sine with beziers.

The orginal postscript version displayed the following copyright 
notice and was released under the terms of the GPL:
Copyright (C) 2001-3 Toby Thain, toby@telegraphics.com.au
'''
import inkex, simplepath, simplestyle
from math import *

def drawwave(period, length, width, height, left, top, 
		fx = "sin(x)", fpx = "cos(x)"):
	step = pi / period
	third = step / 3.0
	
	xoff = left
	yoff = top + (height / 2)
	scalex = width / (step * length)
	scaley = height / 2

	procx = lambda x: x * scalex + xoff
	procy = lambda y: y * scaley + yoff

	f = eval('lambda x: ' + fx)
	fp = eval('lambda x: ' + fpx)

	a = []
	a.append(['M',[procx(0.0), procy(f(0))]])
	for i in range(length):
		x = i * step
		a.append(['C',[procx(x + third), procy(f(x) + (fp(x) * third)), 
			procx(x + (step - third)), procy(f(x + step) - (fp(x + step) * third)),
			procx(x + step), procy(f(x + step))]])
	return a

class Wavy(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)
		self.OptionParser.add_option("-p", "--period",
						action="store", type="float", 
						dest="period", default=4.0,
						help="pi/period interval between samples")
		self.OptionParser.add_option("-l", "--length",
						action="store", type="int", 
						dest="length", default=8,
						help="length of the curve in periods")	
		self.OptionParser.add_option("--fofx",
						action="store", type="string", 
						dest="fofx", default="sin(x)",
						help="f(x) for plotting")	
		self.OptionParser.add_option("--fpofx",
						action="store", type="string", 
						dest="fpofx", default="cos(x)",
						help="f'(x) for plotting")	
	def effect(self):
		for id, node in self.selected.iteritems():
			if node.tagName == 'rect':
				new = self.document.createElement('svg:path')
				x = float(node.attributes.getNamedItem('x').value)
				y = float(node.attributes.getNamedItem('y').value)
				w = float(node.attributes.getNamedItem('width').value)
				h = float(node.attributes.getNamedItem('height').value)

				s = node.attributes.getNamedItem('style').value
				new.setAttribute('style', s)
				try:
					t = node.attributes.getNamedItem('transform').value
					new.setAttribute('transform', t)
				except AttributeError:
					pass
				new.setAttribute('d', simplepath.formatPath(
							drawwave(self.options.period, 
								self.options.length,
								w,h,x,y,
								self.options.fofx, 
								self.options.fpofx)))
				node.parentNode.appendChild(new)
				node.parentNode.removeChild(node)

e = Wavy()
e.affect()
