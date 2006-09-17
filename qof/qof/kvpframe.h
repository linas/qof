/********************************************************************\
 * kvpframe.h -- Implements a key-value frame system                *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/
/** @addtogroup KVP

 * A KvpFrame is a set of associations between character strings
 * (keys) and KvpValue structures.  A KvpValue is a union with
 * possible types enumerated in the KvpValueType enum, and includes, 
 * among other things, ints, doubles, strings, guid's, lists, time
 * and numeric values.  KvpValues may also be other frames, so
 * KVP is inherently hierarchical.
 * 
 * Values are stored in a 'slot' associated with a key.
 * Pointers passed as arguments into set_slot and get_slot are the
 * responsibility of the caller.  Pointers returned by get_slot are
 * owned by the kvp_frame.  Make copies as needed.
 * 
 * A 'path' is a sequence of keys that can be followed to a value.
 * Paths may be specified as varargs (variable number of arguments
 * to a subrutine, NULL-terminated), as a GSList, or as a standard
 * URL-like path name.  The later is parsed and treated in the same
 * way as file paths would be: / separates keys, /./ is treated as /
 * and /../ means backup one level.  Repeated slashes are treated
 * as one slash.
 *
 * Note that although, in principle, keys may contain the / and . and
 * .. characters,  doing so may lead to confusion, and will make
 * path-string parsing routines fail.  In other words, don't use 
 * a key such as 'some/key' or 'some/./other/../key' because you
 * may get unexpected results.
 * 
 * To set a value into a frame, you will want to use one of the 
 * kvp_frame_set_xxx() routines.  Most of the other routines provide
 * only low-level access that you probably shouldn't use.
 
@{
*/
/** @file kvp_frame.h
    @brief A key-value frame system
    @author Copyright (C) 2000 Bill Gribble
    @author Copyright (C) 2003 Linas Vepstas <linas@linas.org>
	@author Copyright (c) 2006 Neil Williams <linux@codehelp.co.uk>
*/

#ifndef KVP_FRAME_H
#define KVP_FRAME_H

#include "qofnumeric.h"
#include "guid.h"
#include "qoftime.h"

#define QOF_MOD_KVP "qof-kvp"

/** Opaque frame structure */
typedef struct _KvpFrame KvpFrame;

/** A KvpValue is a union with possible types enumerated in the
 * KvpValueType enum. */
typedef struct _KvpValue KvpValue;

/** \brief possible types in the union KvpValue 
 * \todo : People have asked for boolean values, 
 *
 * \todo In the long run, this should be synchronized with the 
 * core QOF types, which in turn should be synced to the g_types
 * in GLib.  Unfortunately, this requires writing a pile of code
 * to handle all of the different cases.
 * An alternative might be to make kvp values inherit from the 
 * core g_types (i.e. add new core g_types) ??
 */
typedef enum
{
	/** QOF_TYPE_INT64   gint64 */
	KVP_TYPE_GINT64 = 1,
	/** QOF_TYPE_DOUBLE  gdouble */
	KVP_TYPE_DOUBLE,
	/** QOF_TYPE_NUMERIC numeric*/
	KVP_TYPE_NUMERIC,
	/** QOF_TYPE_STRING  gchar* */
	KVP_TYPE_STRING,
	/** QOF_TYPE_GUID    guid */
	KVP_TYPE_GUID,
#ifndef QOF_DISABLE_DEPRECATED
	/** \deprecated QOF_TYPE_DATE */
	KVP_TYPE_TIMESPEC,
#endif
	/** QOF_TYPE_TIME    time */
	KVP_TYPE_TIME,
	/** no QOF equivalent. */
	KVP_TYPE_BINARY,
	/** no QOF equivalent. */
	KVP_TYPE_GLIST,
	/** no QOF equivalent. */
	KVP_TYPE_FRAME,
	/** QOF_TYPE_BOOLEAN  gboolean */
	KVP_TYPE_BOOLEAN
} KvpValueType;

/** @name KvpFrame Constructors
 @{
*/

/** Return a new empty instance of KvpFrame */
KvpFrame *
kvp_frame_new (void);

/** Perform a deep (recursive) delete of the frame and any subframes. */
void 
kvp_frame_delete (KvpFrame * frame);

/** Perform a deep (recursive) value copy, copying the frame, 
 *  subframes, and the values as well. */
KvpFrame *
kvp_frame_copy (const KvpFrame * frame);

