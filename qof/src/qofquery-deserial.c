/********************************************************************\
 * qofquery-deserial.c -- Convert Qof-Query XML to QofQuery         *
 * Copyright (C) 2004 Linas Vepstas <linas@linas.org>               *
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

#include <qof/qofquery.h>

// #include "config.h"

#include "qofquery-serialize.h"
#include "qofquery-p.h"
#include "qofquerycore-p.h"


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
	xmlNodePtr qp;
	for (qp=qpart; qp; qp = qp->next)
	{
		printf ("duude got a %s\n", qp->name);
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

#define DOPRINT 1
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
