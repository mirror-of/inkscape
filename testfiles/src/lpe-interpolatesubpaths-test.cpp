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

class LPEInterpolateSubPathsTest : public LPESPathsTest {};

// INKSCAPE 0.92.5

TEST_F(LPEInterpolateSubPathsTest, path_0_92_5)
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
   id="svg8"
   inkscape:version="0.92.5 (2060ec1f9f, 2020-04-08)"
   inkscape:test-threshold="0.01">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="interpolate"
       id="path-effect20"
       is_visible="true"
       trajectory="m 87.312499,144.29762 c -157.231352,111.44831 -1.297683,80.66835 8.315479,119.81845"
       equidistant_spacing="false"
       steps="40"
       trajectory-nodetypes="cc" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;stroke:#000000;stroke-width:0.26458332px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 43.845239,197.97024 C 32.254232,155.13267 20.662963,112.29537 9.0714282,69.458337 54.68063,72.985833 100.28995,76.513626 145.89881,80.04167 c -5.29138,29.48172 -10.58307,58.96402 -15.875,88.44642 11.84364,16.88285 23.68677,33.76561 35.52976,50.64881 M 32.379839,204.93847 C 21.434945,162.65656 10.48979,120.37492 -0.45563196,78.093541 44.365311,82.241295 89.186374,86.389367 134.00698,90.537679 c -5.88581,28.771001 -11.77192,57.542571 -17.65828,86.314251 12.24424,16.74716 24.48795,33.49424 36.73153,50.24176 M 22.103329,211.36711 C 11.804548,169.64085 1.5055051,127.91487 -8.7938032,86.189145 35.238881,90.957159 79.271686,95.725509 123.30403,100.49409 c -6.48023,28.06028 -12.96077,56.12113 -19.44154,84.18208 12.64482,16.61148 25.28913,33.22287 37.9333,49.8347 M 12.969231,217.2776 C 3.3165632,176.107 -6.3363664,134.93667 -15.989562,93.766611 27.254864,99.154883 70.499412,104.54351 113.7435,109.93236 c -7.07466,27.34956 -14.149617,54.69968 -21.224819,82.0499 13.045419,16.4758 26.090309,32.95151 39.135069,49.42766 M 4.9310697,222.69141 C -4.0754846,182.07647 -13.082301,141.4618 -22.089383,100.8474 c 42.456167,6.00853 84.912457,12.01743 127.368283,18.02655 -7.669086,26.63884 -15.338461,53.27823 -23.008087,79.91773 13.446016,16.34011 26.891487,32.68014 40.336847,49.02061 M -2.0576306,227.63001 c -8.3604414,-40.05929 -16.7211444,-80.1183 -25.0821134,-120.17705 41.667908,6.62879 83.335941,13.25798 125.003512,19.88736 -8.263516,25.92812 -16.527312,51.85678 -24.791362,77.78556 13.84661,16.20442 27.692674,32.40877 41.538614,48.61355 M -8.0433462,232.11484 C -15.757674,192.61121 -23.472264,153.10786 -31.18712,113.60476 c 40.8796497,7.24905 81.759425,14.49852 122.638737,21.74817 -8.857943,25.2174 -17.716159,50.43533 -26.574634,75.65339 14.247204,16.06873 28.493852,32.1374 42.740387,48.2065 M -13.072553,236.16737 c -7.068215,-38.94797 -14.136692,-77.89567 -21.205435,-116.8431 40.0913916,7.8693 80.18291,15.73905 120.273964,23.60897 -9.45237,24.50668 -18.905008,49.01388 -28.357907,73.52121 14.647798,15.93305 29.295033,31.86604 43.942151,47.79945 M -17.191727,239.80905 c -6.422102,-38.39231 -12.844466,-76.78435 -19.267095,-115.17613 39.3031321,8.48957 78.606393,16.97959 117.909189,25.46978 -10.046797,23.79596 -20.093856,47.59244 -30.141179,71.38904 15.048392,15.79737 30.096213,31.59467 45.143925,47.3924 M -20.447345,243.06136 c -5.775989,-37.83665 -11.55224,-75.67304 -17.328756,-113.50916 38.51487408,9.10983 77.029877,18.22013 115.544416,27.33059 -10.641225,23.08524 -21.282705,46.17098 -31.924452,69.25686 15.448986,15.66168 30.897393,31.3233 46.345695,46.98535 M -22.885883,245.94574 c -5.129875,-37.281 -10.260013,-74.56173 -15.390416,-111.84219 37.72661554,9.73009 75.453361,19.46067 113.179642,29.19139 -11.235652,22.37452 -22.471553,44.74954 -33.707724,67.12469 15.84958,15.526 31.698573,31.05194 47.547466,46.5783 M -24.553816,248.48365 c -4.483762,-36.72533 -8.967786,-73.4504 -13.452076,-110.17521 36.9383564,10.35034 73.876845,20.7012 110.814867,31.0522 -11.830079,21.6638 -23.660401,43.32808 -35.490996,64.99252 16.250174,15.39031 32.499754,30.78056 48.749236,46.17124 M -25.49762,250.69657 c -3.837649,-36.16969 -7.675561,-72.3391 -11.513737,-108.50825 36.15009765,10.9706 72.300328,21.94174 108.450093,32.91301 -12.424506,20.95307 -24.849249,41.90663 -37.274269,62.86034 16.650768,15.25463 33.300934,30.5092 49.951007,45.7642 M -25.763772,252.60593 c -3.191536,-35.61402 -6.383334,-71.22778 -9.575398,-106.84127 35.36183918,11.59086 70.723813,23.18228 106.085319,34.77381 -13.018933,20.24236 -26.038097,40.48519 -39.057541,60.72817 17.051362,15.11894 34.102115,30.23783 51.152778,45.35714 M -25.398749,254.23321 c -2.545422,-35.05837 -5.091107,-70.11646 -7.637058,-105.1743 34.573581,12.21112 69.147297,24.42282 103.720546,36.63462 -13.613361,19.53163 -27.226946,39.06374 -40.840814,58.596 17.451956,14.98325 34.903295,29.96646 52.354548,44.95009 M -24.449024,255.59986 c -1.89931,-34.50271 -3.798882,-69.00515 -5.698719,-103.50732 33.7853219,12.83137 67.57078,25.66335 101.355771,38.49542 -14.207788,18.82091 -28.415794,37.64229 -42.624086,56.46382 17.85255,14.84757 35.704475,29.6951 53.556319,44.54304 M -22.961076,256.72735 c -1.253196,-33.94705 -2.506655,-67.89384 -3.760379,-101.84036 32.997063,13.45164 65.994264,26.90389 98.990997,40.35623 -14.802215,18.1102 -29.604642,36.22084 -44.407358,54.33165 18.253143,14.71189 36.505655,29.42373 54.758089,44.13599 M -20.98138,257.63713 c -0.607083,-33.3914 -1.214428,-66.78253 -1.82204,-100.17339 32.2088052,14.0719 64.417749,28.14443 96.626224,42.21704 -15.396642,17.39947 -30.793491,34.79939 -46.190631,52.19947 18.653738,14.5762 37.306836,29.15237 55.95986,43.72894 M -18.556411,258.35066 c 0.03903,-32.83574 0.0778,-65.67121 0.116299,-98.50642 31.420546,14.69216 62.841233,29.38497 94.26145,44.07784 -15.99107,16.68876 -31.982339,33.37795 -47.973903,50.06731 19.054331,14.44051 38.108016,28.88099 57.16163,43.32189 M -15.732647,258.8894 c 0.685143,-32.28008 1.370024,-64.5599 2.054639,-96.83944 30.632288,15.31241 61.264716,30.6255 91.896675,45.93864 -16.585496,15.97804 -33.171187,31.9565 -49.757175,47.93513 19.454925,14.30483 38.909196,28.60963 58.363401,42.91484 m -99.381456,-39.56376 c 1.331257,-31.72443 2.6622504,-63.44858 3.9929787,-95.17247 29.8440293,15.93267 59.6882003,31.86604 89.5319013,47.79945 -17.179924,15.26731 -34.360035,30.53504 -51.540448,45.80296 19.85552,14.16914 39.710377,28.33826 59.565172,42.50778 M -9.0746346,259.52835 c 1.9773697,-31.16877 3.9544764,-62.33727 5.9313179,-93.5055 29.0557707,16.55293 58.1116837,33.10658 87.1671277,49.66026 -17.774351,14.55659 -35.548884,29.11359 -53.323721,43.67078 20.256114,14.03346 40.511557,28.0669 60.766943,42.10074 M -5.3333387,259.67147 c 2.6234829,-30.6131 5.24670276,-61.22594 7.8696573,-91.83852 28.2675114,17.17319 56.5351684,34.34711 84.8023534,51.52106 -18.368778,13.84588 -36.737732,27.69215 -55.106993,41.53862 20.656708,13.89777 41.312738,27.79552 61.968713,41.69368 M -1.3791511,259.72565 C 1.8904451,229.6682 5.159778,199.61102 8.4288457,169.5541 c 27.4792533,17.79345 54.9586523,35.58765 82.4375793,53.38187 -18.963205,13.13515 -37.92658,26.2707 -56.890265,39.40644 21.057302,13.76208 42.113918,27.52416 63.170484,41.28663 M 2.7414523,259.71234 c 3.9157093,-29.50179 7.8311557,-59.00332 11.7463367,-88.50458 26.690994,18.41371 53.382136,36.82819 80.072805,55.24267 -19.557633,12.42444 -39.115428,24.84925 -58.673537,37.27427 21.457895,13.6264 42.915098,27.25279 64.372253,40.87958 M 6.9819952,259.65299 c 4.5618228,-28.94613 9.1233818,-57.89201 13.6846758,-86.83761 25.902736,19.03397 51.80562,38.06873 77.708031,57.10348 -20.152059,11.71372 -40.304276,23.4278 -60.456809,35.14209 21.858489,13.49072 43.716278,26.98143 65.574027,40.47253 M 11.296002,259.56907 c 5.207935,-28.39048 10.415608,-56.78069 15.623015,-85.17064 25.114477,19.65423 50.229104,39.30927 75.343253,58.96429 -20.746483,11.00299 -41.493121,22.00635 -62.240078,33.00992 22.259083,13.35503 44.517459,26.71006 66.775798,40.06548 M 15.636996,259.48203 c 5.854049,-27.83482 11.707834,-55.66937 17.561354,-83.50366 24.326219,20.27448 48.652588,40.5498 72.97848,60.82509 -21.34091,10.29227 -42.681969,20.58491 -64.023351,30.87775 22.659678,13.21934 45.318639,26.43869 67.977571,39.65842 M 19.958501,259.41334 c 6.500162,-27.27916 13.000061,-54.55806 19.499694,-81.83669 23.53796,20.89475 47.076072,41.79034 70.613705,62.6859 -21.935336,9.58155 -43.870817,19.16346 -65.806622,28.74557 23.060271,13.08366 46.119819,26.16733 69.179332,39.25138 M 24.214042,259.38446 c 7.146275,-26.7235 14.292287,-53.44675 21.438034,-80.16972 22.749701,21.515 45.499555,43.03088 68.248934,64.5467 -22.529767,8.87084 -45.059669,17.74201 -67.589898,26.6134 23.460865,12.94798 46.920999,25.89596 70.381108,38.84433 M 28.357143,259.41684 c 7.792388,-26.16785 15.584513,-52.33543 23.376372,-78.50275 21.961443,22.13526 43.92304,44.27142 65.884165,66.40751 -23.124199,8.16011 -46.248521,16.32056 -69.373175,24.48123 23.861459,12.81229 47.72218,25.62459 71.582875,38.43727 M 32.341326,259.53194 c 8.438502,-25.61219 16.87674,-51.22411 25.314713,-76.83577 21.173184,22.75552 42.346521,45.51195 63.519391,68.26831 -23.718627,7.44939 -47.43737,14.89911 -71.156448,22.34906 24.262053,12.6766 48.52336,25.35322 72.784648,38.03022 M 36.120117,259.75123 c 9.084615,-25.05653 18.168966,-50.1128 27.253052,-75.1688 20.384925,23.37577 40.770011,46.75248 61.154611,70.12912 -24.31305,6.73867 -48.626212,13.47766 -72.939714,20.21688 24.662647,12.54092 49.324544,25.08185 73.986414,37.62317 M 39.647039,260.09616 c 9.730728,-24.50088 19.461192,-49.00149 29.191391,-73.50183 19.596667,23.99603 39.19349,47.99302 58.78984,71.98992 -24.90748,6.02795 -49.815063,12.05621 -74.722989,18.08471 25.063241,12.40523 50.125719,24.81049 75.188189,37.21612 M 42.875616,260.58819 c 10.376841,-23.94522 20.753418,-47.89017 31.129731,-71.83486 18.808408,24.6163 37.616973,49.23356 56.425063,73.85073 -25.5019,5.31723 -51.003909,10.63476 -76.506259,15.95254 25.463835,12.26954 50.926899,24.53911 76.389959,36.80906 M 45.759372,261.24878 c 11.022954,-23.38956 22.045645,-46.77886 33.06807,-70.16789 18.02015,25.23656 36.040458,50.4741 54.060288,75.71154 -26.09633,4.60651 -52.192756,9.21332 -78.28953,13.82036 25.864429,12.13386 51.72808,24.26775 77.59173,36.40202 M 48.251831,262.09939 c 11.669068,-22.8339 23.337871,-45.66754 35.00641,-68.50091 17.231889,25.85681 34.463939,51.71464 51.695519,77.57234 -26.69076,3.89579 -53.381609,7.79187 -80.072808,11.68819 26.265023,11.99818 52.529258,23.99638 78.793498,35.99496 M 50.306518,263.16149 c 12.31518,-22.27825 24.630097,-44.55623 36.944749,-66.83395 16.443633,26.47708 32.887423,52.95518 49.330743,79.43315 -27.28519,3.18507 -54.570456,6.37042 -81.856079,9.55602 26.665618,11.86249 53.330439,23.72501 79.995269,35.58791 m -82.844245,-56.4481 c 12.961293,-21.72259 25.922323,-43.44492 38.883088,-65.16697 15.655377,27.09733 31.310907,54.19571 46.965967,81.29395 -27.87961,2.47435 -55.759302,4.94897 -83.639348,7.42384 27.066211,11.72681 54.131618,23.45365 81.197038,35.18087 M 52.916667,266.00595 c 13.607407,-21.16693 27.21455,-42.3336 40.821428,-63.5 14.867115,27.71759 29.734395,55.43625 44.601195,83.15476 -28.47404,1.76363 -56.948153,3.52752 -85.422624,5.29167 27.466805,11.59112 54.932804,23.18228 82.398814,34.77381"
       id="path01"
       inkscape:connector-curvature="0"
       inkscape:path-effect="#path-effect20"
       inkscape:original-d="M 43.845239,197.97024 C 32.254232,155.13267 20.662963,112.29537 9.0714284,69.458334 54.68063,72.98583 100.28995,76.513623 145.89881,80.041667 c -5.29138,29.481723 -10.58307,58.964023 -15.875,88.446423 11.84364,16.88285 23.68677,33.76561 35.52976,50.64881 M 52.916667,266.00595 c 13.607407,-21.16693 27.21455,-42.3336 40.821428,-63.5 14.867115,27.71759 29.734395,55.43625 44.601195,83.15476 -28.47404,1.76363 -56.948152,3.52752 -85.422623,5.29167 27.466805,11.59112 54.932803,23.18228 82.398813,34.77381"
       transform="translate(30.415662,-31.012048)" />
  </g>
