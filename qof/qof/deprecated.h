/********************************************************************
 *            deprecated.h
 *
 *  Mon Nov 21 14:08:25 2005
 *  Copyright  2005  Neil Williams
 *  linux@codehelp.co.uk
 ********************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef QOF_DISABLE_DEPRECATED

#ifndef _DEPRECATED_H
#define _DEPRECATED_H
#include "qof.h"

/** @file deprecated.h
	@brief transitional header from libqof1 to libqof2
*/

/** \deprecated use ::QofLogLevel instead */
#define gncLogLevel QofLogLevel

/** \deprecated use ::qof_log_init_filename instead */
void gnc_log_init (void);

/** \deprecated use ::qof_log_set_level insead. */
void gnc_set_log_level (QofLogModule module, gncLogLevel level);

/** \deprecated use ::qof_log_set_level_registered instead. */
void gnc_set_log_level_global (gncLogLevel level);

/** \deprecated use ::qof_log_set_level_registered instead.

This function has been deprecated because the function name
is very misleading. It may, in future, be modified to be
truly global (i.e. make changes to log modules where no
log_level has been explicitly set) but, despite the name,
it only ever modified known (gnucash) modules. Future changes
would require that if this function is made truly global it must
preserve the effect of qof_log_set_level_registered and the advantages
of silent log_modules for those programs that do not use _global. Support
could be required for removing log_modules from the hashtable.
*/
void qof_log_set_level_global (QofLogLevel level);

/** \deprecated use ::qof_log_set_file instead. */
void gnc_set_logfile (FILE * outfile);

/** \deprecated use ::qof_log_prettify instead. */
const char *gnc_log_prettify (const char *name);

/** \deprecated use ::qof_log_check instead. */
gboolean gnc_should_log (QofLogModule log_module, gncLogLevel log_level);

/** \deprecated use ::QOF_LOG_FATAL */
#define GNC_LOG_FATAL   QOF_LOG_FATAL
/** \deprecated use ::QOF_LOG_ERROR */
#define GNC_LOG_ERROR   QOF_LOG_ERROR
/** \deprecated use ::QOF_LOG_WARNING */
#define GNC_LOG_WARNING QOF_LOG_WARNING
/** \deprecated use ::QOF_LOG_INFO */
#define GNC_LOG_INFO    QOF_LOG_INFO
/** \deprecated use ::QOF_LOG_DEBUG */
#define GNC_LOG_DEBUG   QOF_LOG_DEBUG
/** \deprecated use ::QOF_LOG_DETAIL */
#define GNC_LOG_DETAIL  QOF_LOG_DETAIL
/** \deprecated use ::QOF_LOG_TRACE */
#define GNC_LOG_TRACE   QOF_LOG_TRACE

/** \deprecated no replacement */
void gnc_start_clock (int, QofLogModule, gncLogLevel, const char *,
					  const char *, ...);
/** \deprecated no replacement */
void gnc_report_clock (int, QofLogModule, gncLogLevel, const char *,
					   const char *, ...);
/** \deprecated no replacement */
void gnc_report_clock_total (int, QofLogModule, gncLogLevel, const char *,
							 const char *, ...);

/** \deprecated use ::QOF_EVENT_NONE instead. */
#define  GNC_EVENT_NONE   QOF_EVENT_NONE
/** \deprecated use ::QOF_EVENT_CREATE instead. */
#define  GNC_EVENT_CREATE QOF_EVENT_CREATE
/** \deprecated use ::QOF_EVENT_MODIFY instead. */
#define  GNC_EVENT_MODIFY QOF_EVENT_MODIFY
/** \deprecated use ::QOF_EVENT_DESTROY instead. */
#define  GNC_EVENT_DESTROY QOF_EVENT_DESTROY
/** \deprecated use ::QOF_EVENT_ADD instead. */
#define  GNC_EVENT_ADD    QOF_EVENT_ADD
/** \deprecated use ::QOF_EVENT_REMOVE instead. */
#define GNC_EVENT_REMOVE  QOF_EVENT_REMOVE
/** \deprecated use ::QOF_EVENT_ALL */
#define GNC_EVENT_ALL     QOF_EVENT_ALL
/** \deprecated use ::QofEventName instead. */
#define GNCEngineEventType QofEventId
/** \deprecated use ::QofEventHandler instead. */
typedef void (*GNCEngineEventHandler) (GUID * entity, QofIdType type,
									   GNCEngineEventType event_type,
									   gpointer user_data);
/** \deprecated For backwards compatibility - New code must not use
this function. The function and the handler prototype will be remove
from qofevent.c in libqof2 */
gint qof_event_register_old_handler (GNCEngineEventHandler old_handler,
									 gpointer user_data);
/** \deprecated use ::qof_event_register_handler instead. */
gint gnc_engine_register_event_handler (GNCEngineEventHandler handler,
										gpointer user_data);
/** \deprecated use ::qof_event_unregister_handler instead. */
void gnc_engine_unregister_event_handler (gint handler_id);
/** \deprecated use ::qof_event_gen instead. */
void gnc_engine_gen_event (QofEntity * entity, GNCEngineEventType event_type);
/** \deprecated use ::qof_event_suspend instead. */
void gnc_engine_suspend_events (void);
/** \deprecated use ::qof_event_resume instead. */
void gnc_engine_resume_events (void);
/** \deprecated use ::qof_event_gen instead. */
void gnc_engine_generate_event (const GUID * guid, QofIdType e_type,
								GNCEngineEventType event_type);
