
/* XXX This example is under construction
 * and doesn't do anything interesting yet.
 */

#include <glib.h>

#include "gncObject.h"
#include "QueryObject.h"

/* XXX */
#define QUERYCORE_INT32 "gint32"

/* ===================================================== */

/* Define my object */
typedef struct myobj_s 
{
   int a;
   int b;
	char *memo;
} MyObj;

GSList *all_my_objs = NULL;

MyObj *
my_obj_new (GNCBook *book)
{
	MyObj *m = g_new0 (MyObj,1);

	all_my_objs = g_slist_prepend (all_my_objs, m);

	return m;
}

int
my_obj_get_a (MyObj *m)
{
	return m->a;
}

int
my_obj_get_b (MyObj *m)
{
	return m->b;
}

/* Loop over every instance of MyObj, and apply the callback to it */
void
my_obj_foreach (GNCBook *book, foreachObjectCB cb, gpointer ud)
{
	GSList *n;
	
	for (n=all_my_objs; n; n=n->next)
	{
		cb (n->data, ud);
	}
}

/* Provide a default mechanism to sort MyObj 
 * This is neeeded so that query results can be returned in
 * some 'reasonable' order.  If you don't want to sort,
 * just have this function always return 0.
 */ 
my_obj_order (MyObj **a, MyObj **b)
{
   if ( (*a) && !(*b) ) return -1;
   if ( !(*a) && (*b) ) return +1;
   if ( !(*a) && !(*b) ) return 0;

	if ((*a)->a > (*b)->a) return +1;
	if ((*a)->a == (*b)->a) return 0;
	return -1;
}

/* ===================================================== */
/* Provide infrastructure to register my object with QOF */

#define MYOBJ_ID  "MyObj"

static GncObject_t myobj_object_def = 
{
   interface_version: GNC_OBJECT_VERSION,
   name:              "MyObj",
   type_label:        MYOBJ_ID,
   book_begin:        NULL,
   book_end:          NULL,
   is_dirty:          NULL,
   mark_clean:        NULL,
   foreach:           my_obj_foreach,
   printable:         NULL,
};

#define MYOBJ_A "MyObjA"
#define MYOBJ_B "MyObjB"
gboolean myObjRegister (void)
{
   static QueryObjectDef params[] = {
	  { MYOBJ_A,  QUERYCORE_INT32, (QueryAccess)my_obj_get_a },
	  { MYOBJ_B,  QUERYCORE_INT32, (QueryAccess)my_obj_get_b },
     { NULL },
   };

   gncQueryObjectRegister (MYOBJ_ID, (QuerySort)my_obj_order, params);
   return gncObjectRegister (&myobj_object_def);
}

/* ===================================================== */

GNCBook *
my_app_init (void)
{
	GNCBook *book;
	
	/* Perform the application object registeration */
   myObjRegister ();

	/* Create a new top-level object container */
	book =  gnc_book_new();

	return book;
}

void
my_app_shutdown (GNCBook *book)
{
	/* Terminate our storage. This prevents the system from
	 * being used any further. */
	gnc_book_destroy (book);
}

void
my_app_create_data (GNCBook *book)
{
   MyObj *m;

	/* Pretend our app has some objects; we will perform
	 * the search over these objects */
	
	m = my_obj_new (book);
	m->a = 1;
	m->b = 1;
	m->memo = "Hiho Silver!";
	
	m = my_obj_new (book);
	m->a = 1;
	m->b = 42;
	m->memo = "The Answer to the Question";
	
	m = my_obj_new (book);
	m->a = 99;
	m->b = 1;
	m->memo = "M M M My Sharona";
}
	
void
my_app_run_query (GNCBook *book)
{
	QueryPredData_t pred_data;
	GSList *param_list;
   QueryNew *q;
	GList *results, *n;

	/* Create a new query */
	q =  gncQueryCreate ();

	/* Set the object type to be searched for */
	gncQuerySearchFor (q, MYOBJ_ID);
	
	/* Set the book to be searched */
	gncQuerySetBook(q, book);

	/* Describe the query to be performed */
	
	pred_data = gncQueryStringPredicate (COMPARE_EQUAL, 
						 "M M M My Sharona",  /* string to match */
						 STRING_MATCH_CASEINSENSITIVE,  /* case matching */
						 FALSE /* use_regexp */);
	
	param_list = NULL;
	gncQueryAddTerm (q, param_list, pred_data, 
						 QUERY_AND /* How to combine terms */);
	
#if 0

	pred_data = gncQueryNumericPredicate (how, sign, amount);
	

    xaccQueryAddValueMatch (q, n, NUMERIC_MATCH_ANY,
	                                COMPARE_EQUAL, QUERY_AND);
		                                                                                 
#endif

printf ("before  runquery\n");
	/* Run the query */
	results = gncQueryRun (q);
printf ("after  runquery\n");

	/* Print out the results */
	for (n=results; n; n=n->next)
	{
	   MyObj *m = n->data;
		printf ("Found a match %d %d %s\n", m->a, m->b, m->memo);
	}
	
	/* The query isn't needed any more; discard it */
	gncQueryDestroy (q);

}

 
int
main (int argc, char *argv[]) 
{
	GNCBook *book;
	
   /* Initialize the QOF framework */
   gnc_engine_get_string_cache();
   xaccGUIDInit ();
   gncObjectInitialize ();
   gncQueryNewInit ();
   gnc_book_register ();
              
	/* Do application-specific things */
	book = my_app_init();
	my_app_create_data(book);
	my_app_run_query (book);
	my_app_shutdown (book);

   /* Perform a clean shutdown */
   gncQueryNewShutdown ();
   gnc_engine_string_cache_destroy ();
   gncObjectShutdown ();
   xaccGUIDShutdown ();
}
