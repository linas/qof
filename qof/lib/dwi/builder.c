/********************************************************************\
 * builder.c : compile SQL queries from C language data             *
 * Copyright (C) 2001 Linas Vepstas <linas@linas.org>               *
 * http://dwi.sourceforge.net                                       *
 *                                                                  *
 * This library is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU Lesser General Public       *
 * License as published by the Free Software Foundation; either     *
 * version 2.1 of the License, or (at your option) any later version.
 *                                                                  *
 * This library is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU Lesser General Public License for more details.              *
 *                                                                  *
 * You should have received a copy of the GNU Lesser General Public *
 * License along with this program; if not, contact:                *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
\********************************************************************/

/*
 * FILE:
 * builder.c
 *
 * FUNCTION:
 * generic postgres backend query builder
 * compiles data types into sql queries
 *
 * Note: Postgres documentation states that the 
 * maximum length of a query is 8192 bytes, and that
 * longer queries are ignored ...
 *
 * TBD hack alert XXX FIXME:
 * -- check for buffer overflow at end of each setter
 */

#define _GNU_SOURCE
#include <glib.h>
#include <string.h>

#include "util.h"
#include "perr.h"
#include "escape.h"
#include "builder.h"



/* ================================================ */

struct SqlBuilder_s 
{
   SqlBuilderQType qtype;

   /* pointers the the tail end of two different assembly areas */
   char * ptag;
   char * pval;

   /* sql needs commas to separate values */
   short  tag_need_comma;
   short  val_need_comma;
   short  where_need_and;
   short  got_where_term;

   /* pointers to the start of two different assembly areas. */
   char * tag_base;
   char * tbl_base;
   char * val_base;
   size_t buflen;

   /* pointer to temp memory used for escaping arguments */
   sqlEscape *escape;
};

/* ================================================ */

#define INITIAL_BUFSZ 16300

SqlBuilder *
sql_builder_new (void)
{
   SqlBuilder *b = g_new (SqlBuilder, 1);

   b->qtype = 0; /* aka 'not initialized' */

   b->tag_base = g_malloc (INITIAL_BUFSZ);
   b->tbl_base = g_malloc (1000);
   b->val_base = g_malloc (INITIAL_BUFSZ);
   b->buflen = INITIAL_BUFSZ;

   b->ptag = b->tag_base;
   b->pval = b->val_base;

   /* null terminated strings */
   *(b->ptag) = 0x0;
   *(b->pval) = 0x0;

   b->tag_need_comma = 0;
   b->val_need_comma = 0;
   b->where_need_and = 0;
   b->got_where_term = 0;

   /* the escape area */
   b->escape = sqlEscape_new ();
   return (b);
}

/* ================================================ */

void
sql_builder_destroy (SqlBuilder *b)
{
   if (!b) return;
   g_free (b->tag_base);   b->tag_base = NULL;
   g_free (b->tbl_base);   b->tbl_base = NULL;
   g_free (b->val_base);   b->val_base = NULL;
   sqlEscape_destroy (b->escape);     b->escape = NULL;
   g_free (b);
}

/* ================================================ */

SqlBuilder *
sql_builder_copy (SqlBuilder *orig)
{
   SqlBuilder *b;

   if (!orig) return sql_builder_new();

   b = g_new (SqlBuilder, 1);

   b->qtype = orig->qtype;

   b->buflen = orig->buflen;
   b->tag_base = g_malloc (orig->buflen);
   b->tbl_base = g_malloc (1000);
   b->val_base = g_malloc (orig->buflen);

   *(orig->ptag) = 0x0;
   *(orig->pval) = 0x0;

   strcpy (b->tag_base, orig->tag_base);
   strcpy (b->tbl_base, orig->tbl_base);
   strcpy (b->val_base, orig->val_base);

   b->ptag = b->tag_base + (orig->ptag - orig->tag_base);
   b->pval = b->val_base + (orig->pval - orig->val_base);

   /* null terminated strings */
   *(b->ptag) = 0x0;
   *(b->pval) = 0x0;

   b->tag_need_comma = orig->tag_need_comma;
   b->val_need_comma = orig->val_need_comma;
   b->where_need_and = orig->where_need_and;
   b->got_where_term = orig->got_where_term;

   /* the escape area */
   b->escape = sqlEscape_new ();
   return (b);
}