/** \deprecated use ::QofBookMergeResult instead. */
#define qof_book_mergeResult QofBookMergeResult
/** \deprecated use ::QofBookMergeRule instead. */
#define qof_book_mergeRule QofBookMergeRule
/** \deprecated use ::QofBookMergeData instead. */
#define qof_book_mergeData QofBookMergeData
/** \deprecated use ::qof_book_merge_init instead. */
QofBookMergeData *qof_book_mergeInit (QofBook * importBook,
									  QofBook * targetBook);
/** \deprecated use ::QofBookMergeRuleForeachCB instead. */
typedef void (*qof_book_mergeRuleForeachCB) (QofBookMergeData *,
											 QofBookMergeRule *, guint);
/** \deprecated use ::qof_book_merge_rule_foreach instead. */
void qof_book_mergeRuleForeach (QofBookMergeData * mergeData,
								QofBookMergeRuleForeachCB callback,
								QofBookMergeResult mergeResult);
/** \deprecated use ::qof_book_merge_update_result instead. */
QofBookMergeData *qof_book_mergeUpdateResult (QofBookMergeData * mergeData,
											  QofBookMergeResult tag);
/** \deprecated use ::qof_book_merge_commit instead. */
gint qof_book_mergeCommit (QofBookMergeData * mergeData);
/** \deprecated Use the function versions, ::safe_strcmp() and
::safe_strcasecmp() instead. */
#define SAFE_STRCMP_REAL(fcn,da,db) {    \
  if ((da) && (db)) {                    \
    if ((da) != (db)) {                  \
      gint retval = fcn ((da), (db));    \
      /* if strings differ, return */    \
      if (retval) return retval;         \
    }                                    \
  } else                                 \
  if ((!(da)) && (db)) {                 \
    return -1;                           \
  } else                                 \
  if ((da) && (!(db))) {                 \
    return +1;                           \
  }                                      \
}
/** \deprecated use ::safe_strcmp() */
#define SAFE_STRCMP(da,db) SAFE_STRCMP_REAL(strcmp,(da),(db))
/** \deprecated use ::safe_strcasecmp() */
#define SAFE_STRCASECMP(da,db) SAFE_STRCMP_REAL(strcasecmp,(da),(db))
/** \deprecated use ::qof_util_string_cache_insert instead. */
gpointer gnc_string_cache_insert (gconstpointer key);
/** \deprecated use ::QOF_SCANF_LLD instead. */
#define GNC_SCANF_LLD QOF_SCANF_LLD
/** \deprecated use ::qof_util_stpcpy instead. */
gchar *gnc_stpcpy (gchar * dest, const gchar * src);
/** \deprecated use ::qof_init instead. */
GCache *gnc_engine_get_string_cache (void);
/** \deprecated use ::qof_init instead. */
GCache *qof_util_get_string_cache (void);
/** \deprecated use ::qof_close instead. */
void gnc_engine_string_cache_destroy (void);
/** \deprecated use ::qof_util_string_cache_remove instead. */
void gnc_string_cache_remove (gconstpointer key);
/** \deprecated no replacement. */
void qof_book_set_schedxactions (QofBook * book, GList * newList);
/** \deprecated use ::QofDateMonthFormat instead. */
typedef enum
{
	/** \deprecated use ::QOF_DATE_MONTH_NUMBER. */
	GNCDATE_MONTH_NUMBER,
	/** \deprecated use ::QOF_DATE_MONTH_ABBREV. */
	GNCDATE_MONTH_ABBREV,
	/** \deprecated use ::QOF_DATE_MONTH_NAME. */
	GNCDATE_MONTH_NAME
} GNCDateMonthFormat;
/** \deprecated use ::qof_date_format_to_name. */
const gchar* gnc_date_dateformat_to_string(QofDateFormat format);
/** \deprecated no replacement. */
const gchar* gnc_date_monthformat_to_string(GNCDateMonthFormat format);
/** \deprecated use ::qof_date_format_from_name.
 
QofDateFormat qof_date_format_from_name ( const gchar * name )

\note The prototype of ::qof_date_string_to_format is not the
same as gnc_date_string_to_dateformat! The format argument type
and the return value have been changed.
*/
gboolean gnc_date_string_to_dateformat (const gchar * format_string,
										QofDateFormat * format);
/** \deprecated no replacement. */
gboolean gnc_date_string_to_monthformat (const gchar * format_string,
										 GNCDateMonthFormat * format);
