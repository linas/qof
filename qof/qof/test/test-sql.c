/***************************************************************************
 *            test-sql.c
 *
 *  Copyright  2008  Neil Williams <linux@codehelp.co.uk>
 ****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
/*
 * lightly test the QofSql conversions.
 *
 * The problem with more intensive testing is that checking the syntax
 * of the returned strings requires complex SQL parsing and not all
 * checks are currently implemented in QOF.
 */
#include <glib.h>
#include "qof.h"
#include "qofsql-p.h"
#include "test-stuff.h"

/** \bug need to enforce no - in names */
#define TEST_MODULE_NAME "object_test"
#define TEST_MODULE_DESC "Test Object"
#define OBJ_AMOUNT "anamount"
#define OBJ_KVP "kvp"

/* simple object structure */
typedef struct d_obj
{
	QofInstance inst;
	gchar *Name;
	gchar flag;
	QofNumeric Amount;
	QofTime *date;
	gdouble discount;			/* cheap pun, I know. */
	gboolean active;
	gint32 version;
	gint64 minor;
} dyn_obj;

static dyn_obj *
dyn_create (QofBook * book)
{
	dyn_obj * d;

	g_return_val_if_fail (book, NULL);
	d = g_new0 (dyn_obj, 1);
	qof_instance_init (&d->inst, TEST_MODULE_NAME, book);
	return d;
}

static void
dyn_setAmount (dyn_obj * e, QofNumeric h)
{
	if (!e)
		return;
	e->Amount = h;
}

static QofNumeric
dyn_getAmount (dyn_obj * e)
{
	if (!e)
		return qof_numeric_zero ();
	return e->Amount;
}

static G_GNUC_UNUSED const gchar *
test_dyn_printable (gpointer obj)
{
	return "test";
}

static QofObject bus_obj = {
	.interface_version = QOF_OBJECT_VERSION,
	.e_type = TEST_MODULE_NAME,
	.type_label = TEST_MODULE_DESC,
	.create = (gpointer)dyn_create,
	.book_begin = NULL,
	.book_end = NULL,
	.is_dirty = qof_collection_is_dirty,
	.mark_clean = qof_collection_mark_clean,
	.foreach = qof_collection_foreach,
	.printable = test_dyn_printable,
	.version_cmp = (gint (*)(gpointer, gpointer)) qof_instance_version_cmp,
};

static gboolean
dyn_objRegister (void)
{
	static QofParam params[] = {
		{OBJ_AMOUNT, QOF_TYPE_NUMERIC, (QofAccessFunc) dyn_getAmount,
		 (QofSetterFunc) dyn_setAmount, NULL},
		{OBJ_KVP, QOF_TYPE_KVP, (QofAccessFunc) qof_instance_get_slots,
			NULL, NULL},
		{QOF_PARAM_BOOK, QOF_ID_BOOK, (QofAccessFunc) qof_instance_get_book,
		 NULL, NULL},
		{QOF_PARAM_GUID, QOF_TYPE_GUID, (QofAccessFunc) qof_instance_get_guid,
		 NULL, NULL},
		{NULL, NULL, NULL, NULL, NULL},
	};

	qof_class_register (TEST_MODULE_NAME, NULL, params);

	return qof_object_register (&bus_obj);
}

static void
dyn_foreach (QofParam * param, gpointer user_data)
{
	do_test (param != NULL, "Fail");
}

static void
dyn_foreach2 (QofEntity * ent, gpointer user_data)
{
	do_test (ent != NULL, "Fail");
}

