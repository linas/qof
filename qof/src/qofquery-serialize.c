
/* 
 * Take a query, serialize it into XML

 * Copyright (C) 2001,2002,2004 Linas Vepstas <linas@linas.org>
 */
#include <stdio.h>

#include <qof/qofquery.h>
#include <qof/qofsql.h>

#include <libxml/tree.h>

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
qof_query_terms_to_xml (QofQuery *q)
{
	return NULL;
}

void
do_qof_query_to_xml (QofQuery *q, xmlNodePtr topnode)
{
	QofIdType search_for = qof_query_get_search_for (q);
	PUT_STR ("search-for", search_for);

	xmlNodePtr *terms = qof_query_terms_to_xml(q);
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

	qof_class_register ("GncABC", NULL, NULL);
	sq = qof_sql_query_new();

	// qof_sql_query_parse (sq, "SELECT * from GncABC WHERE asdf = 123;");
	qof_sql_query_parse (sq, "SELECT * from GncABC;");
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

