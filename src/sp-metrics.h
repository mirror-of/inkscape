#ifndef SP_METRICS_H
#define SP_METRICS_H

#include <gtk/gtkruler.h>
#include "svg/svg.h"

// known metrics so far
// should be extended with meter, feet, yard etc
typedef enum {
      NONE,
	SP_MM,
	SP_CM,
	SP_IN,
	SP_PT,
	SP_PX,
	SP_M
} SPMetric;

#define PT_PER_IN 72.0
#define PX_PER_IN 90.0 // yet another place where the 90dpi is hard-coded
#define M_PER_IN 0.0254
#define CM_PER_IN 2.54
#define MM_PER_IN 25.4
#define MM_PER_MM 1.0
#define MM_PER_CM 10.0
#define MM_PER_M 1000.0
#define IN_PER_PT (1 / PT_PER_IN)
#define IN_PER_CM (1 / CM_PER_IN)
#define IN_PER_MM (1 / MM_PER_IN)
#define PT_PER_CM (PT_PER_IN / CM_PER_IN)
#define M_PER_PT (M_PER_IN / PT_PER_IN)
#define PT_PER_M (PT_PER_IN / M_PER_IN)
#define CM_PER_PT (CM_PER_IN / PT_PER_IN)
#define MM_PER_PT (MM_PER_IN / PT_PER_IN)
#define PT_PER_MM (PT_PER_IN / MM_PER_IN)
#define MM_PER_PX (MM_PER_IN / PX_PER_IN)
#define PT_PER_PT 1.0
#define PT_PER_PX 0.8 // yet another place where the 90dpi is hard-coded
#define IN_PER_IN 1.0

// this is used when we can't figure out the document preferred metric
#define SP_DEFAULT_METRIC SP_PT

gdouble sp_absolute_metric_to_metric (gdouble length_src, const SPMetric metric_src, const SPMetric metric_dst);
GString * sp_metric_to_metric_string (gdouble length,  const SPMetric metric_src, const SPMetric metric_dst, gboolean m);

// convenience since we mostly deal with points
#define SP_METRIC_TO_PT(l,m) sp_absolute_metric_to_metric(l,m,SP_PT);
#define SP_PT_TO_METRIC(l,m) sp_absolute_metric_to_metric(l,SP_PT,m);

#define SP_PT_TO_METRIC_STRING(l,m) sp_metric_to_metric_string(l, SP_PT, m, TRUE)
#define SP_PT_TO_STRING(l,m) sp_metric_to_metric_string(l, SP_PT, m, FALSE)

#define SP_PX_TO_METRIC_STRING(l,m) sp_metric_to_metric_string(l, SP_PX, m, TRUE)
#define SP_PX_TO_STRING(l,m) sp_metric_to_metric_string(l, SP_PX, m, FALSE)

// ruler metrics
static const GtkRulerMetric sp_ruler_metrics[] =
{
// NOTE: the order of records in this struct must correspond to the SPMetric enum.
  {"NONE",  "", 1, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"millimeters",  "mm", PT_PER_MM, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"centimeters", "cm", PT_PER_CM, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"inches",      "in", PT_PER_IN, { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 }, { 1, 2, 4, 8, 16 }},
  {"points",      "pt", PT_PER_PT, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"pixels",      "px", PT_PER_PX, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"meters",      "m", PT_PER_M, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
};

#endif
