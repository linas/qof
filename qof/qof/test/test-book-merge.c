/*********************************************************************
 * test-book-merge.c -- test implementation api for QoFBook merge    *
 * Copyright (C) 2004-2005 Neil Williams <linux@codehelp.co.uk>      *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License as    *
 * published by the Free Software Foundation; either version 2 of    *
 * the License, or (at your option) any later version.               *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License for more details.                      *
 *                                                                   *
 * You should have received a copy of the GNU General Public License *
 * along with this program; if not, contact:                         *
 *                                                                   *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
 *                                                                   *
 ********************************************************************/
 /* Test the qof_book_merge infrastructure.  */

#include <glib.h>
#include "qof.h"
#include "qofinstance-p.h"
#include "qofevent-p.h"
#include "test-stuff.h"

#define TEST_MODULE_NAME "book-merge-test"
#define TEST_MODULE_DESC "Test Book Merge"
#define OBJ_NAME "somename"
#define OBJ_AMOUNT "anamount"
#define OBJ_DATE "nottoday"
#define OBJ_GUID "unique"
#define OBJ_DISCOUNT "hefty"
#define OBJ_VERSION "early"
#define OBJ_MINOR "tiny"
#define OBJ_ACTIVE "ofcourse"
#define OBJ_FLAG   "tiny_flag"

static void test_rule_loop (QofBookMergeData *, QofBookMergeRule *,
							guint);
static void test_merge (void);
gboolean myobjRegister (void);
#ifdef TEST_DEBUG
static QofLogModule log_module = QOF_MOD_MERGE;
#endif

/* simple object structure */
typedef struct obj_s
{
	QofInstance inst;
	gchar *Name;
	gchar flag;
	QofNumeric Amount;
	const GUID *obj_guid;
	QofTime *date;
	gdouble discount;			/* cheap pun, I know. */
	gboolean active;
	gint32 version;
	gint64 minor;
} myobj;

static void
obj_setGUID (myobj * g, const GUID * h)
{
	if (!g)
		return;
	g->obj_guid = h;
}

static myobj *
obj_create (QofBook * book)
{
	myobj *g;
	g_return_val_if_fail (book, NULL);
	g = g_new (myobj, 1);
	qof_instance_init (&g->inst, TEST_MODULE_NAME, book);
	obj_setGUID (g, qof_instance_get_guid (&g->inst));
	g->discount = 0;
	g->active = TRUE;
	g->version = 1;
	g->minor = 1;
	g->flag = 'n';
	qof_event_gen (&g->inst.entity, QOF_EVENT_CREATE, NULL);
	return g;
}

static void
obj_setFlag (myobj * g, char f)
{
	g_return_if_fail (g);
	g->flag = f;
}

static gchar
obj_getFlag (myobj * g)
{
	g_return_val_if_fail (g, 'n');
	return g->flag;
}

static void
obj_setMinor (myobj * g, gint64 h)
{
	g_return_if_fail (g != NULL);
	g->minor = h;
}

static gint64
obj_getMinor (myobj * g)
{
	g_return_val_if_fail ((g != NULL), 0);
	return g->minor;
}

static void
obj_setVersion (myobj * g, gint32 h)
{
	g_return_if_fail (g != NULL);
	g->version = h;
}

static gint32
obj_getVersion (myobj * g)
{
	if (!g)
		return 0;
	return g->version;
}

static void
obj_setActive (myobj * g, gboolean h)
{
	if (!g)
		return;
	g->active = h;
}

static gboolean
obj_getActive (myobj * g)
{
	if (!g)
		return FALSE;
	return g->active;
}

static void
obj_setDiscount (myobj * g, gdouble h)
{
	if (!g)
		return;
	g->discount = h;
}

static gdouble
obj_getDiscount (myobj * g)
{
	if (!g)
		return 0;
	return g->discount;
}

