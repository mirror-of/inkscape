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
#include <src/document.h>
#include <src/inkscape.h>
#include <src/live_effects/lpe-bool.h>
#include <src/object/sp-ellipse.h>
#include <src/object/sp-lpe-item.h>

using namespace Inkscape;
using namespace Inkscape::LivePathEffect;


class LPEBoolTest : public LPESPathsTest {};

TEST_F(LPEBoolTest, canBeApplyedToNonSiblingPaths)
{
    std::string svg("\
<svg width='100' height='100'\
  xmlns:sodipodi='http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd'\
  xmlns:inkscape='http://www.inkscape.org/namespaces/inkscape'>\
  <defs>\
    <inkscape:path-effect\
      id='path-effect1'\
      effect='bool_op'\
      operation='diff'\
      operand-path='#circle1'\
      lpeversion='1'\
      hide-linked='true' />\
  </defs>\
  <path id='rect1'\
    inkscape:path-effect='#path-effect1'\
    sodipodi:type='rect'\
    width='100' height='100' fill='#ff0000' />\
  <g id='group1'>\
    <circle id='circle1'\
      r='40' cy='50' cx='50' fill='#ffffff' style='display:inline'/>\
  </g>\
</svg>");

    SPDocument *doc = SPDocument::createNewDocFromMem(svg.c_str(), svg.size(), true);
    doc->ensureUpToDate();

    auto lpe_item = dynamic_cast<SPLPEItem *>(doc->getObjectById("rect1"));
    ASSERT_TRUE(lpe_item != nullptr);

    auto lpe_bool_op_effect = dynamic_cast<LPEBool *>(lpe_item->getPathEffectOfType(EffectType::BOOL_OP));
    ASSERT_TRUE(lpe_bool_op_effect != nullptr);

    auto operand_path = lpe_bool_op_effect->getParameter("operand-path")->param_getSVGValue();
    auto circle = dynamic_cast<SPGenericEllipse *>(doc->getObjectById(operand_path.substr(1)));
    ASSERT_TRUE(circle != nullptr);
}

// INKSCAPE 1.0.2

