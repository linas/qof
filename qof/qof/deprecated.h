/***************************************************************************
 *            deprecated.h
 *
 *  Mon Nov 21 14:08:25 2005
 *  Copyright  2005  Neil Williams
 *  linux@codehelp.co.uk
 ****************************************************************************/
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

/** \deprecated use ::qof_start_clock */
void gnc_start_clock (int, QofLogModule, gncLogLevel, const char *,
					  const char *, ...);
/** \deprecated use ::qof_report_clock */
void gnc_report_clock (int, QofLogModule, gncLogLevel, const char *,
					   const char *, ...);
/** \deprecated use ::qof_report_clock_total */
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
from qofevent.c in libqoqf2 */
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
/** \deprecated use ::qof_event_generate instead. */
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
/** \deprecated use QofDateMonthFormat instead. */
typedef enum
{
	GNCDATE_MONTH_NUMBER,
						/**< \deprecated use ::QOF_DATE_MONTH_NUMBER. */
	GNCDATE_MONTH_ABBREV,
						/**< \deprecated use ::QOF_DATE_MONTH_ABBREV. */
	GNCDATE_MONTH_NAME	/**< \deprecated use ::QOF_DATE_MONTH_NAME. */
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
/** \deprecated use ::QofTime. */
/*typedef struct timespec64
{
   guint64 tv_sec;     
   glong tv_nsec;
}Timespec;*/
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
/** \deprecated set ::QOF_DATE_FORMAT_ISO8601 and call ::qof_date_time_scan */
Timespec gnc_iso8601_to_timespec_gmt (const gchar *);
/** \deprecated set ::QOF_DATE_FORMAT_ISO8601 and call ::qof_date_time_stamp */
gchar *gnc_timespec_to_iso8601_buff (Timespec ts, gchar * buff);
/** \deprecated use GDate instead. */
void gnc_timespec2dmy (Timespec ts, gint * day, gint * month, gint * year);
/** \deprecated use ::qof_date_time_add_months. */
void date_add_months (struct tm *tm, gint months, gboolean track_last_day);


gboolean qof_date_add_days(Timespec *ts, gint days);

gboolean qof_date_add_months(Timespec *ts, gint months, gboolean track_last_day);

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
use ::qof_date_time_to_gdate.*/
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
/** \deprecated no replacement */
char *qof_print_date (time_t secs);
/** \deprecated no replacement */
const char *gnc_print_date (Timespec ts);
/** \deprecated use ::qof_date_time_scan */
gboolean qof_scan_date (const char *buff, int *day, int *month, int *year);
/** \deprecated no replacement */
gboolean qof_scan_date_secs (const char *buff, time_t * secs);
/** @deprecated use qof_entity_set_guid instead but only in
backends (when reading the GUID from the data source). */
#define qof_book_set_guid(book,guid)    \
         qof_entity_set_guid(QOF_ENTITY(book), guid)
/** \deprecated use QOF_TYPE_TIME instead */
#define QOF_TYPE_DATE      "date"
/** \deprecated use qof_instance_set_update_time instead. */
Timespec qof_instance_get_last_update (QofInstance * inst);
void qof_instance_set_last_update (QofInstance * inst, Timespec ts);
/** \deprecated use kvp_frame_set_time instead. */
void kvp_frame_set_timespec (KvpFrame * frame, const gchar * path,
			 Timespec ts);
/** \deprecated use kvp_frame_add_time instead. */
void kvp_frame_add_timespec (KvpFrame * frame, const gchar * path,
			 Timespec ts);
/** \deprecated use kvp_value_get_time instead. */
Timespec kvp_value_get_timespec (const KvpValue * value);
/** \deprecated use kvp_frame_get_time instead. */
Timespec kvp_frame_get_timespec (const KvpFrame * frame, const gchar * path);
/** \deprecated use kvp_frame_new_time instead. */
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


#endif /* _DEPRECATED_H */
#endif /* QOF_DISABLE_DEPRECATED */