/* ================================================ */

void
sql_builder_table (SqlBuilder *b, const char *tablename, SqlBuilderQType qt)
{
   char * ptbl;

   if (!b || !tablename) return;
   b->qtype = qt;

   b->ptag = b->tag_base;
   b->pval = b->val_base;
   ptbl = b->tbl_base;

   /* null terminated strings */
   *(b->ptag) = 0x0;
   *(b->pval) = 0x0;
   *ptbl = 0x0;

   b->tag_need_comma = 0;
   b->val_need_comma = 0;
   b->where_need_and = 0;
   b->got_where_term = 0;

   switch (qt) 
   {
      case SQL_INSERT:
         b->ptag = stpcpy(b->ptag, "INSERT INTO ");
         b->ptag = stpcpy(b->ptag, tablename);
         b->ptag = stpcpy(b->ptag, " (");

         b->pval = stpcpy(b->pval, ") VALUES (");
         break;

      case SQL_UPDATE:
         b->ptag = stpcpy(b->ptag, "UPDATE ");
         b->ptag = stpcpy(b->ptag, tablename);
         b->ptag = stpcpy(b->ptag, " SET ");

         b->pval = stpcpy(b->pval, " WHERE ");
         break;

      case SQL_SELECT:
         b->ptag = stpcpy(b->ptag, "SELECT ");

         ptbl = stpcpy(ptbl, " FROM ");
         ptbl = stpcpy(ptbl, tablename);

         b->pval = stpcpy(b->pval, " WHERE ");
         break;

      case SQL_DELETE:
         b->ptag = stpcpy(b->ptag, "DELETE ");

         ptbl = stpcpy(ptbl, " FROM ");
         ptbl = stpcpy(ptbl, tablename);

         b->pval = stpcpy(b->pval, " WHERE ");
         break;

   };

}

/* ================================================ */
/* note that val may be NULL if a SELECT statement in being built */

void
sql_builder_set_str (SqlBuilder *b, const char *tag, const char *val)
{
   if (!b || !tag) return;
   if (!val) val= "";

   val = sqlEscapeString (b->escape, val);

   if (b->tag_need_comma) b->ptag = stpcpy(b->ptag, ", ");
   b->tag_need_comma = 1;

   switch (b->qtype) 
   {
      case SQL_INSERT:
         b->ptag = stpcpy(b->ptag, tag);

         if (b->val_need_comma) b->pval = stpcpy(b->pval, ", ");
         b->val_need_comma = 1;
         b->pval = stpcpy(b->pval, "'");
         b->pval = stpcpy(b->pval, val);
         b->pval = stpcpy(b->pval, "'");
         break;

      case SQL_UPDATE:
         b->ptag = stpcpy(b->ptag, tag);
         b->ptag = stpcpy(b->ptag, "='");
         b->ptag = stpcpy(b->ptag, val);
         b->ptag = stpcpy(b->ptag, "' ");
         break;

      case SQL_SELECT:
         b->ptag = stpcpy(b->ptag, tag);
         break;

      case SQL_DELETE:
         break;

      case 0:
         PERR ("must specify a table and a query type first!");
         break;

      default:
         PERR ("mustn't happen");
   };
   
}

/* ================================================ */

void
sql_builder_set_char (SqlBuilder *b, const char *tag, char val)
{
  char buf[2];
  buf[0] = val;
  buf[1] = 0x0;
  sql_builder_set_str (b, tag, buf);
}

/* ================================================ */

void
sql_builder_set_date (SqlBuilder *b, const char *tag, time_t ts)
{
  char buf[120];
  xxxgnc_secs_to_iso8601_buff (ts, buf);
  sql_builder_set_str (b, tag, buf);
}

