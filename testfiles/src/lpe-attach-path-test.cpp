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

class LPEAttachPathTest : public LPESPathsTest {};

// INKSCAPE 0.92.5

TEST_F(LPEAttachPathTest, mixed_0_92_5)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="250mm"
   height="250mm"
   viewBox="0 0 250 250"
   version="1.1"
   id="svg8"
   inkscape:version="0.92.5 (2060ec1f9f, 2020-04-08)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="bspline"
       id="path-effect3912"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false" />
    <inkscape:path-effect
       effect="attach_path"
       id="path-effect189"
       startpath="#path167"
       startposition="5"
       startcurvestart="46.620067,194.49992 , 9.2718385,3.7460659"
       startcurveend="25.160036,146.39985 , 5.9416084,0.83503861"
       endpath="#path3910"
       endposition="0.4"
       endcurvestart="222.00032,102.73978 , 20.299505,0.14171958"
       endcurveend="160.05734,69.186925 , 20.599969,0.035953764" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect175"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect169"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false" />
  </defs>
  <g id="t" transform="scale(0.445)">
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.26458332px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 45.880067,45.7597 c 27.626658,49.086649 55.253363,98.17338 75.110133,97.92656 19.85677,-0.24682 31.94343,-49.826801 31.30256,-72.472701 C 151.65189,48.567658 138.87385,52.666275 118.52344,73.263322 98.173027,93.86037 70.54699,130.85953 42.920062,167.85988"
     id="path167"
     inkscape:connector-curvature="0"
     inkscape:path-effect="#path-effect169"
     inkscape:original-d="M 45.880067,45.7597 C 73.507038,94.846173 101.13374,143.93291 128.76019,193.01991 140.84716,143.43949 152.93382,93.859505 165.02024,44.279698 151.70057,48.719411 138.8738,52.666111 125.80018,56.859717 98.17307,93.860402 70.547033,130.85956 42.920062,167.85988"
     transform="translate(-17.760026,-21.460032)" />
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.26458332px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 25.160036,146.39985 c 4.2239,-4.2613 12.337517,44.00376 21.460031,48.10007 51.80033,1.97335 103.600403,3.94668 132.830333,-11.34675 29.22993,-15.29344 35.88997,-47.85365 42.54992,-80.41339 4.20679,-19.859328 -59.76806,-54.037721 -61.94298,-33.552854"
     id="path01"
     inkscape:connector-curvature="0"
     inkscape:path-effect="#path-effect175;#path-effect189"
     inkscape:original-d="m 46.620067,194.49992 c 51.800341,1.97307 103.600413,3.9464 155.400223,5.92001 6.66025,-32.56016 13.32029,-65.12037 19.98003,-97.68015" />
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.26458332px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 164.28024,36.879686 c -3.94663,27.62641 -7.8933,55.253119 -3.82314,69.683304 4.07017,14.43019 16.15672,15.66351 22.69328,4.93356 6.53656,-10.72994 7.52325,-33.423746 12.94995,-43.66028 5.42671,-10.236534 15.29351,-8.016504 25.15999,-5.796546"
     id="path3910"
     inkscape:connector-curvature="0"
     inkscape:path-effect="#path-effect3912"
     inkscape:original-d="m 164.28024,36.879686 c -3.94641,27.626442 -7.89308,55.253151 -11.84002,82.880124 12.08708,1.23308 24.17363,2.4664 36.26006,3.7 0.98691,-22.69319 1.9736,-45.386995 2.96,-68.080095 9.86683,2.219711 19.73363,4.43974 29.60004,6.660009" />
   </g>
