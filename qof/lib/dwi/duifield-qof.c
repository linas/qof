/********************************************************************\
 * duifield-qof.c -- Implementation of the QOF Object field         *
 * Copyright (C) 2004 Linas Vepstas <linas@linas.org>               *
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
 * Implementation of the QOF Entity field
 *
 * HISTORY:
 * Linas Vepstas April 2004
 */

#include "config.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include <qof/qof.h>
#include <qof/qofid-p.h>

#include "duifield.h"
#include "duifield-qof.h"
#include "duifilter.h"
#include "duiresolver.h"
#include "interface.h"
#include "perr.h"


/* A property (actually, a parameter) on a QOF object */
struct qof_field_s
{
	QofBook *book;
	QofEntity * entity;
	char * property;
	char * value;
};

/* =================================================================== */

#define QOFF(f) ((struct qof_field_s *)(&((f)->u.priv)))

static void
qof_field_clear (DuiField *f)
{
	g_free (f->fieldname);
	g_free (QOFF(f)->property);
	g_free (QOFF(f)->value);
	f->type = DUI_FIELD_NONE;
}

/* =================================================================== */
/* Matcher to find the actual entity */

static gboolean
find_qof_entity (DuiField *matcher)
{
	struct qof_field_s *qf;

	qf = QOFF(matcher);
	if ((NULL != qf->property) && strcmp (qf->property, "guid"))
	{
		/* We could perform a search.  But for now, we only 
		 * search for GUID's */
		PERR ("non-null matching properties not handled at this time.");
		return FALSE;
	}

	/* Nothing works without a book */
	if (NULL == qf->book)
	{
		PERR ("Expecting a book to be set for the matcher");
		return FALSE;
	}

	QofCollection *lection;
	lection = qof_book_get_collection (qf->book, matcher->fieldname);

	GUID guid;
	gboolean ok = string_to_guid (qf->value, &guid);
	if (!ok)
	{
		PERR ("Expecting a GUID, got \"%s\"", qf->value);
		return FALSE;
	}

	QofEntity *ent = qof_collection_lookup_entity (lection, &guid);
	if (NULL == ent)
	{
		ent = qof_object_new_instance (matcher->fieldname, qf->book);
		qof_entity_set_guid (ent, &guid);
	}
	qf->entity = ent;

	return TRUE;
}

static void
resolve_qof_target (DuiField *target, DuiField *matcher)
{
	if (!target || !matcher) return;
	QOFF(target)->book = QOFF(matcher)->book;
	QOFF(target)->entity = QOFF(matcher)->entity;
}

/* =================================================================== */
/* Find the parameter on the entity, use the parameter getter to fetch
 * the parameter value. */

static const char *
get_qof_value (DuiField *fs)
{
	struct qof_field_s *gf;

	gf = QOFF(fs);
	if (!gf->entity)
	{
		PERR ("Can't find the QOF instance \'%s\'\n", fs->fieldname);
		return NULL;
	}

	const QofParam *qp;
	qp = qof_class_get_parameter (fs->fieldname, QOFF(fs)->property);
	if (NULL == qp)
	{
		PERR ("Cannot find parameter to get %s:%s\n", 
		              fs->fieldname, QOFF(fs)->property);
		return NULL;
	}

	gpointer getter;
	getter = qof_class_get_parameter_getter (fs->fieldname, QOFF(fs)->property);

	if (!strcmp (qp->param_type, QOF_TYPE_STRING))
	{
		const char * (*fn) (QofEntity *) =  getter;
		const char * str = fn (gf->entity);
		return str;
	}
	else
	if (!strcmp (qp->param_type, QOF_TYPE_INT32))
	{
		gint32 (*fn) (QofEntity *) = getter;
		gint32 ival = fn (gf->entity);

		if (gf->value) g_free (gf->value);
		gf->value = g_strdup_printf ("%d", ival);
		return gf->value;
	}
	else
	if (!strcmp (qp->param_type, QOF_TYPE_INT64))
	{
		gint64 (*fn) (QofEntity *) = getter;
		gint64 ival = fn (gf->entity);

		if (gf->value) g_free (gf->value);
		gf->value = g_strdup_printf ("%lld", ival);
		return gf->value;
	}
	else
	if (!strcmp (qp->param_type, QOF_TYPE_DOUBLE))
	{
		gdouble (*fn) (QofEntity *) = getter;
		gdouble ival = fn (gf->entity);

		if (gf->value) g_free (gf->value);
		gf->value = g_strdup_printf ("%24.18g", ival);
		return gf->value;
	}
	else
	if (!strcmp (qp->param_type, QOF_TYPE_BOOLEAN))
	{
		gboolean (*fn) (QofEntity *) = getter;
		gboolean ival = fn (gf->entity);

		if (gf->value) g_free (gf->value);
		if (ival)
		{
			gf->value = g_strdup ("T");
		}
		else
		{
			gf->value = g_strdup ("F");
		}
		return gf->value;
	}
	else
	if (!strcmp (qp->param_type, QOF_TYPE_GUID))
	{
		GUID * (*fn) (QofEntity *) = getter;
		GUID *guid = fn (gf->entity);

		if (gf->value) g_free (gf->value);
		gf->value = g_new (char, GUID_ENCODING_LENGTH+1);
		guid_to_string_buff (guid, gf->value);
		return gf->value;
	}
	else
	if (!strcmp (qp->param_type, QOF_TYPE_DATE))
	{
		char buff[200];
		Timespec (*fn) (QofEntity *) = getter;
		Timespec ts = fn (gf->entity);

		if (gf->value) g_free (gf->value);
		gnc_timespec_to_iso8601_buff (ts, buff);
		gf->value = g_strdup (buff);
		return gf->value;
	}


	// XXX add all the other fields too 

	PERR ("unsupported type %s for %s:%s\n", 
	         qp->param_type, fs->fieldname, QOFF(fs)->property);
	return NULL;
}

