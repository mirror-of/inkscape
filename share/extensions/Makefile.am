
extensiondir = $(libdir)/inkscape/extensions

otherstuffdir = $(datadir)/inkscape/extensions

moduledir = $(datadir)/inkscape/modules

extensions = \
	roundhole \
	ill2svg.pl \
	SpSVG.pm \
	svg_dropshadow \
	dia2svg.sh \
	ps2epsi.sh \
	txt2svg.pl

otherstuff = \
	roundhole.xpm

modules = \
	svgz_input.inkmod \
	svgz_output.inkmod \
	ai_input.inkmod \
	roundhole.inkmod \
	dropshadow.inkmod \
	dia.inkmod \
	epsi_output.inkmod \
	txt2svg.inkmod

extension_SCRIPTS = \
	$(extensions)

otherstuff_DATA = \
	$(otherstuff)

module_DATA = \
	$(modules)

EXTRA_DIST = $(extension_SCRIPTS) $(otherstuff_DATA) $(module_DATA)

