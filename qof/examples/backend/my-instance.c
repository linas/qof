
/** @file my-instance.c
 *  @breif Example definition of a QOF Instance.
 *  @author Copyright (c) 2003,2004 Linas Vepstas <linas@linas.org>
 *
 *  This example shows how to derive from the QOF Instance class.
 */

#include <glib.h>
#include "qofsql.h"
#include "my-instance.h"

/* ===================================================== */


MyInst *
my_inst_new (QofBook *book)
{
	MyInst *m = g_new0 (MyInst,1);
	qof_instance_init (&m->inst, MYINST_ID, book);
	return m;
}

/* Generic object getters --------------------------------- */
int
my_inst_get_a (MyInst *m)
{
	return m->a;
}

int
my_inst_get_b (MyInst *m)
{
	return m->b;
}

const char *
my_inst_get_memo (MyInst *m)
{
	return m->memo;
}

/* Generic object setters --------------------------------- */

void
my_inst_set_a (MyInst *m, int x)
{
	m->a = x;
}

void
my_inst_set_b (MyInst *m, int x)
{
	m->b = x;
}

void
my_inst_set_memo (MyInst *m, const char * str)
{
	if (m->memo) g_free (m->memo);
	m->memo = g_strdup (str);
}

/* ===================================================== */
/* Provide a default mechanism to sort MyInst 
 * This is neeeded so that query results can be returned in
 * some 'reasonable' order.  If you don't want to sort,
 * just have this function always return 0.
 */ 
int
my_inst_order (MyInst *left, MyInst *right)
{
	if ( (left) && !(right) ) return -1;
	if ( !(left) && (right) ) return +1;
	if ( !(left) && !(right) ) return 0;

	if ((left)->a > (right)->a) return +1;
	if ((left)->a == (right)->a) return 0;
	return -1;
}

const char * 
my_inst_print (MyInst *m)
{
	static char buff[200];
	char * p = buff;
	p = strcpy (p, "version=");
	p = gnc_timespec_to_iso8601_buff (
	        qof_instance_get_last_update (QOF_INSTANCE(m)), p);
	sprintf (p, " a=%d b=%d memo=%s\n", m->a, m->b, m->memo);
	return buff;
}

/* ===================================================== */
/* Provide infrastructure to register my object with QOF */

static QofObject myent_object_def = 
{
	interface_version: QOF_OBJECT_VERSION,
	e_type:            MYINST_ID,
	type_label:        "My Blinking Object",
	create:            (gpointer (*)(QofBook *)) my_inst_new,
	book_begin:        NULL,
	book_end:          NULL,
	is_dirty:          NULL,
	mark_clean:        NULL,
	foreach:           qof_collection_foreach,
	printable:         (const char * (*)(gpointer)) my_inst_print,
	version_cmp:       (int (*)(gpointer, gpointer)) qof_instance_version_cmp,
};

gboolean 
my_inst_register (void)
{
	/* Associate an ASCII name to each getter, as well as the return type */
	static QofParam params[] = {
	  { MYINST_A,	  QOF_TYPE_INT32, (QofAccessFunc)my_inst_get_a, 
	                                 (QofSetterFunc)my_inst_set_a },
	  { MYINST_B,	  QOF_TYPE_INT32, (QofAccessFunc)my_inst_get_b, 
	                                 (QofSetterFunc)my_inst_set_b },
	  { MYINST_MEMO, QOF_TYPE_STRING, (QofAccessFunc)my_inst_get_memo, 
	                                 (QofSetterFunc)my_inst_set_memo },
	  { QOF_PARAM_GUID, QOF_TYPE_GUID,
	                         (QofAccessFunc)qof_entity_get_guid, NULL },
	  { QOF_PARAM_VERSION, QOF_TYPE_DATE,
	                         (QofAccessFunc)qof_instance_get_last_update,
	                         (QofSetterFunc)qof_instance_set_last_update },
	  { NULL },
	};

	qof_class_register (MYINST_ID, (QofSortFunc)my_inst_order, params);
	return qof_object_register (&myent_object_def);
}

/* =================== END OF FILE ===================== */