/* ================================================ */

void
sql_builder_set_double (SqlBuilder *b, const char *tag, double flt)
{
  char buf[120];
  snprintf (buf, 120, SQL_DBL_FMT, flt);
  sql_builder_set_str (b, tag, buf);
}

/* ================================================ */

void
sql_builder_set_int64 (SqlBuilder *b, const char *tag, gint64 nval)
{
   char val[100];
   if (!b || !tag) return;

   snprintf (val, 100, "%lld", (long long int) nval);
   if (b->tag_need_comma) b->ptag = stpcpy(b->ptag, ", ");
   b->tag_need_comma = 1;

   switch (b->qtype) 
   {
      case SQL_INSERT:
         b->ptag = stpcpy(b->ptag, tag);

         if (b->val_need_comma) b->pval = stpcpy(b->pval, ", ");
         b->val_need_comma = 1;
         b->pval = stpcpy(b->pval, val);
         break;

      case SQL_UPDATE:
         b->ptag = stpcpy(b->ptag, tag);
         b->ptag = stpcpy(b->ptag, "=");
         b->ptag = stpcpy(b->ptag, val);
         break;

      case SQL_SELECT:
         b->ptag = stpcpy(b->ptag, tag);
         break;

      case SQL_DELETE:
         break;

      case 0:
         PERR ("must specify a table and a query type first!");
         break;

      default:
         PERR ("mustn't happen");
   };
}

/* ================================================ */

void
sql_builder_set_int32 (SqlBuilder *b, const char *tag, gint32 nval)
{
   sql_builder_set_int64 (b, tag, (gint64) nval);
}

/* ================================================ */

void
sql_builder_where_str (SqlBuilder *b, const char *tag, 
                                      const char *val, const char * op)
{
   if (!b || !tag || !val) return;
   b->got_where_term = 1;

   switch (b->qtype) 
   {
      case SQL_INSERT:
         /* ther is no where clasue, so we do the set as a utility */
         sql_builder_set_str (b, tag, val);
         break;

      case SQL_UPDATE:
      case SQL_SELECT:
      case SQL_DELETE:
         val = sqlEscapeString (b->escape, val);

         if (b->where_need_and) b->pval = stpcpy(b->pval, " AND ");
         b->where_need_and = 1;

         b->pval = stpcpy(b->pval, tag);
			if (op)
	 		{
         	b->pval = stpcpy(b->pval, " ");
         	b->pval = stpcpy(b->pval, op);
         	b->pval = stpcpy(b->pval, " '");
	 		}
			else
			{
         	b->pval = stpcpy(b->pval, "='");
			}
         b->pval = stpcpy(b->pval, val);
         b->pval = stpcpy(b->pval, "'");

         break;

      case 0:
         PERR ("must specify a table and a query type first!");
         break;


      default:
         PERR ("mustn't happen");
   };
}

/* ================================================ */

void
sql_builder_where_int32 (SqlBuilder *b, const char *tag, 
                                        gint32 val, const char *op)
{
  char str[40];
  snprintf (str, 40, "%d", val);
  sql_builder_where_str (b, tag, str, op);
}

/* ================================================ */

const char *
sql_builder_query (SqlBuilder *b)
{
   if (!b) return NULL;

   switch (b->qtype) 
   {
      case SQL_INSERT:
         b->ptag = stpcpy(b->ptag, b->val_base);
         b->ptag = stpcpy(b->ptag, ");");
         break;

      case SQL_UPDATE:
      case SQL_SELECT:
      case SQL_DELETE:
         b->ptag = stpcpy(b->ptag, b->tbl_base);
         if (b->got_where_term) b->ptag = stpcpy(b->ptag, b->val_base);
         b->ptag = stpcpy(b->ptag, ";");
         break;

      case 0:
         break;

      default:
         PERR ("mustn't happen");
   };
   
   PINFO ("%s\n", b->tag_base);
   return b->tag_base;
}

/* ================ END OF FILE ==================== */