static void
test_sql (void)
{
	QofBook *book;
	KvpFrame * slots;
	const QofParam * param;
	QofEntity * ent;
	QofInstance * inst;
	QofCollection * col;
	gchar * sql_str, * gstr, * test, * err;

	sql_str = NULL;
	book = qof_book_new ();
	do_test ((NULL != book), "book null");
	do_test (dyn_objRegister() == TRUE, "register test object");
	do_test (qof_object_lookup (TEST_MODULE_NAME) == &bus_obj,
				"lookup our installed object");
	do_test (qof_class_is_registered(TEST_MODULE_NAME) == TRUE, "class registration");
	inst = qof_object_new_instance (TEST_MODULE_NAME, book);
	do_test (inst != NULL, "object new instance");
	g_return_if_fail (inst);
	/* create some slots */
	slots = qof_instance_get_slots (inst);
	do_test (slots != NULL, "creating some slots");
	kvp_frame_set_string (qof_instance_get_slots (inst), "debug/test/string", "sdgsrsafsg");
	test = kvp_frame_get_string (qof_instance_get_slots (inst), "debug/test/string");
	err = g_strdup_printf ("compare slots: %s", test);
	do_test (0 == safe_strcasecmp (test, "sdgsrsafsg"), err);
	g_free (test);
	g_free (err);
	ent = (QofEntity*)inst;
	do_test (ent != NULL, "convert to entity");
	qof_class_param_foreach (ent->e_type, dyn_foreach, NULL);
	col = qof_book_get_collection (book, ent->e_type);
	qof_collection_foreach (col, dyn_foreach2, NULL);
	/* test CREATE TABLE */
	sql_str = qof_sql_entity_create_table (ent);
	do_test (0 == safe_strcasecmp (sql_str, 
		"CREATE TABLE object_test ( guid char(32) primary key not null, "
		"anamount text, dbversion int ); CREATE TABLE sql_kvp (kvp_id int "
		"primary key not null, guid char(32), path mediumtext, type mediumtext, "
		"value text,  dbversion int );"),
			 g_strdup_printf ("Create table SQL statement:%s:%s", sql_str, test));
	g_free (sql_str);
	/* test INSERT */
	sql_str = qof_sql_entity_insert (ent);
	gstr = g_strnfill (GUID_ENCODING_LENGTH + 1, ' ');
	guid_to_string_buff (qof_instance_get_guid (inst), gstr);
	test = g_strdup_printf ("INSERT into object_test (guid , anamount) VALUES "
		"('%s' , '0/0'); INSERT into sql_kvp  (kvp_id '%s',  dbversion int );"
		, gstr, gstr);
	err = g_strdup_printf ("Insert entity SQL statement:%s:%s", sql_str, test);
	do_test (0 == safe_strcasecmp (sql_str, test),err);
	g_free (test);
	g_free (err);
	g_free (sql_str);
	/* test UPDATE */
	param = qof_class_get_parameter (TEST_MODULE_NAME, OBJ_AMOUNT);
	/* pretend we are using qof_util_param_edit so that we don't need a backend */
	inst->param = param;
	do_test (param != NULL, "no OBJ_AMOUNT parameter");
	sql_str = qof_sql_entity_update (ent);
	do_test (sql_str != NULL, "failed to mark instance as dirty");
	test = g_strdup_printf ("UPDATE object_test SET anamount = '0/0' WHERE "
		"guid='%s'; UPDATE sql_kvp SET  (kvp_id '%s',  kvp key=string "
		"val=sql_kvp type=string dbversion int );", gstr, gstr);
	err = g_strdup_printf ("Update entity SQL statement: %s", sql_str);
	do_test (0 == safe_strcasecmp (sql_str, test), err);
	g_free (test);
	g_free (err);
	g_free (sql_str);
	/* test update list */
	/// \bug add test support for qof_sql_entity_update_list
	/* test DELETE */
	sql_str = qof_sql_entity_delete (ent);
	test = g_strdup_printf ("DELETE from object_test WHERE guid='%s';", gstr);
	err = g_strdup_printf ("DELETE entity SQL statement: %s", sql_str);
	do_test (0 == safe_strcasecmp (sql_str, test), err);
	g_free (test);
	g_free (err);
	g_free (sql_str);
	sql_str = qof_sql_entity_drop_table (ent);
	/* test DROP TABLE */
	err = g_strdup_printf ("DROP TABLE SQL statement: %s", sql_str);
	do_test (0 == safe_strcasecmp (sql_str, "DROP TABLE object_test;"), err);
	g_free (err);
	g_free (sql_str);
}

int
main (void)
{
	qof_init ();
	test_sql ();
	print_test_results ();
	qof_close ();
	return get_rv ();
}
