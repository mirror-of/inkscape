#include "sp-metrics.h"

/*
 * SPMetric handling and stuff
 * I hope this will be usefull :-) 
 */

gdouble
sp_absolute_metric_to_metric (gdouble length_src, const SPMetric metric_src, const SPMetric metric_dst) {
  gdouble src = 1, dst = 1;

  switch (metric_src) {
  case SP_MM:
    src = MM_PER_IN;
    break;
  case SP_CM:
    src = CM_PER_IN;
    break;
  case SP_IN:
    src = IN_PER_IN;
    break;
  case SP_PT:
    src = PT_PER_IN;
    break;
  case NONE:
    src = 1;
    break;
  }

  switch (metric_dst) {
  case SP_MM:
    dst = MM_PER_IN;
    break;
  case SP_CM:
    dst = CM_PER_IN;
    break;
  case SP_IN:
    dst = IN_PER_IN;
    break;
  case SP_PT:
    dst = PT_PER_IN;
    break;
  case NONE:
    dst = 1;
    break;
  }

  return length_src * (dst/src);
}

GString *
sp_metric_to_metric_string (gdouble length,  const SPMetric metric_src, const SPMetric metric_dst, gboolean m) {
  GString * str;
  gdouble len;

  len = sp_absolute_metric_to_metric (length, metric_src, metric_dst);
  str = g_string_new ("");
  switch (metric_dst) {
  case SP_MM:
    g_string_sprintf (str, "%0.2f%s", len, m?" mm":"");
    break;
  case SP_CM:
    g_string_sprintf (str, "%0.2f%s", len, m?" cm":"");
    break;
  case SP_IN:
    g_string_sprintf (str, "%0.2f%s", len, m?" \"":"");
    break;
  case SP_PT:
    g_string_sprintf (str, "%0.2f%s", len, m?" pt":"");
    break;
  case NONE:
    g_string_sprintf (str, "%s", "ups!");
    break;
  }

  return str;
}




