/********************************************************************\
 * qofsesssion.c -- session access (connection to backend)          *
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
\********************************************************************/

/**
 * @file qofsession.c
 * @brief Encapsulate a connection to a storage backend.
 *
 * HISTORY:
 * Created by Linas Vepstas December 1998

 @author Copyright (c) 1998-2004 Linas Vepstas <linas@linas.org>
 @author Copyright (c) 2000 Dave Peticolas
 @author Copyright (c) 2005-2006 Neil Williams <linux@codehelp.co.uk>
   */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glib.h>
#include <libintl.h>
#include "qof.h"
#include "qoferror-p.h"
#include "qofbackend-p.h"
#include "qofbook-p.h"
#include "qofsession-p.h"
#include "qofobject-p.h"

#define _(String) dgettext (GETTEXT_PACKAGE, String)

static GHookList *session_closed_hooks = NULL;
static QofLogModule log_module = QOF_MOD_SESSION;
static GSList *provider_list = NULL;

/* ============================================================= */

void
qof_backend_register_provider (QofBackendProvider * prov)
{
	provider_list = g_slist_prepend (provider_list, prov);
}

/* =========================================================== */

/* hook routines */

void
qof_session_add_close_hook (GFunc fn, gpointer data)
{
	GHook *hook;

	if (session_closed_hooks == NULL)
	{
		session_closed_hooks = malloc (sizeof (GHookList));	/* LEAKED */
		g_hook_list_init (session_closed_hooks, sizeof (GHook));
	}

	hook = g_hook_alloc (session_closed_hooks);
	if (!hook)
		return;

	hook->func = (GHookFunc) fn;
	hook->data = data;
	g_hook_append (session_closed_hooks, hook);
}

void
qof_session_call_close_hooks (QofSession * session)
{
	GHook *hook;
	GFunc fn;

	if (session_closed_hooks == NULL)
		return;

	hook = g_hook_first_valid (session_closed_hooks, FALSE);
	while (hook)
	{
		fn = (GFunc) hook->func;
		fn (session, hook->data);
		hook = g_hook_next_valid (session_closed_hooks, hook, FALSE);
	}
}

/* =============================================================== */

static void
qof_session_init (QofSession * session)
{
	if (!session)
		return;

#ifndef QOF_DISABLE_DEPRECATED
	session->entity.e_type = QOF_ID_SESSION;
#endif
	session->books = g_list_append (NULL, qof_book_new ());
	session->book_id = NULL;
	session->backend = NULL;
	qof_error_init ();
}

QofSession *
qof_session_new (void)
{
	QofSession *session = g_new0 (QofSession, 1);
	qof_session_init (session);
	return session;
}

QofBook *
qof_session_get_book (QofSession * session)
{
	GList *node;
	if (!session)
		return NULL;

	for (node = session->books; node; node = node->next)
	{
		QofBook *book = node->data;
		if ('y' == book->book_open)
			return book;
	}
	return NULL;
}

void
qof_session_add_book (QofSession * session, QofBook * addbook)
{
	GList *node;
	if (!session)
		return;

	ENTER (" sess=%p book=%p", session, addbook);

	/* See if this book is already there ... */
	for (node = session->books; node; node = node->next)
	{
		QofBook *book = node->data;
		if (addbook == book)
			return;
	}

	if ('y' == addbook->book_open)
	{
		/* hack alert -- someone should free all the books in the list,
		 * but it should probably not be us ... since the books backends
		 * should be shutdown first, etc */
/* XXX this should probably be an error XXX */
		g_list_free (session->books);
		session->books = g_list_append (NULL, addbook);
	}
	else
	{
/* XXX Need to tell the backend to add a book as well */
		session->books = g_list_append (session->books, addbook);
	}

	qof_book_set_backend (addbook, session->backend);
	LEAVE (" ");
}

QofBackend *
qof_session_get_backend (QofSession * session)
{
	if (!session)
		return NULL;
	return session->backend;
}

const gchar *
qof_session_get_file_path (QofSession * session)
{
	if (!session)
		return NULL;
	if (!session->backend)
		return NULL;
	return session->backend->fullpath;
}

const gchar *
qof_session_get_url (QofSession * session)
{
	if (!session)
		return NULL;
	return session->book_id;
}

/* =============================================================== */

typedef struct qof_entity_copy_data
{
	QofEntity *from;
	QofEntity *to;
	QofParam *param;
	GList *referenceList;
	GSList *param_list;
	QofSession *new_session;
	gboolean error;
} QofEntityCopyData;

static void
qof_book_set_partial (QofBook * book)
{
	gboolean partial;

	partial =
		(gboolean)
		GPOINTER_TO_INT (qof_book_get_data (book, PARTIAL_QOFBOOK));
	if (!partial)
	{
		qof_book_set_data (book, PARTIAL_QOFBOOK, GINT_TO_POINTER (TRUE));
	}
}

void
qof_session_update_reference_list (QofSession * session,
	QofEntityReference * reference)
{
	QofBook *book;
	GList *book_ref_list;

	book = qof_session_get_book (session);
	book_ref_list = (GList *) qof_book_get_data (book, ENTITYREFERENCE);
	book_ref_list = g_list_append (book_ref_list, reference);
	qof_book_set_data (book, ENTITYREFERENCE, book_ref_list);
	qof_book_set_partial (book);
}

