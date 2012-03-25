/********************************************************************\
 * qofutil.c -- QOF utility functions                               *
 * Copyright (C) 1997 Robin D. Clark                                *
 * Copyright (C) 1997-2001,2004 Linas Vepstas <linas@linas.org>     *
 * Copyright 2006,2008  Neil Williams  <linux@codehelp.co.uk>       *
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
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
 *                                                                  *
 *   Author: Rob Clark (rclark@cs.hmc.edu)                          *
 *   Author: Linas Vepstas (linas@linas.org)                        *
\********************************************************************/

#include "config.h"

#include <errno.h>
#include <ctype.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "qof.h"
#include "qofundo-p.h"
#include "qofbook-p.h"

static QofLogModule log_module = QOF_MOD_UTIL;

/* Search for str2 in first nchar chars of str1, ignore case..  Return
 * pointer to first match, or null.  */
gchar *
strncasestr (const guchar * str1, const guchar * str2, size_t len)
{
	while (*str1 && len--)
	{
		if (toupper (*str1) == toupper (*str2))
		{
			size_t l;
			l = strlen ((gchar*)str2);
			if (strncasecmp ((gchar*)str1, (gchar*)str2, l) == 0)
				return (gchar *) str1;
		}
		str1++;
	}
	return NULL;
}

#ifndef HAVE_STRCASESTR
/* Search for str2 in str1, ignore case.  Return pointer to first
 * match, or null.  */
gchar *
strcasestr (const gchar * str1, const gchar * str2)
{
	size_t len = strlen (str1);
	gchar *retval = strncasestr (str1, str2, len);
	return retval;
}
#endif

/** \todo replace with g_strcmp0 from glib 2.16 */
gint
safe_strcmp (const gchar * da, const gchar * db)
{
	if ((da) && (db))
	{
		if ((da) != (db))
		{
			gint retval = strcmp ((da), (db));
			/* if strings differ, return */
			if (retval)
				return retval;
		}
	}
	else if ((!(da)) && (db))
		return -1;
	else if ((da) && (!(db)))
		return +1;
	return 0;
}

gint
safe_strcasecmp (const gchar * da, const gchar * db)
{
	if ((da) && (db))
	{
		if ((da) != (db))
		{
			gint retval = strcasecmp ((da), (db));
			/* if strings differ, return */
			if (retval)
				return retval;
		}
	}
	else if ((!(da)) && (db))
		return -1;
	else if ((da) && (!(db)))
		return +1;
	return 0;
}

inline gint
null_strcmp (const gchar * da, const gchar * db)
{
	if (da && db)
		return strcmp (da, db);
	if (!da && db && 0 == db[0])
		return 0;
	if (!db && da && 0 == da[0])
		return 0;
	if (!da && db)
		return -1;
	if (da && !db)
		return +1;
	return 0;
}

#define MAX_DIGITS 50

/* inverse of strtoul */
gchar *
ultostr (gulong val, gint base)
{
	gchar buf[MAX_DIGITS];
	gulong broke[MAX_DIGITS];
	gint i;
	gulong places = 0, reval;

	if ((2 > base) || (36 < base))
		return NULL;

	/* count digits */
	places = 0;
	for (i = 0; i < MAX_DIGITS; i++)
	{
		broke[i] = val;
		places++;
		val /= base;
		if (0 == val)
			break;
	}

	/* normalize */
	reval = 0;
	for (i = places - 2; i >= 0; i--)
	{
		reval += broke[i + 1];
		reval *= base;
		broke[i] -= reval;
	}

	/* print */
	for (i = 0; i < (gint) places; i++)
	{
		if (10 > broke[i])
		{
			buf[places - 1 - i] = 0x30 + broke[i];	/* ascii digit zero */
		}
		else
		{
			buf[places - 1 - i] = 0x41 - 10 + broke[i];	/* ascii capital A */
		}
	}
	buf[places] = 0x0;

	return g_strdup (buf);
}

