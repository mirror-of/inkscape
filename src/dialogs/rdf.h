#ifndef _RDF_H_
#define _RDF_H_

#include <config.h>
#include <glib.h>

#include "helper/sp-intl.h"

struct rdf_license_t {
    gchar *name;
    gchar *work_rdf;
    gchar *license_rdf;
};

extern rdf_license_t rdf_licenses [];

struct rdf_work_entity_t {
    char  *name;
    gchar *title;
    gchar *tag;
};

extern rdf_work_entity_t rdf_work_entities [];

struct rdf_t {
    gchar*                work_title;
    gchar*                work_date;
    gchar*                work_creator;
    gchar*                work_owner;
    gchar*                work_publisher;
    gchar*                work_type;
    gchar*                work_source;
    gchar*                work_subject;
    gchar*                work_description;
    struct rdf_license_t* license;
};

struct rdf_work_entity_t * rdf_find_entity(char * name);

gchar * rdf_get_work_string(char * name);
void    rdf_set_work_string(char * name, gchar * string);

struct rdf_license_t * rdf_get_license();
void                   rdf_set_license(struct rdf_license_t * license);

#endif // _RDF_H_

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
