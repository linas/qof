/***************************************************************************
 *            test-object.c
 *
 *  Copyright  2004  Linas Vepstas <linas@linas.org>
 *  Copyright  2008  Neil Williams <linux@codehelp.co.uk>
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

/*
 * test the QofObject infrastructure with static and dynamic objects.
 */
#include <glib.h>
#include "qof.h"
#include "test-stuff.h"

#define TEST_MODULE_NAME "object-test"
#define TEST_MODULE_DESC "Test Object"
#define DYNAMIC_MOD_NAME "dynamic_test"
#define DYNAMIC_MOD_DESC "Full test of adding arbitrary objects"

static void obj_foreach (QofCollection *, QofEntityForeachCB, gpointer);
static const gchar *printable (gpointer obj);
static void test_printable (const gchar *name, gpointer obj);
static void test_foreach (QofBook *, const gchar *);

static QofObject bus_obj = {
	.interface_version = QOF_OBJECT_VERSION,
	.e_type = TEST_MODULE_NAME,
	.type_label = TEST_MODULE_DESC,
	.create = NULL,
	.book_begin = NULL,
	.book_end = NULL,
	.is_dirty = NULL,
	.mark_clean = NULL,
	.foreach = obj_foreach,
	.printable = printable,
	.version_cmp = NULL,
};

static G_GNUC_UNUSED const gchar *
test_dyn_printable (gpointer obj)
{
	/* actual dynamic objects can call any function here */
	return "test";
}

static QofObject *
dyn_create (QofBook * book)
{
	QofInstance * inst;
	QofCollection *coll;
	GList *all;

	g_return_val_if_fail (book, NULL);
	inst = g_new0 (QofInstance, 1);
	qof_instance_init (inst, "dynamic_test", book);
	coll = qof_book_get_collection (book, "dynamic_test");
	all = qof_collection_get_data (coll);
	all = g_list_prepend (all, inst);
	qof_collection_set_data (coll, all);
	return (QofObject*)inst;
}

/* pointer to an array of dynamically allocated parameters */
static QofParam * p_list;

static gchar *
dynamic_get_string (QofEntity * ent)
{
	/* actual dynamic objects can call any function here */
	do_test (!safe_strcmp (DYNAMIC_MOD_NAME, ent->e_type), "e_type check for string");
	return "test_string";
}

static gint
dynamic_get_int (QofEntity * ent)
{
	/* actual dynamic objects can call any function here */
	do_test (!safe_strcmp (DYNAMIC_MOD_NAME, ent->e_type), "e_type check for int");
	return 1;
}

static gboolean
dynamic_get_boolean (QofEntity * ent)
{
	/* actual dynamic objects can call any function here */
	do_test (!safe_strcmp (DYNAMIC_MOD_NAME, ent->e_type), "e_type check for int");
	return TRUE;
}

static QofParam *
add_boolean_param (void)
{
	QofParam * p;

	p = g_new0 (QofParam, 1);
	p->param_name = "test_boolean";
	p->param_type = QOF_TYPE_BOOLEAN;
	p->param_getfcn = (QofAccessFunc)dynamic_get_boolean;
	return p;
}

static gboolean
test_boolean_param (QofEntity * ent, const QofParam * p)
{
	gboolean b, (*boolean_getter) (QofEntity *, QofParam *);
	/* actual dynamic objects can call any function here */
	do_test (!safe_strcmp (DYNAMIC_MOD_NAME, ent->e_type), "e_type check for bool");
	boolean_getter = (gboolean (*)(QofEntity *, QofParam *)) p->param_getfcn;
	b = boolean_getter (ent, (QofParam*)p);
	return b;
}

static const QofParam *
test_class_register (void)
{
	QofParam * p;
	/* the parameter list needs to be of constant storage size
	and big enough for all dynamic objects. Registration stops
	at the first NULL parameter. */
	static QofParam list[30];

	p = g_new0 (QofParam, 1);
	p->param_name = "test_string";
	p->param_type = QOF_TYPE_STRING;
	p->param_getfcn = (QofAccessFunc)dynamic_get_string;
	list[0] = *p;
	p = g_new0 (QofParam, 1);
	p->param_name = "test_int";
	p->param_type = QOF_TYPE_INT32;
	p->param_getfcn = (QofAccessFunc)dynamic_get_int;
	list[1] = *p;
	list[2] = *add_boolean_param ();
	/* create the terminating NULL */
	p = g_new0 (QofParam, 1);
	list[3] = *p;
	p_list = list;
	return p_list;
}

