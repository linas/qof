
/** @file sql-example.c
 *  @breif Example program showing SQL to perform query over objects.
 *  @author Copyright (c) 2003, 2004 Linas Vepstas <linas@linas.org>
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

/* cheap hack to keep the demo simple */
extern GList *all_my_objs;

void 
my_app_run_query (QofSqlQuery *q, char *sql_str)
{
   GList *results, *n;

   /* Run the query */
   results = qof_sql_query_run (q, sql_str);

   printf ("------------------------------------------\n");
   printf ("Query string is: %s\n", sql_str);
   printf ("Query returned %d results:\n", g_list_length(results));
   for (n=results; n; n=n->next)
   {
      MyObj *m = n->data;
      printf ("Found a matching object, a=%d b=%d memo=\"%s\"\n", 
          m->a, m->b, m->memo);
   }
   printf ("\n");
}

void
my_app_do_some_queries (QofBook *book)
{
   QofSqlQuery *q;
   GList *n;

   /* Print out the baseline: all of the instances in the system */
   printf ("\n");
   printf ("My Object collection contains the following objects:\n");
   for (n=all_my_objs; n; n=n->next)
   {
      MyObj *m = n->data;
      printf ("    a=%d b=%d memo=\"%s\"\n", 
          m->a, m->b, m->memo);
   }
   printf ("\n");

   /* Create a new query */
   q =  qof_sql_query_new ();

   /* Set the book to be searched */
   qof_sql_query_set_book(q, book);

   /* Describe the query to be performed.
    * We want to find all objects whose "memo" field matches
    * a particular string, or all objects whose "b" field is 42.
    */
   char * str = "SELECT * FROM " MYOBJ_ID 
           " WHERE (" MYOBJ_MEMO " = 'M M M My Sharona') OR "
   		  "       (" MYOBJ_B " = 42);";

   my_app_run_query (q, str);

   /* Do some more */
   my_app_run_query (q, "SELECT * FROM MyObj WHERE MyObj_a = 1;");
   my_app_run_query (q, "SELECT * FROM MyObj WHERE (MyObj_a = 1) AND (MyObj_b=42);");
   my_app_run_query (q, "SELECT * FROM MyObj WHERE (MyObj_a = 3);");
   my_app_run_query (q, "SELECT * FROM MyObj;");
   my_app_run_query (q, "SELECT * FROM MyObj ORDER BY MyObj_a DESC;");
   my_app_run_query (q, "SELECT * FROM MyObj ORDER BY MyObj_b DESC;");
   my_app_run_query (q, "SELECT * FROM MyObj WHERE MyObj_a = 1 ORDER BY MyObj_b DESC;");
   

   
   /* The query isn't needed any more; discard it */
   qof_sql_query_destroy (q);
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
   my_app_do_some_queries (book);
   my_app_shutdown (book);

   /* Perform a clean shutdown */
   qof_query_shutdown ();
   qof_object_shutdown ();
   guid_shutdown ();
   gnc_engine_string_cache_destroy ();
}

/* =================== END OF FILE ===================== */
