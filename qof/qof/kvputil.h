/********************************************************************\
 * kvputil.h -- miscellaneous KVP utilities                         *
 * Copyright (C) 2003 Linas Vepstas <linas@linas.org>               *
 * Copyright (c) 2006, 2008 Neil Williams <linux@codehelp.co.uk>    *
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
\********************************************************************/

/** @addtogroup KVP
    @{ 
*/
/** @file kvputil.h 
    @brief QOF KVP utility functions 
 */
/**  @name Hash Utilities
 @{ 
*/

#ifndef KVPUTIL_H
#define KVPUTIL_H

typedef struct
{
	gpointer key;
	gpointer value;
} GHashTableKVPair;

/** 
  Returns a GSList* of all the
  keys and values in a given hash table.  Data elements of lists are
  actual hash elements, so be careful, and deallocation of the
  GHashTableKVPairs in the result list are the caller's
  responsibility.  A typical sequence might look like this:

    GSList *kvps = g_hash_table_key_value_pairs(hash);  
    ... use kvps->data->key and kvps->data->val, etc. here ...
    g_slist_foreach(kvps, g_hash_table_kv_pair_free_gfunc, NULL);
    g_slist_free(kvps);

*/

GSList *g_hash_table_key_value_pairs (GHashTable * table);
void g_hash_table_kv_pair_free_gfunc (gpointer data, gpointer user_data);

/** @name KvpFrame URL handling
 @{
*/
/** The kvp_frame_add_url_encoding() routine will parse the
 *  value string, assuming it to be URL-encoded in the standard way,
 *  turning it into a set of key-value pairs, and adding those to the
 *  indicated frame.  URL-encoded strings are the things that are
 *  returned by web browsers when a form is filled out.  For example,
 *  'start-date=June&end-date=November' consists of two keys, 
 *  'start-date' and 'end-date', which have the values 'June' and 
 *  'November', respectively.  This routine also handles % encoding.
 *
 *  This routine treats all values as strings; it does *not* attempt
 *  to perform any type-conversion.
 * */
void kvp_frame_add_url_encoding (KvpFrame * frame, const gchar * enc);

/** @} */

/**
 * Similar returns as strcmp.
 */
gint kvp_frame_compare (const KvpFrame * fa, const KvpFrame * fb);

gchar *kvp_frame_to_string (const KvpFrame * frame);
gchar *binary_to_string (const void *data, guint32 size);
gchar *kvp_value_glist_to_string (const GList * list);
GHashTable *kvp_frame_get_hash (const KvpFrame * frame);

/** @name KvpBag Bags of GUID Pointers 
 @{ 
*/

/** The qof_kvp_bag_add() routine is used to maintain a collection 
 *  of pointers in a kvp tree.
 *
 *  The thing being pointed at is uniquely identified by its GUID. 
 *  This routine is typically used to create a linked list, and/or
 *  a collection of pointers to objects that are 'related' to each 
 *  other in some way.
 *
 *  The var-args should be pairs of strings (const char *) followed by
 *  the corresponding GUID pointer (const GUID *).  Terminate the varargs
 *  with a NULL as the last string argument.
 *
 *  The actual 'pointer' is stored in a subdirectory in a bag located at
 *  the node directory 'path'.  A 'bag' is merely a collection of
 *  (unamed) values.  The name of our bag is 'path'. A bag can contain
 *  any kind of values, including frames.  This routine will create a
 *  frame, and put it in the bag.  The frame will contain named data
 *  from the subroutine arguments.  Thus, for example:
 *
 *  qof_kvp_array (kvp, "foo", secs, "acct_guid", aguid, 
 *                                   "book_guid", bguid, NULL);
 *
 *  will create a frame containing "/acct_guid" and "/book_guid", whose
 *  values are aguid and bguid respecitvely.  The frame will also
 *  contain "/date", whose value will be secs.  This frame will be
 *  placed into the bag located at "foo". 
 *
 *  This routine returns a pointer to the frame that was created, or 
 *  NULL if an error occured.
 
*/

KvpFrame *
qof_kvp_bag_add (KvpFrame * kvp_root, const gchar *path, 
				QofTime *qt, const gchar *first_name, ...);

/** The qof_kvp_bag_merge() routine will move the bag contents from
 *    the 'kvp_from', to the 'into' bag.  It will then delete the 
 *    'from' bag from the kvp tree.
 
 */
void 
qof_kvp_bag_merge (KvpFrame * kvp_into, const gchar *intopath,
				   KvpFrame * kvp_from, const gchar *frompath);

/** The qof_kvp_bag_find_by_guid() routine examines the bag pointed
 *    located at root.  It looks for a frame in that bag that has the
 *    guid value of "desired_guid" filed under the key name "guid_name".
 *    If it finds that matching guid, then it returns a pointer to 
 *    the KVP frame that contains it.  If it is not found, or if there
 *    is any other error, NULL is returned.
 
 */

KvpFrame *
qof_kvp_bag_find_by_guid (KvpFrame * root, const gchar *path,
						  const gchar *guid_name,
						  GUID * desired_guid);

/** Remove the given frame from the bag.  The frame is removed,
 *  however, it is not deleted.  Note that the frame pointer must
 *  be a pointer to the actual frame (for example, as returned by
 *  gnc_kvp_bag_find_by_guid() for by gnc_kvp_bag_add()), and not
 *  some copy of the frame.
 */

void 
qof_kvp_bag_remove_frame (KvpFrame * root, const gchar *path,
						  KvpFrame * fr);

/***********************************************************************/

/** @} */
/** @} */
#endif /* KVPUTIL_H */