/** Return TRUE if the KvpFrame is empty */
gboolean 
kvp_frame_is_empty (KvpFrame * frame);

/** @} */

/** @name KvpFrame Basic Value Storing
@{
*/

/** \brief store the value of the gint64 at the indicated path.

 If not all frame components of the path exist, they are created.
 */
void 
kvp_frame_set_gint64 (KvpFrame * frame, const gchar * path, gint64 ival);

/** \brief store the value of the double at the indicated path. 

If not all frame components of the path exist, they are created.
*/
void 
kvp_frame_set_double (KvpFrame * frame, const gchar * path, double dval);

/** \brief store the value of the QofNumeric at the indicated path.

 If not all frame components of the path exist, they are created.
 */
void 
kvp_frame_set_numeric (KvpFrame * frame, const gchar * path,
						QofNumeric nval);

/** \brief Store a copy of the string at the indicated path.

 If not all frame components of the path exist, they are created. 
 If there was another string previously stored at that path, 
 the old copy is deleted.
*/
void 
kvp_frame_set_string (KvpFrame * frame, const gchar * path,
					   const gchar *str);

/** \brief Store a copy of the GUID at the indicated path.

 If not all frame components of the path exist, they are created. 
 If there was another GUID previously stored at that path,
 the old copy is deleted.
*/
void 
kvp_frame_set_guid (KvpFrame * frame, const gchar * path,
					 const GUID * guid);

/** \brief Store a copy of the QofTime at the indicated path.

 If not all frame components of the path exist, they are created. 
 If there was another QofTime previously stored at that path,
 the old copy is deleted.
*/
void
kvp_frame_set_time (KvpFrame * frame, const gchar *path, QofTime *qt);

void
kvp_frame_set_boolean (KvpFrame * frame, const gchar * path, gboolean val);

/** \brief Store a copy of the KvpFrame at the indicated path.

 If not all frame components of the path exist, they are created. 
 If there was another QofTime previously stored at that path,
 the old frame is deleted.
*/
void 
kvp_frame_set_frame (KvpFrame * frame, const gchar * path,
					  KvpFrame * chld);

/** \brief Store a KvpFrame at the indicated path without copying.

 If not all frame components of the path exist, they are created. 
 If there was another QofTime previously stored at that path,
 the old frame is deleted.
*/
void kvp_frame_set_frame_nc (KvpFrame * frame, const gchar * path,
							 KvpFrame * chld);

/** The kvp_frame_set_value() routine copies the value into the frame,
 *    at the location 'path'.   If the path contains slashes '/', these 
 *    are assumed to represent a sequence of keys.  The returned value 
 *    is a pointer to the actual frame into which the value was inserted;
 *    it is NULL if the frame couldn't be found (and thus the value wasn't 
 *    inserted).  The old value at this location, if any, is destroyed.
 *
 *    Pointers passed as arguments into this routine are the responsibility 
 *    of the caller; the pointers are *not* taken over or managed.
 */
KvpFrame *
kvp_frame_set_value (KvpFrame * frame, const gchar * path, const KvpValue * value);

/**
 * The kvp_frame_set_value_nc() routine puts the value (without copying
 *    it) into the frame, putting it at the location 'path'.  If the path 
 *    contains slashes '/', these are assumed to represent a sequence of keys.
 *    The returned value is a pointer to the actual frame into which the value 
 *    was inserted; it is NULL if the frame couldn't be found (and thus the 
 *    value wasn't inserted).  The old value at this location, if any,
 *    is destroyed.
 *
 *    This routine is handy for avoiding excess memory allocations & frees.
 *    Note that because the KvpValue was grabbed, you can't just delete 
 *    unless you remove the key as well (or unless you replace the value).
 */
KvpFrame *
kvp_frame_set_value_nc (KvpFrame * frame, const gchar * path, KvpValue * value);

/** The kvp_frame_replace_value_nc() routine places the new value 
 *    at the indicated path.   It returns the old value, if any.  
 *    It returns NULL if there was an error, or if there was no 
 *    old value. If the path doesn't exist, it is created, unless
 *    new_value is NULL.  Passing in a NULL new_value has the 
 *    effect of deleting the trailing slot (i.e. the trailing path
 *    element).  
 */
KvpValue *
kvp_frame_replace_value_nc (KvpFrame * frame, const gchar * slot,
							  KvpValue * new_value);
/** @} */

/** @name KvpFrame Glist Bag Storing 
 @{
*/

