
/** @file kvp-object.h
 *  @breif Example definition of a queriable object. 
 *  @author Copyright (c) 2004 Linas Vepstas <linas@linas.org>
 */

#ifndef KVP_OBJECT_H_
#define KVP_OBJECT_H_

#include <qof/qof.h>

/* String identifying the 'type' or 'class' of this object */
#define QOF_ID_KVP  "Kvp"

/* One can query paths and values on a KvpFrame. */
#define FVP_PATH    "KvpPath"
#define FVP_VALUE   "KvpValue"

gboolean KvpObjRegister (void);

#endif /* KVP_OBJECT_H_ */