/** \deprecated use ::qof_time_equal. */
gboolean timespec_equal (const Timespec * ta, const Timespec * tb);
/** \deprecated use ::qof_time_cmp. */
gint timespec_cmp (const Timespec * ta, const Timespec * tb);
/** \deprecated use ::qof_time_diff. */
Timespec timespec_diff (const Timespec * ta, const Timespec * tb);
/** \deprecated use ::qof_time_abs. */
Timespec timespec_abs (const Timespec * t);
/** \deprecated use ::qof_time_canonical_day_time. */
Timespec timespecCanonicalDayTime (Timespec t);
/** \deprecated use ::qof_time_to_time_t. */
time_t timespecToTime_t (Timespec ts);
/** \deprecated use ::qof_time_from_time_t instead */
void timespecFromTime_t (Timespec * ts, time_t t);
/** \deprecated use GDate instead. */
Timespec gnc_dmy2timespec (gint day, gint month, gint year);
/** \deprecated use GDate instead. */
Timespec gnc_dmy2timespec_end (gint day, gint month, gint year);
/** \deprecated set ::QOF_DATE_FORMAT_ISO8601 and call ::qof_date_parse */
Timespec gnc_iso8601_to_timespec_gmt (const gchar *);
/** \deprecated set ::QOF_DATE_FORMAT_ISO8601 and call ::qof_date_print */
gchar *gnc_timespec_to_iso8601_buff (Timespec ts, gchar * buff);
/** \deprecated use ::QofDate instead. */
void gnc_timespec2dmy (Timespec ts, gint * day, gint * month, gint * year);
/** \deprecated use ::qof_date_time_add_months. */
void date_add_months (struct tm *tm, gint months, gboolean track_last_day);
/** \deprecated use ::qof_date_adddays instead. */
gboolean qof_date_add_days(Timespec *ts, gint days);
/** \deprecated use ::qof_date_addmonths instead. */
gboolean qof_date_add_months(Timespec *ts, gint months, gboolean track_last_day);
/** \deprecated no direct replacement. */
const gchar *qof_date_format_get_string(QofDateFormat df);
/** \deprecated use ::qof_date_dmy_to_sec. */
time_t xaccDMYToSec (gint day, gint month, gint year);
/** \deprecated no replacement */
glong gnc_timezone (struct tm *tm);
/** \deprecated use ::qof_date_format_get_current */
QofDateFormat qof_date_format_get(void);
/** \deprecated use ::qof_date_format_set_current. */
void qof_date_format_set(QofDateFormat df);
/** \deprecated use ::qof_date_format_get_format */
const gchar *qof_date_text_format_get_string(QofDateFormat df);
/** \deprecated use ::qof_date_format_get_date_separator */
char dateSeparator (void);
/** \deprecated returns incorrect values for dates before 1970.
use ::qof_time_dmy_to_time.*/
time_t qof_date_dmy_to_sec (gint day, gint month, gint year);
/** \deprecated no replacement. */
size_t qof_print_hours_elapsed_buff (char *buff, size_t len, int secs,
									 gboolean show_secs);
/** \deprecated no replacement. */
size_t qof_print_minutes_elapsed_buff (char *buff, size_t len, int secs,
									   gboolean show_secs);
/** \deprecated no replacement. */
size_t qof_print_time_buff (char *buff, size_t len, time_t secs);
/** \deprecated no replacement. */
size_t qof_print_date_time_buff (char *buff, size_t len, time_t secs);
/** \deprecated no replacement */
gboolean qof_is_same_day (time_t, time_t);
/** \deprecated no replacement. */
void gnc_tm_get_day_start (struct tm *tm, time_t time_val);
/** \deprecated use ::qof_time_set_day_start instead. */
void gnc_tm_set_day_start (struct tm *tm);
/** \deprecated use ::qof_time_set_day_middle instead. */
void gnc_tm_set_day_middle (struct tm *tm);
/** \deprecated use ::qof_time_set_day_end instead. */
void gnc_tm_set_day_end (struct tm *tm);
/** \deprecated no replacement. */
void gnc_tm_get_day_end (struct tm *tm, time_t time_val);
/** \deprecated no replacement. */
time_t gnc_timet_get_day_start (time_t time_val);
/** \deprecated no replacement. */
time_t gnc_timet_get_day_end (time_t time_val);
/** \deprecated no replacement. */
void gnc_tm_get_today_start (struct tm *tm);
/** \deprecated no replacement */
void gnc_tm_get_today_end (struct tm *tm);
/** \deprecated no replacement */
time_t gnc_timet_get_today_start (void);
/** \deprecated no replacement. */
time_t gnc_timet_get_today_end (void);
#ifndef GNUCASH_MAJOR_VERSION
/** @deprecated no replacement*/
time_t gnc_timet_get_day_start_gdate (GDate * date);
/**  @deprecated no replacement*/
time_t gnc_timet_get_day_end_gdate (GDate * date);
#endif /* GNUCASH_MAJOR_VERSION */
/** \deprecated use g_date_get_days_in_month instead. */
int date_get_last_mday (struct tm *tm);
/** \deprecated use g_date_get_days_in_month instead. */
gboolean date_is_last_mday (struct tm *tm);
/** \deprecated use g_date_get_days_in_month instead. */
int gnc_date_my_last_mday (int month, int year);
/** \deprecated use g_date_get_days_in_month instead. */
int gnc_timespec_last_mday (Timespec ts);
/** \deprecated use qof_time_stamp_now */
char * xaccDateUtilGetStampNow (void);
/** \deprecated no direct replacement, use a QofDateFormat */
char *xaccDateUtilGetStamp (time_t thyme);
/** \deprecated no replacement. */
size_t qof_print_date_dmy_buff (gchar * buff, size_t buflen,
								gint day, gint month, gint year);
