
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

void dwiend_init (DwiBackend *be)
{
	qof_backend_init((QofBackend*)be);
}

/* ============================================================= */

QofBackend *
dwiend_new (void)
{
	DwiBackend *be;

	ENTER(" ");
	be = g_new0 (DwiBackend, 1);
	dwiend_init (be);

	LEAVE(" ")
	return (QofBackend *) be;
}

/* ======================== END OF FILE ========================= */
