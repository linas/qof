
/** @file my-object.c
 *  @breif Example definition of a queriable object. 
 *  @author Copyright (c) 2003 Linas Vepstas <linas@linas.org>
 *
 *  This example program shows how to configure an arbitrary
 *  programmer-defined oject "MyObj" so that the Query routines
 *  can be used on it.  It shows one part of the total:
 *  -- The object definition, showing how to hook into the query system.
 */

#include <glib.h>
#include <qof/qof.h>
#include "my-object.h"

/* ===================================================== */

GList *all_my_objs = NULL;

MyObj *
my_obj_new (QofBook *book)
{
   MyObj *m = g_new0 (MyObj,1);

   /* Make sure we keep track of every object; otherwise we won't
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

/* =================== END OF FILE ===================== */