</svg>
)"""";

   testDoc(svg);
}


// INKSCAPE 1.0.2

TEST_F(LPEAttachPathTest, attachpath_MM_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="250mm"
   height="250mm"
   viewBox="0 0 250 250"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="bspline"
       lpeversion="1"
       id="path-effect3912"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false" />
    <inkscape:path-effect
       effect="attach_path"
       id="path-effect189"
       lpeversion="1"
       startpath="#path167"
       startposition="5"
       startcurvestart="46.620067,194.49992 , 9.2718385,3.7460659"
       startcurveend="25.160036,146.39985 , 5.9416084,0.83503861"
       endpath="#path3910"
       endposition="0.4"
       endcurvestart="222.00032,102.73978 , 20.299505,0.14171958"
       endcurveend="160.05734,69.186925 , 20.599969,0.035953764" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect175"
       lpeversion="1"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect169"
       is_visible="true"
       lpeversion="1"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false" />
  </defs>
  <g id="t" transform="scale(0.445)">
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 39.183448,43.45239 c 27.626658,49.086649 55.253364,98.17338 75.110132,97.92656 19.85677,-0.24682 31.94343,-49.826801 31.30256,-72.472701 C 144.95527,46.260348 132.17723,50.358965 111.82682,70.956012 91.476408,91.55306 63.850371,128.55222 36.223443,165.55257"
     id="path167"
     inkscape:connector-curvature="0"
     inkscape:path-effect="#path-effect169"
     inkscape:original-d="M 39.183448,43.45239 C 66.810419,92.538863 94.437121,141.6256 122.06357,190.7126 134.15054,141.13218 146.2372,91.552195 158.32362,41.972388 145.00395,46.412101 132.17718,50.358801 119.10356,54.552407 91.476451,91.553092 63.850414,128.55225 36.223443,165.55257" />
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 36.223443,165.55257 c 4.2239,-4.2613 12.337517,44.00376 21.460031,48.10007 51.800336,1.97335 103.600406,3.94668 132.830336,-11.34675 29.22993,-15.29344 35.88997,-47.85365 42.54992,-80.41339 4.20679,-19.85933 -59.76806,-54.037719 -61.94298,-33.552852"
     id="path01"
     inkscape:connector-curvature="0"
     inkscape:path-effect="#path-effect175;#path-effect189"
     inkscape:original-d="m 57.683474,213.65264 c 51.800346,1.97307 103.600416,3.9464 155.400226,5.92001 6.66025,-32.56016 13.32029,-65.12037 19.98003,-97.68015" />
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 175.34365,56.032408 c -3.94663,27.62641 -7.8933,55.253122 -3.82314,69.683302 4.07017,14.43019 16.15672,15.66351 22.69328,4.93356 6.53656,-10.72994 7.52325,-33.423744 12.94995,-43.660278 5.42671,-10.236534 15.29351,-8.016504 25.15999,-5.796546"
     id="path3910"
     inkscape:connector-curvature="0"
     inkscape:path-effect="#path-effect3912"
     inkscape:original-d="m 175.34365,56.032408 c -3.94641,27.626442 -7.89308,55.253152 -11.84002,82.880122 12.08708,1.23308 24.17363,2.4664 36.26006,3.7 0.98691,-22.69319 1.9736,-45.386993 2.96,-68.080093 9.86683,2.219711 19.73363,4.43974 29.60004,6.660009" />
   </g>
</svg>
)"""";

   testDoc(svg);
}

// INKSCAPE 1.0.2

TEST_F(LPEAttachPathTest, attachpath_PX_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="250"
   height="250"
   viewBox="0 0 250 250"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="bspline"
       id="path-effect3912"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="attach_path"
       id="path-effect189"
       startpath="#path167"
       startposition="4"
       startcurvestart="57.683474,213.65264 , 9.2718385,3.7460659"
       startcurveend="36.223443,165.55257 , 5.9416084,0.83503861"
       endpath="#path3910"
       endposition="0.40000001"
       endcurvestart="233.06373,121.8925 , 20.299505,0.14171958"
       endcurveend="171.12075,88.339648 , 20.599969,0.035953764"
       is_visible="true"
       lpeversion="1" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect175"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false"
       lpeversion="1" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect169"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false"
       lpeversion="1" />
  </defs>
  <g id="t" transform="scale(0.445)">
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 39.183448,43.45239 c 27.626658,49.086649 55.253364,98.17338 75.110132,97.92656 19.85677,-0.24682 31.94343,-49.826801 31.30256,-72.472701 C 144.95527,46.260348 132.17723,50.358965 111.82682,70.956012 91.476408,91.55306 63.850371,128.55222 36.223443,165.55257"
     id="path167"
     inkscape:connector-curvature="0"
     inkscape:path-effect="#path-effect169"
     inkscape:original-d="M 39.183448,43.45239 C 66.810419,92.538863 94.437121,141.6256 122.06357,190.7126 134.15054,141.13218 146.2372,91.552195 158.32362,41.972388 145.00395,46.412101 132.17718,50.358801 119.10356,54.552407 91.476451,91.553092 63.850414,128.55225 36.223443,165.55257" />
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 36.223443,165.55257 c 4.2239,-4.2613 12.337517,44.00376 21.460031,48.10007 51.800336,1.97335 103.600406,3.94668 132.830336,-11.34675 29.22993,-15.29344 35.88997,-47.85365 42.54992,-80.41339 4.20679,-19.85933 -59.76806,-54.037718 -61.94298,-33.552852"
     id="path01"
     inkscape:connector-curvature="0"
     inkscape:path-effect="#path-effect175;#path-effect189"
     inkscape:original-d="m 57.683474,213.65264 c 51.800346,1.97307 103.600416,3.9464 155.400226,5.92001 6.66025,-32.56016 13.32029,-65.12037 19.98003,-97.68015" />
  <path
     style="fill:none;stroke:#ff0000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
     d="m 175.34365,56.032408 c -3.94663,27.62641 -7.8933,55.253122 -3.82314,69.683302 4.07017,14.43019 16.15672,15.66351 22.69328,4.93356 6.53656,-10.72994 7.52325,-33.423744 12.94995,-43.660278 5.42671,-10.236534 15.29351,-8.016504 25.15999,-5.796546"
     id="path3910"
     inkscape:connector-curvature="0"
     inkscape:path-effect="#path-effect3912"
     inkscape:original-d="m 175.34365,56.032408 c -3.94641,27.626442 -7.89308,55.253152 -11.84002,82.880122 12.08708,1.23308 24.17363,2.4664 36.26006,3.7 0.98691,-22.69319 1.9736,-45.386993 2.96,-68.080093 9.86683,2.219711 19.73363,4.43974 29.60004,6.660009" />
   </g>
</svg>
)"""";

   testDoc(svg);
}