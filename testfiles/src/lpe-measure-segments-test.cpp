// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * LPE Boolean operation test
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2020 Authors
 *
 * Released under GNU GPL version 2 or later, read the file 'COPYING' for more information
 */

#include <gtest/gtest.h>
#include <testfiles/lpespaths-test.h>
#include <src/inkscape.h>

using namespace Inkscape;
using namespace Inkscape::LivePathEffect;

class LPEMeasureSegmentsTest : public LPESPathsTest {};

// INKSCAPE 1.0.2

TEST_F(LPEMeasureSegmentsTest, multi_PX_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="793.70081"
   height="1122.5197"
   viewBox="0 0 793.70081 1122.5197"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="1.svg">
  <style
     id="style837">
.measure-arrow
{
}
.measure-label
{

}
.measure-line
{
}</style>
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="measure_segments"
       id="path-effect950"
       is_visible="true"
       lpeversion="1"
       unit="mm"
       orientation="parallel"
       coloropacity="#000000ff"
       fontbutton="Sans 10"
       precision="2"
       fix_overlaps="0"
       position="5"
       text_top_bottom="0"
       helpline_distance="0"
       helpline_overlap="2"
       line_width="0.25"
       scale="1"
       format="{measure}{unit}"
       blacklist=""
       active_projection="false"
       whitelist="false"
       showindex="false"
       arrows_outside="false"
       flip_side="false"
       scale_sensitive="true"
       local_locale="true"
       rotate_anotation="true"
       hide_back="true"
       hide_arrows="false"
       smallx100="false"
       linked_items=""
       distance_projection="20"
       angle_projection="0"
       avoid_overlapping="true"
       onbbox="false"
       bboxonly="false"
       centers="false"
       maxmin="false"
       helpdata="&lt;b&gt;&lt;big&gt;General&lt;/big&gt;&lt;/b&gt; Display and position dimension lines and labels  &lt;b&gt;&lt;big&gt;Projection&lt;/big&gt;&lt;/b&gt; Show a line with measurements based on the selected items  &lt;b&gt;&lt;big&gt;Options&lt;/big&gt;&lt;/b&gt; Options for color, precision, label formatting and display  &lt;b&gt;&lt;big&gt;Tips&lt;/big&gt;&lt;/b&gt; &lt;b&gt;&lt;i&gt;Custom styling:&lt;/i&gt;&lt;/b&gt; To further customize the styles, use the XML editor to find out the class or ID, then use the Style dialog to apply a new style. &lt;b&gt;&lt;i&gt;Blacklists:&lt;/i&gt;&lt;/b&gt; allow to hide some segments or projection steps. &lt;b&gt;&lt;i&gt;Multiple Measure LPEs:&lt;/i&gt;&lt;/b&gt; In the same object, in conjunction with blacklists,this allows for labels and measurements with different orientations or additional projections. &lt;b&gt;&lt;i&gt;Set Defaults:&lt;/i&gt;&lt;/b&gt; For every LPE, default values can be set at the bottom." />
    <inkscape:path-effect
       effect="measure_segments"
       id="path-effect910"
       is_visible="true"
       lpeversion="1"
       unit="mm"
       orientation="parallel"
       coloropacity="#000000ff"
       fontbutton="Sans 10"
       precision="2"
       fix_overlaps="0"
       position="20"
       text_top_bottom="0"
       helpline_distance="0"
       helpline_overlap="20"
       line_width="0.25"
       scale="1"
       format="{measure}{unit}"
       blacklist=""
       active_projection="false"
       whitelist="false"
       showindex="true"
       arrows_outside="true"
       flip_side="false"
       scale_sensitive="true"
       local_locale="true"
       rotate_anotation="true"
       hide_back="true"
       hide_arrows="false"
       smallx100="true"
       linked_items=""
       distance_projection="20"
       angle_projection="0"
       avoid_overlapping="true"
       onbbox="false"
       bboxonly="false"
       centers="false"
       maxmin="false"
       helpdata="&lt;b&gt;&lt;big&gt;General&lt;/big&gt;&lt;/b&gt; Display and position dimension lines and labels  &lt;b&gt;&lt;big&gt;Projection&lt;/big&gt;&lt;/b&gt; Show a line with measurements based on the selected items  &lt;b&gt;&lt;big&gt;Options&lt;/big&gt;&lt;/b&gt; Options for color, precision, label formatting and display  &lt;b&gt;&lt;big&gt;Tips&lt;/big&gt;&lt;/b&gt; &lt;b&gt;&lt;i&gt;Custom styling:&lt;/i&gt;&lt;/b&gt; To further customize the styles, use the XML editor to find out the class or ID, then use the Style dialog to apply a new style. &lt;b&gt;&lt;i&gt;Blacklists:&lt;/i&gt;&lt;/b&gt; allow to hide some segments or projection steps. &lt;b&gt;&lt;i&gt;Multiple Measure LPEs:&lt;/i&gt;&lt;/b&gt; In the same object, in conjunction with blacklists,this allows for labels and measurements with different orientations or additional projections. &lt;b&gt;&lt;i&gt;Set Defaults:&lt;/i&gt;&lt;/b&gt; For every LPE, default values can be set at the bottom." />
    <inkscape:path-effect
       effect="measure_segments"
       id="path-effect835"
       is_visible="true"
       lpeversion="1"
       unit="mm"
       orientation="parallel"
       coloropacity="#0000c9ff"
       fontbutton="Sans 10"
       precision="3"
       fix_overlaps="0"
       position="20"
       text_top_bottom="0"
       helpline_distance="0"
       helpline_overlap="20"
       line_width="0.25"
       scale="3"
       format="{measure}{unit}"
       blacklist=""
       active_projection="false"
       whitelist="false"
       showindex="false"
       arrows_outside="false"
       flip_side="true"
       scale_sensitive="true"
       local_locale="true"
       rotate_anotation="true"
       hide_back="true"
       hide_arrows="false"
       smallx100="true"
       linked_items=""
       distance_projection="20"
       angle_projection="0"
       avoid_overlapping="true"
       onbbox="false"
       bboxonly="false"
       centers="false"
       maxmin="false"
       helpdata="&lt;b&gt;&lt;big&gt;General&lt;/big&gt;&lt;/b&gt; Display and position dimension lines and labels  &lt;b&gt;&lt;big&gt;Projection&lt;/big&gt;&lt;/b&gt; Show a line with measurements based on the selected items  &lt;b&gt;&lt;big&gt;Options&lt;/big&gt;&lt;/b&gt; Options for color, precision, label formatting and display  &lt;b&gt;&lt;big&gt;Tips&lt;/big&gt;&lt;/b&gt; &lt;b&gt;&lt;i&gt;Custom styling:&lt;/i&gt;&lt;/b&gt; To further customize the styles, use the XML editor to find out the class or ID, then use the Style dialog to apply a new style. &lt;b&gt;&lt;i&gt;Blacklists:&lt;/i&gt;&lt;/b&gt; allow to hide some segments or projection steps. &lt;b&gt;&lt;i&gt;Multiple Measure LPEs:&lt;/i&gt;&lt;/b&gt; In the same object, in conjunction with blacklists,this allows for labels and measurements with different orientations or additional projections. &lt;b&gt;&lt;i&gt;Set Defaults:&lt;/i&gt;&lt;/b&gt; For every LPE, default values can be set at the bottom."
       coloropacity_opacity_LPE="1" />
    <marker
       id="ArrowDIN-start"
       class="rect833 path-effect835 measure-arrow-marker"
       inkscape:stockid="ArrowDIN-start"
       orient="auto"
       refX="0"
       refY="0"
       sodipodi:insensitive="true">
      <path
         d="M -8,0 8,-2.11 v 4.22 z"
         class="rect833 path-effect835 measure-arrow"
         id="ArrowDIN-start_path"
         style="fill:context-stroke;;fill-opacity:1;stroke:none" />
    </marker>
    <marker
       id="ArrowDIN-end"
       class="rect833 path-effect835 measure-arrow-marker"
       inkscape:stockid="ArrowDIN-end"
       orient="auto"
       refX="0"
       refY="0"
       sodipodi:insensitive="true">
      <path
         d="M 8,0 -8,2.11 v -4.22 z"
         class="rect833 path-effect835 measure-arrow"
         id="ArrowDIN-end_path"
         style="fill:context-stroke;;fill-opacity:1;stroke:none" />
    </marker>
    <marker
       id="ArrowDINout-start"
       class="rect833 path-effect835 measure-arrow-marker"
       inkscape:stockid="ArrowDINout-start"
       orient="auto"
       refX="0"
       refY="0"
       sodipodi:insensitive="true">
      <path
         d="M 0,0 -16,2.11 V 0.5 h -10 v -1 h 10 v -1.61 z"
         class="rect833 path-effect835 measure-arrow"
         id="ArrowDINout-start_path"
         style="fill:context-stroke;;fill-opacity:1;stroke:none" />
    </marker>
    <marker
       id="ArrowDINout-end"
       class="rect833 path-effect835 measure-arrow-marker"
       inkscape:stockid="ArrowDINout-end"
       orient="auto"
       refX="0"
       refY="0"
       sodipodi:insensitive="true">
      <path
         d="m 0,0 16,-2.11 v 1.61 h 10 v 1 H 16 v 1.61 z"
         class="rect833 path-effect835 measure-arrow"
         id="ArrowDINout-end_path"
         style="fill:context-stroke;;fill-opacity:1;stroke:none" />
    </marker>
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.35"
     inkscape:cx="387.14286"
     inkscape:cy="560"
     inkscape:document-units="px"
     inkscape:current-layer="layer1"
     inkscape:document-rotation="0"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     units="px" />
  <metadata
     id="metadata5">
    <rdf:RDF>
      <cc:Work
         rdf:about="">
        <dc:format>image/svg+xml</dc:format>
        <dc:type
           rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
        <dc:title></dc:title>
      </cc:Work>
    </rdf:RDF>
  </metadata>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="rect833"
       width="117.99672"
       height="83.999763"
       x="26.071312"
       y="48.204327"
       inkscape:path-effect="#path-effect835"
       d="M 26.071312,48.204327 H 144.06803 V 132.20409 H 26.071312 Z"
       sodipodi:type="rect"
       transform="matrix(2.3080102,0,0,2.3080102,185.18163,137.90286)" />
    <path
       id="rect907"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 49.131645,212.60603 105.650385,-34.72176 48.3026,34.72176 66.48939,70.9784 -66.48939,44.07975 H 49.131645 Z"
       sodipodi:nodetypes="ccccccc"
       inkscape:path-effect="#path-effect910"
       inkscape:original-d="m 49.131645,212.60603 105.650385,-34.72176 48.3026,34.72176 66.48939,70.9784 -66.48939,44.07975 H 49.131645 Z"
       transform="matrix(2.3080102,0,0,2.3080102,185.18163,137.90286)" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M -141.45175,-105.3816 -60.684094,102.56027 154.62064,-61.518024 -15.149269,-100.53631 -27.133264,5.396173 67.63033,-48.095577"
       id="path948"
       inkscape:path-effect="#path-effect950"
       inkscape:original-d="M -141.45175,-105.3816 -60.684094,102.56027 154.62064,-61.518024 -15.149269,-100.53631 -27.133264,5.396173 67.63033,-48.095577"
       transform="matrix(2.3080102,0,0,2.3080102,185.18163,137.90286)" />
  </g>
  <path
     id="infoline-on-start-0-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="m 245.35448,249.15894 34.14214,34.14213"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-0-path-effect835"
     class="rect833 path-effect835 measure-label"
     sodipodi:insensitive="true"
     x="346.19461"
     y="274.49227"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#0000c9;fill-opacity:1;font-size:13.3333px"><tspan
       sodipodi:role="line"
       id="tspan845">216.168mm</tspan></text>
  <path
     id="infoline-on-end-0-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="m 517.69212,249.15894 -34.14214,34.14213"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-0-path-effect835"
     class="rect833 path-effect835 measure-DIM-line measure-line"
     d="m 272.91354,269.15894 h 67.0466 m 83.12632,0 h 67.0466"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-on-start-1-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="m 517.69212,249.15894 -34.14214,34.14213"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-1-path-effect835"
     class="rect833 path-effect835 measure-label"
     sodipodi:insensitive="true"
     x="467.74233"
     y="346.09509"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#0000c9;fill-opacity:1;font-size:13.3333px"
     transform="rotate(-90,503.02545,346.09509)"><tspan
       sodipodi:role="line"
       id="tspan851">153.886mm</tspan></text>
  <path
     id="infoline-on-end-1-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 517.69212,443.03125 483.54998,408.88911"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-1-path-effect835"
     class="rect833 path-effect835 measure-DIM-line measure-line"
     d="m 497.69212,276.71799 v 27.86756 m 0,83.01909 v 27.86755"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-on-start-2-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 517.69212,443.03125 483.54998,408.88911"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-2-path-effect835"
     class="rect833 path-effect835 measure-label"
     sodipodi:insensitive="true"
     x="346.19461"
     y="428.36458"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#0000c9;fill-opacity:1;font-size:13.3333px"><tspan
       sodipodi:role="line"
       id="tspan857">216.168mm</tspan></text>
  <path
     id="infoline-on-end-2-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="m 245.35448,443.03125 34.14214,-34.14214"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-2-path-effect835"
     class="rect833 path-effect835 measure-DIM-line measure-line"
     d="m 490.13306,423.03125 h -67.0466 m -83.12632,0 h -67.0466"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-on-start-3-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="m 245.35448,443.03125 34.14214,-34.14214"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-3-path-effect835"
     class="rect833 path-effect835 measure-label"
     sodipodi:insensitive="true"
     x="235.4047"
     y="346.09509"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#0000c9;fill-opacity:1;font-size:13.3333px"
     transform="rotate(-90,270.68782,346.09509)"><tspan
       sodipodi:role="line"
       id="tspan863">153.886mm</tspan></text>
  <path
     id="infoline-on-end-3-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="m 245.35448,249.15894 34.14214,34.14213"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-3-path-effect835"
     class="rect833 path-effect835 measure-DIM-line measure-line"
     d="m 265.35448,415.47219 v -27.86755 m 0,-83.01909 v -27.86756"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.944882;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-on-start-0-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="M 298.57797,628.59975 286.08922,590.59934"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-0-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="373.4232"
     y="574.59717"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(-18.192992,415.91984,574.59717)"><tspan
       sodipodi:role="line"
       id="tspan914">[0] 67.91mm</tspan></text>
  <path
     id="infoline-on-end-0-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="M 542.42013,548.46157 529.93139,510.46116"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-0-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 292.33359,609.59954 74.42433,-24.45939 m 94.99351,-31.2194 74.42433,-24.45939"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-1-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 542.42013,548.46157 23.34734,-32.47926"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-1-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="564.22563"
     y="576.6216"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(35.710028,606.72227,576.6216)"><tspan
       sodipodi:role="line"
       id="tspan920">[1] 36.33mm</tspan></text>
  <path
     id="infoline-on-end-1-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 653.90303,628.59975 23.34733,-32.47926"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-1-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 554.0938,532.22194 15.14559,10.88723 m 81.19172,58.36372 15.14558,10.88723"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-2-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 653.90303,628.59975 29.19233,-27.34608"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-2-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="698.83934"
     y="700.48229"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(46.870329,741.33598,700.48229)"><tspan
       sodipodi:role="line"
       id="tspan926">[2] 59.39mm</tspan></text>
  <path
     id="infoline-on-end-2-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 807.36122,792.41862 29.19233,-27.34608"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-2-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 668.49919,614.92671 42.54921,45.4219 m 68.35978,72.97507 42.5492,45.4219"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-3-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 807.36122,792.41862 22.10236,33.33895"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-3-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="702.13365"
     y="864.40154"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(-33.542753,744.63029,864.40154)"><tspan
       sodipodi:role="line"
       id="tspan932">[3] 48.71mm</tspan></text>
  <path
     id="infoline-on-end-3-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 653.90303,894.15513 22.10236,33.33895"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-3-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 818.4124,809.08809 -35.0587,23.24249 m -83.34079,55.25154 -35.0587,23.24248"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-4-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 653.90303,894.15513 v 40"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-4-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="433.74386"
     y="919.48846"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"><tspan
       sodipodi:role="line"
       id="tspan938">[4] 94.01mm</tspan></text>
  <path
     id="infoline-on-end-4-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 298.57797,894.15513 v 40"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-4-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="M 653.90303,914.15513 H 526.23654 m -99.99209,0 H 298.57797"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-5-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 298.57797,894.15513 h -40"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-5-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="241.41466"
     y="761.37744"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(-90,283.9113,761.37744)"><tspan
       sodipodi:role="line"
       id="tspan944">[5] 70.26mm</tspan></text>
  <path
     id="infoline-on-end-5-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 298.57797,628.59975 h -40"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-5-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 278.57797,894.15513 v -82.78165 m 0,-99.99209 v -82.78164"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-0-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m -141.29045,-105.31895 6.52508,-2.53443"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-0-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="-80.590642"
     y="131.67052"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(68.773104,-49.597881,131.67052)"><tspan
       sodipodi:role="line"
       id="tspan954">136.22mm</tspan></text>
  <path
     id="infoline-on-end-0-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 45.122122,374.61301 2.797032,-9.9298"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-0-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="m -133.89283,-100.08306 76.064899,195.834314 m 26.403075,67.976526 76.064903,195.83431"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-1-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 45.122122,374.61301 2.797032,-9.9298"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-1-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="261.22978"
     y="186.74793"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(-37.310109,292.18999,186.74793)"><tspan
       sodipodi:role="line"
       id="tspan960">165.31mm</tspan></text>
  <path
     id="infoline-on-end-1-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 542.04764,-4.0813669 -13.46489,2.9071017"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-1-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="M 53.389114,362.02653 259.98705,204.58347 M 317.92755,160.42848 524.52549,2.985416"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-2-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 542.04764,-4.0813669 -13.46489,2.9071017"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-2-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="310.1705"
     y="-39.604577"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(12.943535,341.35207,-39.604577)"><tspan
       sodipodi:role="line"
       id="tspan966">106.37mm</tspan></text>
  <path
     id="infoline-on-end-2-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 150.21696,-94.135969 5.49444,7.759774"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-2-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="M 523.17072,-3.2895026 378.29878,-36.585479 M 306.79459,-53.019315 161.92265,-86.315291"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-3-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 150.21696,-94.135969 5.49444,7.759774"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-3-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="115.87194"
     y="27.190254"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(-83.545649,146.89074,27.190254)"><tspan
       sodipodi:role="line"
       id="tspan972">65.10mm</tspan></text>
  <path
     id="infoline-on-end-3-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 122.55778,150.35728 7.17302,-10.83505"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-3-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="m 153.70593,-80.497305 -8.01251,70.8266223 m -8.20441,72.5228157 -8.01252,70.826627"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-4-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 122.55778,150.35728 7.17302,-10.83505"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-4-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="205.32392"
     y="86.511217"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:13.3333px"
     transform="rotate(-29.443674,236.34272,86.511217)"><tspan
       sodipodi:role="line"
       id="tspan978">66.45mm</tspan></text>
  <path
     id="infoline-on-end-4-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 341.27312,26.897778 -3.44097,-6.095876"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-4-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="M 135.20949,137.47411 201.94174,99.805377 M 265.50031,63.928104 332.23256,26.259368"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.944882;stroke:#000000;stroke-opacity:1" />
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEMeasureSegmentsTest, multi_MM_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="210mm"
   height="297mm"
   viewBox="0 0 210 297"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   sodipodi:docname="1.svg">
  <style
     id="style837">
.measure-arrow
{
}
.measure-label
{

}
.measure-line
{
}</style>
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="measure_segments"
       id="path-effect950"
       is_visible="true"
       lpeversion="1"
       unit="mm"
       orientation="parallel"
       coloropacity="#000000ff"
       fontbutton="Sans 10"
       precision="2"
       fix_overlaps="0"
       position="5"
       text_top_bottom="0"
       helpline_distance="0"
       helpline_overlap="2"
       line_width="0.25"
       scale="1"
       format="{measure}{unit}"
       blacklist=""
       active_projection="false"
       whitelist="false"
       showindex="false"
       arrows_outside="false"
       flip_side="false"
       scale_sensitive="true"
       local_locale="true"
       rotate_anotation="true"
       hide_back="true"
       hide_arrows="false"
       smallx100="false"
       linked_items=""
       distance_projection="20"
       angle_projection="0"
       avoid_overlapping="true"
       onbbox="false"
       bboxonly="false"
       centers="false"
       maxmin="false"
       helpdata="&lt;b&gt;&lt;big&gt;General&lt;/big&gt;&lt;/b&gt;
Display and position dimension lines and labels

&lt;b&gt;&lt;big&gt;Projection&lt;/big&gt;&lt;/b&gt;
Show a line with measurements based on the selected items

&lt;b&gt;&lt;big&gt;Options&lt;/big&gt;&lt;/b&gt;
Options for color, precision, label formatting and display

&lt;b&gt;&lt;big&gt;Tips&lt;/big&gt;&lt;/b&gt;
&lt;b&gt;&lt;i&gt;Custom styling:&lt;/i&gt;&lt;/b&gt; To further customize the styles, use the XML editor to find out the class or ID, then use the Style dialog to apply a new style.
&lt;b&gt;&lt;i&gt;Blacklists:&lt;/i&gt;&lt;/b&gt; allow to hide some segments or projection steps.
&lt;b&gt;&lt;i&gt;Multiple Measure LPEs:&lt;/i&gt;&lt;/b&gt; In the same object, in conjunction with blacklists,this allows for labels and measurements with different orientations or additional projections.
&lt;b&gt;&lt;i&gt;Set Defaults:&lt;/i&gt;&lt;/b&gt; For every LPE, default values can be set at the bottom." />
    <inkscape:path-effect
       effect="measure_segments"
       id="path-effect910"
       is_visible="true"
       lpeversion="1"
       unit="mm"
       orientation="parallel"
       coloropacity="#000000ff"
       fontbutton="Sans 10"
       precision="2"
       fix_overlaps="0"
       position="20"
       text_top_bottom="0"
       helpline_distance="0"
       helpline_overlap="20"
       line_width="0.25"
       scale="1"
       format="{measure}{unit}"
       blacklist=""
       active_projection="false"
       whitelist="false"
       showindex="true"
       arrows_outside="true"
       flip_side="false"
       scale_sensitive="true"
       local_locale="true"
       rotate_anotation="true"
       hide_back="true"
       hide_arrows="false"
       smallx100="true"
       linked_items=""
       distance_projection="20"
       angle_projection="0"
       avoid_overlapping="true"
       onbbox="false"
       bboxonly="false"
       centers="false"
       maxmin="false"
       helpdata="&lt;b&gt;&lt;big&gt;General&lt;/big&gt;&lt;/b&gt;
Display and position dimension lines and labels

&lt;b&gt;&lt;big&gt;Projection&lt;/big&gt;&lt;/b&gt;
Show a line with measurements based on the selected items

&lt;b&gt;&lt;big&gt;Options&lt;/big&gt;&lt;/b&gt;
Options for color, precision, label formatting and display

&lt;b&gt;&lt;big&gt;Tips&lt;/big&gt;&lt;/b&gt;
&lt;b&gt;&lt;i&gt;Custom styling:&lt;/i&gt;&lt;/b&gt; To further customize the styles, use the XML editor to find out the class or ID, then use the Style dialog to apply a new style.
&lt;b&gt;&lt;i&gt;Blacklists:&lt;/i&gt;&lt;/b&gt; allow to hide some segments or projection steps.
&lt;b&gt;&lt;i&gt;Multiple Measure LPEs:&lt;/i&gt;&lt;/b&gt; In the same object, in conjunction with blacklists,this allows for labels and measurements with different orientations or additional projections.
&lt;b&gt;&lt;i&gt;Set Defaults:&lt;/i&gt;&lt;/b&gt; For every LPE, default values can be set at the bottom." />
    <inkscape:path-effect
       effect="measure_segments"
       id="path-effect835"
       is_visible="true"
       lpeversion="1"
       unit="mm"
       orientation="parallel"
       coloropacity="#0000c9ff"
       fontbutton="Sans 10"
       precision="3"
       fix_overlaps="0"
       position="20"
       text_top_bottom="0"
       helpline_distance="0"
       helpline_overlap="20"
       line_width="0.25"
       scale="3"
       format="{measure}{unit}"
       blacklist=""
       active_projection="false"
       whitelist="false"
       showindex="false"
       arrows_outside="false"
       flip_side="true"
       scale_sensitive="true"
       local_locale="true"
       rotate_anotation="true"
       hide_back="true"
       hide_arrows="false"
       smallx100="true"
       linked_items=""
       distance_projection="20"
       angle_projection="0"
       avoid_overlapping="true"
       onbbox="false"
       bboxonly="false"
       centers="false"
       maxmin="false"
       helpdata="&lt;b&gt;&lt;big&gt;General&lt;/big&gt;&lt;/b&gt;
Display and position dimension lines and labels

&lt;b&gt;&lt;big&gt;Projection&lt;/big&gt;&lt;/b&gt;
Show a line with measurements based on the selected items

&lt;b&gt;&lt;big&gt;Options&lt;/big&gt;&lt;/b&gt;
Options for color, precision, label formatting and display

&lt;b&gt;&lt;big&gt;Tips&lt;/big&gt;&lt;/b&gt;
&lt;b&gt;&lt;i&gt;Custom styling:&lt;/i&gt;&lt;/b&gt; To further customize the styles, use the XML editor to find out the class or ID, then use the Style dialog to apply a new style.
&lt;b&gt;&lt;i&gt;Blacklists:&lt;/i&gt;&lt;/b&gt; allow to hide some segments or projection steps.
&lt;b&gt;&lt;i&gt;Multiple Measure LPEs:&lt;/i&gt;&lt;/b&gt; In the same object, in conjunction with blacklists,this allows for labels and measurements with different orientations or additional projections.
&lt;b&gt;&lt;i&gt;Set Defaults:&lt;/i&gt;&lt;/b&gt; For every LPE, default values can be set at the bottom."
       coloropacity_opacity_LPE="1" />
    <marker
       id="ArrowDIN-start"
       class="rect833 path-effect835 measure-arrow-marker"
       inkscape:stockid="ArrowDIN-start"
       orient="auto"
       refX="0.0"
       refY="0.0"
       sodipodi:insensitive="true">
      <path
         d="M -8,0 8,-2.11 8,2.11 z"
         class="rect833 path-effect835 measure-arrow"
         id="ArrowDIN-start_path"
         style="fill:context-stroke;;fill-opacity:1;stroke:none" />
    </marker>
    <marker
       id="ArrowDIN-end"
       class="rect833 path-effect835 measure-arrow-marker"
       inkscape:stockid="ArrowDIN-end"
       orient="auto"
       refX="0.0"
       refY="0.0"
       sodipodi:insensitive="true">
      <path
         d="M 8,0 -8,2.11 -8,-2.11 z"
         class="rect833 path-effect835 measure-arrow"
         id="ArrowDIN-end_path"
         style="fill:context-stroke;;fill-opacity:1;stroke:none" />
    </marker>
    <marker
       id="ArrowDINout-start"
       class="rect833 path-effect835 measure-arrow-marker"
       inkscape:stockid="ArrowDINout-start"
       orient="auto"
       refX="0.0"
       refY="0.0"
       sodipodi:insensitive="true">
      <path
         d="M 0,0 -16,2.11 -16,0.5 -26,0.5 -26,-0.5 -16,-0.5 -16,-2.11 z"
         class="rect833 path-effect835 measure-arrow"
         id="ArrowDINout-start_path"
         style="fill:context-stroke;;fill-opacity:1;stroke:none" />
    </marker>
    <marker
       id="ArrowDINout-end"
       class="rect833 path-effect835 measure-arrow-marker"
       inkscape:stockid="ArrowDINout-end"
       orient="auto"
       refX="0.0"
       refY="0.0"
       sodipodi:insensitive="true">
      <path
         d="M 0,0 16,-2.11 16,-0.5 26,-0.5 26,0.5 16,0.5 16,2.11 z"
         class="rect833 path-effect835 measure-arrow"
         id="ArrowDINout-end_path"
         style="fill:context-stroke;;fill-opacity:1;stroke:none" />
    </marker>
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.35"
     inkscape:cx="387.14286"
     inkscape:cy="557.14286"
     inkscape:document-units="mm"
     inkscape:current-layer="layer1"
     inkscape:document-rotation="0"
     showgrid="false"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1" />
  <metadata
     id="metadata5">
    <rdf:RDF>
      <cc:Work
         rdf:about="">
        <dc:format>image/svg+xml</dc:format>
        <dc:type
           rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
        <dc:title></dc:title>
      </cc:Work>
    </rdf:RDF>
  </metadata>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="rect833"
       width="117.99672"
       height="83.999763"
       x="26.071312"
       y="48.204327"
       inkscape:path-effect="#path-effect835"
       d="M 26.071312,48.204327 H 144.06803 V 132.20409 H 26.071312 Z"
       sodipodi:type="rect" />
    <path
       id="rect907"
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       d="m 49.131645,212.60603 105.650385,-34.72176 48.3026,34.72176 66.48939,70.9784 -66.48939,44.07975 H 49.131645 Z"
       sodipodi:nodetypes="ccccccc"
       inkscape:path-effect="#path-effect910"
       inkscape:original-d="m 49.131645,212.60603 105.650385,-34.72176 48.3026,34.72176 66.48939,70.9784 -66.48939,44.07975 H 49.131645 Z" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M -141.45175,-105.3816 -60.684094,102.56027 154.62064,-61.518024 -15.149269,-100.53631 -27.133264,5.396173 67.63033,-48.095577"
       id="path948"
       inkscape:path-effect="#path-effect950"
       inkscape:original-d="M -141.45175,-105.3816 -60.684094,102.56027 154.62064,-61.518024 -15.149269,-100.53631 -27.133264,5.396173 67.63033,-48.095577" />
  </g>
  <path
     id="infoline-on-start-0-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 26.071312,48.204327 60.213448,82.346462"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-0-path-effect835"
     class="rect833 path-effect835 measure-label"
     sodipodi:insensitive="true"
     x="74.623274"
     y="69.615438"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#0000c9;fill-opacity:1;font-size:3.52778px"><tspan
       sodipodi:role="line"
       id="tspan845">353.990mm</tspan></text>
  <path
     id="infoline-on-end-0-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 144.06803,48.204327 109.9259,82.346462"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-0-path-effect835"
     class="rect833 path-effect835 measure-DIM-line measure-line"
     d="m 48.071312,68.204327 h 24.70848 m 24.579759,0 h 24.708479"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-on-start-1-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 144.06803,48.204327 109.9259,82.346462"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-1-path-effect835"
     class="rect833 path-effect835 measure-label"
     sodipodi:insensitive="true"
     x="115.02758"
     y="90.204208"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#0000c9;fill-opacity:1;font-size:3.52778px"
     transform="rotate(-90,125.47914,90.204208)"><tspan
       sodipodi:role="line"
       id="tspan851">251.999mm</tspan></text>
  <path
     id="infoline-on-end-1-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 144.06803,132.20409 109.9259,98.061954"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-1-path-effect835"
     class="rect833 path-effect835 measure-DIM-line measure-line"
     d="m 124.06803,70.204327 v 7.703922 m 0,24.591921 v 7.70392"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-on-start-2-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 144.06803,132.20409 109.9259,98.061954"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-2-path-effect835"
     class="rect833 path-effect835 measure-label"
     sodipodi:insensitive="true"
     x="74.623274"
     y="113.6152"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#0000c9;fill-opacity:1;font-size:3.52778px"><tspan
       sodipodi:role="line"
       id="tspan857">353.990mm</tspan></text>
  <path
     id="infoline-on-end-2-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 26.071312,132.20409 60.213448,98.061954"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-2-path-effect835"
     class="rect833 path-effect835 measure-DIM-line measure-line"
     d="M 122.06803,112.20409 H 97.359551 m -24.579759,0 h -24.70848"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-on-start-3-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 26.071312,132.20409 60.213448,98.061954"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-3-path-effect835"
     class="rect833 path-effect835 measure-label"
     sodipodi:insensitive="true"
     x="37.030858"
     y="90.204208"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#0000c9;fill-opacity:1;font-size:3.52778px"
     transform="rotate(-90,47.482423,90.204208)"><tspan
       sodipodi:role="line"
       id="tspan863">251.999mm</tspan></text>
  <path
     id="infoline-on-end-3-path-effect835"
     class="rect833 path-effect835 measure-helper-line measure-line"
     d="M 26.071312,48.204327 60.213448,82.346462"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-3-path-effect835"
     class="rect833 path-effect835 measure-DIM-line measure-line"
     d="m 46.071312,110.20409 v -7.70392 m 0,-24.591921 v -7.703922"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.25;stroke:#0000c9;stroke-opacity:1" />
  <path
     id="infoline-on-start-0-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="M 49.131645,212.60603 36.642896,174.60562"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-0-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="83.78686"
     y="177.58552"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(-18.192992,96.153038,177.58552)"><tspan
       sodipodi:role="line"
       id="tspan914">[0] 111.21mm</tspan></text>
  <path
     id="infoline-on-end-0-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="M 154.78203,177.88427 142.29328,139.88386"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-0-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 42.887271,193.60583 39.00402,-12.81859 m 27.642339,-9.08459 39.00403,-12.81858"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-1-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 154.78203,177.88427 23.34733,-32.47926"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-1-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="178.53942"
     y="180.15132"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(35.710028,189.78335,180.15132)"><tspan
       sodipodi:role="line"
       id="tspan920">[1] 59.49mm</tspan></text>
  <path
     id="infoline-on-end-1-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 203.08463,212.60603 23.34733,-32.47926"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-1-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 166.4557,161.64464 13.41028,9.63982 m 21.48204,15.44212 13.41028,9.63982"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-2-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 203.08463,212.60603 29.19233,-27.34607"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-2-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="238.65171"
     y="235.3869"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(46.870329,249.89565,235.3869)"><tspan
       sodipodi:role="line"
       id="tspan926">[2] 97.26mm</tspan></text>
  <path
     id="infoline-on-end-2-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 269.57402,283.58443 29.19233,-27.34607"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-2-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 217.6808,198.93299 24.20124,25.83518 m 18.08691,19.30805 24.20124,25.83517"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-3-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 269.57402,283.58443 22.10236,33.33895"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-3-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="236.91629"
     y="323.4699"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(-33.542753,248.16023,323.4699)"><tspan
       sodipodi:role="line"
       id="tspan932">[3] 79.77mm</tspan></text>
  <path
     id="infoline-on-end-3-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 203.08463,327.66418 22.10236,33.33895"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-3-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 280.6252,300.2539 -22.21937,14.73054 m -22.05065,14.61868 -22.21937,14.73053"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-4-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 203.08463,327.66418 v 40"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-4-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="113.74196"
     y="349.07529"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"><tspan
       sodipodi:role="line"
       id="tspan938">[4] 153.95mm</tspan></text>
  <path
     id="infoline-on-end-4-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 49.131645,327.66418 v 40"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-4-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 203.08463,347.66418 h -62.42805 m -29.09689,0 H 49.131645"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-5-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 49.131645,327.66418 h -40"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-5-path-effect910"
     class="rect907 path-effect910 measure-label"
     sodipodi:insensitive="true"
     x="18.176578"
     y="270.13511"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(-90,30.542756,270.1351)"><tspan
       sodipodi:role="line"
       id="tspan944">[5] 115.06mm</tspan></text>
  <path
     id="infoline-on-end-5-path-effect910"
     class="rect907 path-effect910 measure-helper-line measure-line"
     d="m 49.131645,212.60603 h -40"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-5-path-effect910"
     class="rect907 path-effect910 measure-DIM-line measure-line"
     d="m 29.131645,327.66418 v -42.98063 m 0,-29.09689 v -42.98063"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDINout-start);marker-end:url(#ArrowDINout-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-0-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m -141.45175,-105.3816 6.52508,-2.53444"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-0-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="-108.25485"
     y="-5.807268"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(68.773104,-98.925524,-5.807268)"><tspan
       sodipodi:role="line"
       id="tspan954">223.08mm</tspan></text>
  <path
     id="infoline-on-end-0-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m -60.684094,102.56027 2.797032,-9.929801"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-0-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="m -136.06686,-105.3276 34.48283,88.778403 m 7.947756,20.4620384 34.482828,88.7784056"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-1-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m -60.684094,102.56027 2.797032,-9.929801"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-1-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="33.866681"
     y="18.883627"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(-37.310109,43.196006,18.883627)"><tspan
       sodipodi:role="line"
       id="tspan960">270.70mm</tspan></text>
  <path
     id="infoline-on-end-1-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 154.62064,-61.518024 -13.46489,2.907102"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-1-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="M -56.838588,93.343297 33.611005,24.413949 M 51.070376,11.108604 141.51997,-57.820744"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-2-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 154.62064,-61.518024 -13.46489,2.907102"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-2-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="56.56926"
     y="-75.345639"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(12.943535,65.833989,-75.345639)"><tspan
       sodipodi:role="line"
       id="tspan966">174.20mm</tspan></text>
  <path
     id="infoline-on-end-2-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m -15.149269,-100.53631 5.494441,7.759774"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-2-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="M 141.16152,-59.480983 76.772799,-74.279469 M 55.527331,-79.162323 -8.8613908,-93.960809"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-3-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m -15.149269,-100.53631 5.494441,7.759774"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-3-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="-23.799987"
     y="-48.931375"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(-83.545649,-14.535258,-48.931375)"><tspan
       sodipodi:role="line"
       id="tspan972">106.61mm</tspan></text>
  <path
     id="infoline-on-end-3-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m -27.133264,5.396173 7.173016,-10.8350516"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-3-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="m -11.035396,-92.421467 -3.676779,32.500871 m -2.450501,21.661192 -3.676778,32.5008707"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-on-start-4-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m -27.133264,5.396173 7.173016,-10.8350516"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <text
     xml:space="preserve"
     id="text-on-4-path-effect950"
     class="path948 path-effect950 measure-label"
     sodipodi:insensitive="true"
     x="13.483035"
     y="-26.881641"
     style="font-family:Sans;font-weight:normal;font-style:normal;font-stretch:normal;font-variant:normal;fill:#000000;fill-opacity:1;font-size:3.52778px"
     transform="rotate(-29.443674,22.747764,-26.881641)"><tspan
       sodipodi:role="line"
       id="tspan978">108.82mm</tspan></text>
  <path
     id="infoline-on-end-4-path-effect950"
     class="path948 path-effect950 measure-helper-line measure-line"
     d="m 67.63033,-48.095577 -3.440974,-6.095876"
     sodipodi:insensitive="true"
     inkscape:label="dinhelpline"
     style="stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
  <path
     id="infoline-4-path-effect950"
     class="path948 path-effect950 measure-DIM-line measure-line"
     d="M -19.322598,-4.7543452 12.562236,-22.752561 m 18.983742,-10.715862 31.884835,-17.998215"
     sodipodi:insensitive="true"
     inkscape:label="dinline"
     style="marker-start:url(#ArrowDIN-start);marker-end:url(#ArrowDIN-end);stroke-width:0.25;stroke:#000000;stroke-opacity:1" />
</svg>
)"""";

   testDoc(svg);
}