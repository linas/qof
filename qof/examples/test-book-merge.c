/*********************************************************************
 * test-book-merge.c -- test implementation api for QoFBook merge    *
 * Copyright (C) 2004 Neil Williams <linux@codehelp.co.uk>           *
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
 * Free Software Foundation           Voice:  +1-617-542-5942        *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652        *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                    *
 *                                                                   *
 ********************************************************************/
/*
 * Test the gncBookMerge infrastructure.
 */
#include <glib.h>

#define _GNU_SOURCE

#include "qof/qofinstance-p.h"
#include "qof/qof.h"
#include "qof/qof_book_merge.h"

#define TEST_MODULE_NAME 	"book-merge-test"
#define TEST_MODULE_DESC 	"Test Book Merge"
#define OBJ_NAME 			"somename"
#define OBJ_AMOUNT 			"anamount"
#define OBJ_DATE 			"nottoday"
#define OBJ_GUID 			"unique"
#define OBJ_DISCOUNT 		"hefty"
#define OBJ_VERSION 		"early"
#define OBJ_MINOR 			"tiny"
#define OBJ_ACTIVE 			"ofcourse"

static void test_rule_loop 	(qof_book_mergeRule*, guint);
static void test_merge 		(void);
gboolean 	myobjRegister 	(void);
void 		test_ForeachParam	( QofParam*, 	gpointer );
void 		test_ForeachType  	( QofObject*, 	gpointer );
void 		test_Foreach 	  	( QofEntity*, 	gpointer );

/* simple object structure */
typedef struct obj_s
{
	QofInstance inst;
	char     	*Name;
	gnc_numeric Amount;
	const GUID 	*obj_guid;
	Timespec 	date;
	double 		discount; /* cheap pun, I know. */
	gboolean 	active;
	gint32   	version;
	gint64 		minor;
}myobj;

myobj* obj_create(QofBook*);

/* obvious setter functions */
void obj_setName(myobj*,	char*);
void obj_setGUID(myobj*,	const GUID*);
void obj_setAmount(myobj*,  gnc_numeric);
void obj_setDate(myobj*,	Timespec h);
void obj_setDiscount(myobj*, double);
void obj_setActive(myobj*,  gboolean);
void obj_setVersion(myobj*, gint32);
void obj_setMinor(myobj*,   gint64);

/* obvious getter functions */
char*		obj_getName(myobj*);
const GUID*	obj_getGUID(myobj*);
gnc_numeric obj_getAmount(myobj*);
Timespec   	obj_getDate(myobj*);
double		obj_getDiscount(myobj*);
gboolean	obj_getActive(myobj*);
gint32		obj_getVersion(myobj*);
gint64		obj_getMinor(myobj*);

myobj*
obj_create(QofBook *book)
{
	myobj *g;
	g_return_val_if_fail(book, NULL);
	g = g_new(myobj, 1);
	qof_instance_init (&g->inst, TEST_MODULE_NAME, book);
	obj_setGUID(g,qof_instance_get_guid(&g->inst));
	g->date.tv_nsec = 0;
	g->date.tv_sec = 0;
	g->discount = 0;
	g->active = TRUE;
	g->version = 1;
	g->minor = 1;
	gnc_engine_gen_event(&g->inst.entity, GNC_EVENT_CREATE);
	return g;
}

void
obj_setMinor(myobj *g, gint64 h)
{
	g_return_if_fail(g != NULL);
	g->minor = h;
}

gint64
obj_getMinor(myobj *g)
{
	g_return_val_if_fail((g != NULL),0);
	return g->minor;
}

void
obj_setVersion(myobj *g, gint32 h)
{
	g_return_if_fail(g != NULL);
	g->version = h;
}

gint32
obj_getVersion(myobj *g)
{
	if(!g) return 0;
	return g->version;
}

void
obj_setActive(myobj *g, gboolean h)
{
	if(!g) return;
	g->active = h;
}

gboolean
obj_getActive(myobj *g)
{
	if(!g) return FALSE;
	return g->active;
}

