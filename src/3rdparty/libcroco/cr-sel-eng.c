/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser 
 * General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 * See  COPYRIGHTS file for copyright information.
 */

#include <string.h>
#include "cr-sel-eng.h"
#include "cr-node-iface.h"

/**
 *@CRSelEng:
 *
 *The definition of the  #CRSelEng class.
 *The #CRSelEng is actually the "Selection Engine"
 *class. This is highly experimental for at the moment and
 *its api is very likely to change in a near future.
 */

#define PRIVATE(a_this) (a_this)->priv

struct CRPseudoClassSelHandlerEntry {
        guchar *name;
        enum CRPseudoType type;
        CRPseudoClassSelectorHandler handler;
};

struct _CRSelEngPriv {
        /*not used yet */
        gboolean case_sensitive;

        CRNodeIface const *node_iface;
        CRStyleSheet *sheet;
        /**
         *where to store the next statement
         *to be visited so that we can remember
         *it from one method call to another.
         */
        CRStatement *cur_stmt;
        GList *pcs_handlers;
        gint pcs_handlers_size;
} ;

static gboolean class_add_sel_matches_node (CRAdditionalSel * a_add_sel,
                                            CRNodeIface const * a_node_iface, CRXMLNodePtr a_node);

static gboolean id_add_sel_matches_node (CRAdditionalSel * a_add_sel,
                                         CRNodeIface const * a_node_iface, CRXMLNodePtr a_node);

static gboolean attr_add_sel_matches_node (CRAdditionalSel * a_add_sel,
                                           CRNodeIface const * a_node_iface, CRXMLNodePtr a_node);

static enum CRStatus sel_matches_node_real (CRSelEng * a_this,
                                            CRSimpleSel * a_sel,
                                            CRXMLNodePtr a_node,
                                            gboolean * a_result,
                                            gboolean a_eval_sel_list_from_end,
                                            gboolean a_recurse);

static enum CRStatus cr_sel_eng_get_matched_rulesets_real (CRSelEng * a_this,
                                                           CRStyleSheet *
                                                           a_stylesheet,
                                                           CRXMLNodePtr a_node,
                                                           CRStatement ***
                                                           a_rulesets,
                                                           gulong * a_len,
                                                           gulong * a_capacity);

static enum CRStatus put_css_properties_in_props_list (CRPropList ** a_props,
                                                       CRStatement *
                                                       a_ruleset);

static gboolean pseudo_class_add_sel_matches_node (CRSelEng * a_this,
                                                   CRAdditionalSel * a_add_sel,
                                                   CRXMLNodePtr a_node);

static gboolean empty_pseudo_class_handler (CRSelEng * a_this,
                                            CRAdditionalSel * a_sel,
                                            CRXMLNodePtr a_node);

static gboolean root_pseudo_class_handler (CRSelEng * a_this,
                                           CRAdditionalSel * a_sel,
                                           CRXMLNodePtr a_node);

static gboolean lang_pseudo_class_handler (CRSelEng * a_this,
                                           CRAdditionalSel * a_sel,
                                           CRXMLNodePtr a_node);

static gboolean only_child_pseudo_class_handler (CRSelEng * a_this,
                                                 CRAdditionalSel * a_sel,
                                                 CRXMLNodePtr a_node);

static gboolean first_child_pseudo_class_handler (CRSelEng * a_this,
                                                  CRAdditionalSel * a_sel,
                                                  CRXMLNodePtr a_node);

static gboolean first_of_type_pseudo_class_handler (CRSelEng * a_this,
                                                    CRAdditionalSel * a_sel,
                                                    CRXMLNodePtr a_node);

static gboolean last_child_pseudo_class_handler (CRSelEng * a_this,
                                                 CRAdditionalSel * a_sel,
                                                 CRXMLNodePtr a_node);

static gboolean last_of_type_pseudo_class_handler (CRSelEng * a_this,
                                                   CRAdditionalSel * a_sel,
                                                   CRXMLNodePtr a_node);

static gboolean nth_child_pseudo_class_handler (CRSelEng * a_this,
                                                CRAdditionalSel * a_sel,
                                                CRXMLNodePtr a_node);

static gboolean nth_of_type_pseudo_class_handler (CRSelEng * a_this,
                                                  CRAdditionalSel * a_sel,
                                                  CRXMLNodePtr a_node);

static gboolean nth_last_child_pseudo_class_handler (CRSelEng * a_this,
                                                     CRAdditionalSel * a_sel,
                                                     CRXMLNodePtr a_node);

static gboolean nth_last_of_type_pseudo_class_handler (CRSelEng * a_this,
                                                       CRAdditionalSel * a_sel,
                                                       CRXMLNodePtr a_node);

static CRXMLNodePtr get_next_element_node (CRNodeIface const * a_node_iface, CRXMLNodePtr a_node);

static CRXMLNodePtr get_first_child_element_node (CRNodeIface const * a_node_iface, CRXMLNodePtr a_node);

static CRXMLNodePtr get_prev_element_node (CRNodeIface const * a_node_iface, CRXMLNodePtr a_node);

static CRXMLNodePtr get_next_parent_element_node (CRNodeIface const * a_node_iface, CRXMLNodePtr a_node);

static CRArguments get_arguments_from_function (CRAdditionalSel * a_sel);

void
cr_sel_eng_set_node_iface (CRSelEng *const a_this, CRNodeIface const *const a_node_iface)
{
        /* Allow NULL: the caller may be just ensuring that the previous node_iface
           value doesn't get used until next cr_sel_eng_set_node_iface call. */
        PRIVATE(a_this)->node_iface = a_node_iface;
}

/* Quick strcmp.  Test only for == 0 or != 0, not < 0 or > 0.  */
#define strqcmp(str,lit,lit_len) \
  (strlen (str) != (lit_len) || memcmp (str, lit, lit_len))

static gboolean
root_pseudo_class_handler (CRSelEng *const a_this,
                           CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "root")
            || a_sel->content.pseudo->type != IDENT_PSEUDO) {
                cr_utils_trace_info ("This handler is for :root only");
                return FALSE;
        }

        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);

        // libxml apears to set the parent of the root element to an
        // element of type 'xml'.
        return (parent == NULL || !strcmp(node_iface->getLocalName(parent),"xml") );
}

static gboolean
empty_pseudo_class_handler (CRSelEng *const a_this,
                            CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr cur_node = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "empty")
            || a_sel->content.pseudo->type != IDENT_PSEUDO) {
                cr_utils_trace_info ("This handler is for :empty only");
                return FALSE;
        }
        node_iface = PRIVATE(a_this)->node_iface;

        cur_node = node_iface->getFirstChild (a_node);

        return (cur_node == NULL);
}

static gboolean
lang_pseudo_class_handler (CRSelEng *const a_this,
                           CRAdditionalSel * a_sel, CRXMLNodePtr a_node)
{
        CRNodeIface const *node_iface;
        CRXMLNodePtr node = a_node;
        gboolean result = FALSE;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        node_iface = PRIVATE(a_this)->node_iface;

        /* "xml:lang" needed for SVG */
        if ( (strqcmp (a_sel->content.pseudo->name->stryng->str, "lang", 4 ) &&
              (strqcmp (a_sel->content.pseudo->name->stryng->str, "xml:lang", 8 ) ) )
            || a_sel->content.pseudo->type != FUNCTION_PSEUDO) {
                cr_utils_trace_info ("This handler is for :lang only");
                return FALSE;
        }
        /*lang code should exist and be at least of length 2 */
        if (!a_sel->content.pseudo->term
            || a_sel->content.pseudo->term->type != TERM_IDENT
            || !a_sel->content.pseudo->term->content.str->stryng
            || a_sel->content.pseudo->term->content.str->stryng->len < 2)
                return FALSE;
        for (; node; node = get_next_parent_element_node (node_iface, node)) {
                char *val = node_iface->getProp (node, "lang");
                if (!val) val = node_iface->getProp (node, "xml:lang");
                if (val) {
                        if (!strcasecmp(val, a_sel->content.pseudo->term->content.str->stryng->str)) {
                                result = TRUE;
                                break;
                        }
                        node_iface->freePropVal (val);
                        val = NULL;
                }
        }

        return result;
}

