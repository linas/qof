

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
} MyObj;


MyObj *
my_obj_new (void)
{
	MyObj *m = g_new0 (MyObj,1);
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

/* ===================================================== */
/* Provide infrastructure to register my object with QOF */
#define MYOBJ_ID  "MyObj"

static GncObject_t myobj_object_def = 
{
   interface_version: GNC_OBJECT_VERSION,
   name:              MYOBJ_ID,
   type_label:        "MyObj",
   book_begin:        NULL,
   book_end:          NULL,
   is_dirty:          NULL,
   mark_clean:        NULL,
   foreach:           NULL,
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

   gncQueryObjectRegister (MYOBJ_ID, NULL, params);
   return gncObjectRegister (&myobj_object_def);
}

/* ===================================================== */

void
do_stuff (void)
{
	MyObj *m;

}
 
main () 
{
   /* Initialize the QOF framework */
   gnc_engine_get_string_cache();
   xaccGUIDInit ();
   gncObjectInitialize ();
   gncQueryNewInit ();
   gnc_book_register ();
              
	/* Perform the application object registeration */
   myObjRegister ();

	do_stuff ();

   /* Perform a clean shutdown */
   gncQueryNewShutdown ();
   gnc_engine_string_cache_destroy ();
   gncObjectShutdown ();
   xaccGUIDShutdown ();

}