static void
test_dynamic_object (void)
{
	QofObject * dynamic;
	QofInstance * d_ent;
	const QofObject * check;
	const gchar * message;
	gchar * s;
	gint t, (*int32_getter) (QofEntity *, QofParam *);
	const QofParam * p;
	QofBook *book = qof_book_new ();

	do_test ((NULL != book), "book null");
	dynamic = g_new0(QofObject,1);
	dynamic->interface_version = QOF_OBJECT_VERSION,
	dynamic->e_type = DYNAMIC_MOD_NAME;
	dynamic->type_label = DYNAMIC_MOD_DESC;
	dynamic->foreach = obj_foreach;
	dynamic->create = (gpointer) dyn_create;
	dynamic->printable = test_dyn_printable;
	do_test (qof_object_register (dynamic), "dynamic object registration");
	check = qof_object_lookup (DYNAMIC_MOD_NAME);
	do_test (check != NULL, "dynamic object lookup");
	message = qof_object_get_type_label (DYNAMIC_MOD_NAME);
	do_test (!safe_strcmp(message, "Full test of adding arbitrary objects"), 
		"dynamic object type_label");
	d_ent = qof_object_new_instance (DYNAMIC_MOD_NAME, book);
	do_test (check->printable != NULL, "dynamic printable support");
	message = qof_object_printable (DYNAMIC_MOD_NAME, dynamic);
	do_test (message != NULL, "dynamic object printable");
	message = dynamic->printable(d_ent);
	do_test (message != NULL, "dynamic direct printable");
	qof_class_register(DYNAMIC_MOD_NAME, NULL, test_class_register());
	do_test (qof_class_is_registered (DYNAMIC_MOD_NAME), "class register");
	s = NULL;
	p = qof_class_get_parameter (DYNAMIC_MOD_NAME, "test_string");
	s = p->param_getfcn (d_ent, p);
	do_test (!safe_strcmp(s, "test_string"), "get string from dynamic object");
	t = 0;
	p = qof_class_get_parameter (DYNAMIC_MOD_NAME, "test_int");
	int32_getter = (gint32 (*)(QofEntity *, QofParam *)) p->param_getfcn;
	t = int32_getter ((QofEntity*)d_ent, (QofParam*)p);
	do_test (t == 1, "get int from dynamic object");
	p = qof_class_get_parameter (DYNAMIC_MOD_NAME, "test_boolean");
	do_test (test_boolean_param((QofEntity*)d_ent, p), 
		"get boolean from dynamic object");
}

static void
test_object (void)
{
	QofBook *book = qof_book_new ();

	do_test ((NULL != book), "book null");

	/* Test the global registration and lookup functions */
	{
		do_test (!qof_object_register (NULL), "register NULL");
		do_test (qof_object_register (&bus_obj), "register test object");
		do_test (!qof_object_register (&bus_obj),
				 "register test object again");
		do_test (qof_object_lookup (TEST_MODULE_NAME) == &bus_obj,
				 "lookup our installed object");
		do_test (qof_object_lookup ("snm98sn snml say  dyikh9y9ha") == NULL,
				 "lookup non-existant object object");

		do_test (!safe_strcmp (qof_object_get_type_label (TEST_MODULE_NAME),
							   (TEST_MODULE_DESC)),
				 "test description return");
	}

	test_foreach (book, TEST_MODULE_NAME);
	test_printable (TEST_MODULE_NAME, (gpointer) 1);
}

static void
obj_foreach (QofCollection * col, 
	QofEntityForeachCB cb __attribute__ ((unused)), gpointer u_d)
{
	int *foo = u_d;

	do_test (col != NULL, "foreach: NULL collection");
	success ("called foreach callback");

	*foo = 1;
}

static void
foreachCB (QofEntity * ent __attribute__ ((unused)), 
		gpointer u_d __attribute__ ((unused)))
{
	do_test (FALSE, "FAIL");
}

static const char *
printable (gpointer obj)
{
	do_test (obj != NULL, "printable: object is NULL");
	success ("called printable callback");
	return ((const char *) obj);
}

static void
test_foreach (QofBook * book, const char *name)
{
	int res = 0;

	qof_object_foreach (NULL, NULL, NULL, &res);
	do_test (res == 0, "object: Foreach: NULL, NULL, NULL");
	qof_object_foreach (NULL, NULL, foreachCB, &res);
	do_test (res == 0, "object: Foreach: NULL, NULL, foreachCB");

	qof_object_foreach (NULL, book, NULL, &res);
	do_test (res == 0, "object: Foreach: NULL, book, NULL");
	qof_object_foreach (NULL, book, foreachCB, &res);
	do_test (res == 0, "object: Foreach: NULL, book, foreachCB");

	qof_object_foreach (name, NULL, NULL, &res);
	do_test (res == 0, "object: Foreach: name, NULL, NULL");
	qof_object_foreach (name, NULL, foreachCB, &res);
	do_test (res == 0, "object: Foreach: name, NULL, foreachCB");

	qof_object_foreach (name, book, NULL, &res);
	do_test (res != 0, "object: Foreach: name, book, NULL");

	res = 0;
	qof_object_foreach (name, book, foreachCB, &res);
	do_test (res != 0, "object: Foreach: name, book, foreachCB");
}

static void
test_printable (const char *name, gpointer obj)
{
	const char *res;

	do_test (qof_object_printable (NULL, NULL) == NULL,
			 "object: Printable: NULL, NULL");
	do_test (qof_object_printable (NULL, obj) == NULL,
			 "object: Printable: NULL, object");
	do_test (qof_object_printable (name, NULL) == NULL,
			 "object: Printable: mod_name, NULL");
	res = qof_object_printable (name, obj);
	do_test (res != NULL, "object: Printable: mod_name, object");
}

int
main (void)
{
	qof_init ();
	test_object ();
	test_dynamic_object ();
	print_test_results ();
	qof_close ();
	return get_rv ();
}
