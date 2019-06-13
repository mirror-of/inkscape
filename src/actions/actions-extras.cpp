// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Gio::Actions for output tied to the application and without GUI.
 *
 * Copyright (C) 2019 Tavmjong Bah
 *
 * The contents of this file may be used under the GNU General Public License Version 2 or later.
 *
 */

#include "actions-extras.h"

#include <vector>
#include <iostream>

#include <glibmm/i18n.h>

const std::vector<std::vector<Glib::ustring>> raw_data =
  {
   {"app.inkscape-version",          "InkscapeVersion",         "Base",       N_("Print Inkscape version and exit")                    },
   {"app.extension-directory",       "InkscapeExtensionsDir",   "Base",       N_("Print Extensions directory and exit")                },
   {"app.verb-list",                 "InkscapeVerbs",           "Base",       N_("Print a list of verbs and exit")                     },
   {"app.vacuum-defs",               "VacuumDefs",              "Base",       N_("Remove unused definitions (gradients, etc.)")        },
   {"app.verb",                      "Verb",                    "Base",       N_("Execute verb(s)")                                    },

   {"app.new",                       "NewDocument",             "File",       N_("Create new document from default template")          },
   {"app.quit",                      "Quit",                    "File",       N_("Quit Inkscape")                                      },

   {"app.open-page",                 "ImportPageNumber",        "Import",     N_("Import page number")                                 },
   {"app.convert-dpi-method",        "ImportDPIMethod",         "Import",     N_("Import DPI convert method")                          },
   {"app.no-convert-baseline",       "ImportBaselineConvert",   "Import",     N_("Import convert text baselines")                      },

   {"app.query-x",                   "QueryX",                  "Query",      N_("Query 'x' value(s) of selected objects")             },
   {"app.query-y",                   "QueryY",                  "Query",      N_("Query 'y' value(s) of selected objects")             },
   {"app.query-width",               "QueryWidth",              "Query",      N_("Query 'width' value(s) of object(s)")                },
   {"app.query-height",              "QueryHeight",             "Query",      N_("Query 'height' value(s) of object(s)")               },
   {"app.query-all",                 "QueryAll",                "Query",      N_("Query 'x', 'y', 'width', and 'height'")              },

   {"app.export-type",               "ExportType",              "Export",     N_("Export file type")                                   },
   {"app.export-filename",           "ExportFileName",          "Export",     N_("Export file name")                                   },
   {"app.export-overwrite",          "ExportOverWrite",         "Export",     N_("Export over-write file")                             },

   {"app.export-area",               "ExportArea",              "Export",     N_("Export area")                                        },
   {"app.export-area-drawing",       "ExportAreaDrawing",       "Export",     N_("Export drawing area")                                },
   {"app.export-area-page",          "ExportAreaPage",          "Export",     N_("Export page area")                                   },
   {"app.export-margin",             "ExportMargin",            "Export",     N_("Export margin")                                      },
   {"app.export-area-snap",          "ExportAreaSnap",          "Export",     N_("Export snap area to integer values")                 },
   {"app.export-width",              "ExportWidth",             "Export",     N_("Export width")                                       },
   {"app.export-height",             "ExportHeight",            "Export",     N_("Export height")                                      },

   {"app.export-id",                 "ExportID",                "Export",     N_("Export id(s)")                                       },
   {"app.export-id-only",            "ExportIDOnly",            "Export",     N_("Export id(s) only")                                  },

   {"app.export-plain-svg",          "ExportPlanSVG",           "Export",     N_("Export as plain SVG")                                },        
   {"app.export-dpi",                "ExportDPI",               "Export",     N_("Export DPI")                                         },
   {"app.export-ignore-filters",     "ExportIgnoreFilters",     "Export",     N_("Export ignore filters")                              },
   {"app.export-text-to-path",       "ExportTextToPath",        "Export",     N_("Export convert text to paths")                       },
   {"app.export-ps-level",           "ExportPSLevel",           "Export",     N_("Export PostScript level")                            },
   {"app.export-pdf-version",        "ExportPSVersion",         "Export",     N_("Export PDF version")                                 },
   {"app.export-latex",              "ExportLaTeX",             "Export",     N_("Export LaTeX")                                       },
   {"app.export-use-hints",          "ExportUseHInts",          "Export",     N_("Export using saved hints")                           },
   {"app.export-background",         "ExportBackground",        "Export",     N_("Export background color")                            },
   {"app.export-background-opacity", "ExportBackgroundOpacity", "Export",     N_("Export background opacity")                          },

   {"app.export-do",                 "ExportDo",                "Export",     N_("Do export")                                          },

   {"app.select-clear",              "SelectionClear",          "Select",     N_("Selection clear")                                    },                  
   {"app.select",                    "Selection",               "Select",     N_("Select via ID (Deprecated)")                         },                  
   {"app.select-via-id",             "SelectionViaId",          "Select",     N_("Select via ID")                                      },                  
   {"app.select-via-class",          "SelectionViaClass",       "Select",     N_("Select via class")                                   },                  
   {"app.select-via-element",        "SelectionViaElement",     "Select",     N_("Select via element")                                 },                  
   {"app.select-via-selector",       "SelectionViaSelector",    "Select",     N_("Select via selector")                                },                  

   {"app.transform-rotate",          "TransformRotate",         "Transform",  N_("Transform by rotation")                              },                  

   {"win.canvas-zoom-in",            "ZoomIn",                  "View",       N_("Zoom in")                                            },                  
   {"win.canvas-zoom-out",           "ZoomOut",                 "View",       N_("Zoom out")                                           },
   {"win.canvas-zoom-1-1",           "Zoom1:1",                 "View",       N_("Zoom to 1:1")                                        },
   {"win.canvas-zoom-1-2",           "Zoom1:2",                 "View",       N_("Zoom to 1:2")                                        },
   {"win.canvas-zoom-2-1",           "Zoom2:1",                 "View",       N_("Zoom to 2:1")                                        },
   {"win.canvas-zoom-selection",     "ZoomSelection",           "View",       N_("Zoom to fit selection in window")                    },
   {"win.canvas-zoom-drawing",       "ZoomDrawing",             "View",       N_("Zoom to fit drawing in window")                      },
   {"win.canvas-zoom-page",          "ZoomPage",                "View",       N_("Zoom to fit page in window")                         },
   {"win.canvas-zoom-page-width",    "ZoomPageWidth",           "View",       N_("Zoom to fit page width in window")                   },
   {"win.canvas-zoom-prev",          "ZoomPrev",                "View",       N_("Previous zoom (from the history of zooms)")          },
   {"win.canvas-zoom-next",          "ZoomNext",                "View",       N_("Next zoom (from the history of zooms)")              },
   {"win.canvas-zoom-center-page",   "ZoomCenterPage",          "View",       N_("Center page in window")                              },


   {"doc.snap-global-toggle",        "Snap",                    "Snap",       N_("Toggle snapping")                                    },

   {"doc.snap-bbox",                 "SnapBoundingBox",         "Snap",       ("Toggle snapping to bounding boxes (global)")           },
   {"doc.snap-bbox-edge",            "SnapBBoxEdge",            "Snap",       ("Toggle snapping to bounding box edge")                 },
   {"doc.snap-bbox-corner",          "SnapBBoxCorner",          "Snap",       ("Toggle snapping to bounding box corner")               },     
   {"doc.snap-bbox-edge-midpoint",   "SnapBBoxEdgeMidpoint",    "Snap",       ("Toggle snapping to bounding box edge mid-point")       },
   {"doc.snap-bbox-center",          "SnapBBoxCenter",          "Snap",       ("Toggle snapping to bounding box center")               },

   {"doc.snap-node-category",        "SnapNodes",               "Snap",       ("Toggle snapping to nodes (global)")                    },
   {"doc.snap-path",                 "SnapPath",                "Snap",       ("Toggle snapping to path")                              },
   {"doc.snap-path-intersection",    "SnapPathIntersection",    "Snap",       ("Toggle snapping to path intersection")                 },
   {"doc.snap-node-cusp",            "SnapNodeCusp",            "Snap",       ("Toggle snapping to cusp node")                         },
   {"doc.snap-node-smooth",          "SnapNodeSmooth",          "Snap",       ("Toggle snapping to smooth node")                       },
   {"doc.snap-line-midpoint",        "SnapLineMidpoint",        "Snap",       ("Toggle snapping to midpoint of line")                  },

   {"doc.snap-others",               "SnapOthers",              "Snap",       ("Toggle snapping to misc. point (global)")              },
   {"doc.snap-object-midpoint",      "SnapObjectMidpoint",      "Snap",       ("Toggle snapping to object midpoint")                   },
   {"doc.snap-rotation-center",      "SnapRoationCenter",       "Snap",       ("Toggle snapping to object rotation center")            },
   {"doc.snap-text-baseline",        "SnapTextBaseline",        "Snap",       ("Toggle snapping to text baseline")                     },

   {"doc.snap-page-border",          "SnapPageBorder",          "Snap",       ("Toggle snapping to page border")                       },
   {"doc.snap-grid",                 "SnapGrid",                "Snap",       ("Toggle snapping to grid")                              },
   {"doc.snap-guide",                "SnapGuide",               "Snap",       ("Toggle snapping to guide")                             },

   {"doc.snap-path-mask",            "SnapPathMask",            "Snap",       ("Toggle snapping to path of mask")                      },
   {"doc.snap-path-clip",            "SnapPathClip",            "Snap",       ("Toggle snapping to clip path")                         },

  };

Glib::ustring
InkActionExtras::get_label_for_action(Glib::ustring& action_name)
{
  Glib::ustring value;
  if (data.empty()) {
    read_data();
  }
  auto search = data.find(action_name);
  if (search != data.end()) {
    value = search->second.action_label;
  }
  return value;
}

Glib::ustring
InkActionExtras::get_section_for_action(Glib::ustring& action_name) {

  Glib::ustring value;
  if (data.empty()) {
    read_data();
  }
  auto search = data.find(action_name);
  if (search != data.end()) {
    value = search->second.action_section;
  }
  return value;
}

Glib::ustring
InkActionExtras::get_tooltip_for_action(Glib::ustring& action_name) {

  Glib::ustring value;
  if (data.empty()) {
    read_data();
  }
  auto search = data.find(action_name);
  if (search != data.end()) {
    value = search->second.action_tooltip;
  }
  return value;
}

void
InkActionExtras::read_data()
{
  for (auto raw : raw_data) {
    InkActionData datum(raw[1], raw[2], raw[3]);
    data.emplace(raw[0], datum);
  }
}

std::map<Glib::ustring, InkActionData> InkActionExtras::data;

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
