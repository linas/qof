
/** @file kvp-object.c
 *  @breif Example definition of a queriable object. 
 *  @author Copyright (c) 2003 Linas Vepstas <linas@linas.org>
 *
 */

#include <glib.h>
#include <qof/qof.h>
#include "kvp-object.h"

/* ===================================================== */

/* Loop over every instance of MyObj, and apply the callback to it.
 * This routine must be defined for queries to be possible. */
void
my_obj_foreach (QofCollection *coll, QofEntityForeachCB cb, gpointer ud)
{
   GList *n;

   GList *all_my_objs = qof_collection_get_data (coll);
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

gpointer  get_self (gpointer p) { return p; }

/* ===================================================== */
/* Provide infrastructure to register my object with QOF */

static QofObject kvp_object_def = 
{
   interface_version: QOF_OBJECT_VERSION,
   e_type:            QOF_ID_KVP,
   type_label:        "KvpFrame",
   book_begin:        NULL,
   book_end:          NULL,
   is_dirty:          NULL,
   mark_clean:        NULL,
   foreach:           my_obj_foreach,
   printable:         NULL,
};

gboolean KvpObjRegister (void)
{
   /* Associate an ASCII name to each getter, as well as the return type */
   static QofParam params[] = {
     { MYOBJ_MEMO,  QOF_TYPE_KVP, get_self, NULL },
     { NULL },
   };

   qof_class_register (QOF_ID_KVP, (QofSortFunc)my_obj_order, params);
   return qof_object_register (&myobj_object_def);
}

/* =================== END OF FILE ===================== */