static gboolean
only_child_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;
        CRXMLNodePtr cur_node = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "only-child")
            || a_sel->content.pseudo->type != IDENT_PSEUDO) {
                cr_utils_trace_info ("This handler is for :only-child only");
                return FALSE;
        }
        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);
        if (!parent)
                return FALSE;

        cur_node = get_first_child_element_node (node_iface, parent);
        return (cur_node == a_node &&
                !get_next_element_node(node_iface, cur_node) );
}

static gboolean
only_of_type_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;
        CRXMLNodePtr cur_node = NULL;
        int m = 0;
        int child = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "only-of-type")
            || a_sel->content.pseudo->type != IDENT_PSEUDO) {
                cr_utils_trace_info ("This handler is for :only-of-type selector only");
                return FALSE;
        }
        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);
        if (!parent)
                return FALSE;

        cur_node = get_first_child_element_node (node_iface, parent);

        while (cur_node) {
                if (!strcmp(node_iface->getLocalName(cur_node), a_sel->content.pseudo->sel_name->stryng->str)) {
                        ++m;
                }
                if (cur_node == a_node) {
                        child = m;
                }
                cur_node = get_next_element_node (node_iface, cur_node);
        }
        return (child == m && child == 1);
}

static gboolean
first_child_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr node = NULL;
        CRXMLNodePtr parent = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "first-child")
            || a_sel->content.pseudo->type != IDENT_PSEUDO) {
                cr_utils_trace_info ("This handler is for :first-child only");
                return FALSE;
        }
        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);
        if (!parent)
                return FALSE;
        node = get_first_child_element_node (node_iface, parent);
        return (node == a_node);
}

static gboolean
first_of_type_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;

        // Count which child no. of type
        CRXMLNodePtr cur_node = NULL;
        int child = 0;
        int found = FALSE;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "first-of-type")
            || a_sel->content.pseudo->type != IDENT_PSEUDO) {
                cr_utils_trace_info ("This handler is for :first-of-type only");
                return FALSE;
        }
        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);
        if (!parent)
                return FALSE;

        cur_node = get_first_child_element_node (node_iface, parent);

        while (cur_node) {
                if(!strcmp(node_iface->getLocalName(cur_node), a_sel->content.pseudo->sel_name->stryng->str)) {
                        child++;
                }
                if (cur_node == a_node) {
                        found = TRUE;
                        break;
                }
                cur_node = get_next_element_node (node_iface, cur_node);
        }

        if (!found)
                return FALSE;

        return (child == 1);
}

static gboolean
last_child_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;

        CRXMLNodePtr cur_node = NULL;
        int m = 0;
        int child = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "last-child")
            || a_sel->content.pseudo->type != IDENT_PSEUDO) {
                cr_utils_trace_info ("This handler is for :last-child only");
                return FALSE;
        }
        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);
        if (!parent)
                return FALSE;

        cur_node = get_first_child_element_node (node_iface, parent);
        while (cur_node) {
                ++m;
                if (cur_node == a_node) {
                        child = m;
                }
                cur_node = get_next_element_node (node_iface, cur_node);

        }
        return (m == child);
}

static gboolean
last_of_type_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;

        CRXMLNodePtr cur_node = NULL;
        int m = 0;
        int child = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "last-of-type")
            || a_sel->content.pseudo->type != IDENT_PSEUDO) {
                cr_utils_trace_info ("This handler is for :last-of-type only");
                return FALSE;
        }
        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);
        if (!parent)
                return FALSE;

        cur_node = get_first_child_element_node (node_iface, parent);

        while (cur_node) {
                if (!strcmp(node_iface->getLocalName(cur_node), a_sel->content.pseudo->sel_name->stryng->str)) {
                        ++m;
                }
                if (cur_node == a_node) {
                        child = m;
                }
                cur_node = get_next_element_node (node_iface, cur_node);
        }

        return (m == child);
}

// See https://www.w3.org/TR/selectors/#nth-child-pseudo
static gboolean
nth_child_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;

        /* Count which child this is */
        CRXMLNodePtr cur_node = NULL;
        int child = 0;
        int found = FALSE;
        int a, b;
        CRArguments arg;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "nth-child")
            || a_sel->content.pseudo->type != FUNCTION_PSEUDO) {
                cr_utils_trace_info ("This handler is for :nth-child only");
                return FALSE;
        }

        /*pseude code term should exist */
        if (!a_sel->content.pseudo->term)
                return FALSE;

        arg = get_arguments_from_function (a_sel);

        if (arg.a == 0 && arg.b == 0)
                return FALSE;

        a = arg.a;
        b = arg.b;

        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);
        if (!parent)
                return FALSE;

        cur_node = get_first_child_element_node (node_iface, parent);

        while (cur_node) {
                ++child;
                if (cur_node == a_node) {
                        found = TRUE;
                        break;
                }
                cur_node = get_next_element_node (node_iface, cur_node);
        }

        if (!found)
                return FALSE;

        if (a == 0)
                return (b == child);

        return ((child - b)%a == 0 && (child - b)/a > -1);
}

static gboolean
nth_of_type_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;

        // Count which child no. of required type
        CRXMLNodePtr cur_node = NULL;
        int child = 0;
        int found = FALSE;
        int a, b;
        CRArguments arg;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "nth-of-type")
            || a_sel->content.pseudo->type != FUNCTION_PSEUDO) {
                cr_utils_trace_info ("This handler is for :nth-of-type only");
                return FALSE;
        }

        // pseudo code term should exist
        if (!a_sel->content.pseudo->term)
                return FALSE;

        arg = get_arguments_from_function (a_sel);

        if (arg.a == 0 && arg.b == 0)
                return FALSE;

        a = arg.a;
        b = arg.b;

        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);
        if (!parent)
                return FALSE;

        cur_node = get_first_child_element_node (node_iface, parent);

        while (cur_node) {
                // check if type match
                if (!strcmp(node_iface->getLocalName(cur_node), a_sel->content.pseudo->sel_name->stryng->str))
                        ++child;
                if (cur_node == a_node) {
                        found = TRUE;
                        break;
                }
                cur_node = get_next_element_node (node_iface, cur_node);
        }

        if (!found)
                return FALSE;

        if (a == 0)
                return (b == child);

        return ((child - b)%a == 0 && (child - b)/a > -1);
}

static gboolean
nth_last_child_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;

        /* Count which child this is (child) and total number of children (m). */
        CRXMLNodePtr cur_node = NULL;
        int m = 0;
        int child = 0;
        int found = FALSE;
        int a, b;
        CRArguments arg;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "nth-last-child")) {
                cr_utils_trace_info ("This handler is for :nth-last-child only");
                return FALSE;
        }

        /*pseudo code term should exist */
        if (!a_sel->content.pseudo->term)
                return FALSE;

        arg = get_arguments_from_function (a_sel);

        if (arg.a == 0 && arg.b == 0)
                return FALSE;

        a = arg.a;
        b = arg.b;

        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);

        if (!parent) {
                return FALSE;
        }

        cur_node = get_first_child_element_node (node_iface, parent);

        while (cur_node) {
                if (cur_node == a_node) {
                        found = TRUE;
                        child = m;
                }
                cur_node = get_next_element_node (node_iface,cur_node);
                ++m;
        }

        if (!found)
                return FALSE;

        if (a == 0)
                return ((m - b) == child);

        return ((m - child - b)%a == 0 && (m - child - b)/a > -1);
}

