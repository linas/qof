/********************************************************************\
 * qofquery-deserial.c -- Convert Qof-Query XML to QofQuery         *
 * Copyright (C) 2001,2002,2004 Linas Vepstas <linas@linas.org>     *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

#ifndef QOF_QUERY_DESERIAL_H
#define QOF_QUERY_DESERIAL_H

#include <qof/qofquery.h>
#include <libxml/tree.h>

/** Given an XML tree, reconstruct and return the 
 *  equivalent query. */
QofQuery *qof_query_from_xml (xmlNodePtr);

#endif /* QOF_QUERY_DESERIAL_H */


// #include "config.h"

#include <glib.h>
#include <libxml/parser.h>

#include "qofquery-serialize.h"
#include "qofquery-p.h"
#include "qofquerycore-p.h"
#include "gnc-engine-util.h"

#define CACHE_INSERT(str)  \
    g_cache_insert(gnc_engine_get_string_cache(), (gpointer)(str))
#define CACHE_REMOVE(str)  \
    g_cache_remove(gnc_engine_get_string_cache(), (gpointer)(str))

/* =========================================================== */

#define GET_TEXT(node)  ({                                   \
   char * sstr = NULL;                                       \
   xmlNodePtr text;                                          \
   text = node->xmlChildrenNode;                             \
   if (text && 0 == strcmp ("text", text->name)) {           \
      sstr = text->content;                                  \
   }                                                         \
   sstr;                                                     \
})

#define GET_STR(SELF,FN,TOK)                                 \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      FN (SELF, str);                                        \
   }                                                         \
   else

#define GET_DBL(SELF,FN,TOK)                                 \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      double rate = atof (str);                              \
      FN (SELF, rate);                                       \
   }                                                         \
   else

#define GET_INT32(SELF,FN,TOK)                               \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      gint32 ival = atoi (str);                              \
      FN (SELF, ival);                                       \
   }                                                         \
   else

#define GET_TIM(SELF,FN,TOK)                                 \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      time_t tval = atol (str);                              \
      FN (SELF, tval);                                       \
   }                                                         \
   else

#define GET_BOOL(VAL,TOK)                                    \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      gboolean bval = atol (str);                            \
      VAL = bval;                                            \
   }                                                         \
   else

#define GET_GUID(SELF,FN,TOK)                                \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      GUID guid;                                             \
      string_to_guid (str, &guid);                           \
      FN (SELF, &guid);                                      \
   }                                                         \
   else

