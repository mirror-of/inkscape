#!/usr/bin/env python 
import inkex, os, base64

class MyEffect(inkex.Effect):
	def __init__(self):
		inkex.Effect.__init__(self)

	def effect(self):
		ctx = inkex.xml.xpath.Context.Context(self.document,processorNss=inkex.NSS)
		
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
		xlink = node.attributes.getNamedItemNS(inkex.NSS[u'xlink'],'href')
		if (xlink.value[:4]!='data'):
			absref=node.attributes.getNamedItemNS(inkex.NSS[u'sodipodi'],'absref')
			if (os.path.isfile(absref.value)):
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
					node.removeAttributeNS(inkex.NSS[u'sodipodi'],'absref')
				else:
					inkex.debug("%s is not of type image/png or image/jpg" % absref.value)
			else:
				inkex.debug("Sorry we could not locate %s" % absref.value)
e = MyEffect()
e.affect()