static gboolean
nth_last_of_type_pseudo_class_handler (CRSelEng *const a_this,
                                  CRAdditionalSel * a_sel, CRXMLNodePtr const a_node)
{
        CRNodeIface const *node_iface = NULL;
        CRXMLNodePtr parent = NULL;
        CRXMLNodePtr cur_node = NULL;
        int m = 0;
        int child = 0;
        int found = FALSE;
        CRArguments arg;
        int a, b;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_sel && a_sel->content.pseudo
                              && a_sel->content.pseudo
                              && a_sel->content.pseudo->name
                              && a_sel->content.pseudo->name->stryng
                              && a_node, FALSE);

        if (strcmp (a_sel->content.pseudo->name->stryng->str, "nth-last-of-type")) {
                cr_utils_trace_info ("This handler is for :nth-last-of-type only");
                return FALSE;
        }

        /*pseude code term should exist */
        if (!a_sel->content.pseudo->term)
                return FALSE;

        arg = get_arguments_from_function (a_sel);

        if (arg.a == 0 && arg.b == 0)
                return FALSE;

        a = arg.a;
        b = arg.b;

        node_iface = PRIVATE(a_this)->node_iface;
        parent = node_iface->getParentNode (a_node);
        if (!parent) {
                return FALSE;
        }

        cur_node = get_first_child_element_node (node_iface, parent);

        while (cur_node) {
                if (!strcmp(node_iface->getLocalName(cur_node), a_sel->content.pseudo->sel_name->stryng->str))
                        ++m;
                if (cur_node == a_node) {
                        found = TRUE;
                        child = m;
                }
                cur_node = get_next_element_node (node_iface, cur_node);
        }

        if (!found)
                return FALSE;

        if (a == 0)
                return ((m - b) == child);

        return ((m - child - b +1)%a == 0 && (m - child - b +1)/a > -1);

}

static gboolean
pseudo_class_add_sel_matches_node (CRSelEng * a_this,
                                   CRAdditionalSel * a_add_sel,
                                   CRXMLNodePtr a_node)
{
        enum CRStatus status = CR_OK;
        CRPseudoClassSelectorHandler handler = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_add_sel
                              && a_add_sel->content.pseudo
                              && a_add_sel->content.pseudo->name
                              && a_add_sel->content.pseudo->name->stryng
                              && a_add_sel->content.pseudo->name->stryng->str
                              && a_node, FALSE);

        status = cr_sel_eng_get_pseudo_class_selector_handler
                (a_this, (guchar *) a_add_sel->content.pseudo->name->stryng->str,
                 a_add_sel->content.pseudo->type, &handler);
        if (status != CR_OK || !handler)
                return FALSE;

        return handler (a_this, a_add_sel, a_node);
}

/**
 *@param a_add_sel the class additional selector to consider.
 *@param a_node the xml node to consider.
 *@return TRUE if the class additional selector matches
 *the xml node given in argument, FALSE otherwise.
 */
static gboolean
class_add_sel_matches_node (CRAdditionalSel * a_add_sel,
                            CRNodeIface const * a_node_iface, CRXMLNodePtr a_node)
{
        gboolean result = FALSE;
        char *klass = NULL;

        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == CLASS_ADD_SELECTOR
                              && a_add_sel->content.class_name
                              && a_add_sel->content.class_name->stryng
                              && a_add_sel->content.class_name->stryng->str
                              && a_node, FALSE);

        klass = a_node_iface->getProp (a_node, "class");
        if (klass) {
                char const *cur;
                for (cur = klass; cur && *cur; cur++) {
                        while (cur && *cur
                               && cr_utils_is_white_space (*cur) 
                               == TRUE)
                                cur++;

                        if (!strncmp ((const char *) cur, 
                                      a_add_sel->content.class_name->stryng->str,
                                      a_add_sel->content.class_name->stryng->len)) {
                                cur += a_add_sel->content.class_name->stryng->len;
                                if ((cur && !*cur)
                                    || cr_utils_is_white_space (*cur) == TRUE)
                                        result = TRUE;
                        } else {  /* if it doesn't match,  */
                                /*   then skip to next whitespace character to try again */
                                while (cur && *cur && !(cr_utils_is_white_space(*cur) == TRUE)) 
                                        cur++;
                        }
                        if (cur && !*cur)
                                break ;
                }
                a_node_iface->freePropVal (klass);
                klass = NULL;
        }
        return result;

}

/**
 *@return TRUE if the additional attribute selector matches
 *the current xml node given in argument, FALSE otherwise.
 *@param a_add_sel the additional attribute selector to consider.
 *@param a_node the xml node to consider.
 */
static gboolean
id_add_sel_matches_node (CRAdditionalSel * a_add_sel,
                         CRNodeIface const * a_node_iface, CRXMLNodePtr a_node)
{
        gboolean result = FALSE;
        char *id = NULL;

        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == ID_ADD_SELECTOR
                              && a_add_sel->content.id_name
                              && a_add_sel->content.id_name->stryng
                              && a_add_sel->content.id_name->stryng->str
                              && a_node, FALSE);
        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == ID_ADD_SELECTOR
                              && a_node, FALSE);

        id = a_node_iface->getProp (a_node, "id");
        if (id) {
                if (!strqcmp ((const char *) id, a_add_sel->content.id_name->stryng->str,
                              a_add_sel->content.id_name->stryng->len)) {
                        result = TRUE;
                }
                a_node_iface->freePropVal (id);
                id = NULL;
        }
        return result;
}

/**
 *Returns TRUE if the instance of #CRAdditional selector matches
 *the node given in parameter, FALSE otherwise.
 *@param a_add_sel the additional selector to evaluate.
 *@param a_node the xml node against which the selector is to
 *be evaluated
 *return TRUE if the additional selector matches the current xml node
 *FALSE otherwise.
 */
static gboolean
attr_add_sel_matches_node (CRAdditionalSel * a_add_sel,
                           CRNodeIface const * a_node_iface, CRXMLNodePtr a_node)
{
        CRAttrSel *cur_sel = NULL;

        g_return_val_if_fail (a_add_sel
                              && a_add_sel->type == ATTRIBUTE_ADD_SELECTOR
                              && a_node, FALSE);

        for (cur_sel = a_add_sel->content.attr_sel;
             cur_sel; cur_sel = cur_sel->next) {
                char *value;

                if (!cur_sel->name
                    || !cur_sel->name->stryng
                    || !cur_sel->name->stryng->str)
                        return FALSE;

                value = a_node_iface->getProp (a_node, cur_sel->name->stryng->str);
                if (!value)
                        goto free_and_return_false;

                switch (cur_sel->match_way) {
                case SET:
                        break;

                case EQUALS:
                        if (!cur_sel->value
                            || !cur_sel->value->stryng
                            || !cur_sel->value->stryng->str) {
                                goto free_and_return_false;
                        }
                        if (strcmp 
                            (value, 
                             cur_sel->value->stryng->str)) {
                                goto free_and_return_false;
                        }
                        break;

                case INCLUDES:
                        {
                                char const *ptr1 = NULL,
                                        *ptr2 = NULL,
                                        *cur = NULL;
                                gboolean found = FALSE;

                                /*
                                 *here, make sure value is a space
                                 *separated list of "words", where one
                                 *value is exactly cur_sel->value->str
                                 */
                                for (cur = value; *cur; cur++) {
                                        /*
                                         *set ptr1 to the first non white space
                                         *char addr.
                                         */
                                        while (cr_utils_is_white_space (*cur)
                                               && *cur)
                                                cur++;
                                        if (!*cur)
                                                break;
                                        ptr1 = cur;

                                        /*
                                         *set ptr2 to the end the word.
                                         */
                                        while (!cr_utils_is_white_space (*cur)
                                               && *cur)
                                                cur++;
                                        cur--;
                                        ptr2 = cur;

                                        if (!strncmp
                                            ((const char *) ptr1, 
                                             cur_sel->value->stryng->str,
                                             ptr2 - ptr1 + 1)) {
                                                found = TRUE;
                                                break;
                                        }
                                        ptr1 = ptr2 = NULL;
                                }

                                if (!found) {
                                        goto free_and_return_false;
                                }
                        }
                        break;

                case DASHMATCH:
                        {
                                char const *ptr1 = NULL,
                                        *ptr2 = NULL,
                                        *cur = NULL;
                                gboolean found = FALSE;

                                /*
                                 *here, make sure value is an hyphen
                                 *separated list of "words", each of which
                                 *starting with "cur_sel->value->str"
                                 */
                                for (cur = value; *cur; cur++) {
                                        if (*cur == '-')
                                                cur++;
                                        ptr1 = cur;

                                        while (*cur != '-' && *cur)
                                                cur++;
                                        cur--;
                                        ptr2 = cur;

                                        if (g_strstr_len
                                            ((const gchar *) ptr1, ptr2 - ptr1 + 1,
                                             cur_sel->value->stryng->str)
                                            == ptr1) {
                                                found = TRUE;
                                                break;
                                        }
                                }

                                if (!found) {
                                        goto free_and_return_false;
                                }
                        }
                        break;
                default:
                        goto free_and_return_false;
                }

                a_node_iface->freePropVal (value);
                continue;

        free_and_return_false:
                a_node_iface->freePropVal (value);
                return FALSE;
        }

        return TRUE;
}

