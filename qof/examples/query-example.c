
/** @file query-example.c
 *  @breif Example program showing query object usage. 
 *  @author Copyright (c) 2003 Linas Vepstas <linas@linas.org>
 *
 *  This example program shows how to configure an arbitrary
 *  programmer-defined oject "MyObj" so that the Query routines
 *  can be used on it.  It consists of four basic peices:
 *  -- The object definition, showing how to hook into the query system.
 *     (this part is in the "my-object.c" file)
 *  -- Generic application intialization, including the creation of
 *     a number of instances of MyObj.
 *  -- QOF intialization, required before QOF can be used.
 *  -- The definition and running of a query, and a printout of the
 *     results.
 */

#include <glib.h>
#include <qof/qof.h>
#include "my-object.h"

/* ===================================================== */
QofBook* my_app_init (void);
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

void my_app_shutdown (QofBook *book);
void
my_app_shutdown (QofBook *book)
{
   /* Terminate our storage. This prevents the system from
    * being used any further. */
   qof_book_destroy (book);
}

void my_app_create_data (QofBook *book);
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
void my_app_run_query (QofBook *book);
void
my_app_run_query (QofBook *book)
{
   QofQueryPredData *pred_data;
   QofCollection *coll;
   GSList *param_list;
   QofQuery *q;
   GList *results, *n, *all_my_objs ;

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
   
	/* Handy debug print of the actual query */
	/* qof_query_print (q); */
	
   /* Run the query */
   results = qof_query_run (q);

   /* Print out the results */
	printf ("\n");
   printf ("My Object collection contains the following objects:\n");
   coll = qof_book_get_collection (book, MYOBJ_ID);
   all_my_objs = qof_collection_get_data (coll);
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