#define GET_ENUM_3(SELF,FN,TOK,A,B,C)                        \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      int ival = GTT_##A;                                    \
      if (!strcmp (#A, str)) ival = GTT_##A;                 \
      else if (!strcmp (#B, str)) ival = GTT_##B;            \
      else if (!strcmp (#C, str)) ival = GTT_##C;            \
      else gtt_err_set_code (GTT_UNKNOWN_VALUE);             \
      FN (SELF, ival);                                       \
   }                                                         \
   else

#define GET_ENUM_4(SELF,FN,TOK,A,B,C,D)                      \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      int ival = GTT_##A;                                    \
      if (!strcmp (#A, str)) ival = GTT_##A;                 \
      else if (!strcmp (#B, str)) ival = GTT_##B;            \
      else if (!strcmp (#C, str)) ival = GTT_##C;            \
      else if (!strcmp (#D, str)) ival = GTT_##D;            \
      else gtt_err_set_code (GTT_UNKNOWN_VALUE);             \
      FN (SELF, ival);                                       \
   }                                                         \
   else

#define GET_HOW(VAL,TOK,A,B,C,D,E,F)                         \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      int ival = QOF_COMPARE_##A;                            \
      if (!strcmp (#A, str)) ival = QOF_COMPARE_##A;         \
      else if (!strcmp (#B, str)) ival = QOF_COMPARE_##B;    \
      else if (!strcmp (#C, str)) ival = QOF_COMPARE_##C;    \
      else if (!strcmp (#D, str)) ival = QOF_COMPARE_##D;    \
      else if (!strcmp (#E, str)) ival = QOF_COMPARE_##E;    \
      else if (!strcmp (#F, str)) ival = QOF_COMPARE_##F;    \
      VAL = ival;                                            \
   }                                                         \
   else

#define GET_STRING_MATCH(VAL,TOK,A,B)                        \
   if (0 == strcmp (TOK, node->name))                        \
   {                                                         \
      const char *str = GET_TEXT (node);                     \
      int ival = QOF_STRING_MATCH_##A;                       \
      if (!strcmp (#A, str)) ival = QOF_STRING_MATCH_##A;    \
      else if (!strcmp (#B, str)) ival = QOF_STRING_MATCH_##B;\
      VAL = ival;                                            \
   }                                                         \
   else

/* =============================================================== */

static QofQueryPredData *
qof_query_pred_int32_from_xml (xmlNodePtr root)
{
	xmlNodePtr xp = root->xmlChildrenNode;
	xmlNodePtr node;

	QofQueryCompare how = QOF_COMPARE_EQUAL;
	gint32 val = 0;

	for (node=xp; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) continue;

		GET_HOW (how, "qofquery:compare", LT, LTE, EQUAL, GT, GTE, NEQ);
		GET_INT32 (0, val=, "qofquery:int32");
		{}
	}

	QofQueryPredData *pred;
	pred = qof_query_int32_predicate (how, val);
	return pred;
}

/* =============================================================== */

static QofQueryPredData *
qof_query_pred_string_from_xml (xmlNodePtr root)
{
	xmlNodePtr xp = root->xmlChildrenNode;
	xmlNodePtr node;

	QofQueryCompare how = QOF_COMPARE_EQUAL;
	QofStringMatch sm = QOF_STRING_MATCH_CASEINSENSITIVE;
	gboolean is_regex = FALSE;
	const char *pstr = NULL;

	for (node=xp; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) continue;

		GET_HOW (how, "qofquery:compare", LT, LTE, EQUAL, GT, GTE, NEQ);
		GET_BOOL (is_regex, "qofquery:is-regex");
		GET_STR (0, pstr=, "qofquery:string");
		GET_STRING_MATCH (sm, "qofquery:string-match", NORMAL, CASEINSENSITIVE);
		{}
	}

	QofQueryPredData *pred;
	pred = qof_query_string_predicate (how, pstr, sm , is_regex);
	return pred;
}

/* =============================================================== */

static GSList * 
qof_query_param_path_from_xml (xmlNodePtr root)
{
	xmlNodePtr pterms = root->xmlChildrenNode;
	GSList *plist = NULL;
	xmlNodePtr node;
	for (node=pterms; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) continue;

		if (0 == strcmp (node->name, "qofquery:param"))
		{
			const char *str = GET_TEXT (node);
			plist = g_slist_append (plist, CACHE_INSERT(str));
		}
	}
	return plist;
}

/* =============================================================== */

static void 
qof_query_term_from_xml (QofQuery *q, xmlNodePtr root)
{
	xmlNodePtr node;
	xmlNodePtr term = root->xmlChildrenNode;
	QofQueryPredData *pred = NULL;
	GSList *path = NULL;

	for (node=term; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) continue;
		if (0 == strcmp (node->name, "qofquery:invert"))
		{
			QofQuery *qt = qof_query_create();
printf ("duude invert\n");
			qof_query_term_from_xml (qt, node);
			QofQuery *qinv = qof_query_invert (qt);
printf ("duuuuuu postinvert -- uuuuuuuuuuuuuuuuuuuuuuuuuuuuude\n");
qof_query_print (qinv);
			qof_query_merge_in_place (q, qinv, QOF_QUERY_AND);
printf ("duuuuuu post  nerge -- uuuuuuuuuuuuuuuuuuuuuude\n");
qof_query_print (q);
printf ("---------------------------------------------------\n");
			qof_query_destroy (qinv);
			qof_query_destroy (qt);
			continue;
		}
		else
		if (0 == strcmp (node->name, "qofquery:param-path"))
		{
			path = qof_query_param_path_from_xml (node);
		}
		else
		if (0 == strcmp (node->name, "qofquery:pred-string"))
		{
			pred = qof_query_pred_string_from_xml (node);
		}
		else
		if (0 == strcmp (node->name, "qofquery:pred-int32"))
		{
			pred = qof_query_pred_int32_from_xml (node);
		}
		else
		{
			// warning unhandled predicate type
		}
	}

	/* At this level, the terms should always be anded */
	qof_query_add_term (q, path, pred, QOF_QUERY_AND);
}

/* =============================================================== */

static void 
qof_query_and_terms_from_xml (QofQuery *q, xmlNodePtr root)
{
	xmlNodePtr andterms = root->xmlChildrenNode;
	xmlNodePtr node;
	for (node=andterms; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) continue;

		if (0 == strcmp (node->name, "qofquery:term"))
		{
			qof_query_term_from_xml (q, node);
		}
	}
}

/* =============================================================== */

static void 
qof_query_or_terms_from_xml (QofQuery *q, xmlNodePtr root)
{
	xmlNodePtr andterms = root->xmlChildrenNode;
	xmlNodePtr node;

	for (node=andterms; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) continue;

		if (0 == strcmp (node->name, "qofquery:and-terms"))
		{
			QofQuery *qand = qof_query_create ();
			qof_query_and_terms_from_xml (qand, node);
			qof_query_merge_in_place (q, qand, QOF_QUERY_OR);
			qof_query_destroy (qand);
		}
	}
}

/* =============================================================== */

QofQuery *
qof_query_from_xml (xmlNodePtr root)
{
	QofQuery *q;

	if (!root) return NULL;

	xmlChar * version = xmlGetProp(root, "version");
   if (!root->name || strcmp ("qof:qofquery", root->name))
   {
		// XXX something is wrong. warn ... 
      return NULL;
   }

	q = qof_query_create ();

	xmlNodePtr qpart = root->xmlChildrenNode;
	xmlNodePtr node;
	for (node=qpart; node; node = node->next)
	{
		if (node->type != XML_ELEMENT_NODE) continue;

		GET_STR   (q, qof_query_search_for,      "qofquery:search-for");
		GET_INT32 (q, qof_query_set_max_results, "qofquery:max-results");
		if (0 == strcmp (node->name, "qofquery:or-terms"))
		{
			qof_query_or_terms_from_xml (q, node);
		}
		else 
		if (0 == strcmp (node->name, "qofquery:sort-list"))
		{
// XXX unfinished
		}
		else 
		{
			// XXX unknown node type 
		}
	}

	return q;
}

/* =============================================================== */

#define UNIT_TEST
#ifdef UNIT_TEST

#include <stdio.h>
#include <qof/qofsql.h>

int main (int argc, char * argv[])
{
	QofQuery *q, *qnew;
	QofSqlQuery *sq;

	qof_query_init();
	qof_object_initialize ();

	static QofParam params[] = {
		{ "adate", QOF_TYPE_DATE, NULL, NULL},
		{ "aint", QOF_TYPE_INT32, NULL, NULL},
		{ "astr", QOF_TYPE_STRING, NULL, NULL},
		{ NULL },
   };

	qof_class_register ("GncABC", NULL, params);
	sq = qof_sql_query_new();

	qof_sql_query_parse (sq, 
	    "SELECT * from GncABC WHERE aint = 123 or not astr=\'asdf\';");
	// qof_sql_query_parse (sq, "SELECT * from GncABC;");
	q = qof_sql_query_get_query (sq);

	qof_query_print (q);

   xmlNodePtr topnode = qof_query_to_xml (q);

	qnew = qof_query_from_xml (topnode);
	qof_query_print (qnew);

	gboolean eq = qof_query_equal (q, qnew);
	printf ("Are the two equal? answer=%d\n", eq);

// #define DOPRINT 1
#ifdef DOPRINT
   xmlDocPtr doc = doc = xmlNewDoc("1.0");
	xmlDocSetRootElement(doc,topnode);

	xmlChar *xbuf;
	int bufsz;
	xmlDocDumpFormatMemory (doc, &xbuf, &bufsz, 1);

	printf ("%s\n", xbuf);
	xmlFree (xbuf);
	xmlFreeDoc(doc);
#endif

	return 0;
}

#endif /* UNIT_TEST */

/* ======================== END OF FILE =================== */
