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

class LPEFillBetweenStrokesTest : public LPESPathsTest {};

// INKSCAPE 0.92.5

TEST_F(LPEFillBetweenStrokesTest, path_0_92_5)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="210mm"
   height="297mm"
   viewBox="0 0 210 297"
   version="1.1"
   id="svg45"
   inkscape:version="0.92.5 (2060ec1f9f, 2020-04-08)"
   inkscape:test-threshold="0.01">
  <defs
     id="defs39">
    <inkscape:path-effect
       effect="bspline"
       id="path-effect78"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect58"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false" />
    <inkscape:path-effect
       effect="fill_between_strokes"
       id="path-effect54"
       is_visible="true"
       linkedpath="#path01"
       secondpath="#path03"
       reversesecond="true" />
    <inkscape:path-effect
       effect="fill_between_strokes"
       id="path-effect50"
       is_visible="true"
       linkedpath=""
       secondpath=""
       reversesecond="false" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;stroke:#000000;stroke-width:0.26458332px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 12.85119,220.64881 c 34.018071,5.79567 68.035927,11.5913 87.31261,-3.27581 19.27668,-14.86712 23.81242,-50.39711 18.14298,-62.74433 -5.66944,-12.34722 -21.544573,-1.51181 -37.419875,9.32371"
       id="path03"
       inkscape:connector-curvature="0"
       inkscape:path-effect="#path-effect58"
       inkscape:original-d="m 12.85119,220.64881 c 34.018122,5.79537 68.035979,11.591 102.05357,17.3869 4.53595,-35.5298 9.07169,-71.05979 13.60714,-106.58928 -15.8746,10.83496 -31.749731,21.67037 -47.624995,32.50595" />
    <path
       style="fill:#ffff00;stroke:#000000;stroke-width:0.26458332px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m -57.452381,182.09524 c -13.10322,-20.91476 -26.206393,-41.82944 -15.371,-50.01888 10.835394,-8.18944 45.609322,-3.65371 64.3819922,3.40171 18.7726698,7.05542 21.5445138,16.63088 24.3163888,26.20645 l 65.011905,2.26786 c 15.875302,-10.83552 31.750435,-21.67093 37.419875,-9.32371 5.66944,12.34722 1.1337,47.87721 -18.14298,62.74433 -19.276683,14.86711 -53.294539,9.07148 -87.31261,3.27581"
       id="path02"
       inkscape:connector-curvature="0"
       inkscape:path-effect="#path-effect54"
       inkscape:original-d="M 2.2678571,188.89881 -88.446426,105.74405 60.47619,151.85714"
       transform="translate(60.47619,-105.07738)" />
    <path
       style="fill:none;stroke:#000000;stroke-width:0.26458332px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m -57.452381,182.09524 c -13.10322,-20.91476 -26.206393,-41.82944 -15.371,-50.01888 10.835394,-8.18944 45.609322,-3.65371 64.3819922,3.40171 18.7726698,7.05542 21.5445138,16.63088 24.3163888,26.20645"
       id="path01"
       inkscape:connector-curvature="0"
       inkscape:path-effect="#path-effect78"
       inkscape:original-d="m -57.452381,182.09524 c -13.10291,-20.91495 -26.206085,-41.82963 -39.309524,-62.74405 34.773957,4.53543 69.547885,9.07116 104.3214288,13.60714 2.7720722,9.57507 5.5439152,19.15053 8.3154762,28.72619" />
  </g>
