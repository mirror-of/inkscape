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
	SP_PT
} SPMetric;

#define PT_PER_IN 72.0
#define CM_PER_IN 2.54
#define MM_PER_IN 25.4
#define IN_PER_PT (1 / PT_PER_IN)
#define IN_PER_CM (1 / CM_PER_IN)
#define IN_PER_MM (1 / MM_PER_IN)
#define PT_PER_CM (PT_PER_IN * IN_PER_CM)
#define CM_PER_PT (1 / PT_PER_CM)
#define PT_PER_MM (PT_PER_IN * IN_PER_MM)
#define MM_PER_PT (1 / PT_PER_MM)
#define PT_PER_PT 1.0
#define IN_PER_IN 1.0

// this should be configurable in central place
#define SP_DEFAULT_METRIC SP_MM

gdouble sp_absolute_metric_to_metric (gdouble length_src, const SPMetric metric_src, const SPMetric metric_dst);
GString * sp_metric_to_metric_string (gdouble length,  const SPMetric metric_src, const SPMetric metric_dst, gboolean m);

// convenience since we mostly deal with points
#define SP_METRIC_TO_PT(l,m) sp_absolute_metric_to_metric(l,m,SP_PT);
#define SP_PT_TO_METRIC(l,m) sp_absolute_metric_to_metric(l,SP_PT,m);

#define SP_PT_TO_METRIC_STRING(l,m) sp_metric_to_metric_string(l,SP_PT,m,TRUE)
#define SP_PT_TO_STRING(l,m) sp_metric_to_metric_string(l,SP_PT,m,FALSE)


// ruler metrics
static const GtkRulerMetric sp_ruler_metrics[] =
{
  {"Pixels",      "Pi", PT_PER_PT, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"Millimeter",  "Mm", PT_PER_MM, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"Centimeters", "Cm", PT_PER_CM, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
  {"Inches",      "In", PT_PER_IN, { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 }, { 1, 2, 4, 8, 16 }},
  {"Points",      "Pt", PT_PER_PT, { 1, 2, 5, 10, 25, 50, 100, 250, 500, 1000 }, { 1, 5, 10, 50, 100 }},
};

#endif