TEST_F(LPEBoolTest, multi_PX_1_1)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   width="793.70081"
   height="1122.5197"
   viewBox="0 0 793.70081 1122.5197"
   version="1.1"
   id="svg5"
   inkscape:version="1.2-dev (99544120e7, 2021-04-09)"
   sodipodi:docname="1.svg"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <sodipodi:namedview
     id="namedview7"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     objecttolerance="10.0"
     gridtolerance="10.0"
     guidetolerance="10.0"
     inkscape:pageshadow="2"
     inkscape:pageopacity="0.0"
     inkscape:pagecheckerboard="0"
     inkscape:document-units="px"
     showgrid="false"
     inkscape:zoom="0.32"
     inkscape:cx="23.4375"
     inkscape:cy="520.3125"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     inkscape:current-layer="layer1"
     units="px" />
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="bool_op"
       id="path-effect1716"
       is_visible="true"
       lpeversion="1"
       operand-path="#path1700"
       operation="inters"
       swap-operands="false"
       rmv-inner="false"
       filltype-this="from-curve"
       filter=""
       filltype-operand="from-curve" />
    <inkscape:path-effect
       effect="bool_op"
       id="path-effect1664"
       is_visible="true"
       lpeversion="1"
       operand-path="#path1568"
       operation="diff"
       swap-operands="false"
       rmv-inner="false"
       filltype-this="from-curve"
       filter=""
       filltype-operand="from-curve" />
    <inkscape:path-effect
       effect="bool_op"
       id="path-effect1347"
       is_visible="true"
       lpeversion="1"
       operand-path="#path1251"
       operation="cut"
       swap-operands="false"
       rmv-inner="false"
       filltype-this="from-curve"
       filter=""
       filltype-operand="from-curve" />
    <filter
       id="selectable_hidder_filter"
       width="1.3961189"
       height="1.3961189"
       x="-0.21637641"
       y="-0.1956862"
       style="color-interpolation-filters:sRGB"
       inkscape:label="LPE boolean visibility">
      <feComposite
         id="boolops_hidder_primitive"
         result="composite1"
         operator="arithmetic"
         in2="SourceGraphic"
         in="BackgroundImage"
         k1="0"
         k2="0"
         k3="0"
         k4="0" />
    </filter>
    <inkscape:path-effect
       effect="bool_op"
       id="path-effect908"
       is_visible="true"
       lpeversion="1"
       operand-path="#path892"
       operation="union"
       swap-operands="false"
       rmv-inner="false"
       filltype-this="from-curve"
       filter=""
       filltype-operand="from-curve" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       id="path1204"
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10"
       d="m 201.486,138.78849 17.63349,-9.08084 25.42326,13.64186 -4.63652,-28.47739 20.83049,-19.963338 -28.516,-4.390246 -4.20746,-8.710122 c -15.5844,8.489872 -26.59934,29.304496 -26.59935,53.638056 0,1.12214 0.0259,2.2357 0.0721,3.34202 z m 0,0 c 1.2666,30.32172 19.65063,54.37734 42.14671,54.37734 23.31685,0 42.21876,-25.84185 42.21876,-57.71936 0,-31.87752 -18.90191,-57.719372 -42.21876,-57.719371 -5.51806,0 -10.78793,1.449258 -15.61945,4.081315 l 4.20746,8.710122 28.516,4.390246 -20.83049,19.963338 4.63652,28.47739 -25.42326,-13.64186 z"
       inkscape:path-effect="#path-effect1347"
       inkscape:original-d="m 285.85159,135.44647 a 42.218899,57.719349 0 0 1 -42.2189,57.71935 42.218899,57.719349 0 0 1 -42.2189,-57.71935 42.218899,57.719349 0 0 1 42.2189,-57.719347 42.218899,57.719349 0 0 1 42.2189,57.719347 z"
       transform="matrix(4.5519952,0,0,4.5519952,-91.921108,-319.13174)" />
    <path
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path846"
       inkscape:path-effect="#path-effect908"
       sodipodi:type="arc"
       sodipodi:cx="43.595745"
       sodipodi:cy="101.16588"
       sodipodi:rx="58.604801"
       sodipodi:ry="57.487537"
       d="m 66.764145,22.523932 c -23.160857,0 -42.641981,11.332007 -48.33689,26.713903 -19.7707355,9.237176 -33.436146,29.016325 -33.436146,51.928205 0,31.74949 26.238001,57.48724 58.604538,57.48724 32.366539,0 58.604963,-25.73775 58.604963,-57.48724 0,-5.446231 -0.77266,-10.715793 -2.215284,-15.708681 10.307114,-6.596483 16.803244,-16.212282 16.803244,-26.921575 0,-19.889014 -22.396914,-36.011852 -50.024425,-36.011852 z"
       transform="matrix(4.5519952,0,0,4.5519952,85.117274,123.72117)" />
    <ellipse
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none;filter:url(#selectable_hidder_filter)"
       id="path892"
       cx="80.808784"
       cy="95.01683"
       rx="50.024033"
       ry="36.012249"
       transform="matrix(4.5519952,0,0,4.5519952,21.187096,-42.339506)" />
    <path
       sodipodi:type="star"
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none;filter:url(#selectable_hidder_filter)"
       id="path1251"
       inkscape:flatsided="false"
       sodipodi:sides="5"
       sodipodi:cx="752.93793"
       sodipodi:cy="209.50652"
       sodipodi:r1="269.02332"
       sodipodi:r2="134.51166"
       sodipodi:arg1="0.95094102"
       sodipodi:arg2="1.5792595"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="M 909.21825,428.48141 751.79955,344.01336 592.9737,425.8049 624.66254,249.98875 497.79428,124.21137 674.79776,100.01907 755.2147,-59.507165 832.92003,101.35732 1009.4887,128.54206 880.50976,252.15408 Z"
       transform="matrix(0.7351515,0,0,0.7351515,352.82315,18.395026)"
       inkscape:transform-center-x="-0.11362542"
       inkscape:transform-center-y="-4.040657" />
    <g
       id="g1544"
       transform="matrix(3.0936832,0,0,3.0936832,277.33356,195.8907)"
       inkscape:path-effect="#path-effect1664">
      <ellipse
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse1511"
         cx="212.31941"
         cy="215.65012"
         rx="35.093342"
         ry="34.631622"
         d="m 212.31912,181.01841 c -19.38152,0 -35.09294,15.50491 -35.09294,34.63142 0,19.12652 15.71142,34.63206 35.09294,34.63206 19.38151,0 35.09356,-15.50554 35.09356,-34.63206 0,-19.12651 -15.71205,-34.63142 -35.09356,-34.63142 z m -5.08155,17.0938 c 13.17129,0 23.84836,9.41325 23.84836,21.02507 0,11.61182 -10.67707,21.02508 -23.84836,21.02508 -13.1713,0 -23.849,-9.41326 -23.849,-21.02508 0,-11.61182 10.6777,-21.02507 23.849,-21.02507 z" />
      <path
         id="path1381"
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10"
         d="m 196.62938,188.58864 c -19.38152,0 -35.09356,15.50491 -35.09356,34.63143 0,19.12652 15.71204,34.63144 35.09356,34.63143 19.38152,0 35.09293,-15.50491 35.09293,-34.63143 0,-19.12651 -15.71141,-34.63143 -35.09293,-34.63143 z m 10.60819,9.52357 c 13.17129,0 23.84836,9.41325 23.84836,21.02507 0,11.61182 -10.67707,21.02508 -23.84836,21.02508 -13.1713,0 -23.849,-9.41326 -23.849,-21.02508 0,-11.61182 10.6777,-21.02507 23.849,-21.02507 z"
         inkscape:original-d="m 231.72248,223.22003 a 35.093342,34.631622 0 0 1 -35.09334,34.63162 35.093342,34.631622 0 0 1 -35.09335,-34.63162 35.093342,34.631622 0 0 1 35.09335,-34.63162 35.093342,34.631622 0 0 1 35.09334,34.63162 z" />
      <path
         sodipodi:type="star"
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none"
         id="path1513"
         inkscape:flatsided="false"
         sodipodi:sides="5"
         sodipodi:cx="736.46674"
         sodipodi:cy="843.005"
         sodipodi:r1="144.44247"
         sodipodi:r2="72.221237"
         sodipodi:arg1="0.51800245"
         sodipodi:arg2="1.146321"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 676.97895,711.38022 -1.30998,38.51669 c 0.1359,-5.2e-4 0.26959,-0.01 0.40564,-0.01 49.78128,0 90.13556,35.57766 90.13556,79.46485 0,40.51076 -34.39364,73.91645 -78.8373,78.82061 l 19.85488,76.28413 58.98242,-75.63989 95.75006,5.7076 -53.71143,-79.46963 35.01625,-89.30042 -92.17808,26.52407 z"
         transform="matrix(0.26458333,0,0,0.26458333,28.359495,-0.29549027)"
         inkscape:transform-center-x="2.3906972"
         inkscape:transform-center-y="1.3001589" />
      <path
         id="path1405"
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953"
         inkscape:transform-center-x="2.3906972"
         inkscape:transform-center-y="1.3001589"
         transform="matrix(0.26458333,0,0,0.26458333,12.669221,7.2744254)"
         d="m 676.97861,711.38144 -1.00694,29.62125 c 6.51973,-5.04284 13.8682,-9.26634 21.84728,-12.49609 z m 166.28612,34.3744 -33.81368,9.73058 c 8.40529,10.67274 13.92822,23.23911 15.55271,36.83688 z m -194.37787,77.36507 -55.98547,35.78698 90.16659,32.72321 24.15943,92.82474 58.98243,-75.63993 95.75007,5.70998 -49.27806,-72.91257 c -15.76118,23.12039 -44.47998,38.59305 -77.30541,38.59305 -40.96328,0 -75.52382,-24.09863 -86.48958,-57.08546 z"
         inkscape:original-d="m 861.9598,914.52507 -95.74927,-5.70811 -58.98392,75.63996 -24.15943,-92.82687 -90.1649,-32.72302 80.81792,-51.66206 3.25895,-95.86388 74.10765,60.89796 92.17905,-26.52413 -35.01688,89.29907 z" />
      <path
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="rect1515"
         width="44.248779"
         height="63.078419"
         x="164.83891"
         y="172.53653"
         d="m 164.83892,172.53653 v 63.07838 h 27.59467 c -5.50801,-3.85129 -9.04502,-9.79849 -9.04502,-16.47763 0,-11.61182 10.6777,-21.02507 23.849,-21.02507 0.62351,0 1.23895,0.0274 1.85042,0.0688 v -25.64449 z"
         sodipodi:type="rect" />
      <path
         id="rect1429"
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10"
         d="m 149.14856,180.10614 v 63.07901 h 44.24906 v -6.9307 c -6.05647,-3.81281 -10.00905,-10.05605 -10.00905,-17.11717 0,-7.06111 3.95258,-13.30435 10.00905,-17.11716 v -21.91398 z"
         inkscape:original-d="m 149.14864,180.10645 h 44.24878 v 63.07841 h -44.24878 z" />
    </g>
    <ellipse
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none;filter:url(#selectable_hidder_filter)"
       id="path1568"
       cx="197.11641"
       cy="201.26811"
       rx="16.208378"
       ry="14.28931"
       transform="matrix(4.5519952,0,0,4.5519952,21.187096,-42.339506)" />
    <path
       sodipodi:type="star"
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none"
       id="path1698"
       inkscape:flatsided="false"
       sodipodi:sides="5"
       sodipodi:cx="171.96014"
       sodipodi:cy="934.35638"
       sodipodi:r1="200.09505"
       sodipodi:r2="100.04752"
       sodipodi:arg1="0.66654447"
       sodipodi:arg2="1.294863"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m 105.59294,806.0407 -10.049571,54.44799 -1.414103,13.11288 -62.981278,34.02459 65.316501,29.29402 12.897241,70.41182 48.04234,-53.06632 70.95184,9.49175 -35.62349,-62.09101 30.4114,-63.41586 -13.13725,1.74006 -56.3794,11.82368 z"
       transform="matrix(1.204382,0,0,1.204382,21.187096,-42.339506)"
       inkscape:transform-center-x="20.29295"
       inkscape:transform-center-y="-2.8480539"
       inkscape:path-effect="#path-effect1716" />
    <path
       sodipodi:type="star"
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none;filter:url(#selectable_hidder_filter)"
       id="path1700"
       inkscape:flatsided="false"
       sodipodi:sides="5"
       sodipodi:cx="138.8714"
       sodipodi:cy="903.65356"
       sodipodi:r1="107.79645"
       sodipodi:r2="53.898228"
       sodipodi:arg1="0.59145707"
       sodipodi:arg2="1.2197756"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m 228.35637,963.75776 -70.95172,-9.49256 -48.04336,53.0668 -12.897324,-70.41241 -65.315788,-29.29339 62.980731,-34.02473 7.675981,-71.17117 51.82156,49.38402 70.05981,-14.69281 -30.95325,64.54574 z"
       transform="matrix(1.204382,0,0,1.204382,21.187096,-42.339506)"
       inkscape:transform-center-x="2.4127689"
       inkscape:transform-center-y="0.3248026" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEBoolTest, multi_MM_1_1)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   width="210mm"
   height="297mm"
   viewBox="0 0 210 297"
   version="1.1"
   id="svg5"
   inkscape:version="1.2-dev (99544120e7, 2021-04-09)"
   sodipodi:docname="1.svg"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:svg="http://www.w3.org/2000/svg">
  <sodipodi:namedview
     id="namedview7"
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1.0"
     objecttolerance="10.0"
     gridtolerance="10.0"
     guidetolerance="10.0"
     inkscape:pageshadow="2"
     inkscape:pageopacity="0.0"
     inkscape:pagecheckerboard="0"
     inkscape:document-units="mm"
     showgrid="false"
     inkscape:zoom="0.74653479"
     inkscape:cx="397.16836"
     inkscape:cy="560.59008"
     inkscape:window-width="1920"
     inkscape:window-height="1016"
     inkscape:window-x="0"
     inkscape:window-y="27"
     inkscape:window-maximized="1"
     inkscape:current-layer="layer1" />
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="bool_op"
       id="path-effect1716"
       is_visible="true"
       lpeversion="1"
       operand-path="#path1700"
       operation="inters"
       swap-operands="false"
       rmv-inner="false"
       filltype-this="from-curve"
       filter=""
       filltype-operand="from-curve" />
    <inkscape:path-effect
       effect="bool_op"
       id="path-effect1664"
       is_visible="true"
       lpeversion="1"
       operand-path="#path1568"
       operation="diff"
       swap-operands="false"
       rmv-inner="false"
       filltype-this="from-curve"
       filter=""
       filltype-operand="from-curve" />
    <inkscape:path-effect
       effect="bool_op"
       id="path-effect1347"
       is_visible="true"
       lpeversion="1"
       operand-path="#path1251"
       operation="cut"
       swap-operands="false"
       rmv-inner="false"
       filltype-this="from-curve"
       filter=""
       filltype-operand="from-curve" />
    <filter
       id="selectable_hidder_filter"
       width="1.3961189"
       height="1.3961189"
       x="-0.21637641"
       y="-0.1956862"
       style="color-interpolation-filters:sRGB;"
       inkscape:label="LPE boolean visibility">
      <feComposite
         id="boolops_hidder_primitive"
         result="composite1"
         operator="arithmetic"
         in2="SourceGraphic"
         in="BackgroundImage" />
    </filter>
    <inkscape:path-effect
       effect="bool_op"
       id="path-effect908"
       is_visible="true"
       lpeversion="1"
       operand-path="#path892"
       operation="union"
       swap-operands="false"
       rmv-inner="false"
       filltype-this="from-curve"
       filter=""
       filltype-operand="from-curve" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       id="path1204"
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10"
       d="m 201.48867,138.78531 17.63086,-9.07813 25.42383,13.64258 -4.63672,-28.47656 20.83008,-19.964845 -28.51562,-4.390624 -4.20117,-8.697266 c -15.58554,8.489129 -26.60547,29.292285 -26.60547,53.626955 0,1.12159 0.0281,2.2321 0.0742,3.33789 z m 0,0 c 1.26601,30.32249 19.64807,54.38086 42.14454,54.38086 23.31685,0 42.21875,-25.84124 42.21875,-57.71875 0,-31.87752 -18.9019,-57.720706 -42.21875,-57.720705 -5.51723,0 -10.78235,1.462435 -15.61328,4.09375 l 4.20117,8.697266 28.51562,4.390624 -20.83008,19.964845 4.63672,28.47656 -25.42383,-13.64258 z"
       inkscape:path-effect="#path-effect1347"
       inkscape:original-d="m 285.85159,135.44647 a 42.218899,57.719349 0 0 1 -42.2189,57.71935 42.218899,57.719349 0 0 1 -42.2189,-57.71935 42.218899,57.719349 0 0 1 42.2189,-57.719347 42.218899,57.719349 0 0 1 42.2189,57.719347 z"
       transform="translate(-24.84805,-60.806793)" />
    <path
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
       id="path846"
       inkscape:path-effect="#path-effect908"
       sodipodi:type="arc"
       sodipodi:cx="43.595745"
       sodipodi:cy="101.16588"
       sodipodi:rx="58.604801"
       sodipodi:ry="57.487537"
       d="m 66.764164,22.523051 c -23.160711,0 -42.636983,11.335048 -48.332032,26.716797 -19.772508,9.236517 -33.441406,29.012627 -33.441406,51.925782 0,31.74949 26.238931,57.48828 58.605469,57.48828 32.366538,0 58.603515,-25.73879 58.603515,-57.48828 0,-5.447245 -0.78733,-10.711365 -2.23047,-15.705075 10.31365,-6.59669 16.81836,-16.21064 16.81836,-26.923832 0,-19.889016 -22.39593,-36.013672 -50.023436,-36.013672 z"
       transform="translate(14.04443,36.480855)" />
    <ellipse
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none;filter:url(#selectable_hidder_filter)"
       id="path892"
       cx="80.808784"
       cy="95.01683"
       rx="50.024033"
       ry="36.012249" />
    <path
       sodipodi:type="star"
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none;filter:url(#selectable_hidder_filter)"
       id="path1251"
       inkscape:flatsided="false"
       sodipodi:sides="5"
       sodipodi:cx="752.93793"
       sodipodi:cy="209.50652"
       sodipodi:r1="269.02332"
       sodipodi:r2="134.51166"
       sodipodi:arg1="0.95094102"
       sodipodi:arg2="1.5792595"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="M 909.21825,428.48141 751.79955,344.01336 592.9737,425.8049 624.66254,249.98875 497.79428,124.21137 674.79776,100.01907 755.2147,-59.507165 832.92003,101.35732 1009.4887,128.54206 880.50976,252.15408 Z"
       transform="matrix(0.16150094,0,0,0.16150094,72.855098,13.342398)"
       inkscape:transform-center-x="-0.11362542"
       inkscape:transform-center-y="-4.040657" />
    <g
       id="g1544"
       transform="matrix(0.67963235,0,0,0.67963235,56.271251,52.335337)"
       inkscape:path-effect="#path-effect1664">
      <ellipse
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="ellipse1511"
         cx="212.31941"
         cy="215.65012"
         rx="35.093342"
         ry="34.631622"
         d="m 212.31929,181.01845 c -19.38151,0 -35.09193,15.5056 -35.09193,34.63212 0,19.12652 15.71042,34.63212 35.09193,34.63212 19.38152,0 35.09481,-15.5056 35.09481,-34.63212 -1e-5,-19.12652 -15.71329,-34.63212 -35.09481,-34.63212 z m -5.08086,17.09334 c 13.17129,0 23.84676,9.41287 23.84676,21.02469 0,11.61183 -10.67547,21.02469 -23.84676,21.02469 -13.1713,0 -23.84965,-9.41286 -23.84965,-21.02469 0,-11.61182 10.67835,-21.02469 23.84965,-21.02469 z" />
      <path
         id="path1381"
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10"
         d="m 196.62837,188.58803 c -19.38153,0 -35.09193,15.5056 -35.09193,34.63212 0,19.12651 15.7104,34.63212 35.09193,34.63212 19.38152,0 35.09479,-15.50561 35.09479,-34.63212 0,-19.12652 -15.71327,-34.63212 -35.09479,-34.63212 z m 10.61006,9.52376 c 13.17129,0 23.84676,9.41287 23.84676,21.02469 0,11.61183 -10.67547,21.02469 -23.84676,21.02469 -13.1713,0 -23.84965,-9.41286 -23.84965,-21.02469 0,-11.61182 10.67835,-21.02469 23.84965,-21.02469 z"
         inkscape:original-d="m 231.72248,223.22003 a 35.093342,34.631622 0 0 1 -35.09334,34.63162 35.093342,34.631622 0 0 1 -35.09335,-34.63162 35.093342,34.631622 0 0 1 35.09335,-34.63162 35.093342,34.631622 0 0 1 35.09334,34.63162 z" />
      <path
         sodipodi:type="star"
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none"
         id="path1513"
         inkscape:flatsided="false"
         sodipodi:sides="5"
         sodipodi:cx="736.46674"
         sodipodi:cy="843.005"
         sodipodi:r1="144.44247"
         sodipodi:r2="72.221237"
         sodipodi:arg1="0.51800245"
         sodipodi:arg2="1.146321"
         inkscape:rounded="0"
         inkscape:randomized="0"
         d="m 676.97938,711.38145 -1.31427,38.52604 c 0.13914,-5.6e-4 0.27344,-0.0217 0.41275,-0.0217 49.78128,0 90.12949,35.5762 90.12949,79.4634 0,40.50831 -34.40452,73.87185 -78.84433,78.7791 l 19.86588,76.32442 58.97845,-75.64012 95.7558,5.71318 -53.71056,-79.47425 35.01775,-89.29315 -92.18233,26.52401 z"
         transform="matrix(0.26458333,0,0,0.26458333,28.359495,-0.29549027)"
         inkscape:transform-center-x="2.3906972"
         inkscape:transform-center-y="1.3001589" />
      <path
         id="path1405"
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953"
         inkscape:transform-center-x="2.3906972"
         inkscape:transform-center-y="1.3001589"
         transform="matrix(0.26458333,0,0,0.26458333,12.669221,7.2744254)"
         d="m 676.97686,711.38017 -1.01012,29.70647 c 6.5365,-5.06369 13.89116,-9.29516 21.89697,-12.5343 z m 166.29102,34.37694 -33.84473,9.74283 c 8.41416,10.68205 13.95544,23.24137 15.57551,36.85342 z m -194.3465,77.33454 -56.0241,35.81065 90.17297,32.726 24.15619,92.83404 58.9893,-75.64017 95.74495,5.70239 -49.33338,-72.98991 c -15.75615,23.13922 -44.40606,38.66725 -77.24763,38.66725 -40.9706,0 -75.49903,-24.11431 -86.4583,-57.11025 z"
         inkscape:original-d="m 861.9598,914.52507 -95.74927,-5.70811 -58.98392,75.63996 -24.15943,-92.82687 -90.1649,-32.72302 80.81792,-51.66206 3.25895,-95.86388 74.10765,60.89796 92.17905,-26.52413 -35.01688,89.29907 z" />
      <path
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none"
         id="rect1515"
         width="44.248779"
         height="63.078419"
         x="164.83891"
         y="172.53653"
         d="m 164.83843,172.53788 v 63.07695 h 27.61431 c -5.51445,-3.85105 -9.06396,-9.79466 -9.06396,-16.47835 0,-11.61182 10.67835,-21.02469 23.84965,-21.02469 0.6242,0 1.23571,0.0418 1.84785,0.0833 v -25.65725 z"
         sodipodi:type="rect" />
      <path
         id="rect1429"
         style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10"
         d="m 149.1475,180.10746 v 63.07695 h 44.25072 v -6.94597 c -6.05117,-3.81335 -10.00944,-10.04448 -10.00944,-17.10196 0,-7.05747 3.95827,-13.28574 10.00944,-17.09909 v -21.92993 z"
         inkscape:original-d="m 149.14864,180.10645 h 44.24878 v 63.07841 h -44.24878 z" />
    </g>
    <ellipse
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:10;stroke-miterlimit:4;stroke-dasharray:none;filter:url(#selectable_hidder_filter)"
       id="path1568"
       cx="197.11641"
       cy="201.26811"
       rx="16.208378"
       ry="14.28931" />
    <path
       sodipodi:type="star"
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none"
       id="path1698"
       inkscape:flatsided="false"
       sodipodi:sides="5"
       sodipodi:cx="171.96014"
       sodipodi:cy="934.35638"
       sodipodi:r1="200.09505"
       sodipodi:r2="100.04752"
       sodipodi:arg1="0.66654447"
       sodipodi:arg2="1.294863"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m 105.59055,806.0433 -10.054131,54.51526 -1.409941,13.04379 -62.974903,34.02316 65.314964,29.29134 12.896161,70.41585 48.04134,-53.06842 70.95472,9.49311 -35.625,-62.08906 30.406,-63.41782 -13.17667,1.7495 -56.33858,11.81102 z"
       transform="scale(0.26458333)"
       inkscape:transform-center-x="4.4571803"
       inkscape:transform-center-y="-0.62522407"
       inkscape:path-effect="#path-effect1716" />
    <path
       sodipodi:type="star"
       style="fill:#00ff00;fill-rule:evenodd;stroke:#008000;stroke-width:37.7953;stroke-miterlimit:4;stroke-dasharray:none;filter:url(#selectable_hidder_filter)"
       id="path1700"
       inkscape:flatsided="false"
       sodipodi:sides="5"
       sodipodi:cx="138.8714"
       sodipodi:cy="903.65356"
       sodipodi:r1="107.79645"
       sodipodi:r2="53.898228"
       sodipodi:arg1="0.59145707"
       sodipodi:arg2="1.2197756"
       inkscape:rounded="0"
       inkscape:randomized="0"
       d="m 228.35637,963.75776 -70.95172,-9.49256 -48.04336,53.0668 -12.897324,-70.41241 -65.315788,-29.29339 62.980731,-34.02473 7.675981,-71.17117 51.82156,49.38402 70.05981,-14.69281 -30.95325,64.54574 z"
       transform="scale(0.26458333)"
       inkscape:transform-center-x="2.4127689"
       inkscape:transform-center-y="0.3248026" />
  </g>
</svg>
)"""";

   testDoc(svg);
}
