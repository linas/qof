/* **************************************************************************
 *            qoflog.c
 *
 *  Mon Nov 21 14:41:59 2005
 *  Author: Rob Clark (rclark@cs.hmc.edu)
 *  Copyright (C) 1997-2003 Linas Vepstas <linas@linas.org>
 *  Copyright  2005  Neil Williams
 *  linux@codehelp.co.uk
 *************************************************************************** */
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301,  USA
 */

#include "config.h"

#include <glib.h>
#include <unistd.h>
#include "qof.h"

#define QOF_LOG_MAX_CHARS 50
#define QOF_LOG_INDENT_WIDTH 4

static FILE *fout = NULL;
static gchar *filename = NULL;
static gchar *function_buffer = NULL;
static const gint MAX_TRACE_FILENAME = 100;
static GHashTable *log_table = NULL;
static gint qof_log_num_spaces = 0;

/* uses the enum_as_string macro.
Lookups are done on the string. */
AS_STRING_FUNC (QofLogLevel, LOG_LEVEL_LIST)
FROM_STRING_FUNC (QofLogLevel, LOG_LEVEL_LIST)

void qof_log_add_indent (void)
{
	qof_log_num_spaces += QOF_LOG_INDENT_WIDTH;
}

gint
qof_log_get_indent (void)
{
	return qof_log_num_spaces;
}

void
qof_log_drop_indent (void)
{
	qof_log_num_spaces = (qof_log_num_spaces < QOF_LOG_INDENT_WIDTH) ?
		0 : qof_log_num_spaces - QOF_LOG_INDENT_WIDTH;
}

static void
fh_printer (const gchar * log_domain __attribute__ ((unused)), 
	GLogLevelFlags log_level __attribute__ ((unused)), 
	const gchar * message, gpointer user_data)
{
	FILE *fh = user_data;
	fprintf (fh, "%*s%s\n", qof_log_num_spaces, "", message);
	fflush (fh);
}

void
qof_log_init (void)
{
	if (!fout)	/* allow qof_log_set_file */
	{
		fout = fopen ("/tmp/qof.trace", "w");
	}

	if (!fout && (filename = (gchar *) g_malloc (MAX_TRACE_FILENAME)))
	{
		snprintf (filename, MAX_TRACE_FILENAME - 1, "/tmp/qof.trace.%d",
			getpid ());
		fout = fopen (filename, "w");
		g_free (filename);
	}

	if (!fout)
		fout = stderr;

	g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_MASK, fh_printer, fout);
}

void
qof_log_set_level (QofLogModule log_module, QofLogLevel level)
{
	gchar *level_string;

	if (!log_module || level == 0)
	{
		return;
	}
	level_string = g_strdup (QofLogLevelasString (level));
	if (!log_table)
	{
		log_table = g_hash_table_new (g_str_hash, g_str_equal);
	}
	g_hash_table_insert (log_table, (gpointer) log_module, level_string);
}

static void
log_module_foreach (gpointer key, 
	gpointer value __attribute__ ((unused)), gpointer data)
{
	g_hash_table_insert (log_table, key, data);
}

void
qof_log_set_level_registered (QofLogLevel level)
{
	gchar *level_string;

	if (!log_table || level == 0)
	{
		return;
	}
	level_string = g_strdup (QofLogLevelasString (level));
	g_hash_table_foreach (log_table, log_module_foreach, level_string);
}

void
qof_log_set_file (FILE * outfile)
{
	if (!outfile)
	{
		fout = stderr;
		return;
	}
	fout = outfile;
}

void
qof_log_init_filename (const gchar * logfilename)
{
	if (!logfilename)
	{
		fout = stderr;
	}
	else
	{
		filename = g_strdup (logfilename);
		fout = fopen (filename, "w");
	}
	qof_log_init ();
}

void
qof_log_shutdown (void)
{
	if (fout && fout != stderr)
	{
		fclose (fout);
	}
	if (filename)
	{
		g_free (filename);
	}
	if (function_buffer)
	{
		g_free (function_buffer);
	}
	g_hash_table_destroy (log_table);
}

const gchar *
qof_log_prettify (const gchar *name)
{
	gchar *p, *buffer;
	gint length;

	if (!name)
	{
		return "";
	}
	buffer = g_strndup (name, QOF_LOG_MAX_CHARS - 1);
	length = strlen (buffer);
	p = g_strstr_len (buffer, length, "(");
	if (p)
	{
		*(p + 1) = ')';
		*(p + 2) = 0x0;
	}
	else
	{
		strcpy (&buffer[QOF_LOG_MAX_CHARS - 4], "...()");
	}
	function_buffer = g_strdup (buffer);
	g_free (buffer);
	return function_buffer;
}

gboolean
qof_log_check (QofLogModule log_module, QofLogLevel log_level)
{
	gchar *log_string;
	/* Any positive log_level less than this will be logged. */
	QofLogLevel maximum; 

	log_string = NULL;
	if (log_level > QOF_LOG_TRACE)
		log_level = QOF_LOG_TRACE;
	if (!log_table || log_module == NULL)
	{
		return FALSE;
	}
	log_string = (gchar *) g_hash_table_lookup (log_table, log_module);
	/* if log_module not found, do not log. */
	if (!log_string)
	{
		return FALSE;
	}
	maximum = QofLogLevelfromString (log_string);
	if (log_level <= maximum)
	{
		return TRUE;
	}
	return FALSE;
}

void
qof_log_set_default (QofLogLevel log_level)
{
	qof_log_set_level (QOF_MOD_BACKEND, log_level);
	qof_log_set_level (QOF_MOD_CLASS, log_level);
	qof_log_set_level (QOF_MOD_ENGINE, log_level);
	qof_log_set_level (QOF_MOD_OBJECT, log_level);
	qof_log_set_level (QOF_MOD_KVP, log_level);
	qof_log_set_level (QOF_MOD_MERGE, log_level);
	qof_log_set_level (QOF_MOD_QUERY, log_level);
	qof_log_set_level (QOF_MOD_SESSION, log_level);
	qof_log_set_level (QOF_MOD_CHOICE, log_level);
	qof_log_set_level (QOF_MOD_UTIL, log_level);
	qof_log_set_level (QOF_MOD_TIME, log_level);
	qof_log_set_level (QOF_MOD_DATE, log_level);
	qof_log_set_level (QOF_MOD_UNDO, log_level);
	qof_log_set_level (QOF_MOD_ERROR, log_level);
}

struct hash_s
{
	QofLogCB cb;
	gpointer data;
};

static void
hash_cb (gpointer key, gpointer value, gpointer data)
{
	struct hash_s *qiter;

	qiter = (struct hash_s *) data;
	if (!qiter)
	{
		return;
	}
	(qiter->cb) (key, value, qiter->data);
}

void
qof_log_module_foreach (QofLogCB cb, gpointer data)
{
	struct hash_s qiter;

	if (!cb)
	{
		return;
	}
	qiter.cb = cb;
	qiter.data = data;
	g_hash_table_foreach (log_table, hash_cb, (gpointer) &qiter);
}

gint
qof_log_module_count (void)
{
	if (!log_table)
	{
		return 0;
	}
	return g_hash_table_size (log_table);
}

/* ************************ END OF FILE **************************** */