inline gint
qof_util_double_compare (gdouble d1, gdouble d2)
{
	if (isnan (d1) && isnan (d2))
		return 0;
	if (d1 < d2)
		return -1;
	if (d1 > d2)
		return 1;
	return 0;
}

/* =================================================================== */
/* returns TRUE if the string is a number, possibly with whitespace */
/* =================================================================== */

gboolean
qof_util_string_isnum (const guchar * s)
{
	if (s == NULL)
		return FALSE;
	if (*s == 0)
		return FALSE;

	while (*s && isspace (*s))
		s++;

	if (*s == 0)
		return FALSE;
	if (!isdigit (*s))
		return FALSE;

	while (*s && isdigit (*s))
		s++;

	if (*s == 0)
		return TRUE;

	while (*s && isspace (*s))
		s++;

	if (*s == 0)
		return TRUE;

	return FALSE;
}

/* =================================================================== */
/* Return NULL if the field is whitespace (blank, tab, formfeed etc.)
 * Else return pointer to first non-whitespace character. */
/* =================================================================== */

const gchar *
qof_util_whitespace_filter (const gchar * val)
{
	size_t len;
	if (!val)
		return NULL;

	len = strspn (val, "\a\b\t\n\v\f\r ");
	if (0 == val[len])
		return NULL;
	return val + len;
}

/* =================================================================== */
/* Return integer 1 if the string starts with 't' or 'T' or contains the
 * word 'true' or 'TRUE'; if string is a number, return that number. */
/* =================================================================== */

gint
qof_util_bool_to_int (const gchar * val)
{
	const gchar *p = qof_util_whitespace_filter (val);
	if (!p)
		return 0;
	if ('t' == p[0])
		return 1;
	if ('T' == p[0])
		return 1;
	if ('y' == p[0])
		return 1;
	if ('Y' == p[0])
		return 1;
	if (strstr (p, "true"))
		return 1;
	if (strstr (p, "TRUE"))
		return 1;
	if (strstr (p, "yes"))
		return 1;
	if (strstr (p, "YES"))
		return 1;
	return atoi (val);
}

/* =================================================================== */
/* Entity edit and commit utilities */
/* =================================================================== */

gboolean
qof_util_param_edit (QofInstance * inst, const QofParam *param)
{
	QofBackend *be;
	QofUndo *undo_data;

	if (!inst)
		return FALSE;
	(inst->editlevel)++;
	if (1 < inst->editlevel)
		return FALSE;
	if (0 >= inst->editlevel)
		inst->editlevel = 1;
	be = qof_book_get_backend (inst->book);
	if (param != NULL)
	{
		undo_data = inst->book->undo_data;
		inst->param = param;
		if (undo_data->undo_operation_open)
			qof_undo_modify (inst, param);
	}
	if (be && qof_backend_begin_exists (be))
		qof_backend_run_begin (be, inst);
	else
		inst->dirty = TRUE;
	return TRUE;
}

gboolean
qof_util_param_commit (QofInstance * inst, const QofParam * param)
{
	QofUndo *undo_data;
	QofBackend * be;

	if (!inst)
		return FALSE;
	(inst->editlevel)--;
	if (0 < inst->editlevel)
		return FALSE;
	be = qof_book_get_backend (inst->book);
	inst->param = param;
	if (be && qof_backend_commit_exists (be))
		qof_backend_run_commit (be, inst);
	if (param != NULL)
	{
		undo_data = inst->book->undo_data;
		if (undo_data->undo_operation_open)
			qof_undo_commit (inst, param);
	}
	return TRUE;
}

gchar *
qof_util_make_utf8 (gchar * string)
{
	gchar *value;

	if (!string)
		return NULL;
	if (g_utf8_validate (string, -1, NULL))
		return string;
	value = g_locale_to_utf8 (string, -1, NULL, NULL, NULL);
	if (!value)
	{
		PWARN (" unable to convert from locale %s", string);
		PINFO ("trying to convert from ISO-8859-15.");
		value = g_convert (string, -1, "UTF-8", "ISO-8859-15",
			NULL, NULL, NULL);
		if (!value)
		{
			PERR (" conversion failed");
			return string;
		}
		return value;
	}
	return value;
}