static void
qof_entity_param_cb (QofParam * param, gpointer data)
{
	QofEntityCopyData *qecd;

	g_return_if_fail (data != NULL);
	qecd = (QofEntityCopyData *) data;
	g_return_if_fail (param != NULL);
	/* KVP doesn't need a set routine to be copied. */
	if (0 == safe_strcmp (param->param_type, QOF_TYPE_KVP))
	{
		qecd->param_list = g_slist_prepend (qecd->param_list, param);
		return;
	}
	if ((param->param_getfcn != NULL) && (param->param_setfcn != NULL))
	{
		qecd->param_list = g_slist_prepend (qecd->param_list, param);
	}
}

static void
col_ref_cb (QofEntity * ref_ent, gpointer user_data)
{
	QofEntityReference *ref;
	QofEntityCopyData *qecd;
	QofEntity *ent;
	const GUID *cm_guid;
	gchar cm_sa[GUID_ENCODING_LENGTH + 1];
	gchar *cm_string;

	qecd = (QofEntityCopyData *) user_data;
	ent = qecd->from;
	ref = g_new0 (QofEntityReference, 1);
	ref->type = ent->e_type;
	ref->ref_guid = g_new (GUID, 1);
	ref->ent_guid = &ent->guid;
	ref->param = qof_class_get_parameter (ent->e_type,
		qecd->param->param_name);
	cm_guid = qof_entity_get_guid (ref_ent);
	guid_to_string_buff (cm_guid, cm_sa);
	cm_string = g_strdup (cm_sa);
	if (TRUE == string_to_guid (cm_string, ref->ref_guid))
	{
		g_free (cm_string);
		qof_session_update_reference_list (qecd->new_session, ref);
	}
}