/** \deprecated no replacement. */
size_t qof_print_date_buff (char *buff, size_t buflen, time_t secs);
/** \deprecated no replacement */
size_t qof_print_gdate (char *buf, size_t bufflen, GDate * gd);
/** \deprecated use ::qof_date_print */
char *qof_print_date (time_t secs);
/** \deprecated use ::qof_date_print */
const char *gnc_print_date (Timespec ts);
/** \deprecated use ::qof_date_parse */
gboolean qof_scan_date (const char *buff, int *day, int *month, int *year);
/** \deprecated no replacement */
gboolean qof_scan_date_secs (const char *buff, time_t * secs);
/** @deprecated use qof_entity_set_guid instead but only in
backends (when reading the GUID from the data source). */
#define qof_book_set_guid(book,guid)    \
         qof_entity_set_guid(QOF_ENTITY(book), guid)
/** \deprecated use ::QOF_TYPE_TIME instead */
#define QOF_TYPE_DATE      "date"
/** \deprecated use ::qof_instance_set_update_time instead. */
Timespec qof_instance_get_last_update (QofInstance * inst);
void qof_instance_set_last_update (QofInstance * inst, Timespec ts);
/** \deprecated use ::kvp_frame_set_time instead. */
void kvp_frame_set_timespec (KvpFrame * frame, const gchar * path,
			 Timespec ts);
/** \deprecated use ::kvp_frame_add_time instead. */
void kvp_frame_add_timespec (KvpFrame * frame, const gchar * path,
			 Timespec ts);
/** \deprecated use ::kvp_value_get_time instead. */
Timespec kvp_value_get_timespec (const KvpValue * value);
/** \deprecated use ::kvp_frame_get_time instead. */
Timespec kvp_frame_get_timespec (const KvpFrame * frame, const gchar * path);
/** \deprecated use ::kvp_value_new_time instead. */
KvpValue *kvp_value_new_timespec (Timespec timespec);
/** \deprecated */
#define qof_book_get_guid(X) qof_entity_get_guid (QOF_ENTITY(X))
/** \deprecated no replacement. */
void qof_start_clock (gint clockno, QofLogModule log_module,
	QofLogLevel log_level, const gchar * function_name,
	const gchar * format, ...);
/** \deprecated no replacement. */
void qof_report_clock (gint clockno,
	QofLogModule log_module,
	QofLogLevel log_level,
	const gchar * function_name,
	const gchar * format, ...);
/** \deprecated no replacement. */
void qof_report_clock_total (gint clockno,
	QofLogModule log_module,
	QofLogLevel log_level,
	const gchar * function_name,
	const gchar * format, ...);
/** \deprecated no replacement. */
#define START_CLOCK(clockno,format, args...) do {        \
  if (qof_log_check (log_module, QOF_LOG_INFO))          \
    qof_start_clock (clockno, log_module, QOF_LOG_INFO,  \
             __FUNCTION__, format , ## args);               \
} while (0)
/** \deprecated no replacement. */
#define REPORT_CLOCK(clockno,format, args...) do {       \
  if (qof_log_check (log_module, QOF_LOG_INFO))          \
    qof_report_clock (clockno, log_module, QOF_LOG_INFO, \
             __FUNCTION__, format , ## args);               \
} while (0)
/** \deprecated no replacement. */
#define REPORT_CLOCK_TOTAL(clockno,format, args...) do {       \
  if (qof_log_check (log_module, QOF_LOG_INFO))                \
    qof_report_clock_total (clockno, log_module, QOF_LOG_INFO, \
             __FUNCTION__, format , ## args);               \
} while (0)

/** \deprecated Do not call directly, use QofLog. */
void qof_query_print (QofQuery * query);
/** \deprecated use ::query_time_t instead. */
typedef struct
{
	QofQueryPredData pd;
	QofDateMatch options;
	Timespec date;
} query_date_def, *query_date_t;
/** \deprecated use ::qof_query_time_predicate instead. */
QofQueryPredData *
qof_query_date_predicate (QofQueryCompare how,
						  QofDateMatch options,
						  Timespec date);
/** \deprecated use ::qof_query_time_predicate_get_time instead. */
gboolean 
qof_query_date_predicate_get_date (QofQueryPredData * pd,
								   Timespec * date);
/** \deprecated Each application should keep
their \b own session context.*/
QofSession *qof_session_get_current_session (void);
/** \deprecated Each application should keep
their \b own session context.*/
void qof_session_set_current_session (QofSession * session);
/** \deprecated Each application should keep
their \b own session context.*/
void
qof_session_clear_current_session (void);
/** \deprecated use ::qof_util_string_isnum */
gboolean gnc_strisnum (const guchar * s);
/** \deprecated use ::qof_kvp_bag_add instead. */
KvpFrame *
gnc_kvp_bag_add (KvpFrame * pwd, const char *path,
	time_t secs, const char *first_name, ...);
/** \deprecated use ::qof_kvp_bag_find_by_guid instead. */
KvpFrame *
gnc_kvp_bag_find_by_guid (KvpFrame * root, const char *path,
	const char *guid_name, GUID * desired_guid);
/** \deprecated use ::qof_kvp_bag_remove_frame instead. */
void
gnc_kvp_bag_remove_frame (KvpFrame * root, const char *path, 
						  KvpFrame * fr);
/** \deprecated use ::qof_kvp_bag_merge instead. */
void
gnc_kvp_bag_merge (KvpFrame * kvp_into, const char *intopath,
	KvpFrame * kvp_from, const char *frompath);
