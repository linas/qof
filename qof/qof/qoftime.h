/***************************************************************************
 *        qoftime.h - QofTime, 64bit UTC time handling (seconds).
 *       Rewritten from scratch for QOF 0.7.0
 *
 *  Fri May  5 15:05:32 2006
 *  Copyright  2006  Neil Williams
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA  02110-1301,  USA
 */

#ifndef _QOFTIME_H
#define _QOFTIME_H
/**	@addtogroup QOFTIME

 Universal time is the 'one true time' that is independent of
 one's location on planet Earth. It is measured in seconds
 from midnight January 1, 1970 in localtime-Greenwich (GMT).

 ::QofTime uses a signed 64bit integer  to count the seconds, 
 which differs from GTimeVal (32bit) and most other time 
 handling routines (which only count from the epoch).

 A QofTime where qt_sec == +1 therefore represents
 one second after midnight on 1st January 1970 - the epoch.
 Negative values of QofTime->qt_sec represent times before
 one second before midnight on 31st December 1969. Support for
 times before 1st Jan Year 1 are included.

 \note GTime is defined to always be a 32bit integer,
 unlike time_t which may be 64bit on some systems. Therefore,
 GTime will overflow in the year 2038. (A 32bit time_t 
 will overflow at 2038-01-19T03:14:07Z to be precise, 
 at which point it will likely wrap around to 
 20:45:52 UTC on December 13, 1901.)
 
 ::QofTime is defined as 64bit on all systems and will not 
 overflow until the year 292,471,208,679 (not counting leap years).
 i.e. approx. 9223372036854775808 / (60*60*24*365).
 This also means that some values of QofTime cannot be converted
 to a time_t on systems where time_t is defined as 32bit.

 \note Most conversions of QofTime can cause a loss of data.
 GDate contains no time data, time_t varies between platforms
 and struct tm does not include fractions of a second. \b Avoid
 converting back and forth between time formats. All
 conversions between QofTime and time_t, struct tm or other
 32bit time implementations \b must check the return value of
 QofTime functions to ensure the QofTime was within the range
 supported by the 32bit type.

 QofTime is not directly equivalent to GTimeVal - a
 QofTime can go further into the future. However, within the
 range supported by GTimeVal and GTime (a 32bit integer), the
 value of QofTime->qt_sec is always identical to 
 GTimeVal->tv_sec.

 The use of signed values and the handling of times prior to the 
 epoch means that a QofTime with zero values can no longer be 
 assumed to be invalid or used alone to denote an init value. 
 QofTime is therefore an opaque type. QofTime and QofDate 
 functions set and check QofTime validity as needed.

 QofTime is always and only concerned with seconds. All date
 or calendar handling is performed using ::QofDate.

 \since v0.7.0

 @{
*/
/**
    @file qoftime.h 
    @brief 64bit UTC Time handling routines  
    @author Copyright 2006 Neil Williams <linux@codehelp.co.uk>
*/

#include "config.h"
#include <time.h>

/** log module name */
#define QOF_MOD_TIME "qof-time"
/** \deprecated use ::QofTime. */
#ifndef QOF_DISABLE_DEPRECATED
typedef struct timespec64
{
   guint64 tv_sec;
   glong tv_nsec;
}Timespec;
#endif

/** number of nanoseconds per second. 10^9 */
#define QOF_NSECS 1000000000

/** \name QofTime functions.
 @{
*/
/** \brief Use a 64-bit signed int QofTime

  QofTime is a lot like the unix 'struct timespec'
  except that it uses a 64-bit signed int to store the seconds.
  This should adequately cover dates in the distant future as 
  well as the distant past, as long as these are not more than 
  a couple of dozen times the age of the universe. Values of 
  this type can range from -9,223,372,036,854,775,808 to 
  9,223,372,036,854,775,807.
*/
typedef struct QofTime64 QofTime;

/** \brief Replacement for time_t

Prevents overflows by making all values 64bit on all
platforms.

(time_t will overflow on 32bit systems in 2038)
*/
typedef gint64 QofTimeSecs;

/** \brief Add (or subtract) seconds from a QofTime.

 \param qt A valid QofTime.
 \param secs A 64bit number of seconds to add (or subtract
if secs is negative) from the QofTime.

The QofTime is altered in place. To assign the new value
to a new QofTime, use ::qof_time_add_secs_copy
*/
void
qof_time_add_secs (QofTime * qt, QofTimeSecs secs);