static void 
set_qof_value (DuiField *fs, const char *value)
{
	if (NULL == QOFF(fs)->entity) return;
	ENTER (" %s::%s = %s\n", fs->fieldname, QOFF(fs)->property, value);

	gpointer setter;
	setter = qof_class_get_parameter_setter (fs->fieldname, QOFF(fs)->property);
 	if (!setter) return;

	QofType pt;
	pt = qof_class_get_parameter_type (fs->fieldname, QOFF(fs)->property);
	if (!strcmp (pt, QOF_TYPE_STRING))
	{
		void (*sf) (QofEntity *, const char *) = setter;
		(*sf) (QOFF(fs)->entity, value);
	} 
	else
	if (!strcmp (pt, QOF_TYPE_INT32))
	{
		gint32 ival = atoi (value);
		void (*sf) (QofEntity *, int) = setter;
		(*sf) (QOFF(fs)->entity, ival);
	}
	else
	if (!strcmp (pt, QOF_TYPE_INT64))
	{
		gint64 ival = atoll (value);
		void (*sf) (QofEntity *, gint64) = setter;
		(*sf) (QOFF(fs)->entity, ival);
	}
	else
	if (!strcmp (pt, QOF_TYPE_DOUBLE))
	{
		gdouble ival = strtod (value, NULL);
		void (*sf) (QofEntity *, gdouble) = setter;
		(*sf) (QOFF(fs)->entity, ival);
	}
	else
	if (!strcmp (pt, QOF_TYPE_BOOLEAN))
	{
		gboolean ival = dui_util_bool_to_int (value);
		void (*sf) (QofEntity *, gboolean) = setter;
		(*sf) (QOFF(fs)->entity, ival);
	}
	else
	if (!strcmp (pt, QOF_TYPE_DATE))
	{
		Timespec tval;
		tval = gnc_iso8601_to_timespec_gmt (value);
		void (*sf) (QofEntity *, Timespec) = setter;
		(*sf) (QOFF(fs)->entity, tval);
	}
	else
	if (!strcmp (pt, QOF_TYPE_GUID))
	{
		GUID guid;
		if (TRUE == string_to_guid (value, &guid))
		{
			void (*sf) (QofEntity *, GUID *) = setter;
			(*sf) (QOFF(fs)->entity, &guid);
		}
	}
	else
	if (!strcmp (pt, QOF_TYPE_NUMERIC))
	{
		gnc_numeric num;
		string_to_gnc_numeric (value, &num);
		void (*sf) (QofEntity *, gnc_numeric *) = setter;
		(*sf) (QOFF(fs)->entity, &num);
	}
	else
	{
		/* XXX QOF_TYPE_KVP, QOF_TYPE_CHAR, QOF_TYPE_DEBCRED  etc */
		PERR ("Unsupported type %s", pt);
	}
}

static void 
set_qof_match_value (DuiField *fs, const char *value)
{
	if (!fs) return;
	QOFF(fs)->value = g_strdup (value);
}

void
dui_field_set_qof (DuiField *ft, const char * obj_type,
                                 const char * prop)
{
	struct qof_field_s *gf;

	ft->type = DUI_FIELD_QOF;
	ft->fieldname = g_strdup (obj_type);

	ft->get_field_value = get_qof_value;
	ft->set_field_value = set_qof_value;
	ft->clear_field = qof_field_clear;
	ft->iter_column = resolve_qof_target;

	gf = QOFF(ft);
	gf->entity = NULL;
	gf->property = g_strdup (prop);
	gf->value = NULL;
}

void
dui_field_set_qof_match (DuiField *ft, const char * obj_type,
                                  const char * prop)
{
	struct qof_field_s *gf;

	ft->type = DUI_FIELD_QOF_MATCH;
	ft->fieldname = g_strdup (obj_type);

	ft->get_field_value = NULL;
	ft->set_field_value = set_qof_match_value;
	ft->clear_field = qof_field_clear;
	ft->iter_next = find_qof_entity;

	gf = QOFF(ft);
	gf->entity = NULL;
	gf->property = g_strdup (prop);
	gf->value = NULL;
}

/* =================================================================== */

void 
dui_resolver_resolve_qof (DuiResolver *res , QofBook *book)
{
	GList *node;
	for (node = res->field_list; node; node=node->next)
	{
		DuiField *fld = node->data;
		if ((!DUI_FIELD_IS_TYPE(fld,DUI_FIELD_QOF)) &&
		    (!DUI_FIELD_IS_TYPE(fld,DUI_FIELD_QOF_MATCH))) continue;

		QOFF(fld)->book = book;
	}
}

/* ========================== END OF FILE ============================ */