/** \deprecated use ::qof_util_param_edit instead
to edit at a parameter level, instead of a complete instance.

\warning <b>Important</b> The only workable implementation of
this deprecated routine causes <b>a lot</b> of unnecessary work
in the backend. All users should refactor their code to check 
whether the incoming data is different to the existing data and
avoid editing that parameter.

*/
#define QOF_BEGIN_EDIT(inst)   qof_begin_edit (inst)
/** \deprecated use ::qof_util_param_edit instead
to edit at a parameter level, instead of a complete instance.

\warning <b>Important</b> The only workable implementation of
this deprecated routine causes <b>a lot</b> of unnecessary work
in the backend. All users should refactor their code to check 
whether the incoming data is different to the existing data and
avoid editing that parameter.

\param inst pointer to the instance to prepare to edit.

*/
gboolean qof_begin_edit (QofInstance * inst);
/** \deprecated No replacement. See ::qof_commit_edit */
#define QOF_COMMIT_EDIT_PART1(inst) qof_commit_edit (inst)

/** \deprecated Use ::qof_util_param_commit instead. 

\warning <b>Important</b> The only workable implementation of
this deprecated routine causes <b>a lot</b> of unnecessary work
in the backend. All users should refactor their code to check 
whether the incoming data is different to the existing data and
avoid editing that parameter.

\param inst pointer to the instance to commit.
*/
gboolean qof_commit_edit (QofInstance * inst);

/** \deprecated No replacement.

\note This macro changes programme flow if the instance is freed.

*/
#define QOF_COMMIT_EDIT_PART2(inst,on_error,on_done,on_free)  {  \
  QofBackend * be;                                               \
                                                                 \
  be = qof_book_get_backend ((inst)->book);                      \
  if (be)                                                        \
  {                                                              \
    QofBackendError errcode;                                     \
                                                                 \
    errcode = qof_backend_get_error (be);                        \
    if (ERR_BACKEND_NO_ERR != errcode)                           \
    {                                                            \
      (inst)->do_free = FALSE;                                   \
      qof_backend_set_error (be, errcode);                       \
      (on_error)((inst), errcode);                               \
    }                                                            \
    (inst)->dirty = FALSE;                                       \
  }                                                              \
  (on_done)(inst);                                               \
                                                                 \
  if ((inst)->do_free) {                                         \
     (on_free)(inst);                                            \
     return;                                                     \
  }                                                              \
}