/* =================================================================== */
/* The QOF string cache - reimplements a limited GCache for strings    */
/* =================================================================== */

typedef struct {
	GHashTable *key_table;
	GHashTable *value_table;
}QStrCache;

static QStrCache *qof_string_cache = NULL;

typedef struct {
	gpointer value;
	gint ref_count;
} QStrCacheNode;

static inline QStrCacheNode* g_cache_node_new (gpointer value) {
	QStrCacheNode *node = g_slice_new (QStrCacheNode);
	node->value = value;
	node->ref_count = 1;
	return node;
}

static inline void g_cache_node_destroy (QStrCacheNode *node) {
	g_slice_free (QStrCacheNode, node);
}

static QStrCache* qof_cache_new (void) {
	QStrCache *cache;
	cache = g_slice_new (QStrCache);
	cache->key_table = g_hash_table_new (g_str_hash, g_str_equal);
	cache->value_table = g_hash_table_new (g_str_hash, NULL);
	return cache;
}

static void qof_cache_destroy (QStrCache *cache) {
	g_return_if_fail (cache != NULL);
	g_hash_table_destroy (cache->key_table);
	g_hash_table_destroy (cache->value_table);
	g_slice_free (QStrCache, cache);
}

static gpointer qof_cache_insert (QStrCache *cache, gpointer key) {
	QStrCacheNode *node;
	gpointer value;

	g_return_val_if_fail (cache != NULL, NULL);
	node = g_hash_table_lookup (cache->key_table, key);
	if (node) {
		node->ref_count += 1;
		return node->value;
	}
	key = g_strdup (key);
	value = g_strdup (key);
	node = g_cache_node_new (value);
	g_hash_table_insert (cache->key_table, key, node);
	g_hash_table_insert (cache->value_table, value, key);
	return node->value;
}

static void qof_cache_remove (QStrCache *cache, gconstpointer value) {
	QStrCacheNode *node;
	gpointer key;

	g_return_if_fail (cache != NULL);
	key = g_hash_table_lookup (cache->value_table, value);
	node = g_hash_table_lookup (cache->key_table, key);
	g_return_if_fail (node != NULL);
	node->ref_count -= 1;
	if (node->ref_count == 0) {
		g_hash_table_remove (cache->value_table, value);
		g_hash_table_remove (cache->key_table, key);
		g_free (key);
		g_free (node->value);
		g_cache_node_destroy (node);
	}
}

static QStrCache * qof_util_get_string_cache (void) {
	if (!qof_string_cache) {
		qof_string_cache = qof_cache_new ();
	}
	return qof_string_cache;
}

void qof_util_string_cache_destroy (void) {
	if (qof_string_cache) {
		qof_cache_destroy (qof_string_cache);
	}
	qof_string_cache = NULL;
}

void qof_util_string_cache_remove (gconstpointer key) {
	if (key) {
		qof_cache_remove (qof_util_get_string_cache (), key);
	}
}

gpointer qof_util_string_cache_insert (gconstpointer key) {
	if (key) {
		return qof_cache_insert(qof_util_get_string_cache(), (gpointer)key);
	}
	return NULL;
}

