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

class LPEEllipseFromPointsTest : public LPESPathsTest {};


// INKSCAPE 1.0.2

TEST_F(LPEEllipseFromPointsTest, multi_PX_1_0_2)
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
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect911"
       is_visible="true"
       lpeversion="1"
       method="steiner_inellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect907"
       is_visible="true"
       lpeversion="1"
       method="steiner_inellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect903"
       is_visible="true"
       lpeversion="1"
       method="steiner_ellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect899"
       is_visible="true"
       lpeversion="1"
       method="steiner_ellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect895"
       is_visible="true"
       lpeversion="1"
       method="perspective_circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect891"
       is_visible="true"
       lpeversion="1"
       method="iso_circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect887"
       is_visible="true"
       lpeversion="1"
       method="iso_circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect883"
       is_visible="true"
       lpeversion="1"
       method="iso_circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect879"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect875"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="true"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect871"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="true"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect867"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect863"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="true"
       slice_arc="true"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect859"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="true"
       slice_arc="true"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect855"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="true"
       slice_arc="true"
       gen_isometric_frame="true"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect851"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="true"
       slice_arc="true"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect847"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="true"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect843"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect839"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="false"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect835"
       is_visible="true"
       lpeversion="1"
       method="steiner_inellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="20"
       draw_ori_path="false" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.35"
     inkscape:cx="400"
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
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 298.75542,501.38066 C 276.67931,640.9947 197.08963,744.41913 120.98689,732.38558 44.884144,720.35203 1.086899,597.41737 23.163004,457.80333 45.239108,318.18929 124.82879,214.76486 200.93153,226.79841 c 76.10275,12.03354 119.89999,134.96821 97.82389,274.58225 z"
       id="path837"
       inkscape:path-effect="#path-effect839"
       inkscape:original-d="M 118.17688,256.8181 301.62441,382.03649 194.80846,709.74403 33.46523,637.7233 c -147.66474,117.61731 8.832358,-257.08187 8.832358,-257.08187"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 458.99989,201.83477 c 66.81123,-59.25171 138.9318,-31.43426 170.66599,65.82716 31.73419,97.26142 13.81888,235.57647 -42.39476,327.3085 -56.21364,91.73203 -134.317,110.1047 -184.82366,43.47707 C 351.9408,571.81986 343.7736,439.64033 383.12061,325.6581"
       id="path841"
       inkscape:path-effect="#path-effect843"
       inkscape:original-d="M 458.99989,201.83477 642.44742,327.05318 535.63146,654.76073 374.28823,582.73999 C 226.62351,700.3573 383.12061,325.6581 383.12061,325.6581"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 711.30459,556.29696 c 18.21178,-52.75671 44.95568,-96.39866 75.87928,-123.82333"
       id="path845"
       inkscape:path-effect="#path-effect847"
       inkscape:original-d="M 787.18387,432.47363 970.6314,557.69203 863.81545,885.39957 702.47221,813.37884 c -147.66473,117.6173 8.83238,-257.08188 8.83238,-257.08188"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 542.10986,1021.0722 c 18.21178,-52.7567 44.95569,-96.39864 75.87929,-123.8233 l 42.78232,222.7739 z"
       id="path849"
       inkscape:path-effect="#path-effect851"
       inkscape:original-d="m 617.98915,897.2489 183.44753,125.2184 -106.81595,327.7075 -161.34324,-72.0207 c -147.66473,117.6173 8.83237,-257.0819 8.83237,-257.0819"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 291.5805,1196.5778 c 18.21178,-52.7567 44.95569,-96.3986 75.87929,-123.8233 l 42.78233,222.7739 z m 20.83774,-175.6317 275.59243,43.5774 -79.94468,505.5871 -275.59242,-43.5773 z"
       id="path853"
       inkscape:path-effect="#path-effect855"
       inkscape:original-d="m 367.45979,1072.7545 183.44754,125.2184 -106.81595,327.7075 -161.34325,-72.0207 c -147.66473,117.6173 8.83237,-257.0819 8.83237,-257.0819"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 912.02172,822.13224 c 18.21178,-52.75671 44.95569,-96.39866 75.87929,-123.82333 l 42.78239,222.7739 z m 20.83774,-175.63171 275.59244,43.57734 -79.9446,505.58723 -275.5925,-43.5773 z m -39.97233,252.79361 275.59247,43.57735 m -97.8239,-274.58229 -79.94466,505.5872"
       id="path857"
       inkscape:path-effect="#path-effect859"
       inkscape:original-d="m 987.90101,698.30891 183.44759,125.2184 -106.816,327.70759 -161.34325,-72.0208 c -147.66473,117.6173 8.83237,-257.08186 8.83237,-257.08186"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 948.22648,29.879304 c 29.02447,-31.9500944 67.55452,-53.719087 109.89982,-62.091999 l 39.0285,197.383115 z M 895.95009,-36.034241 H 1298.3594 V 366.37508 H 895.95009 Z m 0,201.204661 H 1298.3594 M 1097.1548,-36.034241 V 366.37508"
       id="path861"
       inkscape:path-effect="#path-effect863"
       inkscape:original-d="M 1050.6965,-69.788181 1234.1441,55.430251 1127.3281,383.13778 965.98485,311.11705 c -147.66472,117.6173 8.83238,-257.081878 8.83238,-257.081878"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 763.98977,518.74312 c 96.90797,-19.16156 193.39244,34.86113 227.70518,127.49458 34.31275,92.63345 -3.69673,196.4746 -89.70306,245.06709 -86.00634,48.59249 -194.56694,27.56156 -256.20509,-49.63338 -61.63814,-77.19495 -58.11988,-187.71791 8.30317,-260.8363 m -52.2764,-65.91354 H 1004.2229 V 917.33088 H 601.81357 Z m 0,201.20466 H 1004.2229 M 803.01823,514.92157 v 402.40931"
       id="path865"
       inkscape:path-effect="#path-effect867"
       inkscape:original-d="M 756.55999,481.16763 940.00754,606.38605 833.19158,934.09359 671.84833,862.07285 C 524.1836,979.69016 680.6807,604.99097 680.6807,604.99097"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 593.61486,1155.9675 c 29.02447,-31.95 67.55451,-53.719 109.89979,-62.0919 m -162.1762,-3.8216 h 402.40928 v 402.4093 H 541.33845 Z m 0,201.2047 H 943.74773 M 742.54309,1090.054 v 402.4093"
       id="path869"
       inkscape:path-effect="#path-effect871"
       inkscape:original-d="m 696.08487,1056.3001 183.44755,125.2184 -106.81596,327.7075 -161.34325,-72.0207 c -147.66473,117.6173 8.83237,-257.0819 8.83237,-257.0819"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 647.76731,29.879307 c 29.02447,-31.950096 67.55453,-53.719088 109.89981,-62.091997 m -162.1762,197.38311 h 402.4093 M 796.69557,-36.034232 V 366.37507"
       id="path873"
       inkscape:path-effect="#path-effect875"
       inkscape:original-d="M 750.23734,-69.788181 933.68489,55.430251 826.86893,383.13778 665.52568,311.11705 C 517.86095,428.73436 674.35805,54.035172 674.35805,54.035172"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 347.94361,22.847636 c 96.90797,-19.1615565 193.39243,34.861139 227.70517,127.494594 34.31274,92.63345 -3.69673,196.47459 -89.70307,245.06708 -86.00634,48.59248 -194.56694,27.56155 -256.20508,-49.63339 -61.63814,-77.19495 -58.11987,-187.71791 8.30318,-260.8363"
       id="path877"
       inkscape:path-effect="#path-effect879"
       inkscape:original-d="M 340.51383,-14.727852 523.96136,110.49056 417.14542,438.1981 255.80216,366.17736 c -147.66473,117.61731 8.83238,-257.08188 8.83238,-257.08188"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 48.253045,588.86044 C 77.749455,498.3665 142.72715,453.03776 193.38479,487.61587 244.04243,522.19398 261.197,623.58492 231.70058,714.07886 202.20417,804.5728 137.22648,849.90154 86.56884,815.32343 35.911201,780.74532 18.756635,679.35438 48.253045,588.86044 Z"
       id="path881"
       inkscape:path-effect="#path-effect883"
       inkscape:original-d="M 101.66102,425.00666 285.10856,550.22508 178.29261,877.93264 16.949357,805.9119 C -130.71537,923.52921 25.781732,548.83 25.781732,548.83"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 329.73452,816.27501 c 29.49641,-90.49394 94.4741,-135.82268 145.13174,-101.24457 50.65764,34.5781 67.81221,135.96904 38.3158,226.46298 -29.49641,90.49398 -94.4741,135.82268 -145.13174,101.24458 -50.65764,-34.5781 -67.81221,-135.96905 -38.3158,-226.46299 z M 459.77409,1105.3472 276.32655,980.12879 383.14249,652.42123 566.59003,777.63964 Z"
       id="path885"
       inkscape:path-effect="#path-effect887"
       inkscape:original-d="M 383.14249,652.42123 566.59003,777.63964 459.77409,1105.3472 298.43083,1033.3264 C 150.7661,1150.9438 307.2632,776.24456 307.2632,776.24456"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 130.04681,825.47888 313.49434,950.69729 206.6784,1278.4048 45.335146,1206.3841 c -147.664726,117.6173 8.832375,-257.08189 8.832375,-257.08189 m 13.965822,72.44549 C 87.421885,926.3186 147.93251,862.47638 203.2876,879.15214 258.64269,895.8279 287.88041,986.70685 268.59187,1082.136 249.30332,1177.5651 188.7927,1241.4073 133.43761,1224.7315 78.082516,1208.0558 48.8448,1117.1768 68.133343,1021.7477 Z m 165.533527,233.178 -200.458523,-60.3883 69.849993,-345.57939 200.45852,60.38826 z m 34.925,-172.7897 -200.458527,-60.3883 m 65.304267,202.9838 69.84999,-345.57936"
       id="path889"
       inkscape:path-effect="#path-effect891"
       inkscape:original-d="M 130.04681,825.47888 313.49434,950.69729 206.6784,1278.4048 45.335146,1206.3841 c -147.664726,117.6173 8.832375,-257.08189 8.832375,-257.08189"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M -5.4035227,221.52055 -17.419962,604.99762 -124.2359,932.70517 -285.57915,860.68444 c -147.66473,117.6173 8.83237,-257.0819 8.83237,-257.0819 m 248.203958,3.38509 c -27.568575,147.92086 -82.861088,284.46578 -123.499298,304.9816 -40.6382,20.51582 -51.23322,-82.76637 -23.66465,-230.68724 27.56858,-147.92086 82.861094,-284.46577 123.499295,-304.98159 40.638202,-20.51582 51.23322823,82.76637 23.664653,230.68723 z M -175.70677,681.28199 -28.542822,606.98763 M -52.207475,376.3004 -152.04212,911.96923"
       id="path893"
       inkscape:path-effect="#path-effect895"
       inkscape:original-d="M -5.4035227,221.52055 -17.419962,604.99762 -124.2359,932.70517 -285.57915,860.68444 c -147.66473,117.6173 8.83237,-257.0819 8.83237,-257.0819"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 460.51157,388.49189 448.49513,771.96895 341.67919,1099.6765 180.33593,1027.6557 C 32.671196,1145.273 189.16831,770.57388 189.16831,770.57388 M 357.00809,1157.873 c -25.4238,40.3008 -19.22145,-108.127 13.85334,-331.52282 C 403.93622,602.95436 451.3587,389.18601 476.7825,348.88521 502.20631,308.5844 496.00395,457.01223 462.92916,680.40805 429.85437,903.80386 382.43189,1117.5722 357.00809,1157.873 Z"
       id="path897"
       inkscape:path-effect="#path-effect899"
       inkscape:original-d="M 460.51157,388.49189 448.49513,771.96895 341.67919,1099.6765 180.33593,1027.6557 c -147.664734,117.6173 8.83238,-257.08182 8.83238,-257.08182"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 1141.8483,87.965008 1129.8318,471.44207 1023.0159,799.1496 861.67264,727.12887 c -147.66473,117.61731 8.83237,-257.08188 8.83237,-257.08188 m 167.83979,387.29913 c -25.4238,40.30081 -19.2214,-108.12701 13.8534,-331.52282 33.0748,-223.39582 80.4972,-437.164163 105.921,-477.464972 25.4238,-40.3008086 19.2214,108.127022 -13.8534,331.522832 -33.0748,223.39581 -80.4972,437.16415 -105.921,477.46496 z M 1204.1531,-24.612741 1084.3786,784.37505 992.31094,930.31719 1112.0854,121.3294 Z"
       id="path901"
       inkscape:path-effect="#path-effect903"
       inkscape:original-d="M 1141.8483,87.965008 1129.8318,471.44207 1023.0159,799.1496 861.67264,727.12887 c -147.66473,117.61731 8.83237,-257.08188 8.83237,-257.08188"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 736.82153,186.0643 724.80509,569.54136 617.98915,897.2489 456.6459,825.22817 c -147.66473,117.61731 8.83237,-257.08189 8.83237,-257.08189 m 197.78338,185.05219 c -12.7119,20.1504 -9.61072,-54.06351 6.92667,-165.76142 16.5374,-111.6979 40.24864,-218.58208 52.96054,-238.73248 12.7119,-20.1504 9.61072,54.06351 -6.92667,165.76142 -16.5374,111.6979 -40.24864,218.58208 -52.96054,238.73248 z m 82.90414,-440.97943 -59.8872,404.4939 -46.03387,72.97106 59.88721,-404.4939 z m -23.01693,36.48553 -59.88721,404.4939 m 52.96054,-238.73248 -46.03387,72.97106"
       id="path905"
       inkscape:path-effect="#path-effect907"
       inkscape:original-d="M 736.82153,186.0643 724.80509,569.54136 617.98915,897.2489 456.6459,825.22817 c -147.66473,117.61731 8.83237,-257.08189 8.83237,-257.08189"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 339.11631,-222.71814 327.09987,160.75889 220.28393,488.46644 58.940673,416.4457 c -147.664727,117.61732 8.83238,-257.08189 8.83238,-257.08189 m 197.783377,185.0522 c -12.7119,20.1504 -9.61072,-54.06351 6.92668,-165.76141 16.53739,-111.697908 40.24863,-218.582079 52.96053,-238.732482 12.7119,-20.150403 9.61072,54.0635096 -6.92667,165.761412 -16.5374,111.6979 -40.24864,218.58208 -52.96054,238.73248 z M 325.44364,-60.077882 265.55643,344.41601 m 52.96054,-238.73248 -46.03386,72.97107"
       id="path909"
       inkscape:path-effect="#path-effect911"
       inkscape:original-d="M 339.11631,-222.71814 327.09987,160.75889 220.28393,488.46644 58.940673,416.4457 c -147.664727,117.61732 8.83238,-257.08189 8.83238,-257.08189"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.498909px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 383.07737,949.9094 c -15.14242,37.37572 -15.78868,-19.20413 -1.44347,-126.37463 14.34522,-107.17049 38.24966,-224.34835 53.39208,-261.72407 15.14242,-37.37572 15.78868,19.20413 1.44346,126.37463 -14.34521,107.17049 -38.24966,224.34835 -53.39207,261.72407 z"
       id="path833"
       inkscape:path-effect="#path-effect835"
       inkscape:original-d="M 452.66795,390.97285 440.65151,774.4499 333.83556,1102.1574 172.49231,1030.1367 c -147.664738,117.6173 8.83237,-257.08188 8.83237,-257.08188"
       sodipodi:nodetypes="ccccc" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEEllipseFromPointsTest, multi_MM_1_0_2)
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
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect911"
       is_visible="true"
       lpeversion="1"
       method="steiner_inellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect907"
       is_visible="true"
       lpeversion="1"
       method="steiner_inellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect903"
       is_visible="true"
       lpeversion="1"
       method="steiner_ellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect899"
       is_visible="true"
       lpeversion="1"
       method="steiner_ellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect895"
       is_visible="true"
       lpeversion="1"
       method="perspective_circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect891"
       is_visible="true"
       lpeversion="1"
       method="iso_circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="11"
       draw_ori_path="true" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect887"
       is_visible="true"
       lpeversion="1"
       method="iso_circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect883"
       is_visible="true"
       lpeversion="1"
       method="iso_circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect879"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect875"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="true"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect871"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="true"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect867"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect863"
       is_visible="true"
       lpeversion="1"
       method="circle"
       gen_arc="true"
       arc_other="true"
       slice_arc="true"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect859"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="true"
       slice_arc="true"
       gen_isometric_frame="true"
       draw_axes="true"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect855"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="true"
       slice_arc="true"
       gen_isometric_frame="true"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect851"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="true"
       slice_arc="true"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect847"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="true"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect843"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect839"
       is_visible="true"
       lpeversion="1"
       method="auto"
       gen_arc="false"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="0"
       draw_ori_path="false" />
    <inkscape:path-effect
       effect="pts2ellipse"
       id="path-effect835"
       is_visible="true"
       lpeversion="1"
       method="steiner_inellipse"
       gen_arc="true"
       arc_other="false"
       slice_arc="false"
       gen_isometric_frame="false"
       draw_axes="false"
       gen_perspective_frame="false"
       draw_perspective_axes="false"
       rot_axes="20"
       draw_ori_path="false" />
  </defs>
  <sodipodi:namedview
     id="base"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     inkscape:pageopacity="0.0"
     inkscape:pageshadow="2"
     inkscape:zoom="0.35"
     inkscape:cx="400"
     inkscape:cy="560"
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
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 31.440651,88.122208 C 3.0914854,137.00933 -51.465491,158.32989 -90.415766,135.74301 -129.36604,113.15612 -137.95994,55.214992 -109.61077,6.3278678 -81.261609,-42.559257 -26.704633,-63.879815 12.245642,-41.292931 51.195918,-18.706048 59.789816,39.235084 31.440651,88.122208 Z"
       id="path837"
       inkscape:path-effect="#path-effect839"
       inkscape:original-d="M -65.094956,-38.387196 46.433585,9.7343587 -18.506114,135.6727 -116.59615,107.99506 c -89.77408,45.20045 5.36971,-98.7968253 5.36971,-98.7968253"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 142.11139,-59.517344 c 40.61848,-22.77049 84.46481,-12.080215 103.75789,25.297448 C 265.16236,3.1577661 254.27059,56.31238 220.09502,91.565089 185.91944,126.8178 138.43581,133.87843 107.72984,108.27337 77.023874,82.668297 72.058552,31.871572 95.979913,-11.931914"
       id="path841"
       inkscape:path-effect="#path-effect843"
       inkscape:original-d="m 142.11139,-59.517344 111.52854,48.12156 -64.9397,125.938344 -98.090037,-27.677644 c -89.77406954,45.200454 5.36972,-98.79683 5.36972,-98.79683"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 295.50225,76.702832 c 11.07201,-20.274457 27.3312,-37.046102 46.13148,-47.58543"
       id="path845"
       inkscape:path-effect="#path-effect847"
       inkscape:original-d="m 341.63373,29.117402 111.52854,48.12156 -64.9397,125.938338 -98.09004,-27.67764 c -89.77407,45.20045 5.36972,-98.796828 5.36972,-98.796828"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 192.63883,255.31644 c 11.07201,-20.27446 27.33121,-37.0461 46.13148,-47.58543 l 26.00989,85.61224 z"
       id="path849"
       inkscape:path-effect="#path-effect851"
       inkscape:original-d="m 238.77031,207.73101 111.52854,48.12156 -64.9397,125.93834 -98.09004,-27.67764 c -89.774069,45.20045 5.36972,-98.79683 5.36972,-98.79683"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 40.327302,322.76341 c 11.07201,-20.27446 27.331205,-37.0461 46.13148,-47.58543 l 26.009898,85.61224 z M 93.273675,231.37507 234.3251,313.16941 131.66368,490.20536 -9.3877429,408.41103 Z"
       id="path853"
       inkscape:path-effect="#path-effect855"
       inkscape:original-d="m 86.458782,275.17798 111.528548,48.12156 -64.9397,125.93834 -98.090048,-27.67764 c -89.774073,45.20045 5.36972,-98.79683 5.36972,-98.79683"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 417.53,178.8636 c 11.07201,-20.27446 27.3312,-37.0461 46.13148,-47.58543 l 26.0099,85.61224 z M 470.47638,87.475257 611.5278,169.2696 508.86638,346.30556 367.81495,264.51121 Z m -51.33071,88.517983 141.05142,81.79434 m -19.195,-129.41515 -102.66142,177.03595"
       id="path857"
       inkscape:path-effect="#path-effect859"
       inkscape:original-d="m 463.66148,131.27817 111.52855,48.12156 -64.9397,125.93834 -98.09005,-27.67764 C 322.38621,322.86088 417.53,178.8636 417.53,178.8636"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 449.91437,-119.58745 c 11.46359,-20.30392 30.25851,-35.45469 52.53301,-42.34741 l 26.99947,87.251468 z m -11.80092,-46.42934 h 182.6668 V 16.650008 h -182.6668 z m 0,91.333398 h 182.6668 m -91.3334,-91.333398 V 16.650008"
       id="path861"
       inkscape:path-effect="#path-effect863"
       inkscape:original-d="m 501.83856,-163.90232 111.52855,48.12157 -64.9397,125.938335 -98.09005,-27.67764 c -89.77407,45.20045 5.36972,-98.796825 5.36972,-98.796825"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 323.62449,49.798015 c 41.86084,-12.953599 87.02246,5.647813 107.61698,44.325867 20.59452,38.678058 10.81771,86.531978 -23.29745,114.032568 -34.11515,27.50058 -82.95387,26.89724 -116.37925,-1.43773 C 258.1394,178.38376 249.54775,130.30291 271.09148,92.145425 M 259.29056,45.716083 h 182.6668 V 228.38288 h -182.6668 z m 0,91.333397 h 182.6668 M 350.62396,45.716083 V 228.38288"
       id="path865"
       inkscape:path-effect="#path-effect867"
       inkscape:original-d="m 323.01567,47.830556 111.52855,48.121566 -64.9397,125.938338 -98.09005,-27.67764 c -89.77407,45.20045 5.36972,-98.796828 5.36972,-98.796828"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 234.3251,313.16941 c 11.46359,-20.30392 30.2585,-35.45469 52.53301,-42.34741 m -64.33393,-4.08193 h 182.6668 v 182.66679 h -182.6668 z m 0,91.3334 h 182.6668 m -91.3334,-91.3334 v 182.66679"
       id="path869"
       inkscape:path-effect="#path-effect871"
       inkscape:original-d="m 286.24929,268.85454 111.52855,48.12157 -64.9397,125.93833 -98.09005,-27.67764 c -89.77407,45.20045 5.36972,-98.79682 5.36972,-98.79682"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 267.24757,-119.58745 c 11.46359,-20.30392 30.25851,-35.45469 52.53301,-42.34741 m -64.33393,87.251469 h 182.6668 m -91.3334,-91.333399 v 182.6668"
       id="path873"
       inkscape:path-effect="#path-effect875"
       inkscape:original-d="m 319.17176,-163.90232 111.52855,48.12157 -64.9397,125.938337 -98.09005,-27.67764 c -89.77407,45.20045 5.36972,-98.796827 5.36972,-98.796827"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 70.685564,-140.77512 c 41.860846,-12.9536 87.022456,5.64781 107.616976,44.325867 20.59452,38.678054 10.81771,86.5319798 -23.29745,114.032563 C 120.88994,45.083892 72.051222,44.480548 38.625847,16.145583 5.2004714,-12.189383 -3.3911751,-60.270229 18.152553,-98.427715"
       id="path877"
       inkscape:path-effect="#path-effect879"
       inkscape:original-d="M 70.076745,-142.74258 181.60529,-94.621018 116.6656,31.317321 18.575545,3.6396808 c -89.774074,45.2004502 5.36972,-98.7968288 5.36972,-98.7968288"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m -107.60577,89.217012 c 17.932603,-34.776914 57.436364,-52.196794 88.234121,-38.90839 30.797758,13.288404 41.227027,52.253038 23.294425,87.029958 -17.932602,34.77691 -57.436363,52.19679 -88.23412,38.90839 -30.797756,-13.28841 -41.227026,-52.25304 -23.294426,-87.029958 z"
       id="path881"
       inkscape:path-effect="#path-effect883"
       inkscape:original-d="M -75.135921,26.247839 36.392624,74.369405 -28.547072,200.30775 -126.63712,172.63011 c -89.77407,45.20045 5.36972,-98.796835 5.36972,-98.796835"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 63.523373,176.61266 c 17.9326,-34.77691 57.436357,-52.19679 88.234117,-38.90839 30.79776,13.2884 41.22703,52.25304 23.29442,87.02995 -17.9326,34.77691 -57.43635,52.19679 -88.234111,38.90839 C 56.020043,250.35421 45.590773,211.38957 63.523373,176.61266 Z M 142.58207,287.70339 31.053528,239.58183 95.993218,113.64349 207.52176,161.76505 Z"
       id="path885"
       inkscape:path-effect="#path-effect887"
       inkscape:original-d="M 95.993218,113.64349 207.52176,161.76505 142.58207,287.70339 44.492015,260.02575 c -89.77407,45.20045 5.36972,-98.79683 5.36972,-98.79683"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m -57.87853,180.14972 111.528543,48.12156 -64.93969,125.93834 -98.090053,-27.67764 c -89.77407,45.20045 5.36972,-98.79683 5.36972,-98.79683 m 8.490643,27.84089 c 11.726639,-36.67351 48.514605,-61.20814 82.168221,-54.79963 33.653617,6.40851 51.428945,41.33338 39.702306,78.00689 -11.726639,36.67351 -48.514605,61.20814 -82.168221,54.79963 -33.653617,-6.40851 -51.428949,-41.33338 -39.702306,-78.00689 z M 5.1182019,345.18656 -116.75232,321.9793 -74.286409,189.17278 47.584118,212.38004 Z M 26.35116,278.7833 -95.519367,255.57604 m 39.702306,78.00689 42.465915,-132.80652"
       id="path889"
       inkscape:path-effect="#path-effect891"
       inkscape:original-d="m -57.87853,180.14972 111.528543,48.12156 -64.93969,125.93834 -98.090053,-27.67764 c -89.77407,45.20045 5.36972,-98.79683 5.36972,-98.79683"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m -140.22675,-51.952078 -7.3055,147.370624 -64.93969,125.938344 -98.09005,-27.67764 c -89.77407,45.20045 5.36972,-98.796834 5.36972,-98.796834 m 148.29936,9.664414 c -18.70207,57.29733 -52.7282,106.39295 -75.99948,109.65816 -23.27129,3.2652 -26.97537,-40.53648 -8.2733,-97.8338 18.70207,-57.297326 52.7282,-106.3929552 75.99948,-109.6581603 23.27129,-3.2652052 26.97537,40.5364803 8.2733,97.8338003 z m -84.27278,11.82436 84.27278,-11.82436 m -8.2733,-97.8338003 -67.72618,207.4919603"
       id="path893"
       inkscape:path-effect="#path-effect895"
       inkscape:original-d="m -140.22675,-51.952078 -7.3055,147.370624 -64.93969,125.938344 -98.09005,-27.67764 c -89.77407,45.20045 5.36972,-98.796834 5.36972,-98.796834"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 143.03043,12.215175 -7.3055,147.370625 -64.939693,125.93833 -98.09005,-27.67764 c -89.774077,45.20045 5.36972,-98.79682 5.36972,-98.79682 M 79.571025,308.40156 C 64.505508,322.22879 68.832218,263.61236 89.235002,177.4781 109.63779,91.34385 138.39052,10.309063 153.45604,-3.5181598 168.52156,-17.345382 164.19485,41.271047 143.79206,127.4053 123.38928,213.53955 94.636541,294.57434 79.571025,308.40156 Z"
       id="path897"
       inkscape:path-effect="#path-effect899"
       inkscape:original-d="m 143.03043,12.215175 -7.3055,147.370625 -64.939693,125.93833 -98.09005,-27.67764 c -89.774077,45.20045 5.36972,-98.79682 5.36972,-98.79682"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 557.25507,-103.27761 -7.3055,147.370624 -64.93969,125.938336 -98.09005,-27.67764 c -89.77407,45.20045 5.36972,-98.796826 5.36972,-98.796826 M 493.79567,192.90878 c -15.06552,13.82723 -10.73881,-44.78921 9.66397,-130.92346 20.40279,-86.134254 49.15552,-167.16904 64.22104,-180.99627 15.06552,-13.82722 10.73881,44.789211 -9.66398,130.923466 C 537.61392,98.04677 508.86118,179.08156 493.79567,192.90878 Z m 101.16354,-336.95613 -73.88501,311.91973 -54.55706,50.0728 73.88501,-311.919724 z"
       id="path901"
       inkscape:path-effect="#path-effect903"
       inkscape:original-d="m 557.25507,-103.27761 -7.3055,147.370624 -64.93969,125.938336 -98.09005,-27.67764 c -89.77407,45.20045 5.36972,-98.796826 5.36972,-98.796826"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 311.0155,-65.577951 303.71,81.79267 238.77031,207.73101 140.68026,180.05337 c -89.774071,45.20045 5.36972,-98.79683 5.36972,-98.79683 m 119.97737,71.37197 c -7.53276,6.91361 -5.3694,-22.39461 4.83199,-65.461732 10.20139,-43.067128 24.57776,-83.5845223 32.11052,-90.4981339 7.53275,-6.9136121 5.3694,22.3946039 -4.83199,65.4617309 C 287.93648,105.1975 273.56011,145.7149 266.02735,152.62851 Z M 316.60912,-15.849557 279.66661,140.11031 252.38809,165.14671 289.33059,9.1868453 Z M 302.96986,-3.3313559 266.02735,152.62851 m 32.11052,-90.498135 -27.27853,25.036403"
       id="path905"
       inkscape:path-effect="#path-effect907"
       inkscape:original-d="M 311.0155,-65.577951 303.71,81.79267 238.77031,207.73101 140.68026,180.05337 c -89.774071,45.20045 5.36972,-98.79683 5.36972,-98.79683"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 69.227114,-222.67346 61.921614,-75.302847 -3.0180762,50.635497 -101.10813,22.957853 c -89.77407,45.200454 5.369723,-98.79683 5.369723,-98.79683 m 119.97737,71.3719714 c -7.532757,6.9136115 -5.369401,-22.3946034 4.831991,-65.4617304 10.201391,-43.067124 24.57776,-83.584524 32.110517,-90.498134 7.532757,-6.91361 5.369402,22.39461 -4.83199,65.461732 -10.201392,43.067127 -24.57776,83.584521 -32.110518,90.4981324 z M 61.181471,-160.42687 24.238963,-4.4670056 M 56.349481,-94.965138 29.070954,-69.928736"
       id="path909"
       inkscape:path-effect="#path-effect911"
       inkscape:original-d="M 69.227114,-222.67346 61.921614,-75.302847 -3.0180762,50.635497 -101.10813,22.957853 c -89.77407,45.200454 5.369723,-98.79683 5.369723,-98.79683"
       sodipodi:nodetypes="ccccc" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.241154px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 95.63474,228.45673 c -9.035866,13.56567 -9.148105,-9.04339 -0.250692,-50.49876 8.897412,-41.45538 23.435212,-86.058748 32.471072,-99.624424 9.03587,-13.565677 9.14811,9.043389 0.2507,50.498764 -8.89742,41.45537 -23.43521,86.05874 -32.47108,99.62442 z"
       id="path833"
       inkscape:path-effect="#path-effect835"
       inkscape:original-d="m 138.26183,13.168611 -7.3055,147.370619 -64.939694,125.93834 -98.090051,-27.67764 c -89.774075,45.20045 5.36972,-98.79683 5.36972,-98.79683"
       sodipodi:nodetypes="ccccc" />
  </g>
</svg>
)"""";

   testDoc(svg);
}