/** \brief add the value of the gint64 to the glist bag

 If not all frame components of the path exist, they are 
 created.  If the value previously stored at this path was 
 not a glist bag, then a bag will be formed there, the old 
 value placed in the bag, and the new value added to the bag.
*/
void 
kvp_frame_add_gint64 (KvpFrame * frame, const gchar * path, gint64 ival);

/** \brief Add the value of the double to the glist bag

 If not all frame components of the path exist, they are 
 created.  If the value previously stored at this path was 
 not a glist bag, then a bag will be formed there, the old 
 value placed in the bag, and the new value added to the bag.
*/
void 
kvp_frame_add_double (KvpFrame * frame, const gchar * path, double dval);

/** \brief Add the value of the QofNumeric to the glist bag.

 If not all frame components of the path exist, they are 
 created.  If the value previously stored at this path was 
 not a glist bag, then a bag will be formed there, the old 
 value placed in the bag, and the new value added to the bag.
*/
void 
kvp_frame_add_numeric (KvpFrame * frame, const gchar * path,
						QofNumeric nval);

/** \brief Add the value of the QofTime to the glist bag

 If not all frame components of the path exist, they are 
 created.  If the value previously stored at this path was 
 not a glist bag, then a bag will be formed there, the old 
 value placed in the bag, and the new value added to the bag.
*/
void
kvp_frame_add_time (KvpFrame * frame, const gchar *path, QofTime *qt);

/** \brief Copy the string to the glist bag at the indicated path.

 If not all frame components of the path exist, they are created.
 If there was another item previously stored at that path, then the
 path is converted to a bag, and the old value, along with the new
 value, is added to the bag.

*/
void kvp_frame_add_string (KvpFrame * frame, const gchar * path,
						   const gchar * str);

void
kvp_frame_add_boolean (KvpFrame * frame, const gchar * path, gboolean val);

/** \brief Copy the GUID to the glist bag at the indicated path.

 If not all frame components of the path exist, they are created.
 If there was another item previously stored at that path, then the
 path is converted to a bag, and the old value, along with the new
 value, is added to the bag.
*/
void kvp_frame_add_guid (KvpFrame * frame, const gchar * path,
						 const GUID * guid);

/** \brief Copy the frame to the glist bag at the indicated path.

 If not all frame components of the path exist, they are created.
 If there was another item previously stored at that path, then the
 path is converted to a bag, and the old value, along with the new
 value, is added to the bag.
*/
void kvp_frame_add_frame (KvpFrame * frame, const gchar * path,
						  KvpFrame * chld);
/** \brief Add the frame to the glist bag \b without copying.

 If not all frame components of the path exist, they are created.
 If there was another item previously stored at that path, then the
 path is converted to a bag, and the old value, along with the new
 value, is added to the bag.
*/
void kvp_frame_add_frame_nc (KvpFrame * frame, const gchar * path,
							 KvpFrame * chld);

/** \brief Add a copy of the value to the glist bag

 If not all frame components of the path exist, they are created.
 If there was another item previously stored at that path, then the
 path is converted to a bag, and the old value, along with the new
 value, is added to the bag. This routine returns the pointer to the
 last frame (the actual frame to which the value was added), or NULL
 if there was an error of any sort (typically, a parse error in the path).
*/
KvpFrame *
kvp_frame_add_value (KvpFrame * frame, const gchar * path,
					   KvpValue * value);

/* \brief Add the value directly to the glist bag

 If not all frame components of the path exist, they are created.
 If there was another item previously stored at that path, then the
 path is converted to a bag, and the old value, along with the new
 value, is added to the bag. This routine returns the pointer to the
 last frame (the actual frame to which the value was added), or NULL
 if there was an error of any sort (typically, a parse error in the path).
*/
KvpFrame *
kvp_frame_add_value_nc (KvpFrame * frame, const gchar * path,
						  KvpValue * value);


/** @} */

/** @name KvpFrame Value Fetching

  Value accessors.  These all take a unix-style slash-separated 
  path as an argument, and return the value stored at that location.
  If the object at the end of that path is not of the type that was
  asked for, then a NULL or a zero is returned.  So, for example,
  asking for a string when the path stored an int will return a NULL.
  In some future date, this may be changed to a looser type system, 
  such as perl's automatic re-typing (e.g. an integer value might be
  converted to a printed string representing that value).

  If any part of the path does not exist, then NULL or zero will be
  returned.

  The values returned for GUID, binary, GList, KvpFrame and string 
  are "non-copying" -- the returned item is the actual item stored.
  Do not delete this item unless you take the required care to avoid
  possible bad pointer derefrences (i.e. core dumps).  Also, be 
  careful hanging on to those references if you are also storing
  at the same path names: the referenced item will be freed during
  the store.

  That is, if you get a string value (or guid, binary or frame),
  and then store something else at that path, the string that you've
  gotten will be freed during the store (internally, by the set_*()
  routines), and you will be left hanging onto an invalid pointer.
@{
*/