static void
obj_setDate (myobj * g, QofTime *h)
{
	if (!g)
		return;
	do_test ((h != NULL), "passed a NULL time");
	do_test ((qof_time_is_valid (h) == TRUE), 
		"passed an invalid time");
	g->date = h;
}

static QofTime *
obj_getDate (myobj * g)
{
	if (!g)
		return NULL;
	do_test ((g->date != NULL), "stored time is NULL");
	do_test ((qof_time_is_valid (g->date) == TRUE), 
		"stored time is invalid");
	return g->date;
}

static const GUID *
obj_getGUID (myobj * g)
{
	if (!g)
		return NULL;
	return g->obj_guid;
}

static void
obj_setName (myobj * g, char *h)
{
	if (!g || !h)
		return;
	g->Name = strdup (h);
}

static gchar *
obj_getName (myobj * g)
{
	if (!g)
		return NULL;
	return g->Name;
}

static void
obj_setAmount (myobj * g, QofNumeric h)
{
	if (!g)
		return;
	g->Amount = h;
}

static QofNumeric
obj_getAmount (myobj * g)
{
	if (!g)
		return qof_numeric_zero ();
	return g->Amount;
}

static QofObject obj_object_def = {
  .interface_version = QOF_OBJECT_VERSION,
  .e_type = TEST_MODULE_NAME,
  .type_label = TEST_MODULE_DESC,
  .create = (gpointer) obj_create,
  .book_begin = NULL,
  .book_end = NULL,
  .is_dirty = NULL,
  .mark_clean = NULL,
  .foreach = qof_collection_foreach,
  .printable = NULL,
  .version_cmp = (gint (*)(gpointer, gpointer)) 
				qof_instance_version_cmp,
};

gboolean
myobjRegister (void)
{
	static QofParam params[] = {
		{OBJ_NAME, QOF_TYPE_STRING, (QofAccessFunc) obj_getName,
		 (QofSetterFunc) obj_setName, NULL},
		{OBJ_AMOUNT, QOF_TYPE_NUMERIC, (QofAccessFunc) obj_getAmount,
		 (QofSetterFunc) obj_setAmount, NULL},
		{OBJ_GUID, QOF_TYPE_GUID, (QofAccessFunc) obj_getGUID,
		 (QofSetterFunc) obj_setGUID, NULL},
		{OBJ_DATE, QOF_TYPE_TIME, (QofAccessFunc) obj_getDate,
		 (QofSetterFunc) obj_setDate, NULL},
		{OBJ_DISCOUNT, QOF_TYPE_DOUBLE, (QofAccessFunc) obj_getDiscount,
		 (QofSetterFunc) obj_setDiscount, NULL},
		{OBJ_ACTIVE, QOF_TYPE_BOOLEAN, (QofAccessFunc) obj_getActive,
		 (QofSetterFunc) obj_setActive, NULL},
		{OBJ_VERSION, QOF_TYPE_INT32, (QofAccessFunc) obj_getVersion,
		 (QofSetterFunc) obj_setVersion, NULL},
		{OBJ_MINOR, QOF_TYPE_INT64, (QofAccessFunc) obj_getMinor,
		 (QofSetterFunc) obj_setMinor, NULL},
		{OBJ_FLAG, QOF_TYPE_CHAR, (QofAccessFunc) obj_getFlag,
		 (QofSetterFunc) obj_setFlag, NULL},
		{QOF_PARAM_BOOK, QOF_ID_BOOK, (QofAccessFunc) qof_instance_get_book,
		 NULL, NULL},
		{QOF_PARAM_GUID, QOF_TYPE_GUID, (QofAccessFunc) qof_instance_get_guid,
		 NULL, NULL},
		{NULL, NULL, NULL, NULL, NULL},
	};

	qof_class_register (TEST_MODULE_NAME, NULL, params);

	return qof_object_register (&obj_object_def);
}