/**
 *Evaluates if a given additional selector matches an xml node.
 *@param a_add_sel the additional selector to consider.
 *@param a_node the xml node to consider.
 *@return TRUE is a_add_sel matches a_node, FALSE otherwise.
 */
static gboolean
additional_selector_matches_node (CRSelEng * a_this,
                                  CRAdditionalSel * a_add_sel,
                                  CRXMLNodePtr a_node)
{
        CRAdditionalSel *cur_add_sel = NULL, *tail = NULL ;
        gboolean evaluated = FALSE ;

        for (tail = a_add_sel ; 
             tail && tail->next; 
             tail = tail->next) ;

        g_return_val_if_fail (tail, FALSE) ;

        for (cur_add_sel = tail ;
             cur_add_sel ;
             cur_add_sel = cur_add_sel->prev) {

                evaluated = TRUE ;
                if (cur_add_sel->type == NO_ADD_SELECTOR) {
                        return FALSE;
                }

                if (cur_add_sel->type == CLASS_ADD_SELECTOR
                    && cur_add_sel->content.class_name
                    && cur_add_sel->content.class_name->stryng
                    && cur_add_sel->content.class_name->stryng->str) {
                        if (!class_add_sel_matches_node (cur_add_sel,
                                                         PRIVATE(a_this)->node_iface,
                                                         a_node)) {
                                return FALSE;
                        }
                        continue ;
                } else if (cur_add_sel->type == ID_ADD_SELECTOR
                           && cur_add_sel->content.id_name
                           && cur_add_sel->content.id_name->stryng
                           && cur_add_sel->content.id_name->stryng->str) {
                        if (!id_add_sel_matches_node (cur_add_sel,
                                                      PRIVATE(a_this)->node_iface,
                                                      a_node)) {
                                return FALSE;
                        }
                        continue ;
                } else if (cur_add_sel->type == ATTRIBUTE_ADD_SELECTOR
                           && cur_add_sel->content.attr_sel) {
                        /*
                         *here, call a function that does the match
                         *against an attribute additional selector
                         *and an xml node.
                         */
                        if (!attr_add_sel_matches_node (cur_add_sel,
                                                       PRIVATE(a_this)->node_iface,
                                                       a_node)) {
                                return FALSE;
                        }
                        continue ;
                } else if (cur_add_sel->type == PSEUDO_CLASS_ADD_SELECTOR
                           && cur_add_sel->content.pseudo) {
                        if (!pseudo_class_add_sel_matches_node
                            (a_this, cur_add_sel, a_node)) {
                                return FALSE;
                        }
                        continue ;
                }
        }
        if (evaluated == TRUE)
                return TRUE;
        return FALSE ;
}

static CRXMLNodePtr
get_next_element_node (CRNodeIface const * a_node_iface, CRXMLNodePtr a_node)
{
        CRXMLNodePtr cur_node = a_node;

        g_return_val_if_fail (a_node, NULL);

        do {
                cur_node = a_node_iface->getNextSibling (cur_node);
        } while (cur_node && !a_node_iface->isElementNode(cur_node));
        return cur_node;
}

static CRXMLNodePtr
get_first_child_element_node (CRNodeIface const * a_node_iface, CRXMLNodePtr a_node)
{
        CRXMLNodePtr cur_node = NULL;

        g_return_val_if_fail (a_node, NULL);

        cur_node = a_node_iface->getFirstChild (a_node);
        if (!cur_node)
                return cur_node;
        if (a_node_iface->isElementNode (cur_node))
                return cur_node;
        return get_next_element_node (a_node_iface, cur_node);
}

static CRXMLNodePtr
get_prev_element_node (CRNodeIface const * a_node_iface, CRXMLNodePtr a_node)
{
        CRXMLNodePtr cur_node = a_node;

        g_return_val_if_fail (a_node, NULL);

        do {
                cur_node = a_node_iface->getPrevSibling (cur_node);
        } while (cur_node && !a_node_iface->isElementNode(cur_node));
        return cur_node;
}

static CRXMLNodePtr
get_next_parent_element_node (CRNodeIface const * a_node_iface, CRXMLNodePtr a_node)
{
        CRXMLNodePtr cur_node = a_node;

        g_return_val_if_fail (a_node, NULL);

        do {
                cur_node = a_node_iface->getParentNode (cur_node);
        } while (cur_node && !a_node_iface->isElementNode (cur_node));
        return cur_node;
}

static CRArguments
get_arguments_from_function (CRAdditionalSel * a_sel)
{
        CRArguments arg;
        arg.a = 0;
        arg.b = 0;
        switch (a_sel->content.pseudo->term->type) {
        case TERM_NUMBER:
                if (a_sel->content.pseudo->term->content.num) {
                        arg.b = a_sel->content.pseudo->term->content.num->val;
                }
                if (a_sel->content.pseudo->term->n) {
                        arg.a = arg.b;
                        arg.b = 0;
                }
                break;

        case TERM_IDENT:
                if (a_sel->content.pseudo->term->content.str) {
                        if (!strcmp(a_sel->content.pseudo->term->content.str->stryng->str, "even")) {
                                arg.a = 2;
                                arg.b = 0;
                        } else if (!strcmp(a_sel->content.pseudo->term->content.str->stryng->str, "odd")) {
                                arg.a = 2;
                                arg.b = 1;
                        } else if (!strcmp(a_sel->content.pseudo->term->content.str->stryng->str, "n")) {
                                /* 'n' without number */
                                arg.a = 1;
                        } else if (!strcmp(a_sel->content.pseudo->term->content.str->stryng->str, "-n")) {
                                /* '-n' without number */
                                arg.a = -1;
                        } else {
                                /* Unknown string */
                                arg.a = 0;
                                arg.b = 0;
                                return (arg);
                        }
                }
                break;

        default:
                cr_utils_trace_info ("Unknown term in nth style handler");
                arg.a = 0;
                arg.b = 0;
                return (arg);
        }

        if (arg.a != 0 && a_sel->content.pseudo->term->next) {
                /* check for b in 'an+b' */
                if (a_sel->content.pseudo->term->next->type == TERM_NUMBER &&
                    a_sel->content.pseudo->term->next->content.num ) {
                        arg.b = a_sel->content.pseudo->term->next->content.num->val;
                }
        }

        return (arg);
}