gchar *
qof_util_param_to_string (QofEntity * ent, const QofParam * param)
{
	gchar *param_string;
	gchar param_sa[GUID_ENCODING_LENGTH + 1];
	gboolean known_type;
	QofType paramType;
	const GUID *param_guid;
	QofNumeric param_numeric, (*numeric_getter) (QofEntity *, const QofParam *);
	gdouble param_double, (*double_getter) (QofEntity *, const QofParam *);
	gboolean param_boolean, (*boolean_getter) (QofEntity *, const QofParam *);
	gint32 param_i32, (*int32_getter) (QofEntity *, const QofParam *);
	gint64 param_i64, (*int64_getter) (QofEntity *, const QofParam *);
	gchar param_char, (*char_getter) (QofEntity *, const QofParam *);

	param_string = NULL;
	known_type = FALSE;
	g_return_val_if_fail (ent && param, NULL);
	paramType = param->param_type;
	if (safe_strcmp (paramType, QOF_TYPE_STRING) == 0)
	{
		param_string = g_strdup (param->param_getfcn (ent, param));
		if (param_string == NULL)
			param_string = g_strup("");
		known_type = TRUE;
		return param_string;
	}
	if (safe_strcmp (paramType, QOF_TYPE_TIME) == 0)
	{
		QofTime *param_qt;
		QofDate *qd;
		param_qt = param->param_getfcn (ent, param);
		qd = qof_date_from_qtime (param_qt);
		return qof_date_print (qd, QOF_DATE_FORMAT_UTC);
	}
	if ((safe_strcmp (paramType, QOF_TYPE_NUMERIC) == 0) ||
		(safe_strcmp (paramType, QOF_TYPE_DEBCRED) == 0))
	{
		numeric_getter =
			(QofNumeric (*)(QofEntity *, const QofParam *)) param->param_getfcn;
		param_numeric = numeric_getter (ent, param);
		param_string = g_strdup (qof_numeric_to_string (param_numeric));
		known_type = TRUE;
		return param_string;
	}
	if (safe_strcmp (paramType, QOF_TYPE_GUID) == 0)
	{
		param_guid = param->param_getfcn (ent, param);
		guid_to_string_buff (param_guid, param_sa);
		param_string = g_strdup (param_sa);
		known_type = TRUE;
		return param_string;
	}
	if (safe_strcmp (paramType, QOF_TYPE_INT32) == 0)
	{
		int32_getter =
			(gint32 (*)(QofEntity *, const QofParam *)) param->param_getfcn;
		param_i32 = int32_getter (ent, param);
		param_string = g_strdup_printf ("%d", param_i32);
		known_type = TRUE;
		return param_string;
	}
	if (safe_strcmp (paramType, QOF_TYPE_INT64) == 0)
	{
		int64_getter =
			(gint64 (*)(QofEntity *, const QofParam *)) param->param_getfcn;
		param_i64 = int64_getter (ent, param);
		param_string = g_strdup_printf ("%" G_GINT64_FORMAT, param_i64);
		known_type = TRUE;
		return param_string;
	}
	if (safe_strcmp (paramType, QOF_TYPE_DOUBLE) == 0)
	{
		double_getter =
			(double (*)(QofEntity *, const QofParam *)) param->param_getfcn;
		param_double = double_getter (ent, param);
		param_string = g_strdup_printf ("%f", param_double);
		known_type = TRUE;
		return param_string;
	}
	if (safe_strcmp (paramType, QOF_TYPE_BOOLEAN) == 0)
	{
		boolean_getter =
			(gboolean (*)(QofEntity *, const QofParam *)) param->param_getfcn;
		param_boolean = boolean_getter (ent, param);
		/* Boolean values need to be lowercase for QSF validation. */
		if (param_boolean == TRUE)
		{
			param_string = g_strdup ("true");
		}
		else
		{
			param_string = g_strdup ("false");
		}
		known_type = TRUE;
		return param_string;
	}
	/* "kvp" contains repeating values, cannot be a single string for the frame. */
	if (safe_strcmp (paramType, QOF_TYPE_KVP) == 0)
	{
		KvpFrame *frame = NULL;
		frame = param->param_getfcn (ent, param);
		known_type = TRUE;
		if (!kvp_frame_is_empty (frame))
		{
			GHashTable *hash = kvp_frame_get_hash (frame);
			param_string = g_strdup_printf ("%s(%d)", QOF_TYPE_KVP,
				g_hash_table_size (hash));
		}
		/* ensure a newly allocated string is returned, even
		if the frame is empty. */
		else
		{
			param_string = g_strdup("");
		}
		return param_string;
	}
	if (safe_strcmp (paramType, QOF_TYPE_CHAR) == 0)
	{
		char_getter =
			(gchar (*)(QofEntity *, const QofParam *)) param->param_getfcn;
		param_char = char_getter (ent, param);
		known_type = TRUE;
		return g_strdup_printf ("%c", param_char);
	}
	/* "collect" contains repeating values, cannot be a single string. */
	if (safe_strcmp (paramType, QOF_TYPE_COLLECT) == 0)
	{
		QofCollection *col = NULL;
		col = param->param_getfcn (ent, param);
		known_type = TRUE;
		return g_strdup_printf ("%s(%d)",
			qof_collection_get_type (col), qof_collection_count (col));
	}
	if (safe_strcmp (paramType, QOF_TYPE_CHOICE) == 0)
	{
		QofEntity *child = NULL;
		child = param->param_getfcn (ent, param);
		if (!child)
		{
			return param_string;
		}
		known_type = TRUE;
		return g_strdup (qof_object_printable (child->e_type, child));
	}
	if (safe_strcmp (paramType, QOF_PARAM_BOOK) == 0)
	{
		QofBackend *be;
		QofBook *book;
		book = param->param_getfcn (ent, param);
		PINFO (" book param %p", book);
		be = qof_book_get_backend (book);
		known_type = TRUE;
		PINFO (" backend=%p", be);
		if (!be)
		{
			return QOF_PARAM_BOOK;
		}
		param_string = g_strdup (be->fullpath);
		PINFO (" fullpath=%s", param_string);
		if (param_string)
		{
			return param_string;
		}
		param_guid = qof_entity_get_guid ((QofEntity*)book);
		guid_to_string_buff (param_guid, param_sa);
		PINFO (" book GUID=%s", param_sa);
		param_string = g_strdup (param_sa);
		return param_string;
	}
	if (!known_type)
	{
		QofEntity *child = NULL;
		child = param->param_getfcn (ent, param);
		if (!child)
		{
			return param_string;
		}
		return g_strdup (qof_object_printable (child->e_type, child));
	}
	return g_strdup ("");
}

