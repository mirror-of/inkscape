#ifndef _RDF_H_
#define _RDF_H_

struct rdf_license_t {
	gchar *name;
	gchar *work_rdf;
	gchar *license_rdf;
};

extern rdf_license_t rdf_licenses [];

struct rdf_t {
	gchar*			work_title;
	gchar*			work_date;
	gchar*			work_description;
	gchar*			work_creator;
	gchar*			work_owner;
	gchar*			work_type;
	gchar*			work_source;
	struct rdf_license_t*	license;
};

#endif // _RDF_H_
