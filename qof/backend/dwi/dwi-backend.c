/********************************************************************\
 * dwi-backend.c -- DWI backend module for QOF                      *
 * Copyright (C) 2004 Linas Vepstas <linas@linas.org>               *
 * Copyright (C) 2008 Neil Williams <linux@codehelp.co.uk>          *
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
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
\********************************************************************/

#include <dwi/database.h>
#include <qof.h>
#include "qofmap.h"

static QofLogModule log_module = DWI_BACKEND;

struct DwiBackend_s 
{
	QofBackend be;
	DuiDatabase *db;
};

typedef struct DwiBackend_s DwiBackend;

/* ============================================================= */

static void
dwiend_load_foundation (QofBackend *qbe,  QofBook *book)
{
	DwiBackend *be = (DwiBackend *) qbe;
	PINFO ("load foundation %p\n", be);
}

static void
dwiend_run_query (QofBackend *qbe, gpointer qry)
{
	DwiBackend *be = (DwiBackend *) qbe;
	PINFO ("giddy yap %p run query %p\n", be, qry);
}

gpointer 
dwiend_compile_query (QofBackend *qbe, QofQuery *qry)
{
	DwiBackend *be = (DwiBackend *) qbe;
	PINFO ("howdy partner %p compile query\n", be);
	return (gpointer)0x1234;
}

/* ============================================================= */

static void
dwiend_session_begin (QofBackend *qbe,
                      QofSession *session,
                      const gchar *book_id,
                      gboolean ignore_lock,
                      gboolean create_if_nonexistent)
{
	DwiBackend *be = (DwiBackend *) qbe;

	PINFO ("duude ses start wi/ id=%s\n", book_id);

	be->db = dui_database_new ("my db object",
	                          // "libdbi", "my-qof-db",
	                          // "odbc", "my-qof-db",
	                          "libpg", "my-qof-db",
	                          // "127.0.0.1", "linas", "abc123");
	                          NULL, "linas", NULL);
}

/* ============================================================= */

static void 
dwiend_init (DwiBackend *be)
{
	qof_backend_init (&be->be);
	be->db = NULL;
	be->be.session_begin = dwiend_session_begin;
	be->be.load = dwiend_load_foundation;
	be->be.compile_query = dwiend_compile_query;
	be->be.run_query = dwiend_run_query;
}

/* ============================================================= */

static QofBackend *
dwiend_new (void)
{
	DwiBackend *be;

	ENTER(" ");
	be = g_new0 (DwiBackend, 1);
	dwiend_init (be);

	LEAVE(" ");
	return (QofBackend *) be;
}

/* ============================================================= */

static void 
dwiend_provider_free (QofBackendProvider *prov)
{
	prov->provider_name = NULL;
	prov->access_method = NULL;
	g_free (prov);
}

/* ============================================================= */

void 
dwiend_provider_init (void)
{
	QofBackendProvider *prov;
	prov = g_new0 (QofBackendProvider, 1);
	prov->provider_name = "DWI Backend Version 0.1";
	prov->access_method = "sql";
	prov->backend_new = dwiend_new;
	prov->provider_free = dwiend_provider_free;
	qof_backend_register_provider (prov);
}

/* ======================== END OF FILE ========================= */