/** \deprecated use ::qof_util_param_to_string instead. */
gchar *
qof_util_param_as_string (QofEntity * ent, QofParam * param);
/** \deprecated */
typedef struct _QofNumeric gnc_numeric;
/** \deprecated use ::QOF_NUMERIC_RND_MASK. */
#define GNC_NUMERIC_RND_MASK     QOF_NUMERIC_RND_MASK
/** \deprecated use ::QOF_NUMERIC_DENOM_MASK. */
#define GNC_NUMERIC_DENOM_MASK   QOF_NUMERIC_DENOM_MASK
/** \deprecated use ::QOF_NUMERIC_SIGFIGS_MASK. */
#define GNC_NUMERIC_SIGFIGS_MASK QOF_NUMERIC_SIGFIGS_MASK
/** \deprecated use ::QOF_HOW_RND_FLOOR. */
#define GNC_HOW_RND_FLOOR		QOF_HOW_RND_FLOOR
/** \deprecated use ::QOF_HOW_RND_CEIL. */
#define GNC_HOW_RND_CEIL		QOF_HOW_RND_CEIL
/** \deprecated use ::QOF_HOW_RND_TRUNC. */
#define GNC_HOW_RND_TRUNC		QOF_HOW_RND_TRUNC
/** \deprecated use ::QOF_HOW_RND_PROMOTE. */
#define GNC_HOW_RND_PROMOTE		QOF_HOW_RND_PROMOTE
/** \deprecated use ::QOF_HOW_RND_ROUND_HALF_DOWN. */
#define GNC_HOW_RND_ROUND_HALF_DOWN QOF_HOW_RND_ROUND_HALF_DOWN
/** \deprecated use ::QOF_HOW_RND_ROUND_HALF_UP. */
#define GNC_HOW_RND_ROUND_HALF_UP   QOF_HOW_RND_ROUND_HALF_UP
/** \deprecated use ::QOF_HOW_RND_ROUND. */
#define GNC_HOW_RND_ROUND		QOF_HOW_RND_ROUND
/** \deprecated use ::QOF_HOW_RND_NEVER. */
#define GNC_HOW_RND_NEVER		QOF_HOW_RND_NEVER
/** \deprecated use ::QOF_HOW_DENOM_EXACT. */
#define GNC_HOW_DENOM_EXACT		QOF_HOW_DENOM_EXACT
/** \deprecated use ::QOF_HOW_DENOM_REDUCE. */
#define GNC_HOW_DENOM_REDUCE	QOF_HOW_DENOM_REDUCE
/** \deprecated use ::QOF_HOW_DENOM_LCD. */
#define GNC_HOW_DENOM_LCD		QOF_HOW_DENOM_LCD
/** \deprecated use ::QOF_HOW_DENOM_FIXED. */
#define GNC_HOW_DENOM_FIXED		QOF_HOW_DENOM_FIXED
/** \deprecated use ::QOF_HOW_DENOM_SIGFIG. */
#define GNC_HOW_DENOM_SIGFIG	QOF_HOW_DENOM_SIGFIG
/** \deprecated use ::QOF_HOW_DENOM_SIGFIGS. */
#define GNC_HOW_DENOM_SIGFIGS	QOF_HOW_DENOM_SIGFIGS
/** \deprecated use ::QOF_HOW_GET_SIGFIGS. */
#define GNC_HOW_GET_SIGFIGS		QOF_HOW_GET_SIGFIGS
/** \deprecated use ::QOF_ERROR_OK. */
#define GNC_ERROR_OK			QOF_ERROR_OK
/** \deprecated use ::QOF_ERROR_ARG. */
#define GNC_ERROR_ARG			QOF_ERROR_ARG
/** \deprecated use ::QOF_ERROR_OVERFLOW. */
#define GNC_ERROR_OVERFLOW		QOF_ERROR_OVERFLOW
/** \deprecated use ::QOF_ERROR_DENOM_DIFF. */
#define GNC_ERROR_DENOM_DIFF	QOF_ERROR_DENOM_DIFF
/** \deprecated use ::QOF_ERROR_REMAINDER. */
#define GNC_ERROR_REMAINDER		QOF_ERROR_REMAINDER
/** \deprecated use ::QofNumericErrorCode. */
#define GNCNumericErrorCode		QofNumericErrorCode
/** \deprecated use ::QOF_DENOM_AUTO. */
#define GNC_DENOM_AUTO			QOF_DENOM_AUTO
/** \deprecated use ::QOF_DENOM_RECIPROCAL. */
#define GNC_DENOM_RECIPROCAL	QOF_DENOM_RECIPROCAL
/** \deprecated use ::qof_numeric_create */
static inline gnc_numeric
gnc_numeric_create (gint64 num, gint64 denom)
{
	QofNumeric out;
	out.num = num;
	out.denom = denom;
	return out;
}
/** \deprecated use ::qof_numeric_zero */
static inline gnc_numeric
gnc_numeric_zero (void)
{
	return qof_numeric_create (0, 1);
}
/** \deprecated use ::qof_numeric_from_double */
gnc_numeric 
double_to_gnc_numeric (double in, gint64 denom, gint how);
/** \deprecated use ::qof_numeric_from_string */
gboolean 
string_to_gnc_numeric (const gchar * str, gnc_numeric * n);
/** \deprecated use ::qof_numeric_error */
gnc_numeric 
gnc_numeric_error (GNCNumericErrorCode error_code);
/** \deprecated use ::qof_numeric_num */
static inline gint64
gnc_numeric_num (gnc_numeric a)
{
	return a.num;
}
/** \deprecated use ::qof_numeric_denom */
static inline gint64
gnc_numeric_denom (gnc_numeric a)
{
	return a.denom;
}
/** \deprecated use ::qof_numeric_to_double */
gdouble 
gnc_numeric_to_double (gnc_numeric in);
/** \deprecated use ::qof_numeric_to_string */
gchar *
gnc_numeric_to_string (gnc_numeric n);
/** \deprecated use ::qof_numeric_dbg_to_string */
gchar *
gnc_num_dbg_to_string (gnc_numeric n);
/** \deprecated use ::qof_numeric_check */
GNCNumericErrorCode 
gnc_numeric_check (gnc_numeric a);
/** \deprecated use ::qof_numeric_compare */
gint 
gnc_numeric_compare (gnc_numeric a, gnc_numeric b);
/** \deprecated use ::qof_numeric_zero_p */
gboolean 
gnc_numeric_zero_p (gnc_numeric a);
/** \deprecated use ::qof_numeric_negative_p */
gboolean 
gnc_numeric_negative_p (gnc_numeric a);
/** \deprecated use ::qof_numeric_positive_p */
gboolean 
gnc_numeric_positive_p (gnc_numeric a);
/** \deprecated use ::qof_numeric_eq */
gboolean 
gnc_numeric_eq (gnc_numeric a, gnc_numeric b);
/** \deprecated use ::qof_numeric_equal */
gboolean 
gnc_numeric_equal (gnc_numeric a, gnc_numeric b);
/** \deprecated use ::qof_numeric_same */
gint 
gnc_numeric_same (gnc_numeric a, gnc_numeric b, 
				  gint64 denom, gint how);
/** \deprecated use ::qof_numeric_add */
gnc_numeric 
gnc_numeric_add (gnc_numeric a, gnc_numeric b,
				 gint64 denom, gint how);
/** \deprecated use ::qof_numeric_sub */
gnc_numeric 
gnc_numeric_sub (gnc_numeric a, gnc_numeric b,
				 gint64 denom, gint how);
/** \deprecated use ::qof_numeric_mul */
gnc_numeric 
gnc_numeric_mul (gnc_numeric a, gnc_numeric b,
				 gint64 denom, gint how);
/** \deprecated use ::qof_numeric_div */
gnc_numeric 
gnc_numeric_div (gnc_numeric x, gnc_numeric y,
				 gint64 denom, gint how);
