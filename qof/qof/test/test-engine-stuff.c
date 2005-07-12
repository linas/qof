/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
/**
 * @file test-engine-stuff.c
 * @brief tools to set up random test data.
 *
 * Created by Linux Developers Group, 2001
 * Updates Linas Vepstas July 2004
 */

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gnc-date.h"
#include "qofquerycore.h"

#include "test-engine-stuff.h"
#include "test-stuff.h"

static gboolean glist_strings_only = FALSE;

static GHashTable *exclude_kvp_types = NULL;
static gint kvp_max_depth = 5;
static gint kvp_frame_max_elements = 10;

gboolean gnc_engine_debug_random = FALSE;

/* ========================================================== */
/* Set control parameters governing the run. */

void
set_max_kvp_depth (gint max_kvp_depth)
{
  kvp_max_depth = MAX (max_kvp_depth, 1);
}

void
set_max_kvp_frame_elements (gint max_kvp_frame_elements)
{
  kvp_frame_max_elements = MAX (max_kvp_frame_elements, 1);
}

void
kvp_exclude_type (KvpValueType kvp_type)
{
  gint *key;

  if (!exclude_kvp_types)
    exclude_kvp_types = g_hash_table_new (g_int_hash, g_int_equal);

  key = g_new (gint, 1);
  *key = kvp_type;

  g_hash_table_insert (exclude_kvp_types, key, exclude_kvp_types);
}

static gboolean
kvp_type_excluded (KvpValueType kvp_type)
{
  gint key = kvp_type;

  if (!exclude_kvp_types)
    return FALSE;

  if (g_hash_table_lookup (exclude_kvp_types, &key))
    return TRUE;

  return FALSE;
}

void
random_glist_strings_only (gboolean strings_only)
{
  glist_strings_only = strings_only;
}

static gboolean zero_nsec = FALSE;

void
random_timespec_zero_nsec (gboolean zero_nsec_in)
{
  zero_nsec = zero_nsec_in;
}

static gboolean usec_resolution = FALSE;

void
random_timespec_usec_resolution (gboolean usec_resolution_in)
{
  usec_resolution = usec_resolution_in;
}

/* ========================================================== */

static gint borked = 80;

static inline gboolean
do_bork (void)
{
  if (1 == get_random_int_in_range (0, borked)) 
  {
    return TRUE;
  }
  return FALSE;
}

/* ========================================================== */
/* GList stuff */
/*
static gpointer
get_random_list_element (GList *list)
{
  g_return_val_if_fail (list, NULL);

  return g_list_nth_data (list,
                          get_random_int_in_range (0,
                                                   g_list_length (list) - 1));
}
*/
static kvp_value* get_random_kvp_value_depth (int type, gint depth);

static GList*
get_random_glist_depth (gint depth)
{
    GList *ret = NULL;
    int count = get_random_int_in_range(1, 5);
    int i;

    if (depth >= kvp_max_depth)
      return NULL;

    for (i = 0; i < count; i++)
    {
        KvpValueType kvpt;
        KvpValue *value;

        kvpt = glist_strings_only ? KVP_TYPE_STRING : -2;

        do
        {
          value = get_random_kvp_value_depth (kvpt, depth + 1);
        }
        while (!value);

        ret = g_list_prepend(ret, value);
    }

    return ret;
}

GList*
get_random_glist(void)
{
  return get_random_glist_depth (0);
}

/* ========================================================== */
/* Time/Date, GUID, binary data stuff */

Timespec*
get_random_timespec(void)
{
  Timespec *ret;

  ret = g_new0(Timespec, 1);

  while (ret->tv_sec <= 0)
    ret->tv_sec = rand();

  if (zero_nsec)
    ret->tv_nsec = 0;
  else
  {
    ret->tv_nsec = rand();

    if (usec_resolution)
    {
      ret->tv_nsec = MIN (ret->tv_nsec, 999999999);
      ret->tv_nsec /= 1000;
      ret->tv_nsec *= 1000;
    }
  }

  return ret;
}

GUID*
get_random_guid(void)
{
    GUID *ret;

    ret = g_new(GUID, 1);
    guid_new(ret);

    return ret;
}

bin_data*
get_random_binary_data(void)
{
    int len;
    bin_data *ret;

    len = get_random_int_in_range(20,100);
    ret = g_new(bin_data, 1);
    ret->data = g_new(guchar, len);
    ret->len = len;

    for(len--; len >= 0; len--)
    {
        ret->data[len] = (char)get_random_int_in_range(0,255);
    }

    return ret;
}

/* ========================================================== */
/* KVP stuff */

static KvpFrame* get_random_kvp_frame_depth (gint depth);