void
obj_setDiscount(myobj *g, double h)
{
	if(!g) return;
	g->discount = h;
}

double
obj_getDiscount(myobj *g)
{
	if(!g) return 0;
	return g->discount;
}

void
obj_setDate(myobj *g, Timespec h)
{
	if(!g) return;
	g->date = h;
}

Timespec
obj_getDate(myobj *g)
{
	Timespec ts;
	if(!g) return ts;
	ts = g->date;
	return ts;
}

void
obj_setGUID(myobj* g, const GUID* h)
{
	if(!g) return;
	g->obj_guid = h;
}

const GUID* 
obj_getGUID(myobj *g)
{
	if(!g) return NULL;
	return g->obj_guid;
}

void 
obj_setName(myobj* g, char* h)
{
	if(!g || !h) return;
	g->Name = strdup(h);
}

char*
obj_getName(myobj *g)
{
	if(!g) return NULL;
	return g->Name;
}

void
obj_setAmount(myobj *g, gnc_numeric h)
{
	if(!g) return;
	g->Amount = h;
}

gnc_numeric
obj_getAmount(myobj *g)
{
	if(!g) return double_to_gnc_numeric(0,0,GNC_HOW_DENOM_EXACT);
	return g->Amount;
}

static QofObject obj_object_def = {
  interface_version:     QOF_OBJECT_VERSION,
  e_type:                TEST_MODULE_NAME,
  type_label:            TEST_MODULE_DESC,
  create:                (gpointer)obj_create,
  book_begin:            NULL,
  book_end:              NULL,
  is_dirty:              NULL,
  mark_clean:            NULL,
  foreach:               qof_collection_foreach,
  printable:             NULL,
  version_cmp:           (int (*)(gpointer,gpointer)) qof_instance_version_cmp,
};

gboolean myobjRegister (void)
{
  static QofParam params[] = {
	{ OBJ_NAME,		QOF_TYPE_STRING,	(QofAccessFunc)obj_getName,		(QofSetterFunc)obj_setName		},
	{ OBJ_AMOUNT,   QOF_TYPE_NUMERIC,   (QofAccessFunc)obj_getAmount,   (QofSetterFunc)obj_setAmount	},
	{ OBJ_GUID,		QOF_TYPE_GUID,		(QofAccessFunc)obj_getGUID,		(QofSetterFunc)obj_setGUID		},
	{ OBJ_DATE,		QOF_TYPE_DATE,		(QofAccessFunc)obj_getDate,		(QofSetterFunc)obj_setDate		},
	{ OBJ_DISCOUNT, QOF_TYPE_DOUBLE,	(QofAccessFunc)obj_getDiscount, (QofSetterFunc)obj_setDiscount  },
	{ OBJ_ACTIVE,   QOF_TYPE_BOOLEAN,   (QofAccessFunc)obj_getActive,   (QofSetterFunc)obj_setActive	},
	{ OBJ_VERSION,  QOF_TYPE_INT32,		(QofAccessFunc)obj_getVersion,  (QofSetterFunc)obj_setVersion   },
	{ OBJ_MINOR,	QOF_TYPE_INT64,		(QofAccessFunc)obj_getMinor,	(QofSetterFunc)obj_setMinor		},
    { QOF_PARAM_BOOK, QOF_ID_BOOK,		(QofAccessFunc)qof_instance_get_book, NULL },
    { QOF_PARAM_GUID, QOF_TYPE_GUID,	(QofAccessFunc)qof_instance_get_guid, NULL },
    { NULL },
  };

  qof_class_register (TEST_MODULE_NAME, NULL, params);

  return qof_object_register (&obj_object_def);
}