gint64 kvp_frame_get_gint64 (const KvpFrame * frame, const gchar * path);
gdouble kvp_frame_get_double (const KvpFrame * frame, const gchar * path);
QofNumeric kvp_frame_get_numeric (const KvpFrame * frame,
								   const gchar * path);
gchar *kvp_frame_get_string (const KvpFrame * frame, const gchar * path);
GUID *kvp_frame_get_guid (const KvpFrame * frame, const gchar * path);
void *kvp_frame_get_binary (const KvpFrame * frame, const gchar * path,
							guint64 * size_return);
/** \todo handle errors */
gboolean
kvp_value_get_boolean (const KvpValue * value);

QofTime *
kvp_frame_get_time (const KvpFrame * frame, const gchar *path);
KvpValue *kvp_frame_get_value (const KvpFrame * frame, const gchar * path);

/** Value accessor.  Takes a unix-style slash-separated path as an
 *  argument, and return the KvpFrame stored at that location.  If the
 *  KvpFrame does not exist, then a NULL is returned.
 *
 *  @note The semantics here have changed: In gnucash-1.8, if the
 *  KvpFrame did not exist, this function automatically created one
 *  and returned it. However, now this function will return NULL in
 *  this case and the caller has to create a KvpFrame on his own. The
 *  old functionality is now implemented by
 *  kvp_frame_get_frame_path(). This happened on 2003-09-14, revision
 *  1.31. FIXME: Is it really a good idea to change the semantics of
 *  an existing function and move the old semantics to a new
 *  function??! It would save us a lot of trouble if the new semantics
 *  would have been available in a new function!
 *
 *  @return The KvpFrame at the specified path, or NULL if it doesn't
 *  exist.
*/
KvpFrame *kvp_frame_get_frame (const KvpFrame * frame, const gchar * path);

/** This routine returns the last frame of the path.
 *  If the frame path doesn't exist, it is created.  
 *  Note that this is *VERY DIFFERENT FROM* kvp_frame_get_frame()
 */
KvpFrame *
kvp_frame_get_frame_path (KvpFrame * frame, const gchar *, ...);

/** This routine returns the last frame of the path.
 *  If the frame path doesn't exist, it is created.  
 *  Note that this is *VERY DIFFERENT FROM* kvp_frame_get_frame()
 */
KvpFrame *
kvp_frame_get_frame_gslist (KvpFrame * frame, GSList * key_path);

/** This routine returns the last frame of the path.
 *  If the frame path doesn't exist, it is created.  
 *  Note that this is *VERY DIFFERENT FROM* kvp_frame_get_frame()
 *
 * The kvp_frame_get_frame_slash() routine takes a single string
 *    where the keys are separated by slashes; thus, for example:
 *    /this/is/a/valid/path  and///so//is////this/
 *    Multiple slashes are compresed.  Leading slash is optional.
 *    The pointers . and .. are *not* currently followed/obeyed.  
 *    (This is a bug that needs fixing).
 */
KvpFrame *kvp_frame_get_frame_slash (KvpFrame * frame, const gchar * path);

/** @} */
/** @name KvpFrame KvpValue low-level storing routines.

You probably shouldn't be using these low-level routines

    All of the kvp_frame_set_slot_*() routines set the slot values
    "destructively", in that if there was an old value there, that
    old value is destroyed (and the memory freed).  Thus, one 
    should not hang on to value pointers, as these will get 
    trashed if set_slot is called on the corresponding key.

    If you want the old value, use kvp_frame_replace_slot().
 @{
*/

/** The kvp_frame_replace_slot_nc() routine places the new value into
 *    the indicated frame, for the given key.  It returns the old
 *    value, if any.  It returns NULL if the slot doesn't exist,
 *    if there was some other an error, or if there was no old value.
 *    Passing in a NULL new_value has the effect of deleting that
 *    slot.
 */
