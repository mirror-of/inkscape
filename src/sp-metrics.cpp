#include "sp-metrics.h"
#include "svg/stringstream.h"

/*
 * SPMetric handling and stuff
 * I hope this will be usefull :-) 
 */

gdouble
sp_absolute_metric_to_metric (gdouble length_src, const SPMetric metric_src, const SPMetric metric_dst)
{
  gdouble src = 1;
  gdouble dst = 1;

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
sp_metric_to_metric_string (gdouble length,  const SPMetric metric_src, const SPMetric metric_dst, gboolean m)
{
  gdouble len = sp_absolute_metric_to_metric (length, metric_src, metric_dst);
  GString *str = g_string_new ("");
  Inkscape::SVGOStringStream os;
  
  switch (metric_dst) {
  case SP_MM:
    os << len << (m?" mm":"");
	g_string_sprintf (str, os.str().c_str());
    break;
  case SP_CM:
    os << len << (m?" cm":"");
	g_string_sprintf (str, os.str().c_str());
    break;
  case SP_IN:
    os << len << (m?" \"":"");
	g_string_sprintf (str, os.str().c_str());
    break;
  case SP_PT:
	os << len << (m?" pt":"");
	g_string_sprintf (str, os.str().c_str());
    break;
  case NONE:
    g_string_sprintf (str, "%s", "ups!");
    break;
  }

  return str;
}





/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
