#ifndef __SP_XMLVIEW_TREE_H__
#define __SP_XMLVIEW_TREE_H__

/*
 * Specialization of GtkCTree for the XML editor
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *
 * Released under the GNU GPL; see COPYING for details
 */

#include <gtk/gtkctree.h>
#include "../xml/repr.h"

#define SP_TYPE_XMLVIEW_TREE (sp_xmlview_tree_get_type ())
#define SP_XMLVIEW_TREE(o) (GTK_CHECK_CAST ((o), SP_TYPE_XMLVIEW_TREE, SPXMLViewTree))
#define SP_IS_XMLVIEW_TREE(o) (GTK_CHECK_TYPE ((o), SP_TYPE_XMLVIEW_TREE))
#define SP_XMLVIEW_TREE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_XMLVIEW_TREE))

typedef struct _SPXMLViewTree SPXMLViewTree;
typedef struct _SPXMLViewTreeClass SPXMLViewTreeClass;

struct _SPXMLViewTree
{
	GtkCTree tree;

	SPRepr * repr;
	gint blocked;
};

struct _SPXMLViewTreeClass
{
	GtkCTreeClass parent_class;
};

GtkType sp_xmlview_tree_get_type (void);
GtkWidget * sp_xmlview_tree_new (SPRepr * repr, void * factory, void * data);

#define SP_XMLVIEW_TREE_REPR(tree) (SP_XMLVIEW_TREE (tree)->repr)

void sp_xmlview_tree_set_repr (SPXMLViewTree * tree, SPRepr * repr);

SPRepr * sp_xmlview_tree_node_get_repr (SPXMLViewTree * tree, GtkCTreeNode * node);
GtkCTreeNode * sp_xmlview_tree_get_repr_node (SPXMLViewTree * tree, SPRepr * repr);

#endif
