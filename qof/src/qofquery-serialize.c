
/* 
 * Take a query, serialize it into XML

 * Copyright (C) 2001,2002,2004 Linas Vepstas <linas@linas.org>
 */
#include <stdio.h>

#include <qof/qofquery.h>
#include <qof/qofsql.h>

#include <libxml/tree.h>

#include "qofquery-p.h"

/* ======================================================= */

#define PUT_STR(TOK,VAL) {                           \
   xmlNodePtr node;                                  \
   const char * str = (VAL);                         \
   if (str && 0 != str[0])                           \
   {                                                 \
      node = xmlNewNode (NULL, TOK);                 \
      xmlNodeAddContent(node, str);                  \
      xmlAddChild (topnode, node);                   \
   }                                                 \
}

#define PUT_INT(TOK,VAL) {                           \
   xmlNodePtr node;                                  \
   char buff[80];                                    \
   g_snprintf (buff, sizeof(buff), "%d", (VAL));     \
   node = xmlNewNode (NULL, TOK);                    \
   xmlNodeAddContent(node, buff);                    \
   xmlAddChild (topnode, node);                      \
}

#define PUT_LONG(TOK,VAL) {                          \
   xmlNodePtr node;                                  \
   char buff[80];                                    \
   g_snprintf (buff, sizeof(buff), "%ld", (VAL));    \
   node = xmlNewNode (NULL, TOK);                    \
   xmlNodeAddContent(node, buff);                    \
   xmlAddChild (topnode, node);                      \
}


#define PUT_DBL(TOK,VAL) {                           \
   xmlNodePtr node;                                  \
   char buff[80];                                    \
   g_snprintf (buff, sizeof(buff), "%.18g", (VAL));  \
   node = xmlNewNode (NULL, TOK);                    \
   xmlNodeAddContent(node, buff);                    \
   xmlAddChild (topnode, node);                      \
}

#define PUT_GUID(TOK,VAL) {                          \
   xmlNodePtr node;                                  \
   char buff[80];                                    \
   guid_to_string_buff ((VAL), buff);                \
   node = xmlNewNode (NULL, TOK);                    \
   xmlNodeAddContent(node, buff);                    \
   xmlAddChild (topnode, node);                      \
}

#define PUT_BOOL(TOK,VAL) {                          \
   xmlNodePtr node;                                  \
   gboolean boll = (VAL);                            \
   node = xmlNewNode (NULL, TOK);                    \
   if (boll) {                                       \
      xmlNodeAddContent(node, "T");                  \
   } else {                                          \
      xmlNodeAddContent(node, "F");                  \
   }                                                 \
   xmlAddChild (topnode, node);                      \
}

#define PUT_ENUM_3(TOK,VAL,A,B,C) {                  \
   xmlNodePtr node;                                  \
   const char * str = #A;                            \
   switch (VAL)                                      \
   {                                                 \
      case GTT_##A: str = #A; break;                 \
      case GTT_##B: str = #B; break;                 \
      case GTT_##C: str = #C; break;                 \
   }                                                 \
   node = xmlNewNode (NULL, TOK);                    \
   xmlNodeAddContent(node, str);                     \
   xmlAddChild (topnode, node);                      \
}

#define PUT_ENUM_4(TOK,VAL,A,B,C, D) {               \
   xmlNodePtr node;                                  \
   const char * str = #A;                            \
   switch (VAL)                                      \
   {                                                 \
      case GTT_##A: str = #A; break;                 \
      case GTT_##B: str = #B; break;                 \
      case GTT_##C: str = #C; break;                 \
      case GTT_##D: str = #D; break;                 \
   }                                                 \
   node = xmlNewNode (NULL, TOK);                    \
   xmlNodeAddContent(node, str);                     \
   xmlAddChild (topnode, node);                      \
}

#define PUT_ENUM_5(TOK,VAL,A,B,C,D,E) {              \
   xmlNodePtr node;                                  \
   const char * str = #A;                            \
   switch (VAL)                                      \
   {                                                 \
      case GTT_##A: str = #A; break;                 \
      case GTT_##B: str = #B; break;                 \
      case GTT_##C: str = #C; break;                 \
      case GTT_##D: str = #D; break;                 \
      case GTT_##E: str = #E; break;                 \
   }                                                 \
   node = xmlNewNode (NULL, TOK);                    \
   xmlNodeAddContent(node, str);                     \
   xmlAddChild (topnode, node);                      \
}