/** \brief Create a new QofTime, secs different to an original.

 \param qt a valid QofTime to use as the base.
 \param secs a 64bit number of seconds to add (or subtract
 if secs is negative) from the original to create the copy.
 
 \return a new, valid, QofTime that is secs different to the
 original.
*/
QofTime *
qof_time_add_secs_copy (QofTime * qt, QofTimeSecs secs);

/** \brief create an empty QofTime

The QofTime becomes the property of the caller and needs to be
freed with qof_time_free when no longer required.
*/
QofTime *
qof_time_new (void);

/** \brief Create a copy of a QofTime.

 \param qt A valid QofTime to copy.
 
 \return NULL on error, otherwise a new, valid and
 normalised QofTime set to the same time as the original.
*/
QofTime *
qof_time_copy (const QofTime *qt);

/** \brief Free a QofTime when no longer required. */
void 
qof_time_free (QofTime * qt);

/** \brief Set the number of seconds

 \param time pointer to a QofTime time, created with 
qof_time_new();
 \param secs Signed 64bit number of seconds where zero 
represents midnight at the epoch: 00:00:00 01/01/1970.
*/
void 
qof_time_set_secs (QofTime * time, QofTimeSecs secs);

/** \brief Set the number of seconds

 \param time pointer to a QofTime time, created with 
qof_time_new();
 \param nano long int number of nanoseconds.
*/
void 
qof_time_set_nanosecs (QofTime * time, glong nano);

/** \brief Get the number of seconds

 \param time pointer to a QofTime time, created with 
qof_time_new();
 \return Signed 64bit number of seconds.
*/
QofTimeSecs 
qof_time_get_secs (const QofTime * time);

/** \brief Get the number of seconds

 \param time pointer to a QofTime time, created with 
qof_time_new();
 \return long int number of nanoseconds.
*/
glong 
qof_time_get_nanosecs (const QofTime * time);
/** @} */
/** \name QofTime manipulation
 @{ 
*/
/** strict equality */
gboolean 
qof_time_equal (const QofTime * ta, const QofTime * tb);

/** comparison:  if (ta < tb) -1; else if (ta > tb) 1; else 0; */
gint 
qof_time_cmp (const QofTime * ta, const QofTime * tb);

/** \brief difference between two QofTimes.

Results are normalised
ie qt_sec and qt_nsec of the result have the same size
abs(result.qt_nsec) <= 1000000000

 \return a new QofTime of the difference. The caller must 
 free the difference QofTime when done.
*/
QofTime *
qof_time_diff (const QofTime * ta, const QofTime * tb);

/** Normalise a QofTime to an absolute value.

 \param t the QofTime to normalise. 
 \return the normalised QofTime t.
*/
QofTime *
qof_time_abs (QofTime * t);

gboolean
qof_time_is_valid (const QofTime * qt);

/** Turns a time_t into a QofTime

 \note On some platforms, time_t is only 32bit. Use
::QofTimeSecs instead.

 \param t integer seconds since the epoch.
 \param nanosecs number of nanoseconds
 \return pointer to a newly allocated QofTime.
*/
QofTime *
qof_time_from_time_t (time_t t, glong nanosecs);

/** Turns a QofTimeSecs into a QofTime

An alternative call that combines ::qof_time_set_secs
and ::qof_time_set_nanosecs.
 \param t 64bit integer number of seconds (t == 0
at the epoch, use negative values for previous times.)
 \param nanosecs number of nanoseconds
 \return pointer to a newly allocated (and validated) QofTime.
*/
QofTime *
qof_time_set (QofTimeSecs t, glong nanosecs);

/** Tries to turn a QofTime into a time_t

\note CARE: QofTime is 64bit, time_t might be 32bit.
GDate has a wider range. time_t may be defined as
an integer on some platforms, causing data loss.

 \param ts A 64bit QofTime.
 \param t pointer to a time_t to store result.
 \param nanosecs pointer to a variable to store the
nanoseconds, if any, from the QofTime conversion.
 \return FALSE on error or if the QofTime is before the epoch
or outside the range of time_t, otherwise TRUE.
*/
gboolean 
qof_time_to_time_t (QofTime * ts, time_t * t, glong * nanosecs);

/** \brief Convert a broken-down into a QofTime

struct tm broken-down time does not support
fractions of a second.

Conversion of a QofTime to a struct tm is
 \b not supported because of the inherent data loss.

 \param tm broken-down time structure.
 \param nanosecs Fractions of a second.
 \return a newly allocated QofTime or NULL on error.
*/
QofTime *
qof_time_from_tm (struct tm *tm, glong nanosecs);