</svg>
)"""";

   testDoc(svg);
}


// INKSCAPE 1.0.2

TEST_F(LPEFillBetweenStrokesTest, multi_PX_1_0_2)
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
       effect="spiro"
       id="path-effect51"
       is_visible="true"
       lpeversion="1" />
    <inkscape:path-effect
       effect="spiro"
       id="path-effect28"
       is_visible="true"
       lpeversion="1" />
    <inkscape:path-effect
       effect="fill_between_strokes"
       id="path-effect24"
       is_visible="true"
       lpeversion="1"
       linkedpath="#path03"
       secondpath="#path01"
       reversesecond="true"
       join="true"
       close="true" />
    <inkscape:path-effect
       effect="spiro"
       id="path-effect22"
       is_visible="true"
       lpeversion="1" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 45.577369,130.63608 c 11.425366,22.13704 25.467015,42.92234 41.739291,61.78502 9.590712,11.11747 20.09919,21.68459 32.68857,29.24021 12.58939,7.55561 27.50608,11.93474 42.05031,9.92345 7.27212,-1.00564 14.35489,-3.61173 20.36712,-7.82457 6.01224,-4.21283 10.92378,-10.05338 13.75511,-16.82675 2.83132,-6.77337 3.5282,-14.47127 1.65793,-21.57036 -1.87028,-7.09909 -6.35301,-13.53267 -12.53451,-17.49298 -4.65441,-2.98195 -10.10099,-4.52067 -15.56472,-5.35927 -5.46372,-0.83859 -11.0044,-1.01464 -16.51121,-1.4949 -5.5068,-0.48027 -11.0462,-1.28107 -16.19203,-3.29997 -5.14583,-2.01891 -9.91275,-5.35447 -12.83882,-10.04421 -3.15418,-5.05534 -3.93648,-11.43118 -2.535,-17.22265 1.40148,-5.79147 4.88497,-10.98318 9.42291,-14.84483 4.53794,-3.86166 10.08992,-6.43442 15.8725,-7.87214 5.78258,-1.43772 11.79945,-1.7707 17.75041,-1.4685 14.90284,0.7568 29.58217,5.53814 42.07186,13.70363"
       id="path03"
       inkscape:path-effect="#path-effect28"
       inkscape:original-d="m 45.577369,130.63608 c 13.914097,20.59601 27.827194,41.19101 41.739291,61.78502 13.9121,20.594 65.32402,-16.36633 97.98453,-24.551 32.66051,-8.18467 -40.73685,-13.46457 -61.10678,-20.19835 -20.36992,-6.73379 27.00822,-27.60441 40.51082,-41.40812 13.50261,-13.803703 28.0489,9.13676 42.07186,13.70363" />
    <path
       style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 45.577369,130.63608 c 11.425366,22.13704 25.467015,42.92234 41.739291,61.78502 9.590712,11.11747 20.09919,21.68459 32.68857,29.24021 12.58939,7.55561 27.50608,11.93474 42.05031,9.92345 7.27212,-1.00564 14.35489,-3.61173 20.36712,-7.82457 6.01224,-4.21283 10.92378,-10.05338 13.75511,-16.82675 2.83132,-6.77337 3.5282,-14.47127 1.65793,-21.57036 -1.87028,-7.09909 -6.35301,-13.53267 -12.53451,-17.49298 -4.65441,-2.98195 -10.10099,-4.52067 -15.56472,-5.35927 -5.46372,-0.83859 -11.0044,-1.01464 -16.51121,-1.4949 -5.5068,-0.48027 -11.0462,-1.28107 -16.19203,-3.29997 -5.14583,-2.01891 -9.91275,-5.35447 -12.83882,-10.04421 -3.15418,-5.05534 -3.93648,-11.43118 -2.535,-17.22265 1.40148,-5.79147 4.88497,-10.98318 9.42291,-14.84483 4.53794,-3.86166 10.08992,-6.43442 15.8725,-7.87214 5.78258,-1.43772 11.79945,-1.7707 17.75041,-1.4685 14.90284,0.7568 29.58217,5.53814 42.07186,13.70363 l 75.19378,-7.40342 C 261.30668,81.797428 231.35585,57.35695 197.07015,43.283148 141.84297,20.613161 75.825772,26.274208 25.264437,58.015677 Z"
       id="path02"
       inkscape:path-effect="#path-effect22;#path-effect24"
       inkscape:original-d="m 31.296121,90.573275 c 1.041529,-22.47889 2.082058,-44.95878 3.121587,-67.43967 1.039529,-22.48088978 54.028844,15.581935 81.041762,23.371402 27.01293,7.789467 5.24696,2.41351 7.86894,3.618764 2.62198,1.205255 27.35969,-16.708547 41.03804,-25.064321 13.67834,-8.355774 36.13826,14.918736 54.20588,22.376604" />
    <path
       style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 25.264437,58.015677 C 75.825772,26.274208 141.84297,20.613161 197.07015,43.283148 c 34.2857,14.073802 64.23653,38.51428 84.90072,69.280692"
       id="path01"
       inkscape:path-effect="#path-effect51"
       inkscape:original-d="M 25.264437,58.015677 C 82.534008,53.105834 139.80258,48.194991 197.07015,43.283148 c 57.26757,-4.911843 56.60148,46.188125 84.90072,69.280692" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEFillBetweenStrokesTest, multi_MM_1_0_2)
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
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)"
   inkscape:test-threshold="0.01">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="spiro"
       id="path-effect51"
       is_visible="true"
       lpeversion="1" />
    <inkscape:path-effect
       effect="spiro"
       id="path-effect28"
       is_visible="true"
       lpeversion="1" />
    <inkscape:path-effect
       effect="fill_between_strokes"
       id="path-effect24"
       is_visible="true"
       lpeversion="1"
       linkedpath="#path03"
       secondpath="#path01"
       reversesecond="true"
       join="true"
       close="true" />
    <inkscape:path-effect
       effect="spiro"
       id="path-effect22"
       is_visible="true"
       lpeversion="1" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 45.577369,130.63608 c 11.425366,22.13704 25.467015,42.92234 41.739291,61.78502 9.590712,11.11747 20.09919,21.68459 32.68857,29.24021 12.58939,7.55561 27.50608,11.93474 42.05031,9.92345 7.27212,-1.00564 14.35489,-3.61173 20.36712,-7.82457 6.01224,-4.21283 10.92378,-10.05338 13.75511,-16.82675 2.83132,-6.77337 3.5282,-14.47127 1.65793,-21.57036 -1.87028,-7.09909 -6.35301,-13.53267 -12.53451,-17.49298 -4.65441,-2.98195 -10.10099,-4.52067 -15.56472,-5.35927 -5.46372,-0.83859 -11.0044,-1.01464 -16.51121,-1.4949 -5.5068,-0.48027 -11.0462,-1.28107 -16.19203,-3.29997 -5.14583,-2.01891 -9.91275,-5.35447 -12.83882,-10.04421 -3.15418,-5.05534 -3.93648,-11.43118 -2.535,-17.22265 1.40148,-5.79147 4.88497,-10.98318 9.42291,-14.84483 4.53794,-3.86166 10.08992,-6.43442 15.8725,-7.87214 5.78258,-1.43772 11.79945,-1.7707 17.75041,-1.4685 14.90284,0.7568 29.58217,5.53814 42.07186,13.70363"
       id="path03"
       inkscape:path-effect="#path-effect28"
       inkscape:original-d="m 45.577369,130.63608 c 13.914097,20.59601 27.827194,41.19101 41.739291,61.78502 13.9121,20.594 65.32402,-16.36633 97.98453,-24.551 32.66051,-8.18467 -40.73685,-13.46457 -61.10678,-20.19835 -20.36992,-6.73379 27.00822,-27.60441 40.51082,-41.40812 13.50261,-13.803703 28.0489,9.13676 42.07186,13.70363" />
    <path
       style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="m 45.577369,130.63608 c 11.425366,22.13704 25.467015,42.92234 41.739291,61.78502 9.590712,11.11747 20.09919,21.68459 32.68857,29.24021 12.58939,7.55561 27.50608,11.93474 42.05031,9.92345 7.27212,-1.00564 14.35489,-3.61173 20.36712,-7.82457 6.01224,-4.21283 10.92378,-10.05338 13.75511,-16.82675 2.83132,-6.77337 3.5282,-14.47127 1.65793,-21.57036 -1.87028,-7.09909 -6.35301,-13.53267 -12.53451,-17.49298 -4.65441,-2.98195 -10.10099,-4.52067 -15.56472,-5.35927 -5.46372,-0.83859 -11.0044,-1.01464 -16.51121,-1.4949 -5.5068,-0.48027 -11.0462,-1.28107 -16.19203,-3.29997 -5.14583,-2.01891 -9.91275,-5.35447 -12.83882,-10.04421 -3.15418,-5.05534 -3.93648,-11.43118 -2.535,-17.22265 1.40148,-5.79147 4.88497,-10.98318 9.42291,-14.84483 4.53794,-3.86166 10.08992,-6.43442 15.8725,-7.87214 5.78258,-1.43772 11.79945,-1.7707 17.75041,-1.4685 14.90284,0.7568 29.58217,5.53814 42.07186,13.70363 l 75.19378,-7.40342 C 261.30668,81.797428 231.35585,57.35695 197.07015,43.283148 141.84297,20.613161 75.825772,26.274208 25.264437,58.015677 Z"
       id="path02"
       inkscape:path-effect="#path-effect22;#path-effect24"
       inkscape:original-d="m 31.296121,90.573275 c 1.041529,-22.47889 2.082058,-44.95878 3.121587,-67.43967 1.039529,-22.48088978 54.028844,15.581935 81.041762,23.371402 27.01293,7.789467 5.24696,2.41351 7.86894,3.618764 2.62198,1.205255 27.35969,-16.708547 41.03804,-25.064321 13.67834,-8.355774 36.13826,14.918736 54.20588,22.376604" />
    <path
       style="fill:none;stroke:#000000;stroke-width:1px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 25.264437,58.015677 C 75.825772,26.274208 141.84297,20.613161 197.07015,43.283148 c 34.2857,14.073802 64.23653,38.51428 84.90072,69.280692"
       id="path01"
       inkscape:path-effect="#path-effect51"
       inkscape:original-d="M 25.264437,58.015677 C 82.534008,53.105834 139.80258,48.194991 197.07015,43.283148 c 57.26757,-4.911843 56.60148,46.188125 84.90072,69.280692" />
  </g>
</svg>
)"""";

   testDoc(svg);
}