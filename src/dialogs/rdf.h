#ifndef _RDF_H_
#define _RDF_H_

#include <config.h>
#include <glib.h>

#include "helper/sp-intl.h"

/**
 * \brief Holds license name and RDF information
 */
struct rdf_license_t {
    gchar *name;        /* localized name of this license */
    gchar *work_rdf;    /* URL for the RDF/Work/license element */
    gchar *license_rdf; /* XML contents for the RDF/License tag */
};

extern rdf_license_t rdf_licenses [];

enum RDFType {
    RDF_CONTENT,
    RDF_AGENT,
    RDF_RESOURCE,
};

/**
 * \brief Holds known RDF/Work tags
 */
struct rdf_work_entity_t {
    char   *name;       /* unique name of this entity for internal reference */
    gchar  *title;      /* localized name of this entity for data entry labels */
    gchar  *tag;        /* namespace tag for the RDF/Work element */
    RDFType datatype;  /* how to extract/inject the RDF information */
};

extern rdf_work_entity_t rdf_work_entities [];

/**
 * \brief Generic collection of RDF information for the RDF debug function
 */
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

const gchar * rdf_get_work_entity(SPDocument * doc,
                                  struct rdf_work_entity_t * entity);
unsigned int  rdf_set_work_entity(SPDocument * doc,
                                  struct rdf_work_entity_t * entity,
                                  const gchar * text);

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
