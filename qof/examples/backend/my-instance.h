
/** @file my-instance.h
 *  @breif Example definition of an object deriving from QOF Instance.
 *  @author Copyright (c) 2003,2004 Linas Vepstas <linas@linas.org>
 *
 *  This example shows how to derive from the QOF Instance class.
 */

#ifndef MY_INSTANCE_H_
#define MY_INSTANCE_H_

#include "qofsql.h"
#include "qofinstance-p.h"
#include "qofobject.h"

/** Define "my" object.  Replace by your object. */
typedef struct myinst_s 
{
	QofInstance inst;  /* Base class */
   int a;             /* Some value */
   int b;             /* Some other value */
   char *memo;        /* Some string value */
} MyInst;

/** String identifying the 'type' or 'class' of this object */
#define MYINST_ID  "MyInst"

/** Some arbitrary names for data that can be queried on this object. */
#define MYINST_A    "MyInst_a"
#define MYINST_B    "MyInst_b"
#define MYINST_MEMO "MyInst_memo"


MyInst * my_inst_new (QofBook *book);

/** Generic object getters */
int my_inst_get_a (MyInst *m);
int my_inst_get_b (MyInst *m);
const char * my_inst_get_memo (MyInst *m);

/** Generic object setters */
void my_inst_set_a (MyInst *, int);
void my_inst_set_b (MyInst *, int);
void my_inst_set_memo (MyInst *, const char *);

/** Register entity with the QOF subsystem */
gboolean my_inst_register (void);

#endif /* MY_INSTANCE_H_ */