static void
qof_entity_foreach_copy (gpointer data, gpointer user_data)
{
	QofEntity *importEnt, *targetEnt /*, *referenceEnt */ ;
	QofEntityCopyData *context;
	QofEntityReference *reference;
	gboolean registered_type;
	/* cm_ prefix used for variables that hold the data to commit */
	QofParam *cm_param;
	gchar *cm_string, *cm_char;
	const GUID *cm_guid;
	KvpFrame *cm_kvp;
	QofCollection *cm_col;
	/* function pointers and variables for parameter getters that don't use pointers normally */
	QofNumeric cm_numeric, (*numeric_getter) (QofEntity *, QofParam *);
	gdouble cm_double, (*double_getter) (QofEntity *, QofParam *);
	gboolean cm_boolean, (*boolean_getter) (QofEntity *, QofParam *);
	gint32 cm_i32, (*int32_getter) (QofEntity *, QofParam *);
	gint64 cm_i64, (*int64_getter) (QofEntity *, QofParam *);
	/* function pointers to the parameter setters */
	void (*string_setter) (QofEntity *, const gchar *);
	void (*numeric_setter) (QofEntity *, QofNumeric);
	void (*guid_setter) (QofEntity *, const GUID *);
	void (*double_setter) (QofEntity *, gdouble);
	void (*boolean_setter) (QofEntity *, gboolean);
	void (*i32_setter) (QofEntity *, gint32);
	void (*i64_setter) (QofEntity *, gint64);
	void (*char_setter) (QofEntity *, gchar *);
	void (*kvp_frame_setter) (QofEntity *, KvpFrame *);

	g_return_if_fail (user_data != NULL);
	context = (QofEntityCopyData *) user_data;
	importEnt = context->from;
	targetEnt = context->to;
	registered_type = FALSE;
	cm_param = (QofParam *) data;
	g_return_if_fail (cm_param != NULL);
	context->param = cm_param;
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_STRING) == 0)
	{
		cm_string = (gchar *) cm_param->param_getfcn (importEnt, cm_param);
		if (cm_string)
		{
			string_setter =
				(void (*)(QofEntity *,
					const char *)) cm_param->param_setfcn;
			if (string_setter != NULL)
			{
				qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
				string_setter (targetEnt, cm_string);
				qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
			}
		}
		registered_type = TRUE;
	}
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_TIME) == 0)
	{
		QofTime *qt;
		void (*time_setter) (QofEntity *, QofTime *);

		qt = cm_param->param_getfcn (importEnt, cm_param);
		time_setter = 
			(void (*)(QofEntity *, QofTime*))cm_param->param_setfcn;
		if (time_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			time_setter (targetEnt, qt);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		registered_type = TRUE;
	}
#ifndef QOF_DISABLE_DEPRECATED
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_DATE) == 0)
	{
		Timespec cm_date, (*date_getter) (QofEntity *, QofParam *);
		void (*date_setter) (QofEntity *, Timespec);

		cm_date.tv_nsec = 0;
		cm_date.tv_sec = 0;
		date_getter =
			(Timespec (*)(QofEntity *, QofParam *)) cm_param->param_getfcn;
		cm_date = date_getter (importEnt, cm_param);
		date_setter =
			(void (*)(QofEntity *, Timespec)) cm_param->param_setfcn;
		if (date_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			date_setter (targetEnt, cm_date);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		registered_type = TRUE;
	}
#endif
	if ((safe_strcmp (cm_param->param_type, QOF_TYPE_NUMERIC) == 0) ||
		(safe_strcmp (cm_param->param_type, QOF_TYPE_DEBCRED) == 0))
	{
		numeric_getter =
			(QofNumeric (*)(QofEntity *,
				QofParam *)) cm_param->param_getfcn;
		cm_numeric = numeric_getter (importEnt, cm_param);
		numeric_setter =
			(void (*)(QofEntity *, QofNumeric)) cm_param->param_setfcn;
		if (numeric_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			numeric_setter (targetEnt, cm_numeric);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		registered_type = TRUE;
	}
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_GUID) == 0)
	{
		cm_guid =
			(const GUID *) cm_param->param_getfcn (importEnt, cm_param);
		guid_setter =
			(void (*)(QofEntity *, const GUID *)) cm_param->param_setfcn;
		if (guid_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			guid_setter (targetEnt, cm_guid);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		registered_type = TRUE;
	}
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_INT32) == 0)
	{
		int32_getter =
			(gint32 (*)(QofEntity *, QofParam *)) cm_param->param_getfcn;
		cm_i32 = int32_getter (importEnt, cm_param);
		i32_setter =
			(void (*)(QofEntity *, gint32)) cm_param->param_setfcn;
		if (i32_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			i32_setter (targetEnt, cm_i32);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		registered_type = TRUE;
	}
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_INT64) == 0)
	{
		int64_getter =
			(gint64 (*)(QofEntity *, QofParam *)) cm_param->param_getfcn;
		cm_i64 = int64_getter (importEnt, cm_param);
		i64_setter =
			(void (*)(QofEntity *, gint64)) cm_param->param_setfcn;
		if (i64_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			i64_setter (targetEnt, cm_i64);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		registered_type = TRUE;
	}
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_DOUBLE) == 0)
	{
		double_getter =
			(gdouble (*)(QofEntity *, QofParam *)) cm_param->param_getfcn;
		cm_double = double_getter (importEnt, cm_param);
		double_setter =
			(void (*)(QofEntity *, gdouble)) cm_param->param_setfcn;
		if (double_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			double_setter (targetEnt, cm_double);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		registered_type = TRUE;
	}
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_BOOLEAN) == 0)
	{
		boolean_getter =
			(gboolean (*)(QofEntity *, QofParam *)) cm_param->param_getfcn;
		cm_boolean = boolean_getter (importEnt, cm_param);
		boolean_setter =
			(void (*)(QofEntity *, gboolean)) cm_param->param_setfcn;
		if (boolean_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			boolean_setter (targetEnt, cm_boolean);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		registered_type = TRUE;
	}
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_KVP) == 0)
	{
		cm_kvp = (KvpFrame *) cm_param->param_getfcn (importEnt, cm_param);
		kvp_frame_setter =
			(void (*)(QofEntity *, KvpFrame *)) cm_param->param_setfcn;
		if (kvp_frame_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			kvp_frame_setter (targetEnt, cm_kvp);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		else
		{
			QofInstance *target_inst;

			target_inst = (QofInstance *) targetEnt;
			kvp_frame_delete (target_inst->kvp_data);
			target_inst->kvp_data = kvp_frame_copy (cm_kvp);
		}
		registered_type = TRUE;
	}
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_CHAR) == 0)
	{
		cm_char = (gchar *) cm_param->param_getfcn (importEnt, cm_param);
		char_setter =
			(void (*)(QofEntity *, char *)) cm_param->param_setfcn;
		if (char_setter != NULL)
		{
			qof_util_param_edit ((QofInstance *) targetEnt, cm_param);
			char_setter (targetEnt, cm_char);
			qof_util_param_commit ((QofInstance *) targetEnt, cm_param);
		}
		registered_type = TRUE;
	}
	if (safe_strcmp (cm_param->param_type, QOF_TYPE_COLLECT) == 0)
	{
		cm_col =
			(QofCollection *) cm_param->param_getfcn (importEnt, cm_param);
		if (cm_col)
		{
			/* create one reference for each member of the collection. */
			qof_collection_foreach (cm_col, col_ref_cb, context);
		}
		registered_type = TRUE;
	}
	if (registered_type == FALSE)
	{
/*		referenceEnt = (QofEntity*)cm_param->param_getfcn(importEnt, cm_param);
		if(!referenceEnt) { return; }
		if(!referenceEnt->e_type) { return; }*/
		reference = qof_entity_get_reference_from (importEnt, cm_param);
		if (reference)
		{
			qof_session_update_reference_list (context->new_session,
				reference);
		}
	}
}

static gboolean
qof_entity_guid_match (QofSession * new_session, QofEntity * original)
{
	QofEntity *copy;
	const GUID *g;
	QofIdTypeConst type;
	QofBook *targetBook;
	QofCollection *coll;

	copy = NULL;
	g_return_val_if_fail (original != NULL, FALSE);
	targetBook = qof_session_get_book (new_session);
	g_return_val_if_fail (targetBook != NULL, FALSE);
	g = qof_entity_get_guid (original);
	type = g_strdup (original->e_type);
	coll = qof_book_get_collection (targetBook, type);
	copy = qof_collection_lookup_entity (coll, g);
	if (copy)
	{
		return TRUE;
	}
	return FALSE;
}