static xmlNodePtr
qof_query_param_path_to_xml (GSList *param_path)
{
	xmlNodePtr topnode = xmlNewNode (NULL, "qofquery:param-path");
	GSList *n = param_path;
	for ( ; n; n=n->next)
	{
		const char *path = n->data;
		if (!path) continue;
		PUT_STR ("qofquery:param", path);
	}
	return topnode;
}

static xmlNodePtr
qof_query_pred_data_to_xml (QofQueryPredData *pd)
{
	xmlNodePtr topnode = xmlNewNode (NULL, "qofquery:pred-data");

	PUT_STR ("qofquery:type", pd->type_name);
// xxxxxxxxxxxx
	return topnode;
}


static xmlNodePtr
qof_query_one_term_to_xml (QofQueryTerm *qt)
{
	xmlNodePtr node;
	xmlNodePtr term = xmlNewNode (NULL, "qofquery:term");

	gboolean invert = qof_query_term_is_inverted (qt);
	GSList *path = qof_query_term_get_param_path (qt);
	QofQueryPredData *pd = qof_query_term_get_pred_data (qt);

	xmlNodePtr topnode = term;
	if (invert)
	{
		/* inverter becomes new top mode */
		topnode = xmlNewNode (NULL, "qofquery:invert");
		xmlAddChild (term, topnode);
	}

	node = qof_query_param_path_to_xml (path);
	if (node) xmlAddChild (topnode, node);

	node = qof_query_pred_data_to_xml (pd);
	if (node) xmlAddChild (topnode, node);

	return term;
}

static xmlNodePtr
qof_query_and_terms_to_xml (GList *and_terms)
{
	xmlNodePtr terms = xmlNewNode (NULL, "qofquery:and-terms");
	GList *n = and_terms;
	for ( ; n; n=n->next)
	{
		QofQueryTerm *qt = n->data;
		if (!qt) continue;

		xmlNodePtr t = qof_query_one_term_to_xml (n->data);
		if (t) xmlAddChild (terms, t);
	}
	return terms;
}

static xmlNodePtr
qof_query_terms_to_xml (QofQuery *q)
{
	xmlNodePtr terms = NULL;
	GList *n = qof_query_get_terms (q);

	if (!n) return NULL;
	terms = xmlNewNode (NULL, "qofquery:or-terms");

	for ( ; n; n=n->next)
	{
		xmlNodePtr andt = qof_query_and_terms_to_xml (n->data);
		if (andt) xmlAddChild (terms, andt);
	}
	return terms;
}

void
do_qof_query_to_xml (QofQuery *q, xmlNodePtr topnode)
{
	QofIdType search_for = qof_query_get_search_for (q);
	PUT_STR ("search-for", search_for);

	xmlNodePtr terms = qof_query_terms_to_xml(q);
	if (terms) xmlAddChild (topnode, terms);

	gint max_results = qof_query_get_max_results (q);
	PUT_INT ("max-results", max_results);

}

xmlNodePtr
qof_query_to_xml (QofQuery *q)
{
	xmlNodePtr topnode;
	xmlNodePtr node;
	xmlNsPtr   ns;

	topnode = xmlNewNode(NULL, "qof:qofquery");
	xmlSetProp(topnode, "version", "1.0.1");

	// XXX path to DTD is wrong
	// ns = xmlNewNs (topnode, "file:" "/usr/share/lib" "/qofquery.dtd", "qof");

	do_qof_query_to_xml (q, topnode);

	return topnode;
}

int main (int argc, char * argv[])
{
	QofQuery *q;
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

   xmlDocPtr doc = doc = xmlNewDoc("1.0");
   xmlNodePtr topnode = qof_query_to_xml (q);
	xmlDocSetRootElement(doc,topnode);

	xmlChar *xbuf;
	int bufsz;
	xmlDocDumpFormatMemory (doc, &xbuf, &bufsz, 1);

	printf ("%s\n", xbuf);
	xmlFree (xbuf);
	xmlFreeDoc(doc);

#if 0
printf ("duude\n");
	// xmlOutputBufferPtr xbuf = xmlAllocOutputBuffer (enc);
	xmlOutputBufferPtr xbuf = xmlOutputBufferCreateFile (stdout, NULL);
printf ("duude\n");

   xbuf = xmlOutputBufferCreateFd (1, NULL);
printf ("duude\n");
	xmlNodeDumpOutput (xbuf, NULL, topnode, 99, 99, "iso-8859-1");
   // xmlElemDump (stdout, NULL, topnode);
#endif

	return 0;
}