/** \deprecated use ::qof_numeric_neg */
gnc_numeric gnc_numeric_neg (gnc_numeric a);
/** \deprecated use ::qof_numeric_abs */
gnc_numeric gnc_numeric_abs (gnc_numeric a);
/** \deprecated use ::qof_numeric_add_fixed */
static inline gnc_numeric
gnc_numeric_add_fixed (gnc_numeric a, gnc_numeric b)
{
	return qof_numeric_add (a, b, QOF_DENOM_AUTO,
						QOF_HOW_DENOM_FIXED | QOF_HOW_RND_NEVER);
}
/** \deprecated use ::qof_numeric_sub_fixed */
static inline gnc_numeric
gnc_numeric_sub_fixed (gnc_numeric a, gnc_numeric b)
{
	return gnc_numeric_sub (a, b, QOF_DENOM_AUTO,
						QOF_HOW_DENOM_FIXED | QOF_HOW_RND_NEVER);
}
/** \deprecated use ::qof_numeric_add_with_error */
gnc_numeric 
gnc_numeric_add_with_error (gnc_numeric a, gnc_numeric b,
							gint64 denom, gint how,
							gnc_numeric * error);
/** \deprecated use ::qof_numeric_sub_with_error */
gnc_numeric 
gnc_numeric_sub_with_error (gnc_numeric a, gnc_numeric b,
							gint64 denom, gint how,
							gnc_numeric * error);
/** \deprecated use ::qof_numeric_mul_with_error */
gnc_numeric 
gnc_numeric_mul_with_error (gnc_numeric a, gnc_numeric b,
							gint64 denom, gint how,
							gnc_numeric * error);
/** \deprecated use ::qof_numeric_div_with_error */
gnc_numeric 
gnc_numeric_div_with_error (gnc_numeric a, gnc_numeric b,
							gint64 denom, gint how,
							gnc_numeric * error);
/** \deprecated use ::qof_numeric_convert */
gnc_numeric 
gnc_numeric_convert (gnc_numeric in, gint64 denom, gint how);
/** \deprecated use ::qof_numeric_convert_with_error */
gnc_numeric 
gnc_numeric_convert_with_error (gnc_numeric in, gint64 denom,
								gint how, gnc_numeric * error);
/** \deprecated use ::qof_numeric_reduce */
gnc_numeric gnc_numeric_reduce (gnc_numeric in);
/** \deprecated use ::QOF_HOW_RND_FLOOR. */
#define GNC_RND_FLOOR	QOF_HOW_RND_FLOOR
/** \deprecated use ::QOF_HOW_RND_CEIL. */
#define GNC_RND_CEIL 	QOF_HOW_RND_CEIL
/** \deprecated use ::QOF_HOW_RND_TRUNC. */
#define GNC_RND_TRUNC	QOF_HOW_RND_TRUNC
/** \deprecated use ::QOF_HOW_RND_PROMOTE. */
#define GNC_RND_PROMOTE 	QOF_HOW_RND_PROMOTE
/** \deprecated use ::QOF_HOW_RND_ROUND_HALF_DOWN. */
#define GNC_RND_ROUND_HALF_DOWN	QOF_HOW_RND_ROUND_HALF_DOWN
/** \deprecated use ::QOF_HOW_RND_ROUND_HALF_UP. */
#define GNC_RND_ROUND_HALF_UP 	QOF_HOW_RND_ROUND_HALF_UP
/** \deprecated use ::QOF_HOW_RND_ROUND. */
#define GNC_RND_ROUND	QOF_HOW_RND_ROUND
/** \deprecated use ::QOF_HOW_RND_NEVER. */
#define GNC_RND_NEVER	QOF_HOW_RND_NEVER
/** \deprecated use ::QOF_HOW_DENOM_EXACT. */
#define GNC_DENOM_EXACT  	QOF_HOW_DENOM_EXACT
/** \deprecated use ::QOF_HOW_DENOM_REDUCE. */
#define GNC_DENOM_REDUCE 	QOF_HOW_DENOM_REDUCE
/** \deprecated use ::QOF_HOW_DENOM_LCD. */
#define GNC_DENOM_LCD   	QOF_HOW_DENOM_LCD
/** \deprecated use ::QOF_HOW_DENOM_FIXED. */
#define GNC_DENOM_FIXED 	QOF_HOW_DENOM_FIXED
/** \deprecated use ::QOF_HOW_DENOM_SIGFIG. */
#define GNC_DENOM_SIGFIG 	QOF_HOW_DENOM_SIGFIG
/** \deprecated use ::QOF_HOW_DENOM_SIGFIGS. */
#define GNC_DENOM_SIGFIGS(X)  QOF_HOW_DENOM_SIGFIGS(X)
/** \deprecated use ::QOF_HOW_GET_SIGFIGS. */
#define GNC_NUMERIC_GET_SIGFIGS(X) QOF_HOW_GET_SIGFIGS(X)
/** \deprecated no replacement. */
QofBackend *gncBackendInit_file (const char *book_id, void *data);