static void
qof_entity_list_foreach (gpointer data, gpointer user_data)
{
	QofEntityCopyData *qecd;
	QofEntity *original;
	QofInstance *inst;
	QofBook *book;
	const GUID *g;

	g_return_if_fail (data != NULL);
	original = (QofEntity *) data;
	g_return_if_fail (user_data != NULL);
	qecd = (QofEntityCopyData *) user_data;
	if (qof_entity_guid_match (qecd->new_session, original))
	{
		return;
	}
	qecd->from = original;
	if (!qof_object_compliance (original->e_type, FALSE))
	{
		qecd->error = TRUE;
		return;
	}
	book = qof_session_get_book (qecd->new_session);
	inst =
		(QofInstance *) qof_object_new_instance (original->e_type, book);
	if (!inst)
	{
		PERR (" failed to create new entity type=%s.", original->e_type);
		qecd->error = TRUE;
		return;
	}
	qecd->to = &inst->entity;
	g = qof_entity_get_guid (original);
	qof_entity_set_guid (qecd->to, g);
	if (qecd->param_list != NULL)
	{
		g_slist_free (qecd->param_list);
		qecd->param_list = NULL;
	}
	qof_class_param_foreach (original->e_type, qof_entity_param_cb, qecd);
	g_slist_foreach (qecd->param_list, qof_entity_foreach_copy, qecd);
}

static void
qof_entity_coll_foreach (QofEntity * original, gpointer user_data)
{
	QofEntityCopyData *qecd;
	const GUID *g;
	QofBook *targetBook;
	QofCollection *coll;
	QofEntity *copy;

	g_return_if_fail (user_data != NULL);
	copy = NULL;
	qecd = (QofEntityCopyData *) user_data;
	targetBook = qof_session_get_book (qecd->new_session);
	g = qof_entity_get_guid (original);
	coll = qof_book_get_collection (targetBook, original->e_type);
	copy = qof_collection_lookup_entity (coll, g);
	if (copy)
	{
		qecd->error = TRUE;
	}
}

static void
qof_entity_coll_copy (QofEntity * original, gpointer user_data)
{
	QofEntityCopyData *qecd;
	QofBook *book;
	QofInstance *inst;
	const GUID *g;

	g_return_if_fail (user_data != NULL);
	qecd = (QofEntityCopyData *) user_data;
	book = qof_session_get_book (qecd->new_session);
	if (!qof_object_compliance (original->e_type, TRUE))
	{
		return;
	}
	inst =
		(QofInstance *) qof_object_new_instance (original->e_type, book);
	qecd->to = &inst->entity;
	qecd->from = original;
	g = qof_entity_get_guid (original);
	qof_entity_set_guid (qecd->to, g);
	g_slist_foreach (qecd->param_list, qof_entity_foreach_copy, qecd);
}

gboolean
qof_entity_copy_to_session (QofSession * new_session, QofEntity * original)
{
	QofEntityCopyData qecd;
	QofInstance *inst;
	QofBook *book;

	if (!new_session || !original)
		return FALSE;
	if (qof_entity_guid_match (new_session, original))
		return FALSE;
	if (!qof_object_compliance (original->e_type, TRUE))
		return FALSE;
	qof_event_suspend ();
	qecd.param_list = NULL;
	book = qof_session_get_book (new_session);
	qecd.new_session = new_session;
	qof_book_set_partial (book);
	inst =
		(QofInstance *) qof_object_new_instance (original->e_type, book);
	qecd.to = &inst->entity;
	qecd.from = original;
	qof_entity_set_guid (qecd.to, qof_entity_get_guid (original));
	qof_class_param_foreach (original->e_type, qof_entity_param_cb, &qecd);
	if (g_slist_length (qecd.param_list) == 0)
		return FALSE;
	g_slist_foreach (qecd.param_list, qof_entity_foreach_copy, &qecd);
	g_slist_free (qecd.param_list);
	qof_event_resume ();
	return TRUE;
}

gboolean
qof_entity_copy_list (QofSession * new_session, GList * entity_list)
{
	QofEntityCopyData *qecd;

	if (!new_session || !entity_list)
	{
		return FALSE;
	}
	ENTER (" list=%d", g_list_length (entity_list));
	qecd = g_new0 (QofEntityCopyData, 1);
	qof_event_suspend ();
	qecd->param_list = NULL;
	qecd->new_session = new_session;
	qof_book_set_partial (qof_session_get_book (new_session));
	g_list_foreach (entity_list, qof_entity_list_foreach, qecd);
	qof_event_resume ();
	if (qecd->error)
	{
		PWARN (" some/all entities in the list could not be copied.");
	}
	g_free (qecd);
	LEAVE (" ");
	return TRUE;
}