/**
 *Evaluate a selector (a simple selectors list) and says
 *if it matches the xml node given in parameter.
 *The algorithm used here is the following:
 *Walk the combinator separated list of simple selectors backward, starting
 *from the end of the list. For each simple selector, looks if
 *if matches the current node.
 *
 *@param a_this the selection engine.
 *@param a_sel the simple selection list.
 *@param a_node the xml node.
 *@param a_result out parameter. Set to true if the
 *selector matches the xml node, FALSE otherwise.
 *@param a_recurse if set to TRUE, the function will walk to
 *the next simple selector (after the evaluation of the current one) 
 *and recursively evaluate it. Must be usually set to TRUE unless you
 *know what you are doing.
 */
static enum CRStatus
sel_matches_node_real (CRSelEng * a_this, CRSimpleSel * a_sel,
                       CRXMLNodePtr a_node, gboolean * a_result,
                       gboolean a_eval_sel_list_from_end,
                       gboolean a_recurse)
{
        CRSimpleSel *cur_sel = NULL;
        CRXMLNodePtr cur_node = NULL;
        CRNodeIface const *node_iface = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_this && a_node
                              && a_result, CR_BAD_PARAM_ERROR);

        node_iface = PRIVATE(a_this)->node_iface;
        *a_result = FALSE;

        if (!node_iface->isElementNode(a_node))
                return CR_OK;

        if (a_eval_sel_list_from_end == TRUE) {
                /*go and get the last simple selector of the list */
                for (cur_sel = a_sel;
                     cur_sel && cur_sel->next; cur_sel = cur_sel->next) ;
        } else {
                cur_sel = a_sel;
        }

        for (cur_node = a_node; cur_sel; cur_sel = cur_sel->prev) {
                if (((cur_sel->type_mask & TYPE_SELECTOR)
                     && (cur_sel->name 
                         && cur_sel->name->stryng
                         && cur_sel->name->stryng->str)
                     && (!strcmp (cur_sel->name->stryng->str,
                                  (const char *) node_iface->getLocalName(cur_node))))
                    || (cur_sel->type_mask & UNIVERSAL_SELECTOR)) {
                        /*
                         *this simple selector
                         *matches the current xml node
                         *Let's see if the preceding
                         *simple selectors also match
                         *their xml node counterpart.
                         */
                        if (cur_sel->add_sel) {
                                if (additional_selector_matches_node (a_this, cur_sel->add_sel, 
                                                                      cur_node) == TRUE) {
                                        goto walk_a_step_in_expr;
                                } else {
                                        goto done;
                                }
                        } else {
                                goto walk_a_step_in_expr;
                        }                                
                } 
                if (!(cur_sel->type_mask & TYPE_SELECTOR)
                    && !(cur_sel->type_mask & UNIVERSAL_SELECTOR)) {
                        if (!cur_sel->add_sel) {
                                goto done;
                        }
                        if (additional_selector_matches_node
                            (a_this, cur_sel->add_sel, cur_node)
                            == TRUE) {
                                goto walk_a_step_in_expr;
                        } else {
                                goto done;
                        }
                } else {
                        goto done ;
                }

        walk_a_step_in_expr:
                if (a_recurse == FALSE) {
                        *a_result = TRUE;
                        goto done;
                }

                /*
                 *here, depending on the combinator of cur_sel
                 *choose the axis of the xml tree traversal
                 *and walk one step in the xml tree.
                 */
                if (!cur_sel->prev)
                        break;

                switch (cur_sel->combinator) {
                case NO_COMBINATOR:
                        break;

                case COMB_WS:  /*descendant selector */
                {
                        CRXMLNodePtr n = NULL;
                        enum CRStatus status = CR_OK;
                        gboolean matches = FALSE;

                        /*
                         *walk the xml tree upward looking for a parent
                         *node that matches the preceding selector.
                         */
                        for (n = node_iface->getParentNode (cur_node);
                             n;
                             n = node_iface->getParentNode (n)) {
                                status = sel_matches_node_real
                                        (a_this, cur_sel->prev,
                                         n, &matches, FALSE, TRUE);

                                if (status != CR_OK)
                                        goto done;

                                if (matches == TRUE) {
                                        cur_node = n ;
                                        break;
                                }
                        }

                        if (!n) {
                                /*
                                 *didn't find any ancestor that matches
                                 *the previous simple selector.
                                 */
                                goto done;
                        }
                        /*
                         *in this case, the preceding simple sel
                         *will have been interpreted twice, which
                         *is a cpu and mem waste ... I need to find
                         *another way to do this. Anyway, this is
                         *my first attempt to write this function and
                         *I am a bit clueless.
                         */
                        break;
                }

                case COMB_PLUS:
                        cur_node = get_prev_element_node (node_iface, cur_node);
                        if (!cur_node)
                                goto done;
                        break;

                case COMB_TILDE: /* General sibling selector. */
                {
                        CRXMLNodePtr n = NULL;
                        enum CRStatus status = CR_OK;
                        gboolean matches = FALSE;

                        /*
                         * Walk through previous sibing nodes looking for a
                         * node that matches the preceding selector.
                         */
                        for (n = get_prev_element_node (node_iface, cur_node);
                             n;
                             n = get_prev_element_node (node_iface, n)) {
                                status = sel_matches_node_real
                                        (a_this, cur_sel->prev,
                                         n, &matches, FALSE, TRUE);

                                if (status != CR_OK)
                                        goto done;

                                if (matches == TRUE) {
                                        cur_node = n ;
                                        break;
                                }
                        }

                        if (!n) {
                                /*
                                 * Didn't find any previous sibling that matches
                                 * the previous simple selector.
                                 */
                                goto done;
                        }
                        /*
                         * See note above in COMB_WS section.
                         */
                        break;
                }

                case COMB_GT:
                        cur_node = get_next_parent_element_node (node_iface, cur_node);
                        if (!cur_node)
                                goto done;
                        break;

                default:
                        goto done;
                }
                continue;
        }

        /*
         *if we reached this point, it means the selector matches
         *the xml node.
         */
        *a_result = TRUE;

 done:
        return CR_OK;
}


/**
 *Returns  array of the ruleset statements that matches the
 *given xml node.
 *The engine keeps in memory the last statement he
 *visited during the match. So, the next call
 *to this function will eventually return a rulesets list starting
 *from the last ruleset statement visited during the previous call.
 *The enable users to get matching rulesets in an incremental way.
 *Note that for each statement returned, 
 *the engine calculates the specificity of the selector
 *that matched the xml node and stores it in the "specifity" field
 *of the statement structure.
 *
 *@param a_sel_eng the current selection engine
 *@param a_node the xml node for which the request
 *is being made.
 *@param a_sel_list the list of selectors to perform the search in.
 *@param a_rulesets in/out parameter. A pointer to the
 *returned array of rulesets statements that match the xml node
 *given in parameter. The caller allocates the array before calling this
 *function.
 *@param a_len in/out parameter the length (in sizeof (#CRStatement*)) 
 *of the returned array.
 *(the length of a_rulesets, more precisely).
 *The caller must set it to the length of a_ruleset prior to calling this
 *function. In return, the function sets it to the length 
 *(in sizeof (#CRStatement)) of the actually returned CRStatement array.
 *@return CR_OUTPUT_TOO_SHORT_ERROR if found more rulesets than the size
 *of the a_rulesets array. In this case, the first *a_len rulesets found
 *are put in a_rulesets, and a further call will return the following
 *ruleset(s) following the same principle.
 *@return CR_OK if all the rulesets found have been returned. In this
 *case, *a_len is set to the actual number of ruleset found.
 *@return CR_BAD_PARAM_ERROR in case any of the given parameter are
 *bad (e.g null pointer).
 *@return CR_ERROR if any other error occurred.
 */
