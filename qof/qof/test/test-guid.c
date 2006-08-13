/***************************************************************************
 *            test-guid.c
 *
 *  Test file created by Linas Vepstas <linas@linas.org>
 *  Try to create duplicate GUID's, which should never happen.
 *  October 2003
 *  Copyright  2003  Linas Vepstas <linas@linas.org>
 ****************************************************************************/
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA  02110-1301,  USA
 */

#include <ctype.h>
#include <glib.h>
#include "qof.h"
#include "test-stuff.h"
#include "test-engine-stuff.h"
#include "qofbook.h"
#include "qofid.h"
#include "qofid-p.h"
#include "qofsession.h"
#include "guid.h"

static void
test_null_guid (void)
{
	GUID g;
	GUID *gp;

	g = guid_new_return ();
	gp = guid_malloc ();
	guid_new (gp);

	do_test (guid_equal (guid_null (), guid_null ()), "null guids equal");
	do_test (!guid_equal (&g, gp), "two guids equal");
}

static void
run_test (void)
{
	int i;
	QofSession *sess;
	QofBook *book;
	QofEntity *eblk;
	QofCollection *col;
	QofIdType type;

	sess = qof_session_new ();
	book = qof_session_get_book (sess);
	do_test ((NULL != book), "book not created");

	col = qof_book_get_collection (book, "asdf");
	type = qof_collection_get_type (col);

#define NENT 500123
	eblk = g_new0 (QofEntity, NENT);
	for (i = 0; i < NENT; i++)
	{
		QofEntity *ent = &eblk[i];
		guid_new (&ent->guid);
		do_test ((NULL == qof_collection_lookup_entity (col, &ent->guid)),
				 "duplicate guid");
		ent->e_type = type;
		qof_collection_insert_entity (col, ent);
	}

	/* Make valgrind happy -- destroy the session. */
	qof_session_destroy (sess);
}

int
main (int argc __attribute__ ((unused)), char **argv __attribute__ ((unused)))
{
	guid_init ();
	g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);

	test_null_guid ();
	run_test ();

	print_test_results ();
	exit (get_rv ());
	guid_shutdown ();
	return get_rv();
}