static void
test_merge (void)
{
	QofBook *target, *import;
	gdouble init_value, discount;
	myobj *import_obj, *target_obj, *new_obj;
	gint result;
	QofTime *base_time, *temp_time;
	gboolean active;
	gint32 version;
	gint64 minor;
	gchar *import_init, *target_init;
	gchar flag, flag_check;
	QofNumeric obj_amount;
	QofBookMergeData *mergeData;

	target = qof_book_new ();
	import = qof_book_new ();
	init_value = 1.00;
	result = 0;
	flag = get_random_character ();
	discount = 0.175;
	active = TRUE;
	version = get_random_int_in_range (0, 10000);
	minor = get_random_int_in_range (1000001, 2000000);
	import_init = "test";
	target_init = "testing";
	base_time = qof_time_set (1153309194, 568714241);
	do_test ((TRUE == qof_time_is_valid (base_time)), 
		"invalid init time");
	{
		gchar *str;
		QofDate *qd;

		qd = qof_date_from_qtime (base_time);
		str = qof_date_print (qd, QOF_DATE_FORMAT_ISO8601);
		do_test ((0 == safe_strcmp (
			"2006-07-19 11:39:54.568714241 +0000", str)),
			"failed to compare base_time correctly.");
		g_free (str);
		qof_date_free (qd);
	}

	do_test ((NULL != target), "#1 target book is NULL");
	do_test ((NULL != import), "#2 import book is NULL");

	/* import book objects - tests used */
	import_obj = g_new (myobj, 1);
	do_test ((NULL != import_obj), "#3 new object create");
	qof_instance_init (&import_obj->inst, TEST_MODULE_NAME, import);
	do_test ((NULL != &import_obj->inst), "#4 instance init");
	obj_setGUID (import_obj, qof_instance_get_guid (&import_obj->inst));
	do_test ((NULL != &import_obj->obj_guid), "#5 guid set");
	qof_event_gen (&import_obj->inst.entity, QOF_EVENT_CREATE, NULL);
	do_test ((NULL != &import_obj->inst.entity), "#6 gnc event create");
	obj_setName (import_obj, import_init);
	do_test ((NULL != &import_obj->Name), "#7 string set");
	obj_amount = qof_numeric_from_double (init_value, 1, QOF_HOW_DENOM_EXACT);
	obj_setAmount (import_obj, obj_amount);
	do_test ((qof_numeric_check (obj_getAmount (import_obj)) == QOF_ERROR_OK),
			 "#8 gnc_numeric set");
	obj_setActive (import_obj, active);
	do_test ((FALSE != &import_obj->active), "#9 gboolean set");
	obj_setDiscount (import_obj, discount);
	obj_setVersion (import_obj, version);
	do_test ((version == import_obj->version), "#11 gint32 set");
	obj_setMinor (import_obj, minor);
	do_test ((minor == import_obj->minor), "#12 gint64 set");
	do_test ((TRUE == qof_time_is_valid (base_time)), 
		"invalid import time ts");
	{
		gchar *str;
		QofDate *qd;

		qd = qof_date_from_qtime (base_time);
		str = qof_date_print (qd, QOF_DATE_FORMAT_ISO8601);
		do_test ((0 == safe_strcmp (
			"2006-07-19 11:39:54.568714241 +0000", str)),
			"failed to compare base_time correctly.");
		g_free (str);
		qof_date_free (qd);
	}
	obj_setDate (import_obj, base_time);
	do_test ((TRUE == qof_time_is_valid (import_obj->date)), 
		"invalid import time");
	do_test ((qof_time_cmp (base_time, import_obj->date) == 0), 
		"test #13 date set");
	obj_setFlag (import_obj, flag);
	do_test ((flag == obj_getFlag (import_obj)), "#14 flag set");

	obj_amount =
		qof_numeric_add (obj_amount, obj_amount, 1, QOF_HOW_DENOM_EXACT);
	discount = get_random_double ();
	version = 2;
	minor = 3;

	/* second import object - test results would be the same, so not tested. */
	new_obj = g_new (myobj, 1);
	qof_instance_init (&new_obj->inst, TEST_MODULE_NAME, import);
	obj_setGUID (new_obj, qof_instance_get_guid (&new_obj->inst));
	qof_event_gen (&new_obj->inst.entity, QOF_EVENT_CREATE, NULL);
	obj_setName (new_obj, import_init);
	obj_setAmount (new_obj, obj_amount);
	obj_setActive (new_obj, active);
	obj_setDiscount (new_obj, discount);
	obj_setVersion (new_obj, version);
	obj_setMinor (new_obj, minor);
	do_test ((TRUE == qof_time_is_valid (base_time)), 
		"second import time invalid");
	{
		gchar *str;
		QofDate *qd;

		qd = qof_date_from_qtime (base_time);
		str = qof_date_print (qd, QOF_DATE_FORMAT_ISO8601);
		do_test ((0 == safe_strcmp (
			"2006-07-19 11:39:54.568714241 +0000", str)),
			"failed to compare base_time correctly.");
		g_free (str);
		qof_date_free (qd);
	}
	obj_setDate (new_obj, base_time);
	obj_setFlag (new_obj, flag);

	obj_amount =
		qof_numeric_add (obj_amount, obj_amount, 1, QOF_HOW_DENOM_EXACT);
	discount = get_random_double ();
	version = 2;
	minor = 3;
	flag = 'z';

	/* target object - test results would be the same, so not tested. */
	target_obj = g_new (myobj, 1);
	qof_instance_init (&target_obj->inst, TEST_MODULE_NAME, target);
	obj_setGUID (target_obj, qof_instance_get_guid (&target_obj->inst));
	qof_event_gen (&target_obj->inst.entity, QOF_EVENT_CREATE, NULL);
	obj_setName (target_obj, target_init);
	obj_setAmount (target_obj, obj_amount);
	obj_setActive (target_obj, active);
	obj_setDiscount (target_obj, discount);
	obj_setVersion (target_obj, version);
	obj_setMinor (target_obj, minor);
	{
		gchar *str;
		QofDate *qd;

		qd = qof_date_from_qtime (base_time);
		str = qof_date_print (qd, QOF_DATE_FORMAT_ISO8601);
		do_test ((0 == safe_strcmp (
			"2006-07-19 11:39:54.568714241 +0000", str)),
			"failed to compare base_time correctly.");
		g_free (str);
		qof_date_free (qd);
	}
	temp_time = qof_time_add_secs_copy (base_time, 65);
	do_test ((TRUE == qof_time_is_valid (temp_time)), 
		"time add secs returned invalid");
	obj_setDate (target_obj, temp_time);
	obj_setFlag (target_obj, flag);
	do_test ((flag == obj_getFlag (target_obj)), "#15 flag set");

	mergeData = qof_book_merge_init (import, target);
	do_test (mergeData != NULL,
			 "FATAL: Merge could not be initialised!\t aborting . . ");
	g_return_if_fail (mergeData != NULL);
	qof_book_merge_rule_foreach (mergeData, test_rule_loop, MERGE_REPORT);
	qof_book_merge_rule_foreach (mergeData, test_rule_loop, MERGE_UPDATE);
	qof_book_merge_rule_foreach (mergeData, test_rule_loop, MERGE_NEW);
	/* reserved calls - test only */
	qof_book_merge_rule_foreach (mergeData, test_rule_loop, MERGE_ABSOLUTE);
	qof_book_merge_rule_foreach (mergeData, test_rule_loop, MERGE_DUPLICATE);

	/* import should not be in the target - pass if import_init fails match with target */
	do_test (((safe_strcmp (obj_getName (import_obj), 
			   obj_getName (target_obj))) != 0), 
			   "Init value test #1");

	/* a good commit returns zero */
	do_test (qof_book_merge_commit (mergeData) == 0, "Commit failed");

	/* import should be in the target - pass if import_init matches target */
	do_test (((safe_strcmp (import_init, obj_getName (target_obj))) == 0),
			 "Merged value test #1");

	/* import should be the same as target - pass if values are the same */
	do_test (((safe_strcmp
			   (obj_getName (target_obj), obj_getName (import_obj))) == 0),
			 "Merged value test #2");

	/* check that the Amount really is a gnc_numeric */
	do_test ((qof_numeric_check (obj_getAmount (import_obj)) == QOF_ERROR_OK),
			 "import gnc_numeric check");
	do_test ((qof_numeric_check (obj_getAmount (target_obj)) == QOF_ERROR_OK),
			 "target gnc_numeric check");

	/* obj_amount was changed after the import object was set, so expect a difference. */
	do_test ((qof_numeric_compare (obj_getAmount (import_obj), obj_amount) !=
			  QOF_ERROR_OK), "gnc_numeric value check #1");

	/* obj_amount is in the target object with the import value, expect a difference/ */
	do_test ((qof_numeric_compare (obj_getAmount (target_obj), obj_amount) !=
			  QOF_ERROR_OK), "gnc_numeric value check #2");

	/* target had a different date, so import date should now be set */
	qof_time_free (temp_time);
	temp_time = target_obj->date;
	{
		gchar *str;
		QofDate *qd;

		qd = qof_date_from_qtime (base_time);
		str = qof_date_print (qd, QOF_DATE_FORMAT_ISO8601);
		do_test ((0 == safe_strcmp (
			"2006-07-19 11:39:54.568714241 +0000", str)),
			"failed to compare base_time after merge.");
		g_free (str);
		qof_date_free (qd);
	}
	{
		gchar *str;
		QofDate *qd;

		qd = qof_date_from_qtime (temp_time);
		str = qof_date_print (qd, QOF_DATE_FORMAT_ISO8601);
		do_test ((0 == safe_strcmp (
			"2006-07-19 11:39:54.568714241 +0000", str)),
			"failed to compare target time after merge.");
		g_free (str);
		qof_date_free (qd);
	}
	do_test ((qof_time_cmp (base_time, temp_time) == 0), 
		"date value check: 1");
#ifdef TEST_DEBUG
	DEBUG (" import<->target=%d\n", 
		(qof_time_cmp (base_time, target_obj->date)));
	{
		QofDate *qd;
		gchar *check;

		qd = qof_date_from_qtime (base_time);
		DEBUG (" base_time=%" G_GINT64_FORMAT
		" nsecs=%ld", qof_time_get_secs (base_time),
		qof_time_get_nanosecs (base_time));
		DEBUG (" import:\nyear=%" G_GINT64_FORMAT
		" month=%ld day=%ld hour=%ld min=%ld sec=%"
		G_GINT64_FORMAT "nsecs=%ld\n",
		qd->qd_year, qd->qd_mon, qd->qd_mday, qd->qd_hour,
		qd->qd_min, qd->qd_sec, qd->qd_nanosecs);
		check = qof_date_print (qd, QOF_DATE_FORMAT_ISO8601);
		DEBUG (" import=%s\n", check);
		qof_date_free (qd);
		qd = qof_date_from_qtime (target_obj->date);
		if (!qd)
			PERR ("qd failed");
		DEBUG (" target:\nyear=%" G_GINT64_FORMAT
		" month=%ld day=%ld hour=%ld min=%ld sec=%"
		G_GINT64_FORMAT "nsecs=%ld\n",
		qd->qd_year, qd->qd_mon, qd->qd_mday, qd->qd_hour,
		qd->qd_min, qd->qd_sec, qd->qd_nanosecs);
		check = qof_date_print (qd, QOF_DATE_FORMAT_ISO8601);
		DEBUG (" target=%s\n", check);
		g_free (check);
		qof_date_free (qd);
	}
#endif
	qof_time_free (base_time);
	/* import should be the same as target - pass if values are the same */
	flag_check = obj_getFlag (target_obj);
	do_test ((flag_check == obj_getFlag (import_obj)), "flag value check: 1");
	do_test ((obj_getFlag (import_obj) == obj_getFlag (target_obj)),
			 "flag value check: 2");
}