/** \brief Convert a QofTime to a GTimeVal

Safe for dates between 1st January Year 1 and 31st December 2037.
 \param qt QofTime to convert
 \param gtv GTimeVal to set, left unchanged if an error occurs.
 \return TRUE on success, FALSE on error.
*/
gboolean 
qof_time_to_gtimeval (QofTime * qt, GTimeVal * gtv);

/** \brief Convert a QofTime to a GTimeVal

Safe for all dates supported by a valid GTimeVal.
 \param qt QofTime to set.
 \param gtv GTimeVal to convert
*/
void 
qof_time_from_gtimeval (QofTime * qt, GTimeVal * gtv);

/** Convert a day, month, and year to a QofTime.

Limited to the range of a GDate.

 \param day day of month, 1 to 31.
 \param month Decimal number of month, 1 to 12.
 \param year signed short containing the year. This value is 
safe for all dates within the range of a GDate.
 \return NULL on error, otherwise the converted QofTime.
*/
QofTime *
qof_time_dmy_to_time (guint8 day, guint8 month, guint16 year);

/** Convert a QofTime to day, month and year.

Usable for all QofTime values within the range of GDate.

 \todo Remove GDate limits and use QofDate.

 \param t The QofTime to use.
 \param day Pointer to a integer to hold day of month, 1 to 31.
 \param month Decimal number of month, 1 to 12.
 \param year signed short containing the year. This value is 
safe for all dates within the range of a GDate.
 \return FALSE if the time cannot be converted, otherwise TRUE.
*/
gboolean
qof_time_to_dmy (QofTime * t, guint8 * day, guint8 * month, guint16 * year);
/** \brief Convert QofTime to GDate

 \warning The GDate will lose time-related data within the 
QofTime. i.e. converting a QofTime to a GDate and back to a 
QofTime causes the final QofTime to be set to the start of the 
particular day, UTC.

 @param time The QofTime to convert
 @return a valid GDate or NULL on error.
*/
GDate *
qof_time_to_gdate (QofTime * time);

/** \brief Convert a GDate to a QofTime

 \warning the QofTime is set to the first second of the 
particular day, UTC.

 @param date The GDate to convert
 @return a valid QofTime or NULL on error.
*/
QofTime *
qof_time_from_gdate (GDate * date);

/** @} */

/** \name Time Start/End Adjustment routines
 * Given a time value, adjust it to be the beginning or end 
of that day.
@{
*/

/** \todo move to a private header; 
used by qofdate.c and test-date.c
*/
GTimeVal *
qof_time_get_current_start (void);

/** \brief Get the current QofTime.

Current implementations can only provide a long
number of seconds (max: 2,147,483,647) and the
number of microseconds (10^-6) not nanoseconds (10^-9).

 \return a newly allocated, valid, QofTime of the
 current time.
 \todo use to replace qof_time_get_current_start 
*/
QofTime *
qof_time_get_current (void);

/** \brief set the given QofTime to midday on the same day.

This routine is limited to dates supported by GDate.

 \todo remove GDate limits.

 \return FALSE on error, otherwise TRUE
*/
gboolean 
qof_time_set_day_middle (QofTime * t);

/** \brief set the given QofTime to the first second of that day.

This routine is limited to dates supported by GDate.

 \todo remove GDate limits.

\return FALSE on error, otherwise TRUE.
*/
gboolean 
qof_time_set_day_start (QofTime * time);

/** \brief set the given QofTime to the last second of that day.

This routine is limited to dates supported by GDate.

 \todo remove GDate limits.

 \return FALSE on error, otherwise TRUE.
*/
gboolean 
qof_time_set_day_end (QofTime * time);

/** Return the number of the last day of the month
for the value contained in the QofTime.

 \todo remove GDate limits.

Only usable within the range of dates supported by GDate.
 \return zero on error.
*/
guint8 
qof_time_last_mday (QofTime * ts);

/** @} */

/** \name Today's Date 
@{
*/
/** return a QofTime of the first second of today. */
QofTime *
qof_time_get_today_start (void);

/** returns a QofTime of the last second of today. */
QofTime *
qof_time_get_today_end (void);

/** Return the current time in UTC textual format.
 @return A pointer to the generated string.
 @note The caller owns this buffer and must free it when 
done.
*/
gchar *
qof_time_stamp_now (void);

/** @} */
/** @} */
/** @} */
#endif /* _QOFTIME_H */
