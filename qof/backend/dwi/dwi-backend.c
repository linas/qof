
#include <qof/qofbackend.h>
#include <qof/qofbackend-p.h>
#include <qof/gnc-trace.h>

static short module = MOD_BACKEND;

struct DwiBackend_s 
{
	QofBackend be;
};

typedef struct DwiBackend_s DwiBackend;

/* ============================================================= */

static void 
dwiend_init (DwiBackend *be)
{
	qof_backend_init((QofBackend*)be);
}

/* ============================================================= */

static QofBackend *
dwiend_new (void)
{
	DwiBackend *be;

	ENTER(" ");
	be = g_new0 (DwiBackend, 1);

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
}

/* ======================== END OF FILE ========================= */