static enum CRStatus
cr_sel_eng_get_matched_rulesets_real (CRSelEng * a_this,
                                      CRStyleSheet * a_stylesheet,
                                      CRXMLNodePtr a_node,
                                      CRStatement *** a_rulesets,
                                      gulong * a_len,
                                      gulong * a_capacity)
{
        CRStatement *cur_stmt = NULL;
        CRSelector *sel_list = NULL,
                *cur_sel = NULL;
        gboolean matches = FALSE;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_this
                              && a_stylesheet
                              && a_node && a_rulesets, CR_BAD_PARAM_ERROR);

        if (!a_stylesheet->statements) {
                return CR_OK;
        }

        /*
         *if this stylesheet is "new one"
         *let's remember it for subsequent calls.
         */
        if (PRIVATE (a_this)->sheet != a_stylesheet) {
                PRIVATE (a_this)->sheet = a_stylesheet;
                PRIVATE (a_this)->cur_stmt = a_stylesheet->statements;
        }

        /*
         *walk through the list of statements and,
         *get the selectors list inside the statements that
         *contain some, and try to match our xml node in these
         *selectors lists.
         */
        for (cur_stmt = PRIVATE (a_this)->cur_stmt;
             (PRIVATE (a_this)->cur_stmt = cur_stmt);
             cur_stmt = cur_stmt->next) {
                /*
                 *initialize the selector list in which we will
                 *really perform the search.
                 */
                sel_list = NULL;

                /*
                 *get the damn selector list in 
                 *which we have to look
                 */
                switch (cur_stmt->type) {
                case RULESET_STMT:
                        if (cur_stmt->kind.ruleset
                            && cur_stmt->kind.ruleset->sel_list) {
                                sel_list = cur_stmt->kind.ruleset->sel_list;
                        }
                        break;

                case AT_MEDIA_RULE_STMT:
                        if (cur_stmt->kind.media_rule
                            && cur_stmt->kind.media_rule->rulesets
                            && cur_stmt->kind.media_rule->rulesets->
                            kind.ruleset
                            && cur_stmt->kind.media_rule->rulesets->
                            kind.ruleset->sel_list) {
                                sel_list =
                                        cur_stmt->kind.media_rule->
                                        rulesets->kind.ruleset->sel_list;
                        }
                        break;

                case AT_IMPORT_RULE_STMT:
                        if (cur_stmt->kind.import_rule) {
                                g_assert(!cur_stmt->kind.import_rule->sheet ||
                                         !cur_stmt->kind.import_rule->sheet->next);
                                cr_sel_eng_get_matched_rulesets_real (
                                        a_this, cur_stmt->kind.import_rule->sheet,
                                        a_node, a_rulesets,
                                        a_len, a_capacity);
                        }
                        break;
                default:
                        break;
                }

                if (!sel_list)
                        continue;

                /*
                 *now, we have a comma separated selector list to look in.
                 *let's walk it and try to match the xml_node
                 *on each item of the list.
                 */
                for (cur_sel = sel_list; cur_sel; cur_sel = cur_sel->next) {
                        if (!cur_sel->simple_sel)
                                continue;

                        status = cr_sel_eng_matches_node
                                (a_this, cur_sel->simple_sel,
                                 a_node, &matches);

                        if (status == CR_OK && matches == TRUE) {
                                /*
                                 *bingo!!! we found one ruleset that
                                 *matches that fucking node.
                                 *lets put it in the out array.
                                 */

                                if (*a_len >= *a_capacity) {
                                        *a_capacity = (*a_len) + 8;
                                        *a_rulesets = (CRStatement **) g_try_realloc (*a_rulesets,
                                                        (*a_capacity) * sizeof (CRStatement *));
                                        if (!*a_rulesets) {
                                                cr_utils_trace_info("Out of memory");
                                                return CR_ERROR;
                                        }
                                }

                                {
                                        (*a_rulesets)[(*a_len)++] = cur_stmt;

                                        /*
                                         *For the cascade computing algorithm
                                         *(which is gonna take place later)
                                         *we must compute the specificity
                                         *(css2 spec chap 6.4.1) of the selector
                                         *that matched the current xml node
                                         *and store it in the css2 statement
                                         *(statement == ruleset here).
                                         */
                                        status = cr_simple_sel_compute_specificity (cur_sel->simple_sel);

                                        g_return_val_if_fail (status == CR_OK,
                                                              CR_ERROR);
                                        cur_stmt->specificity =
                                                cur_sel->simple_sel->
                                                specificity;
                                }
                        }
                }
        }

        /*
         *if we reached this point, it means
         *we reached the end of stylesheet.
         *no need to store any info about the stylesheet
         *anymore.
         */
        g_return_val_if_fail (!PRIVATE (a_this)->cur_stmt, CR_ERROR);
        PRIVATE (a_this)->sheet = NULL;
        return CR_OK;
}

static enum CRStatus
put_css_properties_in_props_list (CRPropList ** a_props, CRStatement * a_stmt)
{
        CRPropList *props = NULL,
                *pair = NULL,
                *tmp_props = NULL;
        CRDeclaration *cur_decl = NULL;

        g_return_val_if_fail (a_props && a_stmt
                              && a_stmt->type == RULESET_STMT
                              && a_stmt->kind.ruleset, CR_BAD_PARAM_ERROR);

        props = *a_props;

        for (cur_decl = a_stmt->kind.ruleset->decl_list;
             cur_decl; cur_decl = cur_decl->next) {
                CRDeclaration *decl;

                decl = NULL;
                pair = NULL;

                if (!cur_decl->property 
                    || !cur_decl->property->stryng
                    || !cur_decl->property->stryng->str)
                        continue;
                /*
                 *First, test if the property is not
                 *already present in our properties list
                 *If yes, apply the cascading rules to
                 *compute the precedence. If not, insert
                 *the property into the list
                 */
                cr_prop_list_lookup_prop (props,
                                          cur_decl->property, 
                                          &pair);

                if (!pair) {
                        tmp_props = cr_prop_list_append2
                                (props, cur_decl->property, cur_decl);
                        if (tmp_props) {
                                props = tmp_props;
                                tmp_props = NULL;
                        }
                        continue;
                }

                /*
                 *A property with the same name already exists.
                 *We must apply here 
                 *some cascading rules
                 *to compute the precedence.
                 */
                cr_prop_list_get_decl (pair, &decl);
                g_return_val_if_fail (decl, CR_ERROR);

                /*
                 *first, look at the origin.
                 *6.4.1 says: 
                 *"for normal declarations, 
                 *author style sheets override user 
                 *style sheets which override 
                 *the default style sheet."
                 */
                if (decl->parent_statement
                    && decl->parent_statement->parent_sheet
                    && (decl->parent_statement->parent_sheet->origin
                        < a_stmt->parent_sheet->origin)) {
                        /*
                         *if the already selected declaration
                         *is marked as being !important the current
                         *declaration must not override it 
                         *(unless the already selected declaration 
                         *has an UA origin)
                         */
                        if (decl->important == TRUE && cur_decl->important != TRUE
                            && decl->parent_statement->parent_sheet->origin
                            != ORIGIN_UA) {
                                continue;
                        }
                        tmp_props = cr_prop_list_unlink (props, pair);
                        if (props) {
                                cr_prop_list_destroy (pair);
                        }
                        props = tmp_props;
                        tmp_props = NULL;
                        props = cr_prop_list_append2
                                (props, cur_decl->property, cur_decl);

                        continue;
                } else if (decl->parent_statement
                           && decl->parent_statement->parent_sheet
                           && (decl->parent_statement->
                               parent_sheet->origin
                               > a_stmt->parent_sheet->origin)) {
                        cr_utils_trace_info
                                ("We should not reach this line\n");
                        continue;
                }

                /*
                 *A property with the same
                 *name and the same origin already exists.
                 *shit. This is lasting longer than expected ...
                 *Luckily, the spec says in 6.4.1:
                 *"more specific selectors will override 
                 *more general ones"
                 *and
                 *"if two rules have the same weight, 
                 *origin and specificity, 
                 *the later specified wins"
                 */
                if (a_stmt->specificity
                    >= decl->parent_statement->specificity) {
                        if (decl->important == TRUE && cur_decl->important != TRUE)
                                continue;
                        props = cr_prop_list_unlink (props, pair);
                        if (pair) {
                                cr_prop_list_destroy (pair);
                                pair = NULL;
                        }
                        props = cr_prop_list_append2 (props,
                                                      cur_decl->property,
                                                      cur_decl);
                }
        }
        /*TODO: this may leak. Check this out */
        *a_props = props;

        return CR_OK;
}

