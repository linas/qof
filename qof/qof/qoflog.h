/********************************************************************\
 * gnc-trace.h -- GnuCash error logging and tracing fcility         *
 * Copyright (C) 1998-2003 Linas Vepstas <linas@linas.org>          *
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
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
 *   Author: Linas Vepstas (linas@linas.org)                        *
\********************************************************************/

/** @addtogroup Trace
    @{ */

/** @file gnc-trace.h 
 *  @brief QOF error logging and tracing facility */

#ifndef GNC_TRACE_H
#define GNC_TRACE_H

#include <glib.h>
#include <stdarg.h>
#include <stdio.h>

#define QOF_MOD_ENGINE "qof-engine"

typedef enum
{
  GNC_LOG_FATAL   = 0,
  GNC_LOG_ERROR   = 1,
  GNC_LOG_WARNING = 2,
  GNC_LOG_INFO    = 3,
  GNC_LOG_DEBUG   = 4,
  GNC_LOG_DETAIL  = 5,
  GNC_LOG_TRACE   = 6,
} gncLogLevel;

#define GNC_TRACE_INDENT_WIDTH 4

/** Initialize the error logging subsystem

\note Applications should call gnc_set_logfile
to set the name of the log file, otherwise the
default of \a /tmp/qof.trace will be used.
*/
void gnc_log_init (void);

/** Set the logging level of the given log_module. */
void gnc_set_log_level(gchar* module, gncLogLevel level);

/** Set the logging level for all known log_modules.

\note Unless a log_module has been registered using
gnc_set_log_level, it will be unaffected by this change.

*/
void gnc_set_log_level_global(gncLogLevel level);

/** Specify an alternate log output, to pipe or file.  By default,
 *  all logging goes to /tmp/qof.trace */
void gnc_set_logfile (FILE *outfile);

/** gnc_log_prettify() cleans up subroutine names. AIX/xlC has the habit
 * of printing signatures not names; clean this up. On other operating
 * systems, truncate name to 30 chars. Note this routine is not thread
 * safe. Note we wouldn't need this routine if AIX did something more
 * reasonable. Hope thread safety doesn't poke us in eye. */
const char * gnc_log_prettify (const char *name);

/** Do not log log_modules that have not been enabled.

 Whether to log cannot be decided inline because a hashtable is
 now used. This is the price of extending logging to non-Gnucash
 log_modules.

*/
gboolean gnc_should_log(gchar* log_module, gncLogLevel log_level);

/** Set the default QOF log_modules to the log level. */
void qof_log_set_default(gncLogLevel log_level);

typedef void (*QofLogCB) (gchar* log_module, gncLogLevel* log_level, gpointer user_data);

/** Iterate over each known log_module

Only log_modules with log_levels set will 
be available.
*/
void qof_log_module_foreach(QofLogCB cb, gpointer data);

/** Number of log_modules registered*/
gint qof_log_module_count(void);

#define FUNK gnc_log_prettify(__FUNCTION__)

/** Log error/waring/info messages to stderr or to other pipe. 
 *  This logging infrastructure is meant for validating the 
 *  correctness of the execution of the code.  'Info' level 
 *  messages help trace program flow. 'Error' messages are 
 *  meant to indicate internal data inconsistencies.
 * 
 * Messages can be logged to stdout, stderr, or to any desired
 * FILE * file handle. Use fdopen() to get a file handle from a 
 * file descriptor. Use gnc_set_logfile to set the logging file 
 * handle.
 */

/** Log an fatal error */
#define FATAL(format, args...) {                     \
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR,          \
      "Fatal Error: %s(): " format, FUNK , ## args); \
}

/** Log an serious error */
#define PERR(format, args...) {                    \
  if (gnc_should_log (log_module, GNC_LOG_ERROR)) {    \
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,     \
      "Error: %s(): " format, FUNK , ## args);     \
  }                                                \
}

/** Log an warning */
#define PWARN(format, args...) {                   \
  if (gnc_should_log (log_module, GNC_LOG_WARNING)) {  \
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,      \
      "Warning: %s(): " format, FUNK , ## args);   \
  }                                                \
}

/** Print an informational note */
#define PINFO(format, args...) {                   \
  if (gnc_should_log (log_module, GNC_LOG_INFO)) {     \
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_INFO,         \
      "Info: %s(): " format,                       \
      FUNK , ## args);                             \
  }                                                \
}

/** Print an debugging message */
#define DEBUG(format, args...) {                   \
  if (gnc_should_log (log_module, GNC_LOG_DEBUG)) {    \
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,        \
      "Debug: %s(): " format,                      \
      FUNK , ## args);                             \
  }                                                \
}

/** Print an function entry debugging message */
#define ENTER(format, args...) {                   \
  extern gint gnc_trace_num_spaces;                \
  if (gnc_should_log (log_module, GNC_LOG_DEBUG)) {    \
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,        \
      "Enter in %s: %s()" format, __FILE__,        \
      FUNK , ## args);                             \
    gnc_trace_num_spaces += GNC_TRACE_INDENT_WIDTH;\
  }                                                \
}

/** Print an function exit debugging message */
#define LEAVE(format, args...) {                   \
  extern gint gnc_trace_num_spaces;                \
  if (gnc_should_log (log_module, GNC_LOG_DEBUG)) {    \
    gnc_trace_num_spaces -= GNC_TRACE_INDENT_WIDTH;\
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,        \
      "Leave: %s()" format,                        \
      FUNK , ## args);                             \
  }                                                \
}

/** Print an function trace debugging message */
#define TRACE(format, args...) {                   \
  if (gnc_should_log (log_module, GNC_LOG_TRACE)) {    \
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,        \
      "Trace: %s(): " format, FUNK , ## args);     \
  }                                                \
}

#define DEBUGCMD(x) { if (gnc_should_log (log_module, GNC_LOG_DEBUG)) { (x); }}

/* -------------------------------------------------------- */
/** Infrastructure to make timing measurements for critical peices 
 * of code. Used for only for performance tuning & debugging. 
 */

void gnc_start_clock (int clockno, gchar* log_module, gncLogLevel log_level,
                      const char *function_name, const char *format, ...);

void gnc_report_clock (int clockno,
                       gchar* log_module,
                       gncLogLevel log_level,
                       const char *function_name,
                       const char *format, ...);

void gnc_report_clock_total (int clockno,
                             gchar* log_module,
                             gncLogLevel log_level,
                             const char *function_name,
                             const char *format, ...);

/** start a particular timer */
#define START_CLOCK(clockno,format, args...) {              \
  if (gnc_should_log (log_module, GNC_LOG_INFO))                \
    gnc_start_clock (clockno, module, GNC_LOG_INFO,         \
             __FUNCTION__, format , ## args);               \
}

/** report elapsed time since last report on a particular timer */
#define REPORT_CLOCK(clockno,format, args...) {             \
  if (gnc_should_log (log_module, GNC_LOG_INFO))                \
    gnc_report_clock (clockno, module, GNC_LOG_INFO,        \
             __FUNCTION__, format , ## args);               \
}

/** report total elapsed time since timer started */
#define REPORT_CLOCK_TOTAL(clockno,format, args...) {       \
  if (gnc_should_log (log_module, GNC_LOG_INFO))                \
    gnc_report_clock_total (clockno, module, GNC_LOG_INFO,  \
             __FUNCTION__, format , ## args);               \
}

#endif /* GNC_TRACE_H */
/* @} */