</svg>
)"""";

   testDoc(svg);
}


// INKSCAPE 1.0.2

TEST_F(LPEInterpolateSubPathsTest, multi_PX_1_0_2)
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
       effect="interpolate"
       id="path-effect20"
       is_visible="true"
       trajectory="m 120.32157,109.54558 c -83.344568,49.87599 -1.29768,80.66835 8.31548,119.81845"
       equidistant_spacing="false"
       steps="19"
       trajectory-nodetypes="cc"
       lpeversion="0" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect18"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false"
       lpeversion="0" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect14"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false"
       lpeversion="0" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;stroke:#000000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 76.854315,163.2182 C 65.262995,120.38071 53.671725,77.543413 70.680705,57.888743 87.689684,38.234074 133.29901,41.761866 153.45766,58.266589 c 20.15865,16.504722 14.86695,45.987021 18.14276,69.169771 3.27581,23.18275 15.11894,40.06551 26.96223,56.9485 M 63.66785,167.27674 C 53.476456,125.64318 43.285098,84.009799 60.140071,65.629038 c 16.854974,-18.38076 60.756409,-13.50903 79.417159,2.897714 18.66076,16.406744 12.08115,44.349128 15.14697,66.614938 3.06583,22.2658 15.7769,38.85458 28.48812,55.44358 M 53.481002,171.27421 C 44.689534,130.84456 35.898087,90.415102 52.599055,73.308251 c 16.700967,-17.10685 58.894505,-10.891184 76.057365,5.417582 17.16286,16.308765 9.29534,42.711247 12.15118,64.060097 2.85585,21.34885 16.43485,37.64365 30.01401,53.93867 M 46.033338,175.21589 C 38.641795,135.99016 31.25026,96.764627 47.797222,80.931686 64.344184,65.098744 104.82983,72.658348 120.4948,88.869135 c 15.66496,16.210785 6.50953,41.073355 9.15539,61.505255 2.64586,20.4319 17.0928,36.43273 31.5399,52.43376 M 41.064424,179.1071 c -5.991618,-38.02182 -11.98324,-76.04342 4.409715,-90.602456 16.392956,-14.559032 55.170711,-5.655491 69.337781,10.457318 14.16707,16.112808 3.72373,39.435458 6.1596,58.950418 2.43588,19.51495 17.75076,35.2218 33.06579,50.92884 M 38.313827,182.95313 c -4.591692,-36.8179 -9.183403,-73.63558 7.055547,-86.9207 16.23895,-13.285122 53.308818,-3.037644 65.977996,12.97719 12.66917,16.01483 0.93791,37.79757 3.1638,56.39557 2.22589,18.59801 18.40871,34.01088 34.59168,49.42393 M 37.521114,186.7593 c -3.191767,-35.61398 -6.383566,-71.22774 9.701378,-83.23895 16.084944,-12.011215 51.446921,-0.4198 62.618198,15.49705 11.17128,15.91685 -1.84789,36.15968 0.16802,53.84074 2.0159,17.68106 19.06666,32.79995 36.11757,47.91901 M 38.42585,190.5309 c -1.791841,-34.41006 -3.583728,-68.8199 12.34721,-79.5572 15.930938,-10.7373 49.58502,2.19805 59.25841,18.01692 9.67339,15.81887 -4.6337,34.52179 -2.82778,51.2859 1.80593,16.76411 19.72462,31.58902 37.64346,46.4141 M 40.767604,194.27324 c -0.391915,-33.20615 -0.783891,-66.41206 14.993042,-75.87545 15.776932,-9.4634 47.723124,4.81589 55.898614,20.53679 8.1755,15.72089 -7.4195,32.8839 -5.82356,48.73106 1.59593,15.84716 20.38256,30.37809 39.16934,44.90918 M 44.285942,197.99162 c 1.00801,-32.00223 2.015946,-64.00422 17.638873,-72.1937 15.622926,-8.18949 45.861225,7.43374 52.538825,23.05666 6.6776,15.62291 -10.20531,31.246 -8.81936,46.17622 1.38596,14.93021 21.04052,29.16716 40.69524,43.40427 M 48.72043,201.69134 c 2.407935,-30.79831 4.815783,-61.59637 20.284704,-68.51195 15.46892,-6.91557 43.999336,10.05159 49.179036,25.57653 5.1797,15.52494 -12.99112,29.60812 -11.81515,43.62138 1.17597,14.01326 21.69847,27.95624 42.22112,41.89936 M 53.810634,205.37771 c 3.807861,-29.59439 7.615621,-59.18853 22.930536,-64.8302 15.314914,-5.64166 42.13744,12.66944 45.81924,28.0964 3.68181,15.42696 -15.77692,27.97023 -14.81094,41.06654 0.96599,13.09631 22.35643,26.74531 43.74702,40.39444 M 59.296123,209.05603 c 5.207786,-28.39048 10.415458,-56.78069 25.576367,-61.14844 15.16091,-4.36776 40.27554,15.28728 42.45945,30.61626 2.18391,15.32898 -18.56273,26.33233 -17.80673,38.5117 0.756,12.17936 23.01438,25.53439 45.2729,38.88953 M 64.916461,212.73161 c 6.607712,-27.18656 13.215296,-54.37285 28.222199,-57.4667 15.0069,-3.09384 38.41364,17.90513 39.09966,33.13613 0.68602,15.231 -21.34854,24.69445 -20.80252,35.95686 0.54601,11.26242 23.67233,24.32347 46.79879,37.38462 M 70.411217,216.40974 c 8.007637,-25.98264 16.015133,-51.96501 30.868033,-53.78494 14.85289,-1.81994 36.55174,20.52297 35.73987,35.65599 -0.81188,15.13303 -24.13435,23.05656 -23.79832,33.40203 0.33603,10.34546 24.33028,23.11253 48.32468,35.8797 M 75.519956,220.09573 c 9.407563,-24.77872 18.814971,-49.55716 33.513864,-50.10319 14.69889,-0.54603 34.68985,23.14082 32.38007,38.17587 -2.30977,15.03504 -26.92015,21.41866 -26.7941,30.84718 0.12605,9.42852 24.98824,21.90161 49.85057,34.37479 M 79.982246,223.79489 c 10.807488,-23.57481 21.614804,-47.14932 36.159694,-46.42144 14.54488,0.72788 32.82795,25.75866 29.02028,40.69573 -3.80766,14.93707 -29.70596,19.78078 -29.78989,28.29235 -0.0839,8.51157 25.64619,20.69068 51.37646,32.86987 M 83.537653,227.51252 c 12.207414,-22.3709 24.414647,-44.74149 38.805527,-42.7397 14.39088,2.0018 30.96605,28.37652 25.66049,43.2156 -5.30556,14.83909 -32.49177,18.14289 -32.78569,25.73751 -0.29392,7.59462 26.30415,19.47976 52.90235,31.36496 M 85.925743,231.25391 c 13.60734,-21.16697 27.214487,-42.33364 41.451357,-39.05794 14.23687,3.2757 29.10416,30.99436 22.3007,45.73547 -6.80345,14.74111 -35.27757,16.50499 -35.78148,23.18267 -0.5039,6.67767 26.9621,18.26882 54.42824,29.86004"
       id="path01"
       inkscape:connector-curvature="0"
       inkscape:path-effect="#path-effect14;#path-effect20"
       inkscape:original-d="M 76.854315,163.2182 C 65.263308,120.38063 53.672039,77.543328 42.080504,34.706292 c 45.609202,3.527496 91.218526,7.055289 136.827386,10.583333 -5.29138,29.481723 -10.58307,58.964025 -15.875,88.446425 11.84364,16.88285 23.68677,33.76561 35.52976,50.64881 M 85.925743,231.25391 c 13.607407,-21.16693 27.214547,-42.3336 40.821427,-63.5 14.86712,27.71759 29.7344,55.43625 44.6012,83.15476 -28.47404,1.76363 -56.94816,3.52752 -85.422627,5.29167 27.466807,11.59112 54.932807,23.18228 82.398817,34.77381" />
  </g>
</svg>
)"""";

   testDoc(svg);
}

