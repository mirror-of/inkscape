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

class LPEDashedStrokeTest : public LPESPathsTest {};

// TEST fails if multiple simplify on stack


// INKSCAPE 1.0.2

TEST_F(LPEDashedStrokeTest, multi_PX_1_0_2)
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
       effect="dashed_stroke"
       id="path-effect857"
       is_visible="true"
       lpeversion="1"
       numberdashes="18"
       holefactor="0.3"
       splitsegments="false"
       halfextreme="false"
       unifysegment="true"
       message="Add &lt;b&gt;&quot;Fill Between Many LPE&quot;&lt;/b&gt; to add fill." />
    <inkscape:path-effect
       effect="dashed_stroke"
       id="path-effect843"
       is_visible="true"
       lpeversion="1"
       numberdashes="36"
       holefactor="0"
       splitsegments="false"
       halfextreme="true"
       unifysegment="false"
       message="Add &lt;b&gt;&quot;Fill Between Many LPE&quot;&lt;/b&gt; to add fill." />
    <inkscape:path-effect
       effect="dashed_stroke"
       id="path-effect839"
       is_visible="true"
       lpeversion="1"
       numberdashes="3"
       holefactor="0"
       splitsegments="true"
       halfextreme="true"
       unifysegment="true"
       message="Add &lt;b&gt;&quot;Fill Between Many LPE&quot;&lt;/b&gt; to add fill." />
    <inkscape:path-effect
       effect="dashed_stroke"
       id="path-effect835"
       is_visible="true"
       lpeversion="1"
       numberdashes="6"
       holefactor="0.2"
       splitsegments="false"
       halfextreme="true"
       unifysegment="true"
       message="Add &lt;b&gt;&quot;Fill Between Many LPE&quot;&lt;/b&gt; to add fill." />
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
     inkscape:cy="805.48669"
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
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:23.7009;stroke-miterlimit:4;stroke-dasharray:none"
       id="path837"
       inkscape:path-effect="#path-effect839"
       sodipodi:type="arc"
       sodipodi:cx="185.69373"
       sodipodi:cy="188.79845"
       sodipodi:rx="245.37454"
       sodipodi:ry="255.91214"
       d="m 426.58408,139.84654 c 2.94281,15.84454 4.48419,32.21049 4.48419,48.95191 0,16.74141 -1.54138,33.10736 -4.48419,48.9519 m -35.54247,91.18597 c -17.65485,28.08271 -40.46299,52.32492 -67.02263,71.26466 m -89.45079,39.43278 c -15.79444,3.32942 -32.13879,5.07683 -48.87446,5.07683 -16.73568,0 -33.08003,-1.74741 -48.87447,-5.07683 M 47.368473,400.20098 C 20.808827,381.26124 -1.9993063,357.01903 -19.654157,328.93632 M -55.19663,237.75035 c -2.942813,-15.84454 -4.484187,-32.21049 -4.484187,-48.9519 0,-16.74142 1.541374,-33.10737 4.484187,-48.95191 m 35.542474,-91.185967 c 17.6548502,-28.08271 40.462984,-52.3249186 67.02263,-71.264655 m 89.450786,-39.432785 c 15.79444,-3.329419 32.13879,-5.076826 48.87447,-5.076826 16.73567,0 33.08002,1.747407 48.87446,5.076826 m 89.45079,39.432785 c 26.55964,18.9397369 49.36778,43.181945 67.02263,71.264656 m 35.54247,91.185966 c 2.94281,15.84454 4.48419,32.21049 4.48419,48.95191" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:23.7009;stroke-miterlimit:4;stroke-dasharray:none"
       id="path833"
       inkscape:path-effect="#path-effect835"
       sodipodi:type="arc"
       sodipodi:cx="1018.8273"
       sodipodi:cy="262.03192"
       sodipodi:rx="245.37454"
       sodipodi:ry="255.91214"
       d="m 1252.7096,184.40382 c 7.4644,24.48391 11.4922,50.56544 11.4922,77.6281 0,27.06267 -4.0278,53.1442 -11.4922,77.62811 m -49.8301,91.62135 c -33.9998,40.18644 -79.9107,69.15147 -131.9714,80.88657 m -104.16164,0 c -52.06074,-11.7351 -97.97158,-40.70013 -131.97141,-80.88658 m -49.83011,-91.62134 c -7.46444,-24.48392 -11.49221,-50.56544 -11.49221,-77.62811 0,-27.06266 4.02777,-53.14419 11.49221,-77.62811 M 834.77505,92.782467 C 868.77488,52.596024 914.68573,23.630988 966.74646,11.895889 m 104.16164,10e-7 c 52.0607,11.7351 97.9716,40.700137 131.9714,80.88658 m 49.8301,91.62135 c 7.4644,24.48391 11.4922,50.56544 11.4922,77.6281" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:23.7009;stroke-miterlimit:4;stroke-dasharray:none"
       id="rect841"
       width="655.96454"
       height="408.09076"
       x="-125.63578"
       y="628.59552"
       inkscape:path-effect="#path-effect843"
       d="m -125.63578,643.37407 v -14.77855 l 14.77855,0 m 29.557087,0 29.557092,0 m 29.557091,0 29.5570919,0 m 29.5570921,0 h 29.557091 m 29.557092,0 H 125.5995 m 29.55709,0 h 29.55709 m 29.55709,0 h 29.55709 m 29.5571,0 h 29.55709 m 29.55709,0 h 29.55709 m 29.55709,0 h 29.55709 m 29.5571,0 h 29.55709 m 29.55709,0 h 20.48707 v 9.07002 m 0,29.55709 v 29.5571 m 0,29.55709 v 29.55709 m 0,29.55709 v 29.55709 m 0,29.55709 v 29.5571 m 0,29.55709 v 29.55709 m 0,29.55709 v 29.55709 m 0,29.55706 v 14.7786 h -14.77855 m -29.55709,0 h -29.55709 m -29.55709,0 h -29.55709 m -29.5571,0 h -29.55709 m -29.55709,0 h -29.55709 m -29.55709,0 H 219.9793 m -29.5571,0 h -29.55709 m -29.55709,0 h -29.55709 m -29.557092,0 H 42.636747 m -29.557092,0 h -29.557091 m -29.557092,0 H -75.59162 m -29.55709,0 h -20.48707 v -9.07 m 0,-29.55713 v -29.5571 m 0,-29.55709 v -29.55709 m 0,-29.55709 v -29.55709 m 0,-29.55709 v -29.5571 m 0,-29.55709 v -29.55709 m 0,-29.55709 v -29.55709 m 0,-29.55709 v -14.77855"
       sodipodi:type="rect" />
    <g
       id="g855"
       inkscape:path-effect="#path-effect857"
       transform="matrix(2.3700906,0,0,2.3700906,188.36861,108.18804)">
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path845"
         cx="349.6864"
         cy="333.73978"
         rx="121.66125"
         ry="84.106392" />
      <path
         id="path847"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 528.81304,260.03356 c 3.03202,6.97963 4.65504,14.34745 4.65504,21.96339 0,7.61593 -1.62302,14.98375 -4.65504,21.96338 m -5.73327,10.3918 c -4.0315,5.97289 -9.14501,11.56286 -15.1779,16.66347 m -9.50302,7.12727 c -6.01906,4.01255 -12.67822,7.61237 -19.86927,10.72862 m -11.08671,4.27387 c -6.92146,2.35637 -14.23006,4.29279 -21.84695,5.75765 m -11.73831,1.8485 c -7.30931,0.90268 -14.8481,1.38023 -22.55409,1.39192 m -11.87796,-0.35396 c -7.70471,-0.47334 -15.20187,-1.41389 -22.42469,-2.77792 m -11.59244,-2.61056 c -7.52435,-1.97747 -14.67931,-4.42971 -21.3783,-7.30007 m -10.70709,-5.15012 c -6.92823,-3.72607 -13.2204,-7.94025 -18.75379,-12.56221 m -8.6064,-8.18422 c -5.32339,-5.80098 -9.55687,-12.07922 -12.51505,-18.71341 m -3.69785,-11.27259 c -0.78491,-3.66676 -1.19075,-7.41307 -1.19075,-11.22142 0,-3.80836 0.40584,-7.55467 1.19075,-11.22143 m 3.69785,-11.27259 c 2.95818,-6.63419 7.19166,-12.91244 12.51505,-18.71341 m 8.60641,-8.18422 c 5.53338,-4.62196 11.82555,-8.83614 18.75378,-12.56221 m 10.70709,-5.15012 c 6.699,-2.87036 13.85395,-5.3226 21.37831,-7.30007 m 11.59244,-2.61056 c 7.22281,-1.36403 14.71998,-2.30458 22.42468,-2.77792 m 11.87796,-0.35396 c 7.706,0.0117 15.24478,0.48924 22.55409,1.39192 m 11.73831,1.8485 c 7.6169,1.46486 14.92549,3.40128 21.84696,5.75765 m 11.08671,4.27387 c 7.19105,3.11625 13.85021,6.71607 19.86926,10.72862 m 9.50302,7.12727 c 6.0329,5.10061 11.1464,10.69058 15.1779,16.66347 m 5.73327,10.3918 c 3.03202,6.97963 4.65504,14.34745 4.65504,21.96338"
         inkscape:original-d="m 533.46808,281.99695 a 122.45444,80.146706 0 0 1 -122.45444,80.1467 122.45444,80.146706 0 0 1 -122.45444,-80.1467 122.45444,80.146706 0 0 1 122.45444,-80.14671 122.45444,80.146706 0 0 1 122.45444,80.14671 z" />
      <path
         sodipodi:type="star"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path850"
         sodipodi:sides="5"
         sodipodi:cx="304.26404"
         sodipodi:cy="345.39322"
         sodipodi:r1="105.2653"
         sodipodi:r2="52.632652"
         sodipodi:arg1="-0.45896326"
         sodipodi:arg2="0.16935527"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 374.07514,298.77112 24.56053,-0.0124 -14.92961,19.50197 m -7.84761,10.25103 -14.92961,19.50196 m -3.22586,11.04132 7.60142,23.35461 m 3.99562,12.27615 7.60143,23.35461 m -8.62485,-0.65804 -23.16097,-8.17246 m -12.17436,-4.29578 -21.08242,-7.43904 -1.78254,1.29647 m -10.44059,7.5936 -19.86259,14.44635 m -10.44059,7.5936 -14.00567,10.18654 0.18144,-7.23994 m 0.32345,-12.90597 0.61534,-24.55282 m 0.32345,-12.90597 0.30767,-12.27641 -9.93859,-7.21314 m -10.44826,-7.58304 -19.87718,-14.42628 m -10.44827,-7.58304 -5.86122,-4.2539 16.59962,-4.93732 m 12.37426,-3.68054 23.54127,-7.00202 m 12.37426,-3.68055 2.11268,-0.62838 6.89775,-21.26567 m 3.98321,-12.28018 7.57782,-23.36229 m 4.71661,-7.25083 13.93397,20.22534 m 7.32426,10.63127 13.93397,20.22533 m 10.73021,4.14481 24.56053,-0.0124 m 12.91002,-0.007 24.56053,-0.0124"
         inkscape:transform-center-x="4.6938556"
         inkscape:transform-center-y="-5.4826267" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEDashedStrokeTest, multi_MM_1_0_2)
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
       effect="dashed_stroke"
       id="path-effect857"
       is_visible="true"
       lpeversion="1"
       numberdashes="18"
       holefactor="0.3"
       splitsegments="false"
       halfextreme="false"
       unifysegment="true"
       message="Add &lt;b&gt;&quot;Fill Between Many LPE&quot;&lt;/b&gt; to add fill." />
    <inkscape:path-effect
       effect="dashed_stroke"
       id="path-effect843"
       is_visible="true"
       lpeversion="1"
       numberdashes="36"
       holefactor="0"
       splitsegments="false"
       halfextreme="true"
       unifysegment="false"
       message="Add &lt;b&gt;&quot;Fill Between Many LPE&quot;&lt;/b&gt; to add fill." />
    <inkscape:path-effect
       effect="dashed_stroke"
       id="path-effect839"
       is_visible="true"
       lpeversion="1"
       numberdashes="3"
       holefactor="0"
       splitsegments="true"
       halfextreme="true"
       unifysegment="true"
       message="Add &lt;b&gt;&quot;Fill Between Many LPE&quot;&lt;/b&gt; to add fill." />
    <inkscape:path-effect
       effect="dashed_stroke"
       id="path-effect835"
       is_visible="true"
       lpeversion="1"
       numberdashes="6"
       holefactor="0.2"
       splitsegments="false"
       halfextreme="true"
       unifysegment="true"
       message="Add &lt;b&gt;&quot;Fill Between Many LPE&quot;&lt;/b&gt; to add fill." />
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
     inkscape:cy="805.48669"
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
       id="path837"
       inkscape:path-effect="#path-effect839"
       sodipodi:type="arc"
       sodipodi:cx="-1.1285975"
       sodipodi:cy="34.011532"
       sodipodi:rx="103.5296"
       sodipodi:ry="107.97568"
       d="m 100.50902,13.357511 c 1.24164,6.685203 1.89198,13.590402 1.89198,20.654021 0,7.063618 -0.65034,13.968818 -1.89198,20.654021 M 85.512765,93.139175 C 78.063745,104.98797 68.440429,115.21636 57.234256,123.2075 m -37.741504,16.63767 c -6.664067,1.40476 -13.5601538,2.14204 -20.6213495,2.14204 -7.0611958,0 -13.9572825,-0.73728 -20.6213495,-2.14204 M -59.491451,123.2075 c -11.206173,-7.99114 -20.82949,-18.21953 -28.278509,-30.068325 m -14.99625,-38.473622 c -1.24165,-6.685203 -1.89199,-13.590403 -1.89199,-20.654021 0,-7.063619 0.65034,-13.968818 1.89199,-20.654022 m 14.99625,-38.473622 c 7.44902,-11.848792 17.072336,-22.07718 28.278509,-30.068325 m 37.741504,-16.63767 c 6.664067,-1.404764 13.5601538,-2.142039 20.6213495,-2.142039 7.0611958,0 13.9572825,0.737275 20.6213495,2.142039 m 37.741504,16.63767 c 11.206173,7.991145 20.82949,18.219534 28.278509,30.068326 m 14.996255,38.473622 c 1.24164,6.685203 1.89198,13.590402 1.89198,20.654021" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path833"
       inkscape:path-effect="#path-effect835"
       sodipodi:type="arc"
       sodipodi:cx="350.39111"
       sodipodi:cy="64.910553"
       sodipodi:rx="103.5296"
       sodipodi:ry="107.97568"
       d="m 449.07187,32.157331 c 3.14943,10.33037 4.84885,21.334813 4.84885,32.753222 0,11.418409 -1.69942,22.422853 -4.84885,32.753223 m -21.02456,38.657314 c -14.34537,16.95566 -33.7163,29.17673 -55.68201,34.12806 m -43.94837,0 c -21.96572,-4.95133 -41.33664,-17.1724 -55.68201,-34.12806 M 251.71036,97.663775 c -3.14943,-10.330369 -4.84885,-21.334813 -4.84885,-32.753222 0,-11.41841 1.69942,-22.422854 4.84885,-32.753224 m 21.02456,-38.6573175 c 14.34537,-16.9556565 33.71629,-29.1767235 55.68201,-34.1280535 m 43.94837,10e-7 c 21.96571,4.95133 41.33664,17.172397 55.68201,34.128054 m 21.02456,38.657318 c 3.14943,10.33037 4.84885,21.334813 4.84885,32.753222" />
    <path
       style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="rect841"
       width="276.7677"
       height="172.18361"
       x="-132.48624"
       y="219.57283"
       inkscape:path-effect="#path-effect843"
       d="m -132.48624,225.80827 v -6.23544 h 6.23544 m 12.47087,0 h 12.47087 m 12.470867,0 h 12.47087 m 12.47087,0 h 12.470869 m 12.47087,0 h 12.47087 m 12.47087,0 h 12.4708693 m 12.4708697,0 h 12.47087 m 12.470869,0 h 12.47087 m 12.47087,0 h 12.470869 m 12.47087,0 h 12.47087 m 12.470867,0 h 12.47087 m 12.47087,0 h 8.644 v 3.82687 m 0,12.47087 v 12.47087 m 0,12.47087 v 12.47087 m 0,12.47087 v 12.47087 m 0,12.47087 v 12.47087 m 0,12.47087 v 12.47087 m 0,12.47086 v 12.47087 m 0,12.47087 v 6.23544 h -6.23543 m -12.47087,0 h -12.47087 m -12.47087,0 H 88.16255 m -12.47087,0 H 63.220811 m -12.47087,0 h -12.47087 m -12.47087,0 H 13.337332 m -12.47086993,0 H -11.604408 m -12.470869,0 h -12.47087 m -12.47087,0 h -12.470869 m -12.47087,0 h -12.47087 m -12.47087,0 h -12.470874 m -12.47086,0 h -8.64401 v -3.82687 m 0,-12.47087 v -12.47087 m 0,-12.47087 v -12.47087 m 0,-12.47087 v -12.47087 m 0,-12.47087 v -12.47087 m 0,-12.47087 v -12.47087 m 0,-12.47087 v -12.47087 m 0,-12.47086 v -6.23544"
       sodipodi:type="rect" />
    <g
       id="g855"
       inkscape:path-effect="#path-effect857">
      <ellipse
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path845"
         cx="349.6864"
         cy="333.73978"
         rx="121.66125"
         ry="84.106392"
         d="m 467.00536,311.38993 c 2.83051,7.11688 4.34229,14.61131 4.34229,22.34985 0,7.73853 -1.51178,15.23295 -4.34229,22.34984 m -5.42852,10.72939 c -3.87694,6.25915 -8.82069,12.12738 -14.67316,17.49543 m -9.33803,7.5894 c -5.96562,4.30648 -12.58706,8.17528 -19.7534,11.5297 m -11.11249,4.62459 c -6.96302,2.5582 -14.32827,4.66242 -22.01314,6.25555 m -11.86776,2.01406 c -7.39641,0.98426 -15.03121,1.50544 -22.8388,1.51819 m -12.03156,-0.38617 c -7.80083,-0.51626 -15.38663,-1.54178 -22.68701,-3.02789 m -11.69739,-2.83999 c -7.5693,-2.1454 -14.7564,-4.80302 -21.47108,-7.91051 m -10.67931,-5.5507 c -6.85775,-3.98816 -13.06933,-8.49029 -18.51056,-13.42055 m -8.37053,-8.64187 c -5.09794,-6.03953 -9.13833,-12.55793 -11.94565,-19.43388 m -3.45733,-11.51317 c -0.72834,-3.72203 -1.10449,-7.52136 -1.10449,-11.38142 0,-3.86006 0.37615,-7.65939 1.10449,-11.38142 m 3.45733,-11.51317 c 2.80732,-6.87595 6.84771,-13.39436 11.94564,-19.43389 m 8.37054,-8.64187 c 5.44123,-4.93026 11.65281,-9.43239 18.51056,-13.42055 m 10.67931,-5.5507 c 6.71467,-3.10748 13.90177,-5.76511 21.47107,-7.9105 m 11.69739,-2.83999 c 7.30039,-1.48612 14.88618,-2.51163 22.68701,-3.02789 m 12.03157,-0.38618 c 7.80759,0.0128 15.44239,0.53394 22.8388,1.51819 m 11.86776,2.01406 c 7.68487,1.59313 15.05011,3.69735 22.01313,6.25555 m 11.1125,4.62459 c 7.16634,3.35442 13.78778,7.22322 19.75339,11.5297 m 9.33804,7.5894 c 5.85247,5.36805 10.79621,11.23628 14.67316,17.49543 m 5.42852,10.72939 c 2.83051,7.11688 4.34229,14.61131 4.34229,22.34984" />
      <path
         id="path847"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         d="m 528.81304,260.03356 c 3.03202,6.97963 4.65504,14.34745 4.65504,21.96339 0,7.61593 -1.62302,14.98375 -4.65504,21.96338 m -5.73327,10.3918 c -4.0315,5.97289 -9.14501,11.56286 -15.1779,16.66347 m -9.50302,7.12727 c -6.01906,4.01255 -12.67822,7.61237 -19.86927,10.72862 m -11.08671,4.27387 c -6.92146,2.35637 -14.23006,4.29279 -21.84695,5.75765 m -11.73831,1.8485 c -7.30931,0.90268 -14.8481,1.38023 -22.55409,1.39192 m -11.87796,-0.35396 c -7.70471,-0.47334 -15.20187,-1.41389 -22.42469,-2.77792 m -11.59244,-2.61056 c -7.52435,-1.97747 -14.67931,-4.42971 -21.3783,-7.30007 m -10.70709,-5.15012 c -6.92823,-3.72607 -13.2204,-7.94025 -18.75379,-12.56221 m -8.6064,-8.18422 c -5.32339,-5.80098 -9.55687,-12.07922 -12.51505,-18.71341 m -3.69785,-11.27259 c -0.78491,-3.66676 -1.19075,-7.41307 -1.19075,-11.22142 0,-3.80836 0.40584,-7.55467 1.19075,-11.22143 m 3.69785,-11.27259 c 2.95818,-6.63419 7.19166,-12.91244 12.51505,-18.71341 m 8.60641,-8.18422 c 5.53338,-4.62196 11.82555,-8.83614 18.75378,-12.56221 m 10.70709,-5.15012 c 6.699,-2.87036 13.85395,-5.3226 21.37831,-7.30007 m 11.59244,-2.61056 c 7.22281,-1.36403 14.71998,-2.30458 22.42468,-2.77792 m 11.87796,-0.35396 c 7.706,0.0117 15.24478,0.48924 22.55409,1.39192 m 11.73831,1.8485 c 7.6169,1.46486 14.92549,3.40128 21.84696,5.75765 m 11.08671,4.27387 c 7.19105,3.11625 13.85021,6.71607 19.86926,10.72862 m 9.50302,7.12727 c 6.0329,5.10061 11.1464,10.69058 15.1779,16.66347 m 5.73327,10.3918 c 3.03202,6.97963 4.65504,14.34745 4.65504,21.96338"
         inkscape:original-d="m 533.46808,281.99695 a 122.45444,80.146706 0 0 1 -122.45444,80.1467 122.45444,80.146706 0 0 1 -122.45444,-80.1467 122.45444,80.146706 0 0 1 122.45444,-80.14671 122.45444,80.146706 0 0 1 122.45444,80.14671 z" />
      <path
         sodipodi:type="star"
         style="fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="path850"
         sodipodi:sides="5"
         sodipodi:cx="304.26404"
         sodipodi:cy="345.39322"
         sodipodi:r1="105.2653"
         sodipodi:r2="52.632652"
         sodipodi:arg1="-0.45896326"
         sodipodi:arg2="0.16935527"
         inkscape:flatsided="false"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 374.07514,298.77112 24.56053,-0.0124 -14.92961,19.50197 m -7.84761,10.25103 -14.92961,19.50196 m -3.22586,11.04132 7.60142,23.35461 m 3.99562,12.27615 7.60143,23.35461 m -8.62485,-0.65804 -23.16097,-8.17246 m -12.17436,-4.29578 -21.08242,-7.43904 -1.78254,1.29647 m -10.44059,7.5936 -19.86259,14.44635 m -10.44059,7.5936 -14.00567,10.18654 0.18144,-7.23994 m 0.32345,-12.90597 0.61534,-24.55282 m 0.32345,-12.90597 0.30767,-12.27641 -9.93859,-7.21314 m -10.44826,-7.58304 -19.87718,-14.42628 m -10.44827,-7.58304 -5.86122,-4.2539 16.59962,-4.93732 m 12.37426,-3.68054 23.54127,-7.00202 m 12.37426,-3.68055 2.11268,-0.62838 6.89775,-21.26567 m 3.98321,-12.28018 7.57782,-23.36229 m 4.71661,-7.25083 13.93397,20.22534 m 7.32426,10.63127 13.93397,20.22533 m 10.73021,4.14481 24.56053,-0.0124 m 12.91002,-0.007 24.56053,-0.0124"
         inkscape:transform-center-x="4.6938556"
         inkscape:transform-center-y="-5.4826267" />
    </g>
  </g>
</svg>
)"""";

   testDoc(svg);
}