static void 
test_merge (void)
{
	QofBook *target, *import;
	double init_value, discount;
	myobj *import_obj, *target_obj, *new_obj;
	int result;
	Timespec ts, tc;
	gboolean active;
	gint32 version;
	gint64 minor;
	gchar *import_init, *target_init;
	gnc_numeric obj_amount;
	
	target = qof_book_new();
	import = qof_book_new();
	init_value = 1.00;
	result = 0;
	discount = 0.5;
	active = TRUE;
	version = 1;
	minor = 1;
	import_init = "test";
	target_init = "testing";
	qof_date_format_set(QOF_DATE_FORMAT_UK);
	timespecFromTime_t(&ts,time(NULL));
	import_obj = g_new(myobj, 1);
	qof_instance_init (&import_obj->inst, TEST_MODULE_NAME, import);
	obj_setGUID(import_obj,qof_instance_get_guid(&import_obj->inst));
	gnc_engine_gen_event(&import_obj->inst.entity, GNC_EVENT_CREATE);
	obj_setName(import_obj, import_init);
	obj_amount = double_to_gnc_numeric(init_value,1, GNC_HOW_DENOM_EXACT);
	obj_setAmount(import_obj, obj_amount);
	obj_setActive(import_obj, active);
	obj_setDiscount(import_obj, discount);
	obj_setVersion(import_obj, version);
	obj_setMinor(import_obj, minor);
	obj_setDate(import_obj, ts );
	tc = import_obj->date;

	obj_amount = gnc_numeric_add(obj_amount, obj_amount, 1, GNC_HOW_DENOM_EXACT);
	discount = 0.25;
	version = 2;
	minor = 3;

	new_obj = g_new(myobj, 1);
	qof_instance_init (&new_obj->inst, TEST_MODULE_NAME, import);
	obj_setGUID(new_obj,qof_instance_get_guid(&new_obj->inst));
	gnc_engine_gen_event (&new_obj->inst.entity, GNC_EVENT_CREATE);
	obj_setName(new_obj, import_init);
	obj_setAmount(new_obj, obj_amount);
	obj_setActive(new_obj, active);
	obj_setDiscount(new_obj, discount);
	obj_setVersion(new_obj, version);
	obj_setMinor(new_obj, minor);
	obj_setDate(new_obj, ts);

	obj_amount = gnc_numeric_add(obj_amount, obj_amount, 1, GNC_HOW_DENOM_EXACT);
	discount = 0.35;
	version = 3;
	minor = 6;
	tc.tv_sec = ts.tv_sec -1;
	tc.tv_nsec = 0;

	target_obj = g_new(myobj, 1);
	qof_instance_init (&target_obj->inst, TEST_MODULE_NAME, target);
	obj_setGUID(target_obj,qof_instance_get_guid(&target_obj->inst));
	gnc_engine_gen_event (&target_obj->inst.entity, GNC_EVENT_CREATE);
	obj_setName(target_obj, target_init);
	obj_setAmount(target_obj, obj_amount);
	obj_setActive(target_obj, active);
	obj_setDiscount(target_obj, discount);
	obj_setVersion(target_obj, version);
	obj_setMinor(target_obj, minor);
	obj_setDate(target_obj, tc );
	
	result = qof_book_mergeInit(import, target);
	g_return_if_fail(result != -1);
 	qof_book_mergeRuleForeach(test_rule_loop, MERGE_REPORT);

 	result = qof_book_mergeCommit();
	g_return_if_fail(result == 0);
	qof_object_foreach_type(test_ForeachType, target);

}