KvpValue *
kvp_frame_replace_slot_nc (KvpFrame * frame, const gchar * slot,
						 KvpValue * new_value);


/** The kvp_frame_set_slot() routine copies the value into the frame,
 *    associating it with a copy of 'key'.  Pointers passed as arguments 
 *    into kvp_frame_set_slot are the responsibility of the caller;
 *    the pointers are *not* taken over or managed.  The old value at
 *    this location, if any, is destroyed.
 */
void kvp_frame_set_slot (KvpFrame * frame,
						 const gchar * key, const KvpValue * value);
/**
 * The kvp_frame_set_slot_nc() routine puts the value (without copying
 *    it) into the frame, associating it with a copy of 'key'.  This 
 *    routine is handy for avoiding excess memory allocations & frees.
 *    Note that because the KvpValue was grabbed, you can't just delete 
 *    unless you remove the key as well (or unless you replace the value).
 *    The old value at this location, if any, is destroyed.
 */
void kvp_frame_set_slot_nc (KvpFrame * frame,
							const gchar * key, KvpValue * value);

/** The kvp_frame_set_slot_path() routine walks the hierarchy,
 *     using the key values to pick each branch.  When the terminal 
 *     node is reached, the value is copied into it.  The old value 
 *     at this location, if any, is destroyed.
 */
void kvp_frame_set_slot_path (KvpFrame * frame,
							  const KvpValue * value,
							  const gchar * first_key, ...);

/** The kvp_frame_set_slot_path_gslist() routine walks the hierarchy,
 *     using the key values to pick each branch.  When the terminal node
 *     is reached, the value is copied into it.  The old value at this
 *     location, if any, is destroyed.
 */
void kvp_frame_set_slot_path_gslist (KvpFrame * frame,
									 const KvpValue * value,
									 GSList * key_path);

/** @} */

/** @name KvpFrame KvpValue Low-Level Retrieval Routines

  You probably shouldn't be using these low-level routines

  Returns the KvpValue in the given KvpFrame 'frame' that is 
  associated with 'key'.  If there is no key in the frame, NULL
  is returned.  If the value associated with the key is NULL, 
  NULL is returned.

  Pointers passed as arguments into get_slot are the responsibility
  of the caller.  Pointers returned by get_slot are owned by the
  kvp_frame.  Make copies as needed.
 @{
*/
KvpValue *kvp_frame_get_slot (const KvpFrame * frame, const gchar * key);

/** This routine return the value at the end of the
 * path, or NULL if any portion of the path doesn't exist.
 */
KvpValue *kvp_frame_get_slot_path (KvpFrame * frame,
								   const gchar * first_key, ...);

/** This routine return the value at the end of the
 * path, or NULL if any portion of the path doesn't exist.
 */
KvpValue *kvp_frame_get_slot_path_gslist (KvpFrame * frame,
										  GSList * key_path);

/** @} */

/** 
 kvp_glist_compare() compares <b>GLists of KvpValue values</b> (not to
 be confused with GLists of something else):  it iterates over
 the list elements, performing a kvp_value_compare on each.
 @{
*/
gint kvp_glist_compare (const GList * list1, const GList * list2);

/** kvp_glist_copy() performs a deep copy of a <b>GList of 
 *     kvp_values</b> (not to be confused with GLists of something
 *     else): same as mapping kvp_value_copy() over the elements and
 *     then copying the spine.
 */
GList *kvp_glist_copy (const GList * list);

/** kvp_glist_delete() performs a deep delete of a <b>GList of
 *     kvp_values</b> (not to be confused with GLists of something
 *     else): same as mapping * kvp_value_delete() over the elements
 *     and then deleting the GList.
 */
void kvp_glist_delete (GList * list);


/** @name KvpValue Constructors

 The following routines are constructors for kvp_value.
 Those with pointer arguments copy in the value.
 The *_nc() versions do *not* copy in their values, 
 but use them directly.
 @{
 */
KvpValue *kvp_value_new_gint64 (gint64 value);
KvpValue *kvp_value_new_double (double value);

KvpValue *kvp_value_new_numeric (QofNumeric value);
KvpValue *kvp_value_new_string (const gchar * value);
KvpValue *kvp_value_new_guid (const GUID * guid);
KvpValue *kvp_value_new_time (QofTime *value);
/** \since 0.7.2 */
KvpValue *kvp_value_new_boolean (gboolean value);

KvpValue *kvp_value_new_binary (const void *data, guint64 datasize);
KvpValue *kvp_value_new_frame (const KvpFrame * value);