gboolean
qof_entity_copy_coll (QofSession * new_session,
	QofCollection * entity_coll)
{
	QofEntityCopyData qecd;

	g_return_val_if_fail (new_session, FALSE);
	if (!entity_coll)
	{
		return FALSE;
	}
	qof_event_suspend ();
	qecd.param_list = NULL;
	qecd.new_session = new_session;
	qof_book_set_partial (qof_session_get_book (qecd.new_session));
	qof_collection_foreach (entity_coll, qof_entity_coll_foreach, &qecd);
	qof_class_param_foreach (qof_collection_get_type (entity_coll),
		qof_entity_param_cb, &qecd);
	qof_collection_foreach (entity_coll, qof_entity_coll_copy, &qecd);
	if (qecd.param_list != NULL)
	{
		g_slist_free (qecd.param_list);
	}
	qof_event_resume ();
	return TRUE;
}

struct recurse_s
{
	QofSession *session;
	gboolean success;
	GList *ref_list;
	GList *ent_list;
};

static void
recurse_collection_cb (QofEntity * ent, gpointer user_data)
{
	struct recurse_s *store;

	if (user_data == NULL)
	{
		return;
	}
	store = (struct recurse_s *) user_data;
	if (!ent || !store)
	{
		return;
	}
	store->success = qof_entity_copy_to_session (store->session, ent);
	if (store->success)
	{
		store->ent_list = g_list_append (store->ent_list, ent);
	}
}

static void
recurse_ent_cb (QofEntity * ent, gpointer user_data)
{
	GList *ref_list, *i, *j, *ent_list, *child_list;
	QofParam *ref_param;
	QofEntity *ref_ent, *child_ent;
	QofSession *session;
	struct recurse_s *store;
	gboolean success;

	if (user_data == NULL)
	{
		return;
	}
	store = (struct recurse_s *) user_data;
	session = store->session;
	success = store->success;
	ref_list = NULL;
	child_ent = NULL;
	ref_list = g_list_copy (store->ref_list);
	if ((!session) || (!ent))
	{
		return;
	}
	ent_list = NULL;
	child_list = NULL;
	i = NULL;
	j = NULL;
	for (i = ref_list; i != NULL; i = i->next)
	{
		if (i->data == NULL)
		{
			continue;
		}
		ref_param = (QofParam *) i->data;
		if (ref_param->param_name == NULL)
		{
			continue;
		}
		if (0 == safe_strcmp (ref_param->param_type, QOF_TYPE_COLLECT))
		{
			QofCollection *col;

			col = ref_param->param_getfcn (ent, ref_param);
			if (col)
			{
				qof_collection_foreach (col, recurse_collection_cb, store);
			}
			continue;
		}
		ref_ent = (QofEntity *) ref_param->param_getfcn (ent, ref_param);
		if ((ref_ent) && (ref_ent->e_type))
		{
			store->success = qof_entity_copy_to_session (session, ref_ent);
			if (store->success)
			{
				ent_list = g_list_append (ent_list, ref_ent);
			}
		}
	}
	for (i = ent_list; i != NULL; i = i->next)
	{
		if (i->data == NULL)
		{
			continue;
		}
		child_ent = (QofEntity *) i->data;
		if (child_ent == NULL)
		{
			continue;
		}
		ref_list = qof_class_get_referenceList (child_ent->e_type);
		for (j = ref_list; j != NULL; j = j->next)
		{
			if (j->data == NULL)
			{
				continue;
			}
			ref_param = (QofParam *) j->data;
			ref_ent = ref_param->param_getfcn (child_ent, ref_param);
			if (ref_ent != NULL)
			{
				success = qof_entity_copy_to_session (session, ref_ent);
				if (success)
				{
					child_list = g_list_append (child_list, ref_ent);
				}
			}
		}
	}
	for (i = child_list; i != NULL; i = i->next)
	{
		if (i->data == NULL)
		{
			continue;
		}
		ref_ent = (QofEntity *) i->data;
		if (ref_ent == NULL)
		{
			continue;
		}
		ref_list = qof_class_get_referenceList (ref_ent->e_type);
		for (j = ref_list; j != NULL; j = j->next)
		{
			if (j->data == NULL)
			{
				continue;
			}
			ref_param = (QofParam *) j->data;
			child_ent = ref_param->param_getfcn (ref_ent, ref_param);
			if (child_ent != NULL)
			{
				qof_entity_copy_to_session (session, child_ent);
			}
		}
	}
}

gboolean
qof_entity_copy_coll_r (QofSession * new_session, QofCollection * coll)
{
	struct recurse_s store;
	gboolean success;

	if ((!new_session) || (!coll))
	{
		return FALSE;
	}
	store.session = new_session;
	success = TRUE;
	store.success = success;
	store.ent_list = NULL;
	store.ref_list =
		qof_class_get_referenceList (qof_collection_get_type (coll));
	success = qof_entity_copy_coll (new_session, coll);
	if (success)
	{
		qof_collection_foreach (coll, recurse_ent_cb, &store);
	}
	return success;
}

gboolean
qof_entity_copy_one_r (QofSession * new_session, QofEntity * ent)
{
	struct recurse_s store;
	QofCollection *coll;
	gboolean success;

	if ((!new_session) || (!ent))
	{
		return FALSE;
	}
	store.session = new_session;
	success = TRUE;
	store.success = success;
	store.ref_list = qof_class_get_referenceList (ent->e_type);
	success = qof_entity_copy_to_session (new_session, ent);
	if (success == TRUE)
	{
		coll =
			qof_book_get_collection (qof_session_get_book (new_session),
			ent->e_type);
		if (coll)
		{
			qof_collection_foreach (coll, recurse_ent_cb, &store);
		}
	}
	return success;
}