/** \deprecated use QofError instead.
backend errors are to be specific to the backend responsible.
QofBackend itself registers some errors.
*/
#define ENUM_LIST_DEP(_) \
	_(ERR_BACKEND_NO_ERR, =0) \
	_(ERR_BACKEND_NO_HANDLER,) \
	_(ERR_BACKEND_NO_BACKEND,) \
	_(ERR_BACKEND_BAD_URL,) \
	_(ERR_BACKEND_NO_SUCH_DB,) \
	_(ERR_BACKEND_CANT_CONNECT,) \
	_(ERR_BACKEND_CONN_LOST,) \
	_(ERR_BACKEND_LOCKED,) \
	_(ERR_BACKEND_READONLY,) \
	_(ERR_BACKEND_TOO_NEW,) \
	_(ERR_BACKEND_DATA_CORRUPT,) \
	_(ERR_BACKEND_SERVER_ERR,) \
	_(ERR_BACKEND_ALLOC,) \
	_(ERR_BACKEND_PERM,) \
	_(ERR_BACKEND_MODIFIED,) \
	_(ERR_BACKEND_MOD_DESTROY,) \
	_(ERR_BACKEND_MISC,) \
	_(ERR_QSF_INVALID_OBJ,) \
	_(ERR_QSF_INVALID_MAP,) \
	_(ERR_QSF_BAD_OBJ_GUID,) \
	_(ERR_QSF_BAD_QOF_VERSION,) \
	_(ERR_QSF_BAD_MAP,) \
	_(ERR_QSF_NO_MAP,) \
	_(ERR_QSF_WRONG_MAP,) \
	_(ERR_QSF_MAP_NOT_OBJ,) \
	_(ERR_QSF_OVERFLOW,) \
	_(ERR_QSF_OPEN_NOT_MERGE,) \
	_(ERR_FILEIO_FILE_BAD_READ, =1000) \
	_(ERR_FILEIO_FILE_EMPTY,) \
	_(ERR_FILEIO_FILE_LOCKERR,) \
	_(ERR_FILEIO_FILE_NOT_FOUND,) \
	_(ERR_FILEIO_FILE_TOO_OLD,) \
	_(ERR_FILEIO_UNKNOWN_FILE_TYPE,) \
	_(ERR_FILEIO_PARSE_ERROR,) \
	_(ERR_FILEIO_BACKUP_ERROR,) \
	_(ERR_FILEIO_WRITE_ERROR,) \
	_(ERR_FILEIO_READ_ERROR,) \
	_(ERR_FILEIO_NO_ENCODING,) \
	_(ERR_NETIO_SHORT_READ, =2000) \
	_(ERR_NETIO_WRONG_CONTENT_TYPE,) \
	_(ERR_NETIO_NOT_GNCXML,) \
	_(ERR_SQL_MISSING_DATA, =3000) \
	_(ERR_SQL_DB_TOO_OLD,) \
	_(ERR_SQL_DB_BUSY,) \
	_(ERR_RPC_HOST_UNK, =4000) \
	_(ERR_RPC_CANT_BIND,) \
	_(ERR_RPC_CANT_ACCEPT,) \
	_(ERR_RPC_NO_CONNECTION,) \
	_(ERR_RPC_BAD_VERSION,) \
	_(ERR_RPC_FAILED,) \
	_(ERR_RPC_NOT_ADDED,)
	
DEFINE_ENUM(QofBackendError, ENUM_LIST_DEP)

AS_STRING_DEC(QofBackendError, ENUM_LIST_DEP)

/** \deprecated use ::qof_util_param_commit instead. */
gboolean
qof_commit_edit_part2 (QofInstance * inst,
	void (*on_error) (QofInstance *, QofBackendError),
	void (*on_done) (QofInstance *),
	void (*on_free) (QofInstance *));

/** \deprecated use qof_error_set instead. */
void 
qof_session_push_error (QofSession * session, QofBackendError err,
						const gchar *message);
/** \deprecated use qof_error_get_message instead but
note that this clears the error from the session stack. */
const gchar *
qof_session_get_error_message (QofSession * session);
/** \deprecated use ::qof_error_get_id or
::qof_error_check instead. */
QofErrorId 
qof_session_pop_error (QofSession * session);
QofErrorId 
qof_session_get_error (QofSession * session);
/** \deprecated use qof_error_set_be instead. */
void 
qof_backend_set_error (QofBackend * be, QofErrorId err);
/** \deprecated use qof_error_get_be instead. */
QofErrorId 
qof_backend_get_error (QofBackend * be);
/** \deprecated use qof_error_register instead. */
void 
qof_backend_set_message (QofBackend * be, const gchar * format, ...);
/** \deprecated use qof_error_get_message instead.

\note Unlike the deprecated function, the string
returned by qof_error_get_message
must \b NOT be freed by the caller.

*/
gchar *
qof_backend_get_message (QofBackend * be);
/** \deprecated Deprecated backwards compat token */
#define kvp_frame KvpFrame
/** \deprecated Deprecated backwards compat token */
#define kvp_value KvpValue
/** \deprecated Deprecated backwards compat token */
#define kvp_value_t KvpValueType
/** \deprecated Use kvp_frame_set_numeric instead. */
#define kvp_frame_set_gnc_numeric kvp_frame_set_numeric
/** \deprecated Use kvp_frame_set_string instead. */
#define kvp_frame_set_str kvp_frame_set_string
/** \deprecated Use kvp_frame_add_numeric instead */
#define kvp_frame_add_gnc_numeric kvp_frame_add_numeric
/** \deprecated Use kvp_frame_add_string instead */
#define kvp_frame_add_str kvp_frame_add_string
/** \deprecated Use kvp_value_new_numeric instead */
#define kvp_value_new_gnc_numeric kvp_value_new_numeric
/** \deprecated use qof_util_double_compare instead. */
gint double_compare (gdouble d1, gdouble d2);

#endif /* _DEPRECATED_H */
#endif /* QOF_DISABLE_DEPRECATED */
