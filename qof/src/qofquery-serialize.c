
/* 
 * Take a query, serialize it into XML
 */
#include <stdio.h>

#include <qof/qofquery.h>
#include <qof/qofsql.h>

#include <libxml/tree.h>

static xmlNodePtr
do_qof_query_to_xml (QofQuery *q)
{
	// topnode = xmlNewNode (NULL, "gtt:project");
	return NULL;
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

	node = do_qof_query_to_xml (q);
	if (node) xmlAddChild (topnode, node);

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