/* ============================================================== */

/** Programs that use their own backends also need to call
the default QOF ones. The backends specified here are
loaded only by applications that do not have their own. */
struct backend_providers
{
	const gchar *libdir;
	const gchar *filename;
	const gchar *init_fcn;
};

/* All available QOF backends need to be described here
and the last entry must be three NULL's.
Remember: Use the libdir from the current build environment
and use JUST the module name without .so - .so is not portable! */
struct backend_providers backend_list[] = {
	{QOF_LIB_DIR, QSF_BACKEND_LIB, QSF_MODULE_INIT},
	{QOF_LIB_DIR, "libqof-backend-sqlite", "qof_sqlite_provider_init"},
	{QOF_LIB_DIR, "libqof-backend-gda", "qof_gda_provider_init"},
#ifdef HAVE_DWI
	{QOF_LIB_DIR, "libqof_backend_dwi", "dwiend_provider_init"},
#endif
	{NULL, NULL, NULL}
};

static void
qof_session_load_backend (QofSession * session, gchar *access_method)
{
	GSList *p;
	GList *node;
	QofBackendProvider *prov;
	QofBook *book;
	gint num;
	gboolean prov_type;
	gboolean (*type_check) (const gchar *);

	ENTER (" list=%d", g_slist_length (provider_list));
	prov_type = FALSE;
	if (NULL == provider_list)
	{
		for (num = 0; backend_list[num].filename != NULL; num++)
		{
			if (!qof_load_backend_library (backend_list[num].libdir,
					backend_list[num].filename,
					backend_list[num].init_fcn))
			{
				PWARN (" failed to load %s from %s using %s",
					backend_list[num].filename, backend_list[num].libdir,
					backend_list[num].init_fcn);
			}
		}
	}
	p = g_slist_copy (provider_list);
	while (p != NULL)
	{
		prov = p->data;
		/* Does this provider handle the desired access method? */
		if (0 == strcasecmp (access_method, prov->access_method))
		{
			/* More than one backend could provide this
			   access method, check file type compatibility. */
			type_check =
				(gboolean (*)(const gchar *)) prov->check_data_type;
			prov_type = (type_check) (session->book_id);
			if (!prov_type)
			{
				PINFO (" %s not usable", prov->provider_name);
				p = p->next;
				continue;
			}
			PINFO (" selected %s", prov->provider_name);
			if (NULL == prov->backend_new)
			{
				p = p->next;
				continue;
			}
			/* Use the providers creation callback */
			session->backend = (*(prov->backend_new)) ();
			session->backend->provider = prov;
			/* Tell the books about the backend that they'll be using. */
			for (node = session->books; node; node = node->next)
			{
				book = node->data;
				qof_book_set_backend (book, session->backend);
			}
			LEAVE (" ");
			return;
		}
		p = p->next;
	}
	qof_error_clear (session);
	LEAVE (" ");
}

/* =============================================================== */

static void
qof_session_destroy_backend (QofSession * session)
{
	g_return_if_fail (session);

	if (session->backend)
	{
		/* Then destroy the backend */
		if (session->backend->destroy_backend)
		{
			session->backend->destroy_backend (session->backend);
		}
		else
		{
			g_free (session->backend);
		}
	}

	session->backend = NULL;
}

void
qof_session_begin (QofSession * session, const gchar *book_id,
	gboolean ignore_lock, gboolean create_if_nonexistent)
{
	gchar *p, *access_method;

	if (!session)
		return;

	ENTER (" sess=%p ignore_lock=%d, book-id=%s",
		session, ignore_lock, book_id ? book_id : "(null)");

	/* Clear the error condition of previous errors */
	qof_error_clear (session);

	/* Check to see if this session is already open */
	if (session->book_id)
	{
		qof_error_set (session, qof_error_register
		(_("This book appears to be open already."), FALSE));
		LEAVE (" push error book is already open ");
		return;
	}

	if (!book_id)
	{
		LEAVE (" using stdout");
		return;
	}

	/* Store the session URL  */
	session->book_id = g_strdup (book_id);

	/* destroy the old backend */
	qof_session_destroy_backend (session);

	/* Look for something of the form of "file:/", "http://" or 
	 * "postgres://". Everything before the colon is the access 
	 * method.  Load the first backend found for that access method.
	 */
	p = strchr (book_id, ':');
	if (p)
	{
		access_method = g_strdup (book_id);
		p = strchr (access_method, ':');
		*p = 0;
		qof_session_load_backend (session, access_method);
		g_free (access_method);
	}
	else
	{
		/* If no colon found, assume it must be a file-path */
		qof_session_load_backend (session, "file");
	}

	/* No backend was found. That's bad. */
	if (NULL == session->backend)
	{
		gchar * msg;

		msg = g_strdup_printf (_("Unable to locate a "
		"suitable backend for '%s' - please check "
		"your installation."), book_id);
		qof_error_set (session, qof_error_register
			(msg, FALSE));
		LEAVE (" BAD: no backend: sess=%p book-id=%s",
			session, book_id ? book_id : "(null)");
		g_free (msg);
		return;
	}

	/* If there's a begin method, call that. */
	if (session->backend->session_begin)
	{
		(session->backend->session_begin) (session->backend, session,
			session->book_id, ignore_lock, create_if_nonexistent);
		PINFO (" Done running session_begin on backend");
		if (qof_error_check(session) != QOF_SUCCESS)
		{
			g_free (session->book_id);
			session->book_id = NULL;
			LEAVE (" backend error ");
			return;
		}
	}
	qof_error_clear (session);
	LEAVE (" sess=%p book-id=%s", session, book_id ? book_id : "(null)");
}