TEST_F(LPEInterpolateSubPathsTest, multi_MM_1_0_2)
{
   std::string svg = R""""(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   width="250mm"
   height="250mm"
   viewBox="0 0 250 249.99999"
   version="1.1"
   id="svg8"
   inkscape:version="1.0.2 (e86c870879, 2021-01-15)">
  <defs
     id="defs2">
    <inkscape:path-effect
       effect="interpolate"
       id="path-effect20"
       is_visible="true"
       trajectory="m 120.32157,109.54558 c -83.344568,49.87599 -1.29768,80.66835 8.31548,119.81845"
       equidistant_spacing="false"
       steps="19"
       trajectory-nodetypes="cc"
       lpeversion="0" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect18"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false"
       lpeversion="0" />
    <inkscape:path-effect
       effect="bspline"
       id="path-effect14"
       is_visible="true"
       weight="33.333333"
       steps="2"
       helper_size="0"
       apply_no_weight="true"
       apply_with_weight="true"
       only_selected="false"
       lpeversion="0" />
  </defs>
  <g
     inkscape:label="Layer 1"
     inkscape:groupmode="layer"
     id="layer1">
    <path
       style="fill:none;stroke:#000000;stroke-width:0.264583px;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1"
       d="M 76.854315,163.2182 C 65.262995,120.38071 53.671725,77.543413 70.680705,57.888743 87.689684,38.234074 133.29901,41.761866 153.45766,58.266589 c 20.15865,16.504722 14.86695,45.987021 18.14276,69.169771 3.27581,23.18275 15.11894,40.06551 26.96223,56.9485 M 63.66785,167.27674 C 53.476456,125.64318 43.285098,84.009799 60.140071,65.629038 c 16.854974,-18.38076 60.756409,-13.50903 79.417159,2.897714 18.66076,16.406744 12.08115,44.349128 15.14697,66.614938 3.06583,22.2658 15.7769,38.85458 28.48812,55.44358 M 53.481002,171.27421 C 44.689534,130.84456 35.898087,90.415102 52.599055,73.308251 c 16.700967,-17.10685 58.894505,-10.891184 76.057365,5.417582 17.16286,16.308765 9.29534,42.711247 12.15118,64.060097 2.85585,21.34885 16.43485,37.64365 30.01401,53.93867 M 46.033338,175.21589 C 38.641795,135.99016 31.25026,96.764627 47.797222,80.931686 64.344184,65.098744 104.82983,72.658348 120.4948,88.869135 c 15.66496,16.210785 6.50953,41.073355 9.15539,61.505255 2.64586,20.4319 17.0928,36.43273 31.5399,52.43376 M 41.064424,179.1071 c -5.991618,-38.02182 -11.98324,-76.04342 4.409715,-90.602456 16.392956,-14.559032 55.170711,-5.655491 69.337781,10.457318 14.16707,16.112808 3.72373,39.435458 6.1596,58.950418 2.43588,19.51495 17.75076,35.2218 33.06579,50.92884 M 38.313827,182.95313 c -4.591692,-36.8179 -9.183403,-73.63558 7.055547,-86.9207 16.23895,-13.285122 53.308818,-3.037644 65.977996,12.97719 12.66917,16.01483 0.93791,37.79757 3.1638,56.39557 2.22589,18.59801 18.40871,34.01088 34.59168,49.42393 M 37.521114,186.7593 c -3.191767,-35.61398 -6.383566,-71.22774 9.701378,-83.23895 16.084944,-12.011215 51.446921,-0.4198 62.618198,15.49705 11.17128,15.91685 -1.84789,36.15968 0.16802,53.84074 2.0159,17.68106 19.06666,32.79995 36.11757,47.91901 M 38.42585,190.5309 c -1.791841,-34.41006 -3.583728,-68.8199 12.34721,-79.5572 15.930938,-10.7373 49.58502,2.19805 59.25841,18.01692 9.67339,15.81887 -4.6337,34.52179 -2.82778,51.2859 1.80593,16.76411 19.72462,31.58902 37.64346,46.4141 M 40.767604,194.27324 c -0.391915,-33.20615 -0.783891,-66.41206 14.993042,-75.87545 15.776932,-9.4634 47.723124,4.81589 55.898614,20.53679 8.1755,15.72089 -7.4195,32.8839 -5.82356,48.73106 1.59593,15.84716 20.38256,30.37809 39.16934,44.90918 M 44.285942,197.99162 c 1.00801,-32.00223 2.015946,-64.00422 17.638873,-72.1937 15.622926,-8.18949 45.861225,7.43374 52.538825,23.05666 6.6776,15.62291 -10.20531,31.246 -8.81936,46.17622 1.38596,14.93021 21.04052,29.16716 40.69524,43.40427 M 48.72043,201.69134 c 2.407935,-30.79831 4.815783,-61.59637 20.284704,-68.51195 15.46892,-6.91557 43.999336,10.05159 49.179036,25.57653 5.1797,15.52494 -12.99112,29.60812 -11.81515,43.62138 1.17597,14.01326 21.69847,27.95624 42.22112,41.89936 M 53.810634,205.37771 c 3.807861,-29.59439 7.615621,-59.18853 22.930536,-64.8302 15.314914,-5.64166 42.13744,12.66944 45.81924,28.0964 3.68181,15.42696 -15.77692,27.97023 -14.81094,41.06654 0.96599,13.09631 22.35643,26.74531 43.74702,40.39444 M 59.296123,209.05603 c 5.207786,-28.39048 10.415458,-56.78069 25.576367,-61.14844 15.16091,-4.36776 40.27554,15.28728 42.45945,30.61626 2.18391,15.32898 -18.56273,26.33233 -17.80673,38.5117 0.756,12.17936 23.01438,25.53439 45.2729,38.88953 M 64.916461,212.73161 c 6.607712,-27.18656 13.215296,-54.37285 28.222199,-57.4667 15.0069,-3.09384 38.41364,17.90513 39.09966,33.13613 0.68602,15.231 -21.34854,24.69445 -20.80252,35.95686 0.54601,11.26242 23.67233,24.32347 46.79879,37.38462 M 70.411217,216.40974 c 8.007637,-25.98264 16.015133,-51.96501 30.868033,-53.78494 14.85289,-1.81994 36.55174,20.52297 35.73987,35.65599 -0.81188,15.13303 -24.13435,23.05656 -23.79832,33.40203 0.33603,10.34546 24.33028,23.11253 48.32468,35.8797 M 75.519956,220.09573 c 9.407563,-24.77872 18.814971,-49.55716 33.513864,-50.10319 14.69889,-0.54603 34.68985,23.14082 32.38007,38.17587 -2.30977,15.03504 -26.92015,21.41866 -26.7941,30.84718 0.12605,9.42852 24.98824,21.90161 49.85057,34.37479 M 79.982246,223.79489 c 10.807488,-23.57481 21.614804,-47.14932 36.159694,-46.42144 14.54488,0.72788 32.82795,25.75866 29.02028,40.69573 -3.80766,14.93707 -29.70596,19.78078 -29.78989,28.29235 -0.0839,8.51157 25.64619,20.69068 51.37646,32.86987 M 83.537653,227.51252 c 12.207414,-22.3709 24.414647,-44.74149 38.805527,-42.7397 14.39088,2.0018 30.96605,28.37652 25.66049,43.2156 -5.30556,14.83909 -32.49177,18.14289 -32.78569,25.73751 -0.29392,7.59462 26.30415,19.47976 52.90235,31.36496 M 85.925743,231.25391 c 13.60734,-21.16697 27.214487,-42.33364 41.451357,-39.05794 14.23687,3.2757 29.10416,30.99436 22.3007,45.73547 -6.80345,14.74111 -35.27757,16.50499 -35.78148,23.18267 -0.5039,6.67767 26.9621,18.26882 54.42824,29.86004"
       id="path01"
       inkscape:connector-curvature="0"
       inkscape:path-effect="#path-effect14;#path-effect20"
       inkscape:original-d="M 76.854315,163.2182 C 65.263308,120.38063 53.672039,77.543328 42.080504,34.706292 c 45.609202,3.527496 91.218526,7.055289 136.827386,10.583333 -5.29138,29.481723 -10.58307,58.964025 -15.875,88.446425 11.84364,16.88285 23.68677,33.76561 35.52976,50.64881 M 85.925743,231.25391 c 13.607407,-21.16693 27.214547,-42.3336 40.821427,-63.5 14.86712,27.71759 29.7344,55.43625 44.6012,83.15476 -28.47404,1.76363 -56.94816,3.52752 -85.422627,5.29167 27.466807,11.59112 54.932807,23.18228 82.398817,34.77381" />
  </g>
</svg>
)"""";

   testDoc(svg);
}