static void
test_rule_loop (qof_book_mergeRule *rule, guint remainder)
{
	GSList *user_reports;
	QofParam *one_param;
	gchar *importstring, *targetstring;
	gint resolution, count;
	gboolean input_ok;
	gchar y;

	resolution = 0;
	count = 1;
	importstring = targetstring = NULL;
	input_ok = FALSE;
	g_return_if_fail(rule != NULL);
	user_reports = rule->mergeParam;
	if(remainder == 1) {
		printf("\n\t\t%i conflict needs to be resolved.\n", remainder);
	}
	else {
		printf("\n\t\t%i conflicts need to be resolved.\n", remainder);
	}
	printf("\n%i parameter values for this \"%s\" object.\n", 
		   g_slist_length(user_reports), rule->targetEnt->e_type);
	while(user_reports != NULL) {
		one_param = user_reports->data;
		printf("%i:\tParameter name:\t\t%s\n", count, one_param->param_name);
		importstring = qof_book_merge_param_as_string(one_param, rule->importEnt);
		printf("\tImport data :\t\t%s\n", importstring);
		targetstring = qof_book_merge_param_as_string(one_param, rule->targetEnt);
		printf("\tOriginal data :\t\t%s\n", targetstring);
		user_reports = g_slist_next(user_reports);
		count++;
	}
	while(!input_ok) {
		resolution = 0;
		printf("\nPlease resolve this conflict. Enter\n\t1 to use the import data or ");
		printf("\n\t2 to keep the original data or");
		/* if rule->mergeAbsolute is TRUE, the GUID matches and a NEW object would corrupt
			the target book. The user must be forced to try again if it is selected in error. */
		if(rule->mergeAbsolute == FALSE) {
			printf("\n\t3 to import the data as a NEW object or");
		}
		printf("\n\t9 to abort the entire merge operation.");
		printf("\nDecision? (1, 2");
		if(rule->mergeAbsolute == FALSE) {
			printf(", 3 ");
		}
		printf("or 9) : "); 
		scanf("%i", &resolution);
		/* example of generic collision resolution handling 
			note that all possible changes are shown. */
		
		switch(resolution) {
			case 1 : { qof_book_mergeUpdateResult(rule, MERGE_UPDATE); input_ok = TRUE; break; }
			case 2 : { 
				if(rule->mergeAbsolute == FALSE) { qof_book_mergeUpdateResult(rule, MERGE_DUPLICATE); }
				if(rule->mergeAbsolute == TRUE) { qof_book_mergeUpdateResult(rule, MERGE_ABSOLUTE); }
				input_ok = TRUE; 
				break; 
			}
			case 3 : { 
				if(rule->mergeAbsolute == FALSE) { 
					qof_book_mergeUpdateResult(rule, MERGE_NEW); 
					input_ok = TRUE; 
				}
				/* if rule->mergeAbsolute is TRUE, the GUID matches and a NEW object would corrupt
					the target book. By not setting a result, the user is forced to try again. */
				break; 
			}
			case 9 : {
				printf("Are you sure you want to abort the entire merge operation?\n");
				printf("The rest of the import data will not be processed.\n");
				printf("Your original data will not be modified. Abort? y/n : ");
				scanf("%s", &y);
				
				if((safe_strcmp("y",&y) == 0)||(safe_strcmp("",&y) == 0)) {
					printf("Aborting . . \n\n");
					qof_book_mergeUpdateResult(rule, MERGE_INVALID);
					input_ok = TRUE;
				}
				break;
			}
			default : break;
		}
	}
}

void 
test_ForeachParam( QofParam* param, gpointer user_data) 
{
	QofEntity *ent;
	char *importstring;

	ent = (QofEntity*)user_data;
	importstring = NULL;
	importstring = qof_book_merge_param_as_string(param, ent);
	printf("%-20s\t\t%s\t\t%s\n", param->param_name, param->param_type, importstring);
}

void
test_Foreach ( QofEntity* ent, gpointer user_data) 
{
	qof_class_param_foreach(ent->e_type, test_ForeachParam , ent);
}

void 
test_ForeachType ( QofObject* obj, gpointer user_data) 
{
	QofBook *book;
	
	book = (QofBook*)user_data;
	printf("\n%s\n", obj->e_type);
	printf("Parameter name\t\t\tData type\t\tValue\n");
	qof_object_foreach(obj->e_type, book, test_Foreach, NULL);

}

int
main (int argc, char **argv)
{
	/* Initialize the QOF framework */
	gnc_engine_get_string_cache();
	guid_init();
	qof_object_initialize ();
	qof_query_init ();
	qof_book_register ();
	
	myobjRegister();
	test_merge();
	
	/* Perform a clean shutdown */
	qof_query_shutdown ();
	qof_object_shutdown ();
	guid_shutdown ();
	gnc_engine_string_cache_destroy ();
	return 0;
}
