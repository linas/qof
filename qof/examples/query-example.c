
/** @file query-example.c
 *  @breif Example program showing query object usage. 
 *  @author Copyright (c) 2003 Linas Vepstas <linas@linas.org>
 *
 *  This example program shows how to configure an arbitrary
 *  programmer-defined oject "MyObj" so that the Query routines
 *  can be used on it.  It consists of four basic peices:
 *  -- The object definition, showing how to hook into the query system.
 *  -- Generic application intialization, including the creation of
 *     a number of instances of MyObj.
 *  -- QOF intialization, required before QOF can be used.
 *  -- The definition and running of a query, and a printout of the
 *     results.
 */

#include <glib.h>
#include <qof/qof.h>

/* ===================================================== */

/* Define "my" object.  Replace by your object. */
typedef struct myobj_s 
{
   int a;          /* Some value */
   int b;          /* Some other value */
   char *memo;     /* Some string value */
} MyObj;

GList *all_my_objs = NULL;

MyObj *
my_obj_new (QofBook *book)
{
   MyObj *m = g_new0 (MyObj,1);

   /* Make sure we keep track of ever object; otherwise we won't
    * be able to search over them.  */
   all_my_objs = g_list_prepend (all_my_objs, m);

   return m;
}

/* Generic object getters */
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

const char *
my_obj_get_memo (MyObj *m)
{
   return m->memo;
}

/* Loop over every instance of MyObj, and apply the callback to it.
 * This routine must be defined for queries to be possible. */
void
my_obj_foreach (QofCollection *coll, QofEntityForeachCB cb, gpointer ud)
{
   GList *n;
   
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
int
my_obj_order (MyObj *a, MyObj *b)
{
   if ( (a) && !(b) ) return -1;
   if ( !(a) && (b) ) return +1;
   if ( !(a) && !(b) ) return 0;

   if ((a)->a > (b)->a) return +1;
   if ((a)->a == (b)->a) return 0;
   return -1;
}

/* ===================================================== */
/* Provide infrastructure to register my object with QOF */

#define MYOBJ_ID  "MyObj"

static QofObject myobj_object_def = 
{
   interface_version: QOF_OBJECT_VERSION,
   e_type:            MYOBJ_ID,
   type_label:        "My Blinking Object",
   book_begin:        NULL,
   book_end:          NULL,
   is_dirty:          NULL,
   mark_clean:        NULL,
   foreach:           my_obj_foreach,
   printable:         NULL,
};

/* Some arbitrary names for data getters that the query will employ */
#define MYOBJ_A    "MyObj_a"
#define MYOBJ_B    "MyObj_b"
#define MYOBJ_MEMO "MyObj_memo"
gboolean myObjRegister (void)
{
   /* Associate an ASCII name to each getter, as well as the return type */
   static QofParam params[] = {
     { MYOBJ_A,     QOF_TYPE_INT32, (QofAccessFunc)my_obj_get_a, NULL },
     { MYOBJ_B,     QOF_TYPE_INT32, (QofAccessFunc)my_obj_get_b, NULL },
     { MYOBJ_MEMO,  QOF_TYPE_STRING, (QofAccessFunc)my_obj_get_memo, NULL },
     { NULL },
   };

   qof_class_register (MYOBJ_ID, (QofSortFunc)my_obj_order, params);
   return qof_object_register (&myobj_object_def);
}

/* ===================================================== */

QofBook *
my_app_init (void)
{
   QofBook *book;
   
   /* Perform the application object registeration */
   myObjRegister ();

   /* Create a new top-level object container */
   book =  qof_book_new();

   return book;
}

void
my_app_shutdown (QofBook *book)
{
   /* Terminate our storage. This prevents the system from
    * being used any further. */
   qof_book_destroy (book);
}

void
my_app_create_data (QofBook *book)
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
   
/* ===================================================== */
/* A routine that will build an actual query, run it, and
 * print the results.
 */

void
my_app_run_query (QofBook *book)
{
   QofQueryPredData *pred_data;
   GSList *param_list;
   QofQuery *q;
   GList *results, *n;

   /* Create a new query */
   q =  qof_query_create ();

   /* Set the object type to be searched for */
   qof_query_search_for (q, MYOBJ_ID);
   
   /* Set the book to be searched */
   qof_query_set_book(q, book);

   /* Describe the query to be performed.
    * We want to find all objects whose "memo" field matches
    * a particular string, or all objects whose "b" field is 42.
    */

   param_list = qof_query_build_param_list (MYOBJ_MEMO, /* field to match */
                   NULL); 
   pred_data = qof_query_string_predicate (
                   QOF_COMPARE_EQUAL,                 /* comparison to make */
                   "M M M My Sharona",                /* string to match */
                   QOF_STRING_MATCH_CASEINSENSITIVE,  /* case matching */
                   FALSE);                            /* use_regexp */
   qof_query_add_term (q, param_list, pred_data, 
                   QOF_QUERY_FIRST_TERM);             /* How to combine terms */
   
   param_list = qof_query_build_param_list (MYOBJ_B,  /* field to match */
                   NULL); 
   pred_data = qof_query_int32_predicate (
                   QOF_COMPARE_EQUAL,                 /* comparison to make */
                   42);                               /* value to match */
   
   qof_query_add_term (q, param_list, pred_data, 
                   QOF_QUERY_OR);                     /* How to combine terms */
   
   /* Run the query */
   results = qof_query_run (q);

   /* Print out the results */
	printf ("\n");
   printf ("My Object collection contains the following objects:\n");
   for (n=all_my_objs; n; n=n->next)
   {
      MyObj *m = n->data;
      printf ("    a=%d b=%d memo=\"%s\"\n", 
          m->a, m->b, m->memo);
   }
	printf ("\n");

   printf ("Query returned %d results:\n", g_list_length(results));
   for (n=results; n; n=n->next)
   {
      MyObj *m = n->data;
      printf ("Found a matching object, a=%d b=%d memo=\"%s\"\n", 
          m->a, m->b, m->memo);
   }
	printf ("\n");
   
   /* The query isn't needed any more; discard it */
   qof_query_destroy (q);

}

/* ===================================================== */
 
int
main (int argc, char *argv[]) 
{
   QofBook *book;
   
   /* Initialize the QOF framework */
   gnc_engine_get_string_cache();
   guid_init();
   qof_object_initialize ();
   qof_query_init ();
   qof_book_register ();
              
   /* Do application-specific things */
   book = my_app_init();
   my_app_create_data(book);
   my_app_run_query (book);
   my_app_shutdown (book);

   /* Perform a clean shutdown */
   qof_query_shutdown ();
   qof_object_shutdown ();
   guid_shutdown ();
   gnc_engine_string_cache_destroy ();
}

/* =================== END OF FILE ===================== */