static void
set_style_from_props (CRStyle * a_style, CRPropList * a_props)
{
        CRPropList *cur = NULL;
        CRDeclaration *decl = NULL;

        for (cur = a_props; cur; cur = cr_prop_list_get_next (cur)) {
                cr_prop_list_get_decl (cur, &decl);
                cr_style_set_style_from_decl (a_style, decl);
                decl = NULL;
        }
}

/****************************************
 *PUBLIC METHODS
 ****************************************/

/**
 * cr_sel_eng_new:
 *Creates a new instance of #CRSelEng.
 *
 *@a_node_iface: Node interface
 *
 *Returns the newly built instance of #CRSelEng of
 *NULL if an error occurs.
 */
CRSelEng *
cr_sel_eng_new (CRNodeIface const * a_node_iface)
{
        CRSelEng *result = NULL;

        result = (CRSelEng *) g_try_malloc (sizeof (CRSelEng));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRSelEng));

        PRIVATE (result) = (CRSelEngPriv *) g_try_malloc (sizeof (CRSelEngPriv));
        if (!PRIVATE (result)) {
                cr_utils_trace_info ("Out of memory");
                g_free (result);
                return NULL;
        }
        memset (PRIVATE (result), 0, sizeof (CRSelEngPriv));
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "root",
                 IDENT_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 root_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "empty",
                 IDENT_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 empty_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "lang",
                 FUNCTION_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 lang_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "only-child",
                 IDENT_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 only_child_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "only-of-type",
                 IDENT_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 only_of_type_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "first-child",
                 IDENT_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 first_child_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "first-of-type",
                 IDENT_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 first_of_type_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "last-child",
                 IDENT_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 last_child_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "last-of-type",
                 IDENT_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 last_of_type_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "nth-child",
                 FUNCTION_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 nth_child_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "nth-of-type",
                 FUNCTION_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 nth_of_type_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "nth-last-child",
                 FUNCTION_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 nth_last_child_pseudo_class_handler);
        cr_sel_eng_register_pseudo_class_sel_handler
                (result, (guchar *) "nth-last-of-type",
                 FUNCTION_PSEUDO, /*(CRPseudoClassSelectorHandler)*/
                 nth_last_of_type_pseudo_class_handler);

        cr_sel_eng_set_node_iface (result, a_node_iface);

        return result;
}

/**
 * cr_sel_eng_register_pseudo_class_sel_handler:
 *@a_this: the current instance of #CRSelEng
 *@a_pseudo_class_sel_name: the name of the pseudo class selector.
 *@a_pseudo_class_type: the type of the pseudo class selector.
 *@a_handler: the actual handler or callback to be called during
 *the selector evaluation process.
 *
 *Adds a new handler entry in the handlers entry table.
 *
 *Returns CR_OK, upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_sel_eng_register_pseudo_class_sel_handler (CRSelEng * a_this,
                                              guchar * a_name,
                                              enum CRPseudoType a_type,
                                              CRPseudoClassSelectorHandler
                                              a_handler)
{
        struct CRPseudoClassSelHandlerEntry *handler_entry = NULL;
        GList *list = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_handler && a_name, CR_BAD_PARAM_ERROR);

        handler_entry = (struct CRPseudoClassSelHandlerEntry *) g_try_malloc
                (sizeof (struct CRPseudoClassSelHandlerEntry));
        if (!handler_entry) {
                return CR_OUT_OF_MEMORY_ERROR;
        }
        memset (handler_entry, 0,
                sizeof (struct CRPseudoClassSelHandlerEntry));
        handler_entry->name = (guchar *) g_strdup ((const gchar *) a_name);
        handler_entry->type = a_type;
        handler_entry->handler = a_handler;
        list = g_list_append (PRIVATE (a_this)->pcs_handlers, handler_entry);
        if (!list) {
                return CR_OUT_OF_MEMORY_ERROR;
        }
        PRIVATE (a_this)->pcs_handlers = list;
        return CR_OK;
}

enum CRStatus
cr_sel_eng_unregister_pseudo_class_sel_handler (CRSelEng * a_this,
                                                guchar * a_name,
                                                enum CRPseudoType a_type)
{
        GList *elem = NULL,
                *deleted_elem = NULL;
        gboolean found = FALSE;
        struct CRPseudoClassSelHandlerEntry *entry = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        for (elem = PRIVATE (a_this)->pcs_handlers;
             elem; elem = g_list_next (elem)) {
                entry = (struct CRPseudoClassSelHandlerEntry *) elem->data;
                if (!strcmp ((const char *) entry->name, (const char *) a_name)
                    && entry->type == a_type) {
                        found = TRUE;
                        break;
                }
        }
        if (found == FALSE)
                return CR_PSEUDO_CLASS_SEL_HANDLER_NOT_FOUND_ERROR;
        PRIVATE (a_this)->pcs_handlers = g_list_delete_link
                (PRIVATE (a_this)->pcs_handlers, elem);
        entry = (struct CRPseudoClassSelHandlerEntry *) elem->data;
        if (entry->name) {
                g_free (entry->name);
                entry->name = NULL;
        }
        g_free (elem);
        g_list_free (deleted_elem);

        return CR_OK;
}

/**
 * cr_sel_eng_unregister_all_pseudo_class_sel_handlers:
 *@a_this: the current instance of #CRSelEng .
 *
 *Unregisters all the pseudo class sel handlers
 *and frees all the associated allocated datastructures.
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_sel_eng_unregister_all_pseudo_class_sel_handlers (CRSelEng * a_this)
{
        GList *elem = NULL;
        struct CRPseudoClassSelHandlerEntry *entry = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        if (!PRIVATE (a_this)->pcs_handlers)
                return CR_OK;
        for (elem = PRIVATE (a_this)->pcs_handlers;
             elem; elem = g_list_next (elem)) {
                entry = (struct CRPseudoClassSelHandlerEntry *) elem->data;
                if (!entry)
                        continue;
                if (entry->name) {
                        g_free (entry->name);
                        entry->name = NULL;
                }
                g_free (entry);
                elem->data = NULL;
        }
        g_list_free (PRIVATE (a_this)->pcs_handlers);
        PRIVATE (a_this)->pcs_handlers = NULL;
        return CR_OK;
}

enum CRStatus
cr_sel_eng_get_pseudo_class_selector_handler (CRSelEng * a_this,
                                              guchar * a_name,
                                              enum CRPseudoType a_type,
                                              CRPseudoClassSelectorHandler *
                                              a_handler)
{
        GList *elem = NULL;
        struct CRPseudoClassSelHandlerEntry *entry = NULL;
        gboolean found = FALSE;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_name, CR_BAD_PARAM_ERROR);

        for (elem = PRIVATE (a_this)->pcs_handlers;
             elem; elem = g_list_next (elem)) {
                entry = (struct CRPseudoClassSelHandlerEntry *) elem->data;
                if (!strcmp ((const char *) a_name, (const char *) entry->name)
                    && entry->type == a_type) {
                        found = TRUE;
                        break;
                }
        }

        if (found == FALSE)
                return CR_PSEUDO_CLASS_SEL_HANDLER_NOT_FOUND_ERROR;
        *a_handler = entry->handler;
        return CR_OK;
}

/**
 * cr_sel_eng_matches_node:
 *@a_this: the selection engine.
 *@a_sel: the simple selector against which the xml node 
 *is going to be matched.
 *@a_node: the node against which the selector is going to be matched.
 *@a_result: out parameter. The result of the match. Is set to
 *TRUE if the selector matches the node, FALSE otherwise. This value
 *is considered if and only if this functions returns CR_OK.
 *
 *Evaluates a chained list of simple selectors (known as a css2 selector).
 *Says whether if this selector matches the xml node given in parameter or
 *not.
 *
 *Returns the CR_OK if the selection ran correctly, an error code otherwise.
 */
