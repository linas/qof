
#include <dwi/database.h>
#include <qof/qofbackend.h>
#include <qof/qofbackend-p.h>
#include <qof/gnc-trace.h>

static short module = MOD_BACKEND;

struct DwiBackend_s 
{
	QofBackend be;
	DuiDatabase *db;
};

typedef struct DwiBackend_s DwiBackend;

/* ============================================================= */

static void
dwiend_session_begin (QofBackend *qbe,
                      QofSession *session,
                      const char *book_id,
                      gboolean ignore_lock,
                      gboolean create_if_nonexistent)
{
	DwiBackend *be = (DwiBackend *) qbe;

	printf ("duude ses start wi/ id=%s\n", book_id);

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
}

/* ============================================================= */

static QofBackend *
dwiend_new (void)
{
	DwiBackend *be;

	ENTER(" ");
	be = g_new0 (DwiBackend, 1);
	dwiend_init (be);

	LEAVE(" ")
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