static void
test_rule_loop (QofBookMergeData * mergeData, QofBookMergeRule * rule,
				guint remainder)
{
	GSList *testing;
	QofParam *eachParam;
	gchar *importstring;
	gchar *targetstring;
	gboolean skip_target;

	importstring = NULL;
	targetstring = NULL;
	skip_target = FALSE;
	mergeData->currentRule = rule;
	do_test ((rule != NULL), "loop:#1 Rule is NULL");
	do_test (remainder > 0, "loop:#2 remainder error.");
	do_test ((safe_strcmp (NULL, rule->mergeLabel) != 0),
			 "loop:#3 object label\n");
	do_test ((rule->importEnt != NULL), 
		"loop:#4 empty import entity");
	/* targetEnt is always NULL at this stage if MERGE_NEW is set */
	if (rule->targetEnt == NULL)
	{
		skip_target = TRUE;
	}
	if (!skip_target)
	{
		do_test ((safe_strcmp
				  (rule->importEnt->e_type, rule->targetEnt->e_type) == 0),
				 "loop: entity type mismatch");
	}
	do_test ((rule->mergeParam != NULL), 
		"loop: empty parameter list");
	testing = rule->mergeParam;

	while (testing != NULL)
	{							// start of param loop
		eachParam = testing->data;
		do_test ((eachParam != NULL), "loop:#8 no QofParam data");
		do_test ((eachParam->param_name != NULL),
				 "loop:#9 no parameter name");
		do_test ((eachParam->param_getfcn != NULL),
				 "loop:#10 no get function");
		do_test ((eachParam->param_setfcn != NULL),
				 "loop:#11 no set function");
		/* non-generic - test routines only! */
		if (safe_strcmp (eachParam->param_type, QOF_TYPE_STRING) == 0)
		{
			importstring =
				g_strdup (eachParam->
						  param_getfcn (rule->importEnt, eachParam));
			do_test ((importstring != NULL),
					 "loop:#12 direct get_fcn import");
			do_test ((safe_strcmp (importstring, "test") == 0),
					 "loop:#13 direct import comparison");
			if (!skip_target)
			{
				targetstring =
					eachParam->param_getfcn (rule->targetEnt, eachParam);
				do_test ((targetstring != NULL),
						 "loop:#14 direct get_fcn target");
				do_test ((safe_strcmp (targetstring, "testing") == 0),
						 "loop:#15 direct target comparison");
			}
		}
		/* param_as_string does the conversion for display purposes only */
		/* do NOT use as_string for calculations or set_fcn */
		importstring =
			qof_book_merge_param_as_string (eachParam, rule->importEnt);
		do_test ((importstring != NULL),
				 "loop:#16 import param_as_string is null");
		if (!skip_target)
		{
			targetstring =
				qof_book_merge_param_as_string (eachParam, rule->targetEnt);
			do_test ((targetstring != NULL),
					 "loop:#17 target param_as_string is null");
		}
		testing = g_slist_next (testing);
	}		// end param loop
	/* set each rule dependent on the user involvement response above. */
	/* test routine just sets all MERGE_REPORT to MERGE_UPDATE */
	mergeData = qof_book_merge_update_result (mergeData, MERGE_UPDATE);
	do_test ((rule->mergeResult != MERGE_REPORT), 
		"update result fail");
}

int
main (void)
{
	qof_init ();
	myobjRegister ();
	test_merge ();
	print_test_results ();
	qof_close ();
	return get_rv();
}
