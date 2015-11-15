#!/usr/bin/env python
'''
Copyright (C) 2006 Aaron Spike, aaron@ekips.org
Copyright (C) 2007 Terry Brown, terry_n_brown@yahoo.com
Copyright (C) 2010 Nicolas Dufour, nicoduf@yahoo.fr (color options)
Copyright (C) 2014, 2015 ~suv <suv-sf@users.sf.net>

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
# standard library
import random
import copy
import re
import sys
# local library
import inkex
import simplestyle
import simpletransform
import voronoi2svg


try:
    inkex.localize()
except:
    import gettext
    _ = gettext.gettext


DEBUG = 0
MARKER_WRAP = 'marker_wrap'


def run(command_format, prog_name, stdin_str=None, verbose=False):
    """run command"""
    if verbose:
        inkex.debug(command_format)
    msg = None
    try:
        try:
            from subprocess import Popen, PIPE
            if isinstance(command_format, list):
                p = Popen(command_format, shell=False, stdin=PIPE, stdout=PIPE, stderr=PIPE)
                out, err = p.communicate(stdin_str)
            elif isinstance(command_format, str):
                p = Popen(command_format, shell=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
                out, err = p.communicate(stdin_str)
            else:
                msg = "unsupported command format %s" % type(command_format)
        except ImportError:
            msg = "subprocess.Popen not available"
        # if err and msg is None:
        #     msg = "%s failed:\n%s\n%s\n" % (prog_name, out, err)
    except Exception, inst:
        msg = "Error attempting to run %s: %s" % (prog_name, str(inst))
    if msg is None:
        return out
    else:
        inkex.errormsg(msg)
        sys.exit(1)


def query_bbox(svg_file, obj_id):
    '''
    parameters: svg file, id
    queries inkscape on the command line for x, y, width, height of object with id
    returns: list with x, y, width, height
    '''
    opts = ['inkscape', '--shell']
    stdin_str = ""
    for arg in ['x', 'y', 'width', 'height']:
        stdin_str += '--file={0} --query-id={1} --query-{2}\n'.format(svg_file, obj_id, arg)
    stdout_str = run(opts, 'inkscape', stdin_str, verbose=False).split('>')
    if len(stdout_str) >= 5:
        return stdout_str[1:5]


class CustomMarkers(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)
        self.OptionParser.add_option("--marker_fill",
                                     action="store", type="string",
                                     dest="marker_fill", default="stroke",
                                     help="Match markers' fill with object color")
        self.OptionParser.add_option("--marker_stroke",
                                     action="store", type="string",
                                     dest="marker_stroke", default="stroke",
                                     help="Match markers' stroke with object color")
        self.OptionParser.add_option("-c", "--context",
                                     action="store", type="inkbool",
                                     dest="context", default=False,
                                     help="Use context paint (SVG2)")
        self.OptionParser.add_option("-i", "--invert",
                                     action="store", type="inkbool",
                                     dest="invert", default=False,
                                     help="Invert fill and stroke colors")
        self.OptionParser.add_option("-a", "--alpha",
                                     action="store", type="inkbool",
                                     dest="assign_alpha", default=True,
                                     help="Assign the object fill and stroke alpha to the markers")
        self.OptionParser.add_option("--fill_type",
                                     action="store", type="string",
                                     dest="fill_type", default="assign",
                                     help="Match markers' fill with custom color")
        self.OptionParser.add_option("--fill_color",
                                     action="store", type="int",
                                     dest="fill_color", default=1364325887,
                                     help="Choose a custom fill color")
        self.OptionParser.add_option("--stroke_type",
                                     action="store", type="string",
                                     dest="stroke_type", default="assign",
                                     help="Match markers' stroke with custom color")
        self.OptionParser.add_option("--stroke_color",
                                     action="store", type="int",
                                     dest="stroke_color", default=1364325887,
                                     help="Choose a custom stroke color")
        self.OptionParser.add_option("-g", "--getput",
                                     action="store", type="string",
                                     dest="getput", default="Apply",
                                     help="Apply or Edit marker from path")
        self.OptionParser.add_option("-m", "--mid",
                                     action="store", type="inkbool",
                                     dest="mid", default=True,
                                     help="Act on mid-points")
        self.OptionParser.add_option("-s", "--start",
                                     action="store", type="inkbool",
                                     dest="start", default=True,
                                     help="Act on start-points")
        self.OptionParser.add_option("-e", "--end",
                                     action="store", type="inkbool",
                                     dest="end", default=False,
                                     help="Act on end-points")
        self.OptionParser.add_option("-o", "--orient",
                                     action="store", type="inkbool",
                                     dest="orient", default=True,
                                     help="Orient to line")
        self.OptionParser.add_option("--angle",
                                     action="store", type="float",
                                     dest="angle", default=0.0,
                                     help="Fixed angle for marker orientation")
        self.OptionParser.add_option("--scale",
                                     action="store", type="inkbool",
                                     dest="scale", default=True,
                                     help="Scale with stroke width")
        self.OptionParser.add_option("--modify_for_object",
                                     action="store", type="inkbool",
                                     dest="modify_for_object", default=False,
                                     help="Do not create a copy, modify the markers")
        self.OptionParser.add_option("--modify_for_custom",
                                     action="store", type="inkbool",
                                     dest="modify_for_custom", default=False,
                                     help="Do not create a copy, modify the markers")
        self.OptionParser.add_option("--debug",
                                     action="store", type="inkbool",
                                     dest="debug", default=False,
                                     help="Show debug messages")
        self.OptionParser.add_option("--tab",
                                     action="store", type="string",
                                     dest="tab",
                                     help="The selected UI-tab when OK was pressed")
        self.OptionParser.add_option("--colortab",
                                     action="store", type="string",
                                     dest="colortab",
                                     help="The selected cutom color tab when OK was pressed")
        self.OptionParser.add_option("--apply_edit_tab",
                                     action="store", type="string",
                                     dest="apply_edit_tab",
                                     help="The selected mode for Apply/Edit")
        self.msg = {
            'Apply': "The 'Apply' mode requires a path and a marker as selection.",
            'Edit': "The 'Edit' mode requires a single path with applied marker(s) as selection.",
            'Positions': "Nothing to do (no marker positions specified.)",
        }

    # --- Helper methods ---

    def get_defs(self, defs=None):
        defs = self.xpathSingle('/svg:svg//svg:defs')
        if defs is None:
            defs = inkex.etree.Element(inkex.addNS('defs', 'svg'))
            self.document.getroot().append(defs)
        return defs

    def get_options(self, opts, olist=[]):
        for i in opts:
            if getattr(self.options, i):
                olist.append(i)
        return olist

    def get_style(self, node):
        try:
            style = simplestyle.parseStyle(node.get('style'))
        except:
            inkex.errormsg(_("No style attribute found for id: %s") % node.get('id'))
            style = None
        return style

    def wrap_group(self, node, string):
        node_wrap = inkex.etree.Element(inkex.addNS('g', 'svg'))
        node_wrap.set('id', self.uniqueId(string))
        node_wrap.append(node)
        return node_wrap

    # --- Edit and Apply ---

    def edit_markers(self, path, defs, positions):
        style = self.get_style(path)
        if len(style):
            for position in positions:
                mprop = 'marker-' + position
                if mprop not in style:
                    continue
                target_id = style[mprop][5:-1]
                target = self.getElementById(target_id)
                for i in target.iterchildren():
                    if i.get('id').startswith(MARKER_WRAP):
                        self.current_layer.append(copy.deepcopy(i[0]))
                    else:
                        self.current_layer.append(copy.deepcopy(i))
                del style[mprop]
            path.set('style', simplestyle.formatStyle(style))
        else:
            inkex.errormsg(self.msg['Edit'])

    def get_marker_bbox(self, marker):
        vis_bbox = query_bbox(self.svg_file, marker.get('id'))
        if len(vis_bbox) == 4:
            x, y, width, height = [float(i) for i in vis_bbox]
            bbox_center = [x + width/2, y + height/2]
            # Compensate offsets of parent containers
            ident_mat = [[1.0, 0.0, 0.0], [0.0, 1.0, 0.0]]
            mat = simpletransform.composeParents(marker.getparent(), ident_mat)
            invert_mat = voronoi2svg.Voronoi2svg().invertTransform(mat)
            simpletransform.applyTransformToPoint(invert_mat, bbox_center)
            # wrap around preserved transforms of marker
            if marker.get('transform') is not None:
                marker = self.wrap_group(marker, MARKER_WRAP)
        else:
            inkex.errormsg("Inkscape query for visual bbox failed, falling back to geom bbox.")
            # wrap around preserved transforms of marker
            if marker.get('transform') is not None:
                marker = self.wrap_group(marker, MARKER_WRAP)
            xmin, xmax, ymin, ymax = simpletransform.computeBBox([marker])
            bbox_center = [xmin + (xmax - xmin)/2.0, ymin + (ymax - ymin)/2.0]
            stroke_factor = 1.2
            width = (xmax - xmin) * stroke_factor
            height = (ymax - ymin) * stroke_factor
        return marker, [width, height], bbox_center

    def apply_marker(self, marker, path, defs, positions):
        # get marker size & ref
        marker, bbox, ref_point = self.get_marker_bbox(marker)
        # create marker definition in defs
        marker_def = inkex.etree.SubElement(defs, inkex.addNS('marker', 'svg'))
        marker_def_id = self.uniqueId('marker')
        marker_def.set('id', marker_def_id)
        marker_def.set('markerWidth', str(bbox[0]))
        marker_def.set('markerHeight', str(bbox[1]))
        marker_def.set('refX', str(ref_point[0]))
        marker_def.set('refY', str(ref_point[1]))
        marker_def.set('overflow', 'visible')  # TODO: do we need this?
        if self.options.orient:
            marker_def.set('orient', 'auto')
        else:
            marker_def.set('orient', str(self.options.angle))
        if not self.options.scale:
            marker_def.set('markerUnits', 'userSpaceOnUse')
        # append marker content
        marker_def.append(marker)
        # make the path use it
        style = self.get_style(path)
        if len(style):
            for position in positions:
                mprop = 'marker-' + position
                style[mprop] = "url(#%s)" % marker_def_id
            path.set('style', simplestyle.formatStyle(style))
        else:
            inkex.errormsg(self.msg['Apply'])

    # --- Color Markers ---

    def get_colors_object(self, style):
        # object color
        obj_stroke = ('context-stroke' if self.options.context else style.get('stroke', '#000000'))
        obj_fill = ('context-fill' if self.options.context else style.get('fill', '#000000'))
        # object opacity
        stroke_opacity = (style.get('stroke-opacity', '1') if self.options.assign_alpha else None)
        fill_opacity = (style.get('fill-opacity', '1') if self.options.assign_alpha else None)
        # marker fill
        if self.options.marker_fill == "fill":
            fill = (obj_fill if not self.options.invert else obj_stroke)
        elif self.options.marker_fill == "stroke":
            fill = (obj_stroke if not self.options.invert else obj_fill)
        elif self.options.marker_fill == "none":
            fill = 'none'
        else:  # keep
            fill = None
        # marker stroke
        if self.options.marker_stroke == "stroke":
            stroke = (obj_stroke if not self.options.invert else obj_fill)
        elif self.options.marker_stroke == "fill":
            stroke = (obj_fill if not self.options.invert else obj_stroke)
        elif self.options.marker_stroke == "none":
            stroke = 'none'
        else:  # keep
            stroke = None
        # all set
        return {'fill': fill, 'stroke': stroke,
                'fill-opacity': fill_opacity,
                'stroke-opacity': stroke_opacity}

    def get_colors_custom(self):
        # marker opacity
        fill_opacity = None
        stroke_opacity = None
        # marker fill
        if self.options.fill_type == "assign":
            fill_red = ((self.options.fill_color >> 24) & 255)
            fill_green = ((self.options.fill_color >> 16) & 255)
            fill_blue = ((self.options.fill_color >> 8) & 255)
            fill = "rgb(%s,%s,%s)" % (fill_red, fill_green, fill_blue)
            fill_opacity = (((self.options.fill_color) & 255) / 255.0)
        elif self.options.fill_type == "cstroke":
            fill = 'context-stroke'
        elif self.options.fill_type == "cfill":
            fill = 'context-fill'
        elif self.options.fill_type == "none":
            fill = 'none'
        else:  # keep
            fill = None
        # marker stroke
        if self.options.stroke_type == "assign":
            stroke_red = ((self.options.stroke_color >> 24) & 255)
            stroke_green = ((self.options.stroke_color >> 16) & 255)
            stroke_blue = ((self.options.stroke_color >> 8) & 255)
            stroke = "rgb(%s,%s,%s)" % (stroke_red, stroke_green, stroke_blue)
            stroke_opacity = (((self.options.stroke_color) & 255) / 255.0)
        elif self.options.stroke_type == "cstroke":
            stroke = 'context-stroke'
        elif self.options.stroke_type == "cfill":
            stroke = 'context-fill'
        elif self.options.stroke_type == "none":
            stroke = 'none'
        else:  # keep
            stroke = None
        # all set
        return {'fill': fill, 'stroke': stroke,
                'fill-opacity': fill_opacity,
                'stroke-opacity': stroke_opacity}

    def get_marker_def(self, defs, style, mprop):
        marker_def = None
        if mprop in style and style[mprop] != 'none' and style[mprop][:5] == 'url(#':
            marker_id = style[mprop][5:-1]
            try:
                orig_marker_def = self.xpathSingle('/svg:svg//svg:marker[@id="%s"]' % marker_id)
                if not self.options.modify:
                    marker_def = copy.deepcopy(orig_marker_def)
                else:
                    marker_def = orig_marker_def
            except:
                inkex.errormsg(_("unable to locate marker: %s") % marker_id)
            if marker_def is not None:
                new_id = self.uniqueId(marker_id, not self.options.modify)
                style[mprop] = "url(#%s)" % new_id
                marker_def.set('id', new_id)
                marker_def.set(inkex.addNS('stockid', 'inkscape'), new_id)
                defs.append(marker_def)
        return (style, marker_def)

    def set_marker_style(self, node, markerstyle):
        cstyle = simplestyle.parseStyle(node.get('style'))
        # marker fill
        if 'fill' in cstyle:  # does marker have fill set?
            if DEBUG:
                inkex.debug("id: %s, fill: %s" % (node.get('id'), cstyle['fill']))
            if not cstyle['fill'] == "none" and markerstyle['fill'] is not None:
                cstyle['fill'] = markerstyle['fill']
        elif markerstyle['fill'] is not None:  # unset fill is black
            cstyle['fill'] = markerstyle['fill']
        # marker stroke
        if 'stroke' in cstyle:  # does marker have stroke set?
            if DEBUG:
                inkex.debug("id: %s, stroke: %s" % (node.get('id'), cstyle['stroke']))
            if not cstyle['stroke'] == "none" and markerstyle['stroke'] is not None:
                cstyle['stroke'] = markerstyle['stroke']
        else:  # unset stroke is not visible
            pass
        # marker opacity
        for prop in ('fill-opacity', 'stroke-opacity'):
            if markerstyle[prop] is not None:
                cstyle[prop] = markerstyle[prop]
        node.set('style', simplestyle.formatStyle(cstyle))

    def color_markers(self, defs, style, markerstyle):
        marker_props = ['marker', 'marker-start', 'marker-mid', 'marker-end']
        for marker_prop in marker_props:
            style, marker_def = self.get_marker_def(defs, style, marker_prop)
            if marker_def is not None:
                marker = marker_def.xpath('.//*[@style]', namespaces=inkex.NSS)
                for node in marker:
                    self.set_marker_style(node, markerstyle)
        return style

    # --- main effect ---

    def effect(self):

        global DEBUG
        DEBUG = self.options.debug

        defs = self.get_defs()

        if self.options.tab == '"apply_edit"':
            # Edit and apply markers
            if len(self.options.ids) >= 1:
                positions = self.get_options(['start', 'mid', 'end'])
                if len(positions):
                    path = self.selected[self.options.ids[0]]
                    if self.options.apply_edit_tab == '"apply"':
                        if len(self.options.ids) == 2:
                            marker = self.selected[self.options.ids[1]]
                            self.apply_marker(marker, path, defs, positions)
                        else:  # no marker selected
                            inkex.errormsg(self.msg['Apply'])
                    elif self.options.apply_edit_tab == '"edit"':
                        self.edit_markers(path, defs, positions)
                else:  # no marker positions active
                    inkex.errormsg(self.msg['Positions'])
            else:  # nothing selected
                inkex.errormsg(self.msg[self.options.getput])

        else:
            # Apply custom colors
            if self.options.tab == '"custom"':
                markerstyle = self.get_colors_custom()
                self.options.modify = self.options.modify_for_custom
            for id_, node in self.selected.iteritems():
                style = self.get_style(node)
                # Use object colors
                if self.options.tab == '"object"':
                    markerstyle = self.get_colors_object(style)
                    self.options.modify = self.options.modify_for_object
                # modify marker styles, return object style with new ref
                style = self.color_markers(defs, style, markerstyle)
                # update object style with new marker ref
                node.set('style', simplestyle.formatStyle(style))


if __name__ == '__main__':
    e = CustomMarkers()
    e.affect()

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99