/* ============================================================== */

void
qof_session_load (QofSession * session, QofPercentageFunc percentage_func)
{
	QofBook *newbook, *ob;
	QofBookList *oldbooks, *node;
	QofBackend *be;

	if (!session)
		return;
	if ((!session->book_id) ||
		(0 == safe_strcasecmp(session->book_id, QOF_STDOUT)))
		return;

	ENTER (" sess=%p book_id=%s", session, session->book_id
		? session->book_id : "(null)");

	/* At this point, we should are supposed to have a valid book 
	 * id and a lock on the file. */

	oldbooks = session->books;

	/* XXX why are we creating a book here? I think the books
	 * need to be handled by the backend ... especially since 
	 * the backend may need to load multiple books ... XXX. FIXME.
	 */
	newbook = qof_book_new ();
	session->books = g_list_append (NULL, newbook);
	PINFO (" new book=%p", newbook);

	qof_error_clear (session);

	/* This code should be sufficient to initialize *any* backend,
	 * whether http, postgres, or anything else that might come along.
	 * Basically, the idea is that by now, a backend has already been
	 * created & set up.  At this point, we only need to get the
	 * top-level account group out of the backend, and that is a
	 * generic, backend-independent operation.
	 */
	be = session->backend;
	qof_book_set_backend (newbook, be);

	/* Starting the session should result in a bunch of accounts
	 * and currencies being downloaded, but probably no transactions;
	 * The GUI will need to do a query for that.
	 */
	if (be)
	{
		be->percentage = percentage_func;

		if (be->load)
		{
			be->load (be, newbook);
		}
	}

	if (qof_error_check(session) != QOF_SUCCESS)
	{
		/* Something broke, put back the old stuff */
		qof_book_set_backend (newbook, NULL);
		qof_book_destroy (newbook);
		g_list_free (session->books);
		session->books = oldbooks;
		g_free (session->book_id);
		session->book_id = NULL;
		LEAVE (" error from backend ");
		return;
	}

	for (node = oldbooks; node; node = node->next)
	{
		ob = node->data;
		qof_book_set_backend (ob, NULL);
		qof_book_destroy (ob);
	}
	g_list_free (oldbooks);

	LEAVE (" sess = %p, book_id=%s", session, session->book_id
		? session->book_id : "(null)");
}

/* ============================================================= */

gboolean
qof_session_save_may_clobber_data (QofSession * session)
{
	if (!session)
		return FALSE;
	if (!session->backend)
		return FALSE;
	if (!session->backend->save_may_clobber_data)
		return FALSE;

	return (*(session->backend->save_may_clobber_data)) (session->backend);
}