enum CRStatus
cr_sel_eng_matches_node (CRSelEng * a_this, CRSimpleSel * a_sel,
                         CRXMLNodePtr a_node, gboolean * a_result)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_this && a_node
                              && a_result, CR_BAD_PARAM_ERROR);

        if (!PRIVATE(a_this)->node_iface->isElementNode (a_node)) {
                *a_result = FALSE;
                return CR_OK;
        }

        return sel_matches_node_real (a_this, a_sel, 
                                      a_node, a_result, 
                                      TRUE, TRUE);
}

/**
 * cr_sel_eng_get_matched_rulesets:
 *@a_this: the current instance of the selection engine.
 *@a_sheet: the stylesheet that holds the selectors.
 *@a_node: the xml node to consider during the walk through
 *the stylesheet.
 *@a_rulesets: out parameter. A pointer to an array of
 *rulesets statement pointers. *a_rulesets is allocated by
 *this function and must be freed by the caller. However, the caller
 *must not alter the rulesets statements pointer because they
 *point to statements that are still in the css stylesheet.
 *@a_len: the length of *a_ruleset.
 *
 *Returns an array of pointers to selectors that matches
 *the xml node given in parameter.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_sel_eng_get_matched_rulesets (CRSelEng * a_this,
                                 CRStyleSheet * a_sheet,
                                 CRXMLNodePtr a_node,
                                 CRStatement *** a_rulesets, gulong * a_len)
{
        CRStatement **stmts_tab = NULL;
        enum CRStatus status = CR_OK;
        gulong capacity = 0;

        g_return_val_if_fail (a_this
                              && a_sheet
                              && a_node
                              && a_rulesets && *a_rulesets == NULL
                              && a_len, CR_BAD_PARAM_ERROR);

        *a_len = 0;

        status = cr_sel_eng_get_matched_rulesets_real
                (a_this, a_sheet, a_node, &stmts_tab, a_len, &capacity);
        if (status == CR_ERROR)
                goto error;

        *a_rulesets = stmts_tab;

        return CR_OK;

      error:

        if (stmts_tab) {
                g_free (stmts_tab);
                stmts_tab = NULL;

        }

        *a_len = 0;
        return status;
}

/**
 * Like cr_sel_eng_get_matched_rulesets_real, but process an entire (linked)
 * list of stylesheets, not only a single one.
 */
static
enum CRStatus
cr_sel_eng_process_stylesheet ( CRSelEng * a_eng,
                                CRXMLNodePtr a_node,
                                CRStyleSheet * a_stylesheet,
                                CRStatement *** stmts_tab,
                                gulong * tab_size,
                                gulong * tab_len,
                                gulong * index)
{
        enum CRStatus status = CR_OK;
        CRStyleSheet *cur = NULL;

        for (cur = a_stylesheet; cur && status == CR_OK; cur = cur->next) {
                status = cr_sel_eng_get_matched_rulesets_real
                        (a_eng, cur, a_node, stmts_tab, index, tab_size);
        }

        return status;
}

enum CRStatus
cr_sel_eng_get_matched_properties_from_cascade (CRSelEng * a_this,
                                                CRCascade * a_cascade,
                                                CRXMLNodePtr a_node,
                                                CRPropList ** a_props)
{
        CRStatement **stmts_tab = NULL;
        enum CRStatus status = CR_OK;
        gulong tab_size = 0,
                tab_len = 0,
                i = 0,
                index = 0;
        enum CRStyleOrigin origin;
        CRStyleSheet *sheet = NULL;

        g_return_val_if_fail (a_this
                              && a_cascade
                              && a_node && a_props, CR_BAD_PARAM_ERROR);

        for (origin = ORIGIN_UA; origin < NB_ORIGINS; origin = (enum CRStyleOrigin) (origin + 1)) {
                sheet = cr_cascade_get_sheet (a_cascade, origin);
                if (!sheet)
                        continue;

                status = cr_sel_eng_process_stylesheet (a_this, a_node, sheet, &stmts_tab, &tab_size, &tab_len, &index);
                if (status != CR_OK) {
                        cr_utils_trace_info ("Error while running "
                                             "selector engine");
                        return status;
                }
        }

        /*
         *TODO, walk down the stmts_tab and build the
         *property_name/declaration hashtable.
         *Make sure one can walk from the declaration to
         *the stylesheet.
         */
        for (i = 0; i < index; i++) {
                CRStatement *stmt = stmts_tab[i];
                if (!stmt)
                        continue;
                switch (stmt->type) {
                case RULESET_STMT:
                        if (!stmt->parent_sheet)
                                continue;
                        status = put_css_properties_in_props_list
                                (a_props, stmt);
                        break;
                default:
                        break;
                }

        }
        status = CR_OK ;
        if (stmts_tab) {
                g_free (stmts_tab);
                stmts_tab = NULL;
        }

        return status;
}

enum CRStatus
cr_sel_eng_get_matched_style (CRSelEng * a_this,
                              CRCascade * a_cascade,
                              CRXMLNodePtr a_node,
                              CRStyle * a_parent_style, 
                              CRStyle ** a_style,
                              gboolean a_set_props_to_initial_values)
{
        enum CRStatus status = CR_OK;

        CRPropList *props = NULL;

        g_return_val_if_fail (a_this && a_cascade
                              && a_node && a_style, CR_BAD_PARAM_ERROR);

        status = cr_sel_eng_get_matched_properties_from_cascade
                (a_this, a_cascade, a_node, &props);

        g_return_val_if_fail (status == CR_OK, status);
        if (props) {
                if (!*a_style) {
                        *a_style = cr_style_new (a_set_props_to_initial_values) ;
                        g_return_val_if_fail (*a_style, CR_ERROR);
                } else {
                        if (a_set_props_to_initial_values == TRUE) {
                                cr_style_set_props_to_initial_values (*a_style) ;
                        } else {
                                cr_style_set_props_to_default_values (*a_style);
                        }
                }
                (*a_style)->parent_style = a_parent_style;

                set_style_from_props (*a_style, props);
                if (props) {
                        cr_prop_list_destroy (props);
                        props = NULL;
                }
        }
        return CR_OK;
}

/**
 * cr_sel_eng_destroy:
 *@a_this: the current instance of the selection engine.
 *
 *The destructor of #CRSelEng
 */
void
cr_sel_eng_destroy (CRSelEng * a_this)
{
        g_return_if_fail (a_this);

        if (!PRIVATE (a_this))
                goto end ;
        if (PRIVATE (a_this)->pcs_handlers) {
                cr_sel_eng_unregister_all_pseudo_class_sel_handlers
                        (a_this) ;
                PRIVATE (a_this)->pcs_handlers = NULL ;
        }
        g_free (PRIVATE (a_this));
        PRIVATE (a_this) = NULL;
 end:
        if (a_this) {
                g_free (a_this);
        }
}
