
extensiondir = $(datadir)/inkscape/extensions

otherstuffdir = $(datadir)/inkscape/extensions

moduledir = $(datadir)/inkscape/extensions

extensions = \
	ill2svg.pl \
	SpSVG.pm \
	svg_dropshadow \
	dia2svg.sh \
	sk2svg.sh \
	ps2epsi.sh \
	ps2pdf.sh \
	txt2svg.pl \
	inkscape-shadow.sh \
	inkscape-shadow-white.sh \
	embed_raster_in_svg.pl

otherstuff =

modules = \
	svgz_input.inx \
	svgz_output.inx \
	ai_input.inx \
	ai_output.inx \
	ps_input.inx \
	eps_input.inx \
	sk_input.inx \
	wmf_input.inx \
	dropshadow.inx \
	dia.inx \
	epsi_output.inx \
	pdf_output.inx \
	txt2svg.inx \
	gimpgrad.inx \
	grid.inx \
	bluredge.inx \
	randompnt.inx \
	randompos.inx

extension_SCRIPTS = \
	$(extensions)

otherstuff_DATA = \
	$(otherstuff)

module_DATA = \
	$(modules)

EXTRA_DIST = $(extension_SCRIPTS) $(otherstuff_DATA) $(module_DATA)