void
qof_session_save (QofSession * session, 
				  QofPercentageFunc percentage_func)
{
	GList *node;
	QofBackend *be;
	gboolean partial, change_backend;
	QofBackendProvider *prov;
	GSList *p;
	QofBook *book, *abook;
	gint num;
	gchar *msg, *book_id;

	if (!session)
		return;
	ENTER (" sess=%p book_id=%s",
		session, session->book_id ? session->book_id : "(null)");
	/* Partial book handling. */
	book = qof_session_get_book (session);
	partial =
		(gboolean)
		GPOINTER_TO_INT (qof_book_get_data (book, PARTIAL_QOFBOOK));
	change_backend = FALSE;
	msg = g_strdup_printf (" ");
	book_id = g_strdup (session->book_id);
	if (partial == TRUE)
	{
		if (session->backend && session->backend->provider)
		{
			prov = session->backend->provider;
			if (TRUE == prov->partial_book_supported)
			{
				/* if current backend supports partial, leave alone. */
				change_backend = FALSE;
			}
			else
			{
				change_backend = TRUE;
			}
		}
		/* If provider is undefined, assume partial not supported. */
		else
		{
			change_backend = TRUE;
		}
	}
	if (change_backend == TRUE)
	{
		qof_session_destroy_backend (session);
		if (NULL == provider_list)
		{
			for (num = 0; backend_list[num].filename != NULL; num++)
			{
				qof_load_backend_library (backend_list[num].libdir,
					backend_list[num].filename,
					backend_list[num].init_fcn);
			}
		}
		p = g_slist_copy (provider_list);
		while (p != NULL)
		{
			prov = p->data;
			if (TRUE == prov->partial_book_supported)
			{
			/** \todo check the access_method too, not in scope here, yet. */
				/*  if((TRUE == prov->partial_book_supported) && 
				   (0 == strcasecmp (access_method, prov->access_method)))
				   { */
				if (NULL == prov->backend_new)
					continue;
				/* Use the providers creation callback */
				session->backend = (*(prov->backend_new)) ();
				session->backend->provider = prov;
				if (session->backend->session_begin)
				{
					/* Call begin - backend has been changed,
					   so make sure a file can be written,
					   use ignore_lock and create_if_nonexistent */
					g_free (session->book_id);
					session->book_id = NULL;
					(session->backend->session_begin) (session->backend,
						session, book_id, TRUE, TRUE);
					PINFO
						(" Done running session_begin on changed backend");
					if (qof_error_check (session) != QOF_SUCCESS)
					{
						g_free (session->book_id);
						session->book_id = NULL;
						LEAVE (" changed backend error");
						return;
					}
				}
				/* Tell the books about the backend that they'll be using. */
				for (node = session->books; node; node = node->next)
				{
					book = node->data;
					qof_book_set_backend (book, session->backend);
				}
				p = NULL;
			}
			if (p)
			{
				p = p->next;
			}
		}
		if (!session->backend)
		{
			msg = g_strdup_printf (" failed to load backend");
			qof_error_set (session, qof_error_register
			(_("Failed to load backend, no suitable handler."), 
			FALSE));
			return;
		}
	}
	/* If there is a backend, and the backend is reachable
	 * (i.e. we can communicate with it), then synchronize with 
	 * the backend.  If we cannot contact the backend (e.g.
	 * because we've gone offline, the network has crashed, etc.)
	 * then give the user the option to save to the local disk. 
	 *
	 * hack alert -- FIXME -- XXX the code below no longer
	 * does what the words above say.  This needs fixing.
	 */
	be = session->backend;
	if (be)
	{
		for (node = session->books; node; node = node->next)
		{
			abook = node->data;
			/* if invoked as SaveAs(), then backend not yet set */
			qof_book_set_backend (abook, be);
			be->percentage = percentage_func;
			if (be->sync)
				(be->sync) (be, abook);
		}
		/* If we got to here, then the backend saved everything 
		 * just fine, and we are done. So return. */
		/* Return the book_id to previous value. */
		qof_error_clear (session);
		LEAVE (" Success");
		return;
	}
	else
	{
		msg = g_strdup_printf (" failed to load backend");
		qof_error_set (session, qof_error_register
			(_("Failed to load backend, no suitable handler."), 
			FALSE));
	}
	LEAVE (" error -- No backend!");
}

/* ============================================================= */

void
qof_session_end (QofSession * session)
{
	if (!session)
		return;

	ENTER (" sess=%p book_id=%s", session, session->book_id
		? session->book_id : "(null)");

	/* close down the backend first */
	if (session->backend && session->backend->session_end)
	{
		(session->backend->session_end) (session->backend);
	}

	qof_error_clear (session);

	g_free (session->book_id);
	session->book_id = NULL;

	LEAVE (" sess=%p book_id=%s", session, session->book_id
		? session->book_id : "(null)");
}

void
qof_session_destroy (QofSession * session)
{
	GList *node;
	if (!session)
		return;

	ENTER (" sess=%p book_id=%s", session, session->book_id
		? session->book_id : "(null)");

	qof_session_end (session);

	/* destroy the backend */
	qof_session_destroy_backend (session);

	for (node = session->books; node; node = node->next)
	{
		QofBook *book = node->data;
		qof_book_set_backend (book, NULL);
		qof_book_destroy (book);
	}

	session->books = NULL;
#ifndef QOF_DISABLE_DEPRECATED
	if (session == qof_session_get_current_session())
		qof_session_clear_current_session();
#endif
	g_free (session);
	qof_error_close ();

	LEAVE (" sess=%p", session);
}

/* ============================================================= */

void
qof_session_swap_data (QofSession * session_1, QofSession * session_2)
{
	GList *books_1, *books_2, *node;

	if (session_1 == session_2)
		return;
	if (!session_1 || !session_2)
		return;

	ENTER (" sess1=%p sess2=%p", session_1, session_2);

	books_1 = session_1->books;
	books_2 = session_2->books;

	session_1->books = books_2;
	session_2->books = books_1;

	for (node = books_1; node; node = node->next)
	{
		QofBook *book_1 = node->data;
		qof_book_set_backend (book_1, session_2->backend);
	}
	for (node = books_2; node; node = node->next)
	{
		QofBook *book_2 = node->data;
		qof_book_set_backend (book_2, session_1->backend);
	}

	LEAVE (" ");
}

/* ============================================================= */

gboolean
qof_session_events_pending (QofSession * session)
{
	if (!session)
		return FALSE;
	if (!session->backend)
		return FALSE;
	if (!session->backend->events_pending)
		return FALSE;

	return session->backend->events_pending (session->backend);
}

gboolean
qof_session_process_events (QofSession * session)
{
	if (!session)
		return FALSE;
	if (!session->backend)
		return FALSE;
	if (!session->backend->process_events)
		return FALSE;

	return session->backend->process_events (session->backend);
}

/* ============== END OF FILE ========================== */