/** Creates a KvpValue from a <b>GList of kvp_value's</b>! (Not to be
 *  confused with GList's of something else!) */
KvpValue *kvp_value_new_glist (const GList * value);

/** value constructors (non-copying - KvpValue takes pointer ownership)
   values *must* have been allocated via glib allocators! (gnew, etc.) */
KvpValue *kvp_value_new_binary_nc (void *data, guint64 datasize);

/** Creates a KvpValue from a <b>GList of kvp_value's</b>! (Not to be
 * confused with GList's of something else!) 
 *
 * This value constructor is non-copying (KvpValue takes pointer
 * ownership). The values *must* have been allocated via glib
 * allocators! (gnew, etc.) */
KvpValue *kvp_value_new_glist_nc (GList * lst);

/** value constructors (non-copying - KvpValue takes pointer ownership)
   values *must* have been allocated via glib allocators! (gnew, etc.) */
KvpValue *kvp_value_new_frame_nc (KvpFrame * value);

/** This is a deep (recursive) delete. */
void kvp_value_delete (KvpValue * value);

/** This is a deep value copy. */
KvpValue *kvp_value_copy (const KvpValue * value);

/** Replace old frame value with new, return old frame */
KvpFrame *kvp_value_replace_frame_nc (KvpValue * value, KvpFrame * newframe);

/** Replace old glist value with new, return old glist */
GList *kvp_value_replace_glist_nc (KvpValue * value, GList * newlist);

/** @} */


/** @name KvpValue Value access

 @{
*/

KvpValueType kvp_value_get_type (const KvpValue * value);


/** Value accessors. Those for GUID, binary, GList, KvpFrame and
 *   string are non-copying -- the caller can modify the value 
 *   directly. Just don't free it, or you screw up everything.
 *   Note that if another value is stored at the key location
 *   that this value came from, then this value will be 
 *   uncermoniously deleted, and you will be left pointing to 
 *   garbage.  So don't store values at the same time you are
 *   examining their contents.
 */

gint64 kvp_value_get_gint64 (const KvpValue * value);
double kvp_value_get_double (const KvpValue * value);
QofNumeric kvp_value_get_numeric (const KvpValue * value);

/** Value accessor. This one is non-copying -- the caller can modify
 * the value directly. */
char *kvp_value_get_string (const KvpValue * value);

/** Value accessor. This one is non-copying -- the caller can modify
 * the value directly. */
GUID *kvp_value_get_guid (const KvpValue * value);

/** Value accessor. This one is non-copying -- the caller can modify
 * the value directly. */
void *kvp_value_get_binary (const KvpValue * value, guint64 * size_return);

/** Returns the GList of kvp_frame's (not to be confused with GList's
 * of something else!) from the given kvp_frame.  This one is
 * non-copying -- the caller can modify the value directly. */
GList *kvp_value_get_glist (const KvpValue * value);

/** Value accessor. This one is non-copying -- the caller can modify
 * the value directly. */
KvpFrame *kvp_value_get_frame (const KvpValue * value);

QofTime*
kvp_value_get_time (const KvpValue * value);

/**
 * Similar returns as strcmp.
 **/
gint kvp_value_compare (const KvpValue * va, const KvpValue * vb);

/** @} */

/** \brief General purpose function to convert any KvpValue to a string.

Only the bare string is returned, there is no debugging information.
*/
gchar *kvp_value_to_bare_string (const KvpValue * val);

/** \brief Debug version

This version is used only by ::qof_query_printValueForParam,
itself a debugging and development utility function.
*/
gchar *kvp_value_to_string (const KvpValue * val);

/** Manipulator: 
 *
 * copying - but more efficient than creating a new KvpValue manually. */
gboolean kvp_value_binary_append (KvpValue * v, void *data, guint64 size);

/** @name  Iterators
@{
*/

/** \since 0.7.2 */
typedef void (*KvpValueForeachCB) (const char *key, KvpValue * value, gpointer data);

/** Traverse all of the slots in the given kvp_frame.  This function
   does not descend recursively to traverse any kvp_frames stored as
   slot values.  You must handle that in proc, with a suitable
   recursive call if desired. */
void 
kvp_frame_for_each_slot (KvpFrame * f, KvpValueForeachCB, gpointer data);
//  void (*proc) (const char *key, KvpValue * value, gpointer data), gpointer data);

/** @} */


/** @} */
#endif
