#!/usr/bin/env python 
import inkex, base64

#a dictionary of all of the xmlns prefixes in a standard inkscape doc
NSS = {u'sodipodi':u'http://inkscape.sourceforge.net/DTD/sodipodi-0.dtd',
u'cc':u'http://web.resource.org/cc/',
u'svg':u'http://www.w3.org/2000/svg',
u'dc':u'http://purl.org/dc/elements/1.1/',
u'rdf':u'http://www.w3.org/1999/02/22-rdf-syntax-ns#',
u'inkscape':u'http://www.inkscape.org/namespaces/inkscape',
u'xlink':u'http://www.w3.org/1999/xlink'}

class MyEffect(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)

	def effect(self):
		ctx = inkex.xml.xpath.Context.Context(self.document,processorNss=NSS)
		
		# if there is a selection only embed selected images
		# otherwise embed all images
		if (self.options.ids):
			for id, node in self.selected.iteritems():
				if node.tagName == 'image':
					self.embedImage(node)
		else:
			path = '//image'
			for node in inkex.xml.xpath.Evaluate(path,self.document, context=ctx):
				self.embedImage(node)
	def embedImage(self, node):
		xlink = node.attributes.getNamedItemNS(NSS[u'xlink'],'href')
		if (xlink.value[:4]!='data'):
			absref=node.attributes.getNamedItemNS(NSS[u'sodipodi'],'absref')
			file = open(absref.value,"rb").read()
			embed=True
			if (file[:4]=='\x89PNG'):
				type='image/png'
			elif (file[:2]=='\xff\xd8'):
				type='image/jpg'
			else:
				embed=False
			if (embed):
				xlink.value = 'data:%s;base64,%s' % (type, base64.encodestring(file))
				node.removeAttributeNS(NSS[u'sodipodi'],'absref')
		
e = MyEffect()
e.affect()
