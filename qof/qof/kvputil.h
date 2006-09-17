/********************************************************************\
 * kvputil.h -- miscellaneous KVP utilities                         *
 * Copyright (C) 2003 Linas Vepstas <linas@linas.org>               *
 * Copyright (c) 2006 Neil Williams <linux@codehelp.co.uk>          *
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

/***********************************************************************/

/** @} */
/** @} */
#endif /* KVPUTIL_H */