static KvpValue*
get_random_kvp_value_depth (int type, gint depth)
{
    int datype = type;
    KvpValue *ret;

    if (datype == -1)
    {
        datype = get_random_int_in_range(KVP_TYPE_GINT64, KVP_TYPE_FRAME);
    }

    if (datype == -2)
    {
        datype = get_random_int_in_range(KVP_TYPE_GINT64, KVP_TYPE_FRAME - 1);
    }

    if (datype == KVP_TYPE_FRAME && depth >= kvp_max_depth)
      return NULL;

    if (datype == KVP_TYPE_GLIST && depth >= kvp_max_depth)
      return NULL;

    if (kvp_type_excluded (datype))
      return NULL;

    switch(datype)
    {
    case KVP_TYPE_GINT64:
        ret = kvp_value_new_gint64(get_random_gint64());
        break;

    case KVP_TYPE_DOUBLE:
        ret = NULL;
        break;

    case KVP_TYPE_NUMERIC:
        ret = kvp_value_new_gnc_numeric(get_random_gnc_numeric());
        break;

    case KVP_TYPE_STRING:
    {
        gchar *tmp_str;
        tmp_str = get_random_string();
        if(!tmp_str)
	  return NULL;
        
        ret = kvp_value_new_string(tmp_str);
        g_free(tmp_str);
    }
        break;

    case KVP_TYPE_GUID:
    {
        GUID *tmp_guid;
        tmp_guid = get_random_guid();
        ret = kvp_value_new_guid(tmp_guid);
        g_free(tmp_guid);
    }
        break;

    case KVP_TYPE_TIMESPEC:
    {
        Timespec *ts = get_random_timespec();
        ret = kvp_value_new_timespec (*ts);
        g_free(ts);
    }
	break;
    
    case KVP_TYPE_BINARY:
    {
        bin_data *tmp_data;
        tmp_data = get_random_binary_data();
        ret = kvp_value_new_binary(tmp_data->data, tmp_data->len);
        g_free(tmp_data->data);
        g_free(tmp_data);
    }
        break;
 
    case KVP_TYPE_GLIST:
        ret = kvp_value_new_glist_nc(get_random_glist_depth (depth + 1));
        break;

    case KVP_TYPE_FRAME:
    {
        KvpFrame *tmp_frame;
        tmp_frame = get_random_kvp_frame_depth(depth + 1);
        ret = kvp_value_new_frame(tmp_frame);
        kvp_frame_delete(tmp_frame);
    }
        break;

    default:
        ret = NULL;
        break;
    }
    return ret;
}

static KvpFrame*
get_random_kvp_frame_depth (gint depth)
{
    KvpFrame *ret;
    int vals_to_add;
    gboolean val_added;

    if (depth >= kvp_max_depth)
      return NULL;

    ret = kvp_frame_new();

    vals_to_add = get_random_int_in_range(1,kvp_frame_max_elements);
    val_added = FALSE;

    for (;vals_to_add > 0; vals_to_add--)
    {
        gchar *key;
        KvpValue *val;

	key = NULL;
	while (key == NULL) {
	  key = get_random_string_without("/");
	  if (*key == '\0') {
	    g_free(key);
	    key = NULL;
	  }
	}
	
        val = get_random_kvp_value_depth (-1, depth + 1);
        if (!val)
        {
	  g_free(key);
          if (!val_added)
            vals_to_add++;
          continue;
        }

        val_added = TRUE;

        kvp_frame_set_slot_nc(ret, key, val);

        g_free(key);
    }

    return ret;
}

KvpFrame *
get_random_kvp_frame (void)
{
  return get_random_kvp_frame_depth (0);
}

KvpValue *
get_random_kvp_value(int type)
{
  return get_random_kvp_value_depth (type, 0);
}

/* ================================================================= */
/* Numeric stuff */

#define RAND_IN_RANGE(X) (((X)*((gint64) (rand()+1)))/RAND_MAX)

gnc_numeric
get_random_gnc_numeric(void)
{
    gint64 numer;
    gint64 deno;

    if (RAND_MAX/8 > rand())
    {
       /* Random number between 1 and 6000 */
       deno = RAND_IN_RANGE(6000ULL);
    }
    else
    {
       gint64 norm = RAND_IN_RANGE (10ULL);

       /* multiple of 10, between 1 and 10 000 million */
       deno = 1;
       while (norm) 
       {
          deno *= 10;
          norm --;
       }
    }

    /* Arbitrary random numbers can cause pointless overflow 
     * during calculations.  Limit dynamic range in hopes 
     * of avoiding overflow. */
    numer = get_random_gint64()/100000;
    if (0 == numer) numer = 1;
    return gnc_numeric_create(numer, deno);
}
/*
static GList *
get_random_guids(int max)
{
  GList *guids = NULL;
  int num_guids;

  if (max < 1) return NULL;

  num_guids = get_random_int_in_range (1, max);

  while (num_guids-- > 0)
    g_list_prepend (guids, get_random_guid ());

  return guids;
}
*//*
static void
free_random_guids(GList *guids)
{
  GList *node;

  for (node = guids; node; node = node->next)
    g_free (node->data);

  g_list_free (guids);
}
*//*
static QofQueryOp
get_random_queryop(void)
{
  QofQueryOp op = get_random_int_in_range (1, QOF_QUERY_XOR);
  if (gnc_engine_debug_random) printf ("op = %d, ", op);
  return op;
}
*//*
static GSList *
get_random_kvp_path (void)
{
  GSList *path;
  gint len;

  path = NULL;
  len = get_random_int_in_range (1, kvp_max_depth);

  while (len--)
    path = g_slist_prepend (path, get_random_string ());

  return g_slist_reverse (path);
}
*//*
static void
free_random_kvp_path (GSList *path)
{
  GSList *node;

  for (node = path; node; node = node->next)
    g_free (node->data);

  g_slist_free (path);
}
*/
typedef enum {
  BY_STANDARD = 1,
  BY_DATE,
  BY_DATE_ENTERED,
  BY_DATE_RECONCILED,
  BY_NUM,
  BY_AMOUNT,
  BY_MEMO,
  BY_DESC,
  BY_NONE
} sort_type_t;

typedef struct
{
  QofIdType where;
  GSList *path;
  QofQuery *q;
} KVPQueryData;

TestQueryTypes
get_random_query_type (void)
{
  switch (get_random_int_in_range (0, 4))
  {
    case 0: return SIMPLE_QT;
    case 4: return GUID_QT;
    default: return SIMPLE_QT;
  }
}