gboolean
qof_util_param_set_string (QofEntity * ent, const QofParam * param,
	const gchar * value_string)
{
	void (*string_setter) (QofEntity *, const gchar *);
	void (*time_setter) (QofEntity *, QofTime *);
	void (*numeric_setter) (QofEntity *, QofNumeric);
	void (*guid_setter) (QofEntity *, const GUID *);
	void (*double_setter) (QofEntity *, gdouble);
	void (*boolean_setter) (QofEntity *, gboolean);
	void (*i32_setter) (QofEntity *, gint32);
	void (*i64_setter) (QofEntity *, gint64);
	void (*char_setter) (QofEntity *, gchar);
/*	void (*kvp_frame_setter) (QofEntity *, KvpFrame *);
	void (*reference_setter) (QofEntity *, QofEntity *);
	void (*collection_setter) (QofEntity *, QofCollection *);*/

	g_return_val_if_fail (ent, FALSE);
	g_return_val_if_fail (param, FALSE);
	g_return_val_if_fail (value_string, FALSE);

	if (safe_strcmp (param->param_type, QOF_TYPE_STRING) == 0)
	{
		string_setter =
			(void (*)(QofEntity *,
				const gchar *)) param->param_setfcn;
		if (string_setter != NULL)
			string_setter (ent, value_string);
//		registered_type = TRUE;
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_TIME) == 0)
	{
		QofTime *qt;
		QofDate *qd;

		qd = qof_date_parse (value_string, QOF_DATE_FORMAT_UTC);
		if (!qd)
			return FALSE;
		qt = qof_date_to_qtime (qd);
		time_setter =
			(void (*)(QofEntity *, QofTime *))
			param->param_setfcn;
		if ((time_setter != NULL) && (qof_time_is_valid (qt)))
			time_setter (ent, qt);
		qof_date_free (qd);
//		registered_type = TRUE;
	}
	if ((safe_strcmp (param->param_type, QOF_TYPE_NUMERIC) == 0) ||
		(safe_strcmp (param->param_type, QOF_TYPE_DEBCRED) == 0))
	{
		QofNumeric num;
		numeric_setter =
			(void (*)(QofEntity *,
				QofNumeric)) param->param_setfcn;
		if (!qof_numeric_from_string (value_string, &num) ||
			(qof_numeric_check (num) != QOF_ERROR_OK))
			return FALSE;
		if (numeric_setter != NULL)
			numeric_setter (ent, num);
//		registered_type = TRUE;
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_GUID) == 0)
	{
		GUID * guid;

		guid = guid_malloc();
		guid_new (guid);
		guid_setter =
			(void (*)(QofEntity *,
				const GUID *)) param->param_setfcn;
		if (!string_to_guid(value_string, guid))
			return FALSE;
		if (guid_setter != NULL)
			guid_setter (ent, guid);
//		registered_type = TRUE;
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_INT32) == 0)
	{
		gint32 i32;
		gchar *tail;

		errno = 0;
		i32_setter =
			(void (*)(QofEntity *, gint32)) param->param_setfcn;
		i32 =
			(gint32) strtol (value_string, &tail, 0);
		if ((i32_setter != NULL) && (errno == 0))

			i32_setter (ent, i32);
//		registered_type = TRUE;
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_INT64) == 0)
	{
		gint64 i64;
		gchar *tail;

		errno = 0;
		i64 = strtoll (value_string, &tail, 0);
		i64_setter =
			(void (*)(QofEntity *, gint64)) param->param_setfcn;
		if ((i64_setter != NULL) && (errno == 0))
			i64_setter (ent, i64);
//		registered_type = TRUE;
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_DOUBLE) == 0)
	{
		gdouble db;
		gchar *tail;

		errno = 0;
		db = strtod (value_string, &tail);
		double_setter =
			(void (*)(QofEntity *, gdouble)) param->param_setfcn;
		if ((double_setter != NULL) && (errno == 0))
			double_setter (ent, db);
//		registered_type = TRUE;
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_BOOLEAN) == 0)
	{
		gint val;
		gboolean G_GNUC_UNUSED b;

		boolean_setter =
			(void (*)(QofEntity *, gboolean)) param->param_setfcn;
		val = qof_util_bool_to_int(value_string);
		if ((val > 1) || (val < 0))
			return FALSE;
		b = (val == 1) ? TRUE : FALSE;
		if (boolean_setter != NULL)
			boolean_setter (ent, val);
//		registered_type = TRUE;
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_KVP) == 0)
	{
		/* unsupported */
		return FALSE;
/*		KvpFrame * frame;
		KvpValue * value;

		kvp_frame_setter =
			(void (*)(QofEntity *, KvpFrame *)) param->param_setfcn;
		if (kvp_frame_setter != NULL)
			kvp_frame_setter (rule->targetEnt, cm_kvp);
//		registered_type = TRUE;*/
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_CHAR) == 0)
	{
		char_setter =
			(void (*)(QofEntity *, gchar)) param->param_setfcn;
		if (char_setter != NULL)
			char_setter (ent, value_string[0]);
//		registered_type = TRUE;
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_COLLECT) == 0)
	{
		/* unsupported */
		return FALSE;
	}
	if (safe_strcmp (param->param_type, QOF_TYPE_CHOICE) == 0)
	{
		/* unsupported*/
		return FALSE;
	}
/*	if (registered_type == FALSE)
	{
		referenceEnt =
			cm_param->param_getfcn (rule->importEnt, cm_param);
		if (referenceEnt)
		{
			reference_setter =
				(void (*)(QofEntity *, QofEntity *)) cm_param->
				param_setfcn;
			if (reference_setter != NULL)
			{
				reference_setter (rule->targetEnt, referenceEnt);
			}
		}
	}*/
	return TRUE;
}


void
qof_init (void)
{
	qof_util_get_string_cache ();
	guid_init ();
	qof_date_init ();
	qof_object_initialize ();
	qof_query_init ();
	qof_book_register ();
}

void
qof_close (void)
{
	qof_query_shutdown ();
	qof_object_shutdown ();
	guid_shutdown ();
	qof_date_close ();
	qof_util_string_cache_destroy ();
}

/* ************************ END OF FILE ***************************** */
