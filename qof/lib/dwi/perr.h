
/*
 * FILE:
 * perr.h
 *
 * FUNCTION:
 * Error message conduit.
 *
 * HISTORY:
 * Linas Vepstas March 2002
 */

#ifndef PERR_H
#define PERR_H

#ifndef G_LOG_DOMAIN
#define G_LOG_DOMAIN my_domain
static char * my_domain = "dui";
#endif

#include <stdio.h>
#include <glib.h>

void dui_show_message (const char *, GLogLevelFlags, const char *, gpointer);

#ifndef HAVE_QOF

typedef enum {
	LOG_FATAL,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG
} duiLogLevel;

static duiLogLevel log_level = LOG_WARN;

#define FATAL(fmt, args...)                           \
	if (LOG_FATAL <= log_level) {                      \
		g_error ("Fatal: %s(): " fmt, __FUNCTION__, ## args);  \
	}

#define PERR(fmt, args...)                            \
	if (LOG_ERROR <= log_level) {                      \
		g_critical ("Error: %s(): " fmt, __FUNCTION__, ## args);  \
	}

#define PWARN(fmt, args...)                           \
	if (LOG_WARN <= log_level) {                       \
		g_warning ("Warning: %s(): " fmt, __FUNCTION__, ## args);  \
	}

#define PINFO(fmt, args...) \
	if (LOG_INFO <= log_level) { g_message ("%s(): " fmt, __FUNCTION__, ## args); }

#define ENTER(fmt, args...) \
	if (LOG_INFO <= log_level) { g_message ("Enter %s" fmt, __FUNCTION__, ## args); }

#define LEAVE(fmt, args...) \
	if (LOG_INFO <= log_level) { g_message ("Leave %s" fmt, __FUNCTION__, ## args); }

#else /* HAVE_QOF */
static const int module = 1;
#endif /* HAVE_QOF */

#define USERERR(fmt, args...) {                       \
      g_message (fmt, ## args);                       \
	}

#define SYNTAX(x,args...) USERERR ("Syntax Error: " x, ##args)


#endif /* PERR_H */

