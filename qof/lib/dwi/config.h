/*
 * FILE:
 * config.h
 *
 * FUNCTION:
 * Temporary hacks 
 *
 * HISTORY:
 * Linas Vepstas April 2002
 */


/* Need to define the G_LOG_DOMAIN *before* including glib.h, else
 * everything gets hosed.  So we will define it here, since config.h 
 * always comes first.
 */
#ifndef G_LOG_DOMAIN
#define G_LOG_DOMAIN my_domain
static char * my_domain = "dui";
#endif

#define HAVE_QOF
