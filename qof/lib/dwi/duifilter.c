/********************************************************************\
 * duifilter.c -- set of data filters that turn strings             *
 * Copyright (C) 2002 Linas Vepstas <linas@linas.org>               *
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
 * FUNCTION: 
 * Provide a set of data filters that turn strings of one 
 * type into strings of another.
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */

#include <stdlib.h>
#include <string.h>

#include "duifilter.h"
#include "perr.h"

struct DuiFilter_s 
{
	char * name;
	DuiFilter *rev_filter;
	const char * (*filter) (const char *);

	GHashTable *lookup_table;
};

static void filters_init (void);

static GHashTable * filter_table = NULL;

/* =================================================================== */

DuiFilter *
dui_filter_new (const char * name, const char * revname)
{
	DuiFilter *filt;

	filt = g_new (DuiFilter, 1);
	filt->name = g_strdup (name);
	filt->rev_filter = NULL;
	filt->filter = NULL;
	filt->lookup_table = NULL;

	if (!filter_table)
	{
		filters_init();
	}

	g_hash_table_insert (filter_table, filt->name, filt);
	if (revname) 
	{
		filt->rev_filter = dui_filter_new (revname, NULL);
	}

	return filt;
}

/* =================================================================== */
/* =================================================================== */
/* predefined filters */
/* =================================================================== */

static const char * 
is_null (const char * val)
{
	if (!val) return "null";
	return NULL;
}

/* =================================================================== */

static const char * 
is_whitespace_or_null (const char * val)
{
	val = whitespace_filter (val);
	if (!val) return "null";
	return NULL;
}

/* =================================================================== */
/* return NULL if the field is whitespace (blank, tab, formfeed etc.)  */

const char *
whitespace_filter (const char * val)
{
	size_t len;
	if (!val) return NULL;

	len = strspn (val, "\a\b\t\n\v\f\r ");
	if (0 == val[len]) return NULL;
	return val+len;
}

/* =================================================================== */
/* strips out decimal points, whitespace, etc. and returns a pure int */

static const char * 
to_int_filter (const char * val)
{
	int ival;
	static char intbuff[30];    /* hack alert not thread safe */
	if (!val) return "0";
	ival = atoi (val);
	snprintf (intbuff, 30, "%d", ival);
	return intbuff;
}

/* =================================================================== */
/* return integer 1 if the string starts with 't' or 'T" or contians the 
 * word 'true' or 'TRUE'; if string is a number, return that number. */

int
dui_util_bool_to_int (const char * val)
{
	const char * p = whitespace_filter (val);
	if (!p) return 0;
	if ('t' == p[0]) return 1;
	if ('T' == p[0]) return 1;
	if ('y' == p[0]) return 1;
	if ('Y' == p[0]) return 1;
	if (strstr (p, "true")) return 1;
	if (strstr (p, "TRUE")) return 1;
	if (strstr (p, "yes")) return 1;
	if (strstr (p, "YES")) return 1;
	return atoi (val);
}

/* =================================================================== */
/* =================================================================== */

void
dui_filter_add_lookup (DuiFilter *filt, const char * key, const char * val)
{
	char *keyc, *valc;
	if (!filt) return;

	if (!filt->lookup_table)
	{
		filt->lookup_table = g_hash_table_new (g_str_hash, g_str_equal);
	}

	keyc = g_strdup (key);
	valc = g_strdup (val);
	g_hash_table_insert (filt->lookup_table, keyc, valc);
	
	/* if we have a reversed filter, then add key-val reveresed */
	if (filt->rev_filter)
	{
		dui_filter_add_lookup (filt->rev_filter, val, key);
	}
}

/* =================================================================== */

#define ADD_FILT(name,func) {                    \
	DuiFilter *filt = dui_filter_new(name, NULL); \
	filt->filter = func;                          \
}

static void
filters_init (void)
{
	filter_table = g_hash_table_new (g_str_hash, g_str_equal);

	/* add default, system-defined filters */
	ADD_FILT ("is_null", is_null);
	ADD_FILT ("is_whitespace_or_null", is_whitespace_or_null);
	ADD_FILT ("to_int", to_int_filter);
	ADD_FILT ("whitespace", whitespace_filter);
}

/* =================================================================== */

const char *
dui_filter_apply (DuiFilter *filt, const char *str)
{
	if (!filt) return str;

	if (filt->filter)
	{
		return (filt->filter) (str);
	}

	if (!str) return NULL;
	if (filt->lookup_table)
	{
		return g_hash_table_lookup (filt->lookup_table, str);
	}
	PWARN ("filter \"%s\" can't be found", filt->name);
	return str;
}

/* =================================================================== */

DuiFilter *
dui_filter_find_by_name (const char *filtername)
{
	DuiFilter *filt;
	if (!filtername) return NULL;
	
	if (!filter_table)
	{
		filters_init();
	}

	filt = g_hash_table_lookup (filter_table, filtername);
	if (filt) return filt;

	SYNTAX ("unknown filter \"%s\"", filtername);
	return NULL;
}

/* =========================== END OF FILE ======================= */

