/***************************************************************************
 *            test-numeric.c
 *
 * Test file created by Linas Vepstas <linas@linas.org>
 * Review operation of the gnc-numeric tools by verifying results
 * of various operations.
 *
 * June 2004 
 *  Copyright  2004 Linas Vepstas <linas@linas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 */

#include <ctype.h>
#include <glib.h>
#include "qof.h"
#include "test-stuff.h"
#include "test-engine-stuff.h"
#include "qofnumeric.h"

#define NREPS 2000

static char *
qof_numeric_print (QofNumeric in)
{
	char *retval;
	if (qof_numeric_check (in))
	{
		retval =
			g_strdup_printf ("<ERROR> [%" G_GINT64_FORMAT " / %"
							 G_GINT64_FORMAT "]", in.num, in.denom);
	}
	else
	{
		retval =
			g_strdup_printf ("[%" G_GINT64_FORMAT " / %" G_GINT64_FORMAT "]",
							 in.num, in.denom);
	}
	return retval;
}

/* ======================================================= */

static void
check_unary_op (gboolean (*eqtest) (QofNumeric, QofNumeric),
				QofNumeric expected,
				QofNumeric actual, QofNumeric input, const gchar *errmsg)
{
	gchar *e = qof_numeric_print (expected);
	gchar *r = qof_numeric_print (actual);
	gchar *a = qof_numeric_print (input);
	gchar *str = g_strdup_printf (errmsg, e, r, a);

	do_test (eqtest (expected, actual), str);

	g_free (a);
	g_free (r);
	g_free (e);
	g_free (str);
}

/* ======================================================= */

static void
check_binary_op (QofNumeric expected,
				 QofNumeric actual,
				 QofNumeric input_a, QofNumeric input_b, const gchar *errmsg)
{
	gchar *e = qof_numeric_print (expected);
	gchar *r = qof_numeric_print (actual);
	gchar *a = qof_numeric_print (input_a);
	gchar *b = qof_numeric_print (input_b);
	gchar *str = g_strdup_printf (errmsg, e, r, a, b);

	do_test (qof_numeric_eq (expected, actual), str);

	g_free (a);
	g_free (b);
	g_free (r);
	g_free (e);
	g_free (str);
}

/* ======================================================= */

static gboolean
qof_numeric_unequal (QofNumeric a, QofNumeric b)
{
	return (0 == qof_numeric_equal (a, b));
}

/* ======================================================= */

/* Make sure that the equivalence operator we use for 
 * later tests actually works */
static void
check_eq_operator (void)
{
	QofNumeric a = qof_numeric_create (42, 58);
	QofNumeric b = qof_numeric_create (42, 58);
	QofNumeric c = qof_numeric_create (40, 58);

	/* Check strict equivalence and non-equivalence */
	do_test (qof_numeric_eq (a, a), "expected self-equivalence");
	do_test (qof_numeric_eq (a, b), "expected equivalence");
	do_test (0 == qof_numeric_eq (a, c), "expected inequivalence");
}

/* ======================================================= */

static void
check_reduce (void)
{
	QofNumeric one, rone;
	QofNumeric four, rfour;
	QofNumeric val, rval;
	/* Check common factor elimination (needed for equality checks) */
	one = qof_numeric_create (1, 1);
	rone = qof_numeric_create (1000000, 1000000);
	rone = qof_numeric_reduce (rone);
	do_test (qof_numeric_eq (one, rone), "reduce to one");

	four = qof_numeric_create (4, 1);
	rfour = qof_numeric_create (480, 120);
	rfour = qof_numeric_reduce (rfour);
	do_test (qof_numeric_eq (four, rfour), "reduce to four");

	val = qof_numeric_create (10023234LL, 334216654LL);
	rval = qof_numeric_reduce (val);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (5011617, 167108327),
					rval,
					val, "check_reduce(1) expected %s = %s = reduce(%s)");

	val = qof_numeric_create (17474724864LL, 136048896LL);
	rval = qof_numeric_reduce (val);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (4 * 17 * 17, 9),
					rval,
					val, "check_reduce(2) expected %s = %s = reduce(%s)");

	val = qof_numeric_create (1024LL, 1099511627776LL);
	rval = qof_numeric_reduce (val);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (1, 1024 * 1024 * 1024),
					rval,
					val, "check_reduce(3): expected %s = %s = reduce(%s)");
}

/* ======================================================= */

static void
check_equality_operator (void)
{
	gint i, m;
	gint mult;
	gint64 f, deno, numer;
	QofNumeric big, rbig;
	QofNumeric val, mval;
	QofNumeric bval, rval;
	/* Check equality operator for some large numer/denom values */
	numer = 1 << 30;
	numer <<= 30;				/* we don't trust cpp to compute 1<<60 correctly */
	deno = 1 << 30;
	deno <<= 20;
	rbig = qof_numeric_create (numer, deno);

	big = qof_numeric_create (1 << 10, 1);
	do_test (qof_numeric_equal (big, rbig), "equal to billion");

	big = qof_numeric_create (1 << 20, 1 << 10);
	do_test (qof_numeric_equal (big, rbig), "equal to 1<<20/1<<10");

	big = qof_numeric_create (1 << 30, 1 << 20);
	do_test (qof_numeric_equal (big, rbig), "equal to 1<<30/1<<20");

	numer = 1 << 30;
	numer <<= 30;				/* we don't trust cpp to compute 1<<60 correctly */
	deno = 1 << 30;
	rbig = qof_numeric_create (numer, deno);

	big = qof_numeric_create (1 << 30, 1);
	do_test (qof_numeric_equal (big, rbig), "equal to 1<<30");

	numer = 1 << 30;
	numer <<= 10;
	big = qof_numeric_create (numer, 1 << 10);
	do_test (qof_numeric_equal (big, rbig), "equal to 1<<40/1<<10");

	numer <<= 10;
	big = qof_numeric_create (numer, 1 << 20);
	do_test (qof_numeric_equal (big, rbig), "equal to 1<<50/1<<20");

	/* We assume RAND_MAX is less that 1<<32 */
	for (i = 0; i < NREPS; i++)
	{
		deno = rand () / 2;
		mult = rand () / 2;
		numer = rand () / 2;

		val = qof_numeric_create (numer, deno);
		mval = qof_numeric_create (numer * mult, deno * mult);

		/* The reduced version should be equivalent */
		bval = qof_numeric_reduce (val);
		rval = qof_numeric_reduce (mval);
		check_unary_op (qof_numeric_eq,
						bval, rval, mval, "expected %s = %s = reduce(%s)");

		/* The unreduced versions should be equal */
		check_unary_op (qof_numeric_equal,
						val, mval, mval, "expected %s = %s");

		/* Certain modulo's should be very cleary un-equal; this
		 * helps stop funky modulo-64 aliasing in compares that 
		 * might creep in. */
		mval.denom >>= 1;
		mval.num >>= 1;
		m = 0;
		f = mval.denom;
		while (f % 2 == 0)
		{
			f >>= 1;
			m++;
		}
		if (1 < m)
		{
			gint64 nn = 1 << (32 - m);
			nn <<= 32;
			nn += mval.num;
			val = qof_numeric_create (2 * nn, 2 * mval.denom);
			check_unary_op (qof_numeric_unequal,
							val, mval, mval, "expected unequality %s != %s");

		}
	}
}

/* ======================================================= */

static void
check_rounding (void)
{
	QofNumeric val;

	val = qof_numeric_create (7, 16);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (43, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_FLOOR),
					val, "expected %s = %s = (%s as 100th's floor)");
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (44, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_CEIL),
					val, "expected %s = %s = (%s as 100th's ceiling)");
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (43, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_TRUNC),
					val, "expected %s = %s = (%s as 100th's trunc)");
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (44, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_ROUND),
					val, "expected %s = %s = (%s as 100th's round)");

	val = qof_numeric_create (1511, 1000);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (151, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_ROUND),
					val, "expected %s = %s = (%s as 100th's round)");

	val = qof_numeric_create (1516, 1000);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (152, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_ROUND),
					val, "expected %s = %s = (%s as 100th's round)");

	/* Half-values always get rounded to nearest even number */
	val = qof_numeric_create (1515, 1000);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (152, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_ROUND),
					val, "expected %s = %s = (%s as 100th's round)");

	val = qof_numeric_create (1525, 1000);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (152, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_ROUND),
					val, "expected %s = %s = (%s as 100th's round)");

	val = qof_numeric_create (1535, 1000);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (154, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_ROUND),
					val, "expected %s = %s = (%s as 100th's round)");

	val = qof_numeric_create (1545, 1000);
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (154, 100),
					qof_numeric_convert (val, 100, QOF_HOW_RND_ROUND),
					val, "expected %s = %s = (%s as 100th's round)");
}

/* ======================================================= */

static void
check_double (void)
{
	gdouble flo;
	QofNumeric val = qof_numeric_create (0, 1);

	check_unary_op (qof_numeric_eq,
					qof_numeric_create (112346, 100000),
					qof_numeric_from_double (1.1234567890123,
										   QOF_DENOM_AUTO,
										   QOF_HOW_DENOM_SIGFIGS (6) |
										   QOF_HOW_RND_ROUND),
					val, "expected %s = %s double 6 figs");

	check_unary_op (qof_numeric_eq,
					qof_numeric_create (112346, 10000000),
					qof_numeric_from_double (0.011234567890123,
										   QOF_DENOM_AUTO,
										   QOF_HOW_DENOM_SIGFIGS (6) |
										   QOF_HOW_RND_ROUND),
					val, "expected %s = %s double 6 figs");

	check_unary_op (qof_numeric_eq,
					qof_numeric_create (112346, 100),
					qof_numeric_from_double (1123.4567890123,
										   QOF_DENOM_AUTO,
										   QOF_HOW_DENOM_SIGFIGS (6) |
										   QOF_HOW_RND_ROUND),
					val, "expected %s = %s double 6 figs");
	check_unary_op (qof_numeric_eq,
					qof_numeric_create (112346, 10000000000LL),
					qof_numeric_from_double (1.1234567890123e-5,
										   QOF_DENOM_AUTO,
										   QOF_HOW_DENOM_SIGFIGS (6) |
										   QOF_HOW_RND_ROUND),
					val, "expected %s = %s double 6 figs");

	flo = qof_numeric_to_double (qof_numeric_create (7, 16));
	do_test ((0.4375 == flo), "float pt conversion");
}

/* ======================================================= */

static void
check_neg (void)
{
	QofNumeric a = qof_numeric_create (2, 6);
	QofNumeric b = qof_numeric_create (1, 4);
	QofNumeric c = qof_numeric_neg (a);
	QofNumeric d = qof_numeric_neg (b);

	check_unary_op (qof_numeric_eq,
					qof_numeric_create (-2, 6), c,
					a, "expected %s = %s = -(%s)");

	check_unary_op (qof_numeric_eq,
					qof_numeric_create (-1, 4), d,
					b, "expected %s = %s = -(%s)");

}

/* ======================================================= */

static void
check_add_subtract (void)
{
	gint i;
	QofNumeric a, b, c, d, z;
#if CHECK_ERRORS_TOO
	QofNumeric c;
#endif

	a = qof_numeric_create (2, 6);
	b = qof_numeric_create (1, 4);

	/* Well, actually 14/24 would be acceptable/better in this case */
	check_binary_op (qof_numeric_create (7, 12),
					 qof_numeric_add (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s + %s for add exact");

	check_binary_op (qof_numeric_create (58, 100),
					 qof_numeric_add (a, b, 100, QOF_HOW_RND_ROUND),
					 a, b,
					 "expected %s got %s = %s + %s for add 100ths (banker's)");

	check_binary_op (qof_numeric_create (5833, 10000),
					 qof_numeric_add (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_SIGFIGS (4) |
									  QOF_HOW_RND_ROUND),
					 a, b, "expected %s got %s = %s + %s for add 4 sig figs");

	check_binary_op (qof_numeric_create (583333, 1000000),
					 qof_numeric_add (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_SIGFIGS (6) |
									  QOF_HOW_RND_ROUND),
					 a, b, "expected %s got %s = %s + %s for add 6 sig figs");

	check_binary_op (qof_numeric_create (1, 12),
					 qof_numeric_sub (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s - %s for sub exact");

	/* We should try something trickier for reduce & lcd */
	check_binary_op (qof_numeric_create (1, 12),
					 qof_numeric_sub (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_REDUCE), a, b,
					 "expected %s got %s = %s - %s for sub reduce");

	check_binary_op (qof_numeric_create (1, 12),
					 qof_numeric_sub (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_LCD), a, b,
					 "expected %s got %s = %s - %s for sub reduce");

	check_binary_op (qof_numeric_create (8, 100),
					 qof_numeric_sub (a, b, 100, QOF_HOW_RND_ROUND),
					 a, b,
					 "expected %s got %s = %s - %s for sub 100ths (banker's)");

	/* ------------------------------------------------------------ */
	/* This test has failed before */
	c = qof_numeric_neg (a);
	d = qof_numeric_neg (b);
	z = qof_numeric_zero ();
	check_binary_op (c, qof_numeric_add_fixed (z, c),
					 z, c, "expected %s got %s = %s + %s for add fixed");

	check_binary_op (d, qof_numeric_add_fixed (z, d),
					 z, d, "expected %s got %s = %s + %s for add fixed");

	/* ------------------------------------------------------------ */
	/* Same as above, but with signs reviersed */
	a = c;
	b = d;
	/* Well, actually 14/24 would be acceptable/better in this case */
	check_binary_op (qof_numeric_create (-7, 12),
					 qof_numeric_add (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s + %s for add exact");

	check_binary_op (qof_numeric_create (-58, 100),
					 qof_numeric_add (a, b, 100, QOF_HOW_RND_ROUND),
					 a, b,
					 "expected %s got %s = %s + %s for add 100ths (banker's)");

	check_binary_op (qof_numeric_create (-5833, 10000),
					 qof_numeric_add (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_SIGFIGS (4) |
									  QOF_HOW_RND_ROUND),
					 a, b, "expected %s got %s = %s + %s for add 4 sig figs");

	check_binary_op (qof_numeric_create (-583333, 1000000),
					 qof_numeric_add (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_SIGFIGS (6) |
									  QOF_HOW_RND_ROUND),
					 a, b, "expected %s got %s = %s + %s for add 6 sig figs");

	check_binary_op (qof_numeric_create (-1, 12),
					 qof_numeric_sub (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s - %s for sub exact");

	/* We should try something trickier for reduce & lcd */
	check_binary_op (qof_numeric_create (-1, 12),
					 qof_numeric_sub (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_REDUCE), a, b,
					 "expected %s got %s = %s - %s for sub reduce");

	check_binary_op (qof_numeric_create (-1, 12),
					 qof_numeric_sub (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_LCD), a, b,
					 "expected %s got %s = %s - %s for sub reduce");

	check_binary_op (qof_numeric_create (-8, 100),
					 qof_numeric_sub (a, b, 100, QOF_HOW_RND_ROUND),
					 a, b,
					 "expected %s got %s = %s - %s for sub 100ths (banker's)");

	/* ------------------------------------------------------------ */
#if CHECK_ERRORS_TOO
	c = qof_numeric_add_with_error (a, b, 100, QOF_HOW_RND_ROUND, &err);
	printf ("add 100ths/error : %s + %s = %s + (error) %s\n\n",
			qof_numeric_print (a), qof_numeric_print (b),
			qof_numeric_print (c), qof_numeric_print (err));

	c = qof_numeric_sub_with_error (a, b, 100, QOf_HOW_RND_FLOOR, &err);
	printf ("sub 100ths/error : %s - %s = %s + (error) %s\n\n",
			qof_numeric_print (a), qof_numeric_print (b),
			qof_numeric_print (c), qof_numeric_print (err));

#endif

	/* ------------------------------------------------------------ */
	/* Add and subtract some random numbers */
	for (i = 0; i < NREPS; i++)
	{
		QofNumeric e;
		gint64 deno = rand () + 1;
		gint64 na = get_random_gint64 ();
		gint64 nb = get_random_gint64 ();
		gint64 ne;

		/* avoid overflow; */
		na /= 2;
		nb /= 2;

		a = qof_numeric_create (na, deno);
		b = qof_numeric_create (nb, deno);

		/* Add */
		ne = na + nb;
		e = qof_numeric_create (ne, deno);
		check_binary_op (e,
						 qof_numeric_add (a, b, QOF_DENOM_AUTO,
										  QOF_HOW_DENOM_EXACT), a, b,
						 "expected %s got %s = %s + %s for exact addition");

		/* Subtract */
		ne = na - nb;
		e = qof_numeric_create (ne, deno);
		check_binary_op (e,
						 qof_numeric_sub (a, b, QOF_DENOM_AUTO,
										  QOF_HOW_DENOM_EXACT), a, b,
						 "expected %s got %s = %s - %s for exact subtraction");
	}
}

/* ======================================================= */

static void
check_mult_div (void)
{
	gint i, j;
	gint64 v;
	QofNumeric c, d;
	QofNumeric amt_a, amt_tot, frac, val_tot, val_a;
	QofNumeric a, b;

	a = qof_numeric_create (-100, 100);
	b = qof_numeric_create (1, 1);
	check_binary_op (qof_numeric_create (-100, 100),
					 qof_numeric_div (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s / %s div exact");

	a = qof_numeric_create (-100, 100);
	b = qof_numeric_create (-1, 1);
	check_binary_op (qof_numeric_create (100, 100),
					 qof_numeric_div (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s / %s div exact");

	a = qof_numeric_create (-100, 100);
	b = qof_numeric_create (-1, 1);
	check_binary_op (qof_numeric_create (100, 100),
					 qof_numeric_mul (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s * %s mult exact");

	a = qof_numeric_create (2, 6);
	b = qof_numeric_create (1, 4);

	check_binary_op (qof_numeric_create (2, 24),
					 qof_numeric_mul (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s * %s for mult exact");

	check_binary_op (qof_numeric_create (1, 12),
					 qof_numeric_mul (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_REDUCE), a, b,
					 "expected %s got %s = %s * %s for mult reduce");

	check_binary_op (qof_numeric_create (8, 100),
					 qof_numeric_mul (a, b, 100, QOF_HOW_RND_ROUND),
					 a, b, "expected %s got %s = %s * %s for mult 100th's");

	check_binary_op (qof_numeric_create (8, 6),
					 qof_numeric_div (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s / %s for div exact");

	check_binary_op (qof_numeric_create (4, 3),
					 qof_numeric_div (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_REDUCE), a, b,
					 "expected %s got %s = %s / %s for div reduce");

	check_binary_op (qof_numeric_create (133, 100),
					 qof_numeric_div (a, b, 100, QOF_HOW_RND_ROUND),
					 a, b, "expected %s got %s = %s * %s for div 100th's");

#if CHECK_ERRORS_TOO
	QofNumeric c;
	c = qof_numeric_mul_with_error (a, b, 100, QOF_HOW_RND_ROUND, &err);
	printf ("mul 100ths/error : %s * %s = %s + (error) %s\n\n",
			qof_numeric_print (a), qof_numeric_print (b),
			qof_numeric_print (c), qof_numeric_print (err));

	c = qof_numeric_div_with_error (a, b, 100, QOF_HOW_RND_ROUND, &err);
	printf ("div 100ths/error : %s / %s = %s + (error) %s\n\n",
			qof_numeric_print (a), qof_numeric_print (b),
			qof_numeric_print (c), qof_numeric_print (err));

#endif

	/* Check for math with 2^63 < num*num < 2^64 which previously failed 
	 * see http://bugzilla.gnome.org/show_bug.cgi?id=144980 
	 */
	v = 1000000;
	a = qof_numeric_create (1 * v, v);
	b = qof_numeric_create (10000000 * v, v);

	check_binary_op (b,
					 qof_numeric_mul (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_LCD), a, b,
					 "expected %s got %s = %s * %s for multiply");

	/* Multiply some random numbers.  This test presumes that
	 * RAND_MAX is approx 2^32 
	 */
	for (i = 0; i < NREPS; i++)
	{
		gint64 deno = 1;
		gint64 na = rand ();
		gint64 nb = rand ();
		gint64 ne;

		/* avoid overflow; */
		na /= 2;
		nb /= 2;
		ne = na * nb;

		a = qof_numeric_create (na, deno);
		b = qof_numeric_create (nb, deno);

		check_binary_op (qof_numeric_create (ne, 1),
						 qof_numeric_mul (a, b, QOF_DENOM_AUTO,
										  QOF_HOW_DENOM_EXACT), a, b,
						 "expected %s got %s = %s * %s for mult exact");

		/* Force 128-bit math to come into play */
		for (j = 1; j < 31; j++)
		{
			a = qof_numeric_create (na << j, 1 << j);
			b = qof_numeric_create (nb << j, 1 << j);
			check_binary_op (qof_numeric_create (ne, 1),
							 qof_numeric_mul (a, b, QOF_DENOM_AUTO,
											  QOF_HOW_DENOM_REDUCE), a, b,
							 "expected %s got %s = %s * %s for mult reduce");
		}

		/* Do some hokey random 128-bit division too */
		b = qof_numeric_create (deno, nb);

		check_binary_op (qof_numeric_create (ne, 1),
						 qof_numeric_div (a, b, QOF_DENOM_AUTO,
										  QOF_HOW_DENOM_EXACT), a, b,
						 "expected %s got %s = %s / %s for div exact");

		/* avoid overflow; */
		na /= 2;
		nb /= 2;
		ne = na * nb;
		for (j = 1; j < 16; j++)
		{
			a = qof_numeric_create (na << j, 1 << j);
			b = qof_numeric_create (1 << j, nb << j);
			check_binary_op (qof_numeric_create (ne, 1),
							 qof_numeric_div (a, b, QOF_DENOM_AUTO,
											  QOF_HOW_DENOM_REDUCE), a, b,
							 "expected %s got %s = %s / %s for div reduce");
		}
	}

	a = qof_numeric_create (782592055622866ULL, 89025);
	b = qof_numeric_create (2222554708930978ULL, 85568);
	/* Dividing the above pair overflows, in that after
	 * the division the denominator won't fit into a 
	 * 64-bit quantity.  This can be seen from
	 * the factorization int primes:
	 * 782592055622866 = 2 * 2283317 * 171371749
	 * (yes, thats a seven and a nine digit prime)
	 * 2222554708930978 = 2 * 1111277354465489
	 * (yes, that's a sixteen-digit prime number)
	 * 89025 = 3*5*5*1187 
	 * 85568= 64*7*191   
	 * If the rounding method is exact/no-round, then 
	 * an overflow error should be signalled; else the 
	 * divide routine should shift down the results till
	 * the overflow is eliminated.
	 * 
	 */
	check_binary_op (qof_numeric_error (QOF_ERROR_OVERFLOW),
					 qof_numeric_div (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_RND_NEVER |
									  QOF_HOW_DENOM_EXACT), a, b,
					 "expected %s got %s = %s / %s for div exact");

	check_binary_op (qof_numeric_create (338441, 1000000),
					 qof_numeric_div (a, b, QOF_DENOM_AUTO,
									  QOF_HOW_DENOM_SIGFIGS (6) |
									  QOF_HOW_RND_ROUND), a, b,
					 "expected %s got %s = %s / %s for div round");

	/* Below is a 'typical' value calculation: 
	 * value_frac = value_tot * amt_frace / amt_tot
	 * and has some typical potential-overflow values. 
	 * 82718 = 2 * 59 * 701
	 * 47497125586 = 2 * 1489 * 15949337
	 * 69100955 = 5 * 7 * 11 * 179483
	 * 32005637020 = 4 * 5 * 7 * 43 * 71 * 103 * 727
	 */
	a = qof_numeric_create (-47497125586LL, 82718);
	b = qof_numeric_create (-69100955LL, 55739);
	c = qof_numeric_mul (a, b, QOF_DENOM_AUTO, QOF_HOW_DENOM_EXACT);
	d = qof_numeric_create (-32005637020LL, 55739);

	check_binary_op (qof_numeric_create (-102547458LL, 82718),
					 qof_numeric_div (c, d, 82718,
									  QOF_HOW_DENOM_EXACT),
					 c, d, "expected %s got %s = %s / %s for div round");

	/* If we specify QOF_HOW_RND_NEVER, then we should get an error,
	 * since the exact result won't fit into a 64-bit quantity. */
	check_binary_op (qof_numeric_error (QOF_ERROR_REMAINDER),
					 qof_numeric_div (c, d, 82718,
									  QOF_HOW_DENOM_EXACT |
									  QOF_HOW_RND_NEVER), c, d,
					 "expected %s got %s = %s / %s for div round");

	/* A simple irreducible ratio, involving negative numbers */
	amt_a = qof_numeric_create (-6005287905LL, 40595);
	amt_tot = qof_numeric_create (-8744187958LL, 40595);
	frac = qof_numeric_div (amt_a, amt_tot,
							QOF_DENOM_AUTO, QOF_HOW_DENOM_REDUCE);

	check_binary_op (qof_numeric_create (6005287905LL, 8744187958LL),
					 frac, amt_a, amt_tot,
					 "expected %s got %s = %s / %s for div reduce");

	/* Another overflow-prone condition */
	val_tot = qof_numeric_create (-4280656418LL, 19873);
	val_a = qof_numeric_mul (frac, val_tot,
							 qof_numeric_denom (val_tot),
							 QOF_HOW_RND_ROUND | QOF_HOW_DENOM_EXACT);
	check_binary_op (qof_numeric_create (-2939846940LL, 19873),
					 val_a, val_tot, frac,
					 "expected %s got %s = %s * %s for mult round");

	frac = qof_numeric_create (396226789777979LL, 328758834367851752LL);
	val_tot = qof_numeric_create (467013515494988LL, 100);
	val_a = qof_numeric_mul (frac, val_tot,
							 qof_numeric_denom (val_tot),
							 QOF_HOW_RND_ROUND | QOF_HOW_DENOM_EXACT);
	check_binary_op (qof_numeric_create (562854125307LL, 100),
					 val_a, val_tot, frac,
					 "expected %s got %s = %s * %s for mult round");

	/* Yet another bug from bugzilla ... */
	a = qof_numeric_create (40066447153986554LL, 4518);
	b = qof_numeric_create (26703286457229LL, 3192);
	frac = qof_numeric_div (a, b,
							QOF_DENOM_AUTO,
							QOF_HOW_DENOM_SIGFIGS (6) | QOF_HOW_RND_ROUND);

	check_binary_op (qof_numeric_create (106007, 100),
					 frac, a, b,
					 "expected %s got %s = %s / %s for mult sigfigs");

}

static void
check_reciprocal (void)
{
	QofNumeric a, b, ans, val;
	gdouble flo;

	val = qof_numeric_create (-60, 20);
	check_unary_op (qof_numeric_eq, qof_numeric_create (-3, -1),
					qof_numeric_convert (val, QOF_DENOM_RECIPROCAL (1),
										 QOF_HOW_RND_NEVER),
					val, "expected %s = %s = (%s as RECIP(1))");

	a = qof_numeric_create (200, 100);
	b = qof_numeric_create (300, 100);

	/* 2 + 3 = 5 */
	ans = qof_numeric_add (a, b, QOF_DENOM_RECIPROCAL (1), QOF_HOW_RND_NEVER);
	check_binary_op (qof_numeric_create (5, -1),
					 ans, a, b,
					 "expected %s got %s = %s + %s for reciprocal");

	/* 2 + 3 = 5 */
	a = qof_numeric_create (2, -1);
	b = qof_numeric_create (300, 100);
	ans = qof_numeric_add (a, b, QOF_DENOM_RECIPROCAL (1), QOF_HOW_RND_NEVER);
	check_binary_op (qof_numeric_create (5, -1),
					 ans, a, b,
					 "expected %s got %s = %s + %s for reciprocal");


	/* 2 + 3 = 5 */
	a = qof_numeric_create (2, -1);
	b = qof_numeric_create (300, 100);
	ans = qof_numeric_add (a, b, QOF_DENOM_RECIPROCAL (1), QOF_HOW_RND_NEVER);
	check_binary_op (qof_numeric_create (5, -1),
					 ans, a, b, "expected %s got %s = %s + %s for recirocal");

	/* check gnc_numeric_to_double */
	flo = qof_numeric_to_double (qof_numeric_create (5, -1));
	do_test ((5.0 == flo), "reciprocal conversion");

	/* check gnc_numeric_compare */
	a = qof_numeric_create (2, 1);
	b = qof_numeric_create (2, -1);
	do_test ((0 == qof_numeric_compare (a, b)), " 2 == 2 ");
	a = qof_numeric_create (2, 1);
	b = qof_numeric_create (3, -1);
	do_test ((-1 == qof_numeric_compare (a, b)), " 2 < 3 ");
	a = qof_numeric_create (-2, 1);
	b = qof_numeric_create (2, -1);
	do_test ((-1 == qof_numeric_compare (a, b)), " -2 < 2 ");
	a = qof_numeric_create (2, -1);
	b = qof_numeric_create (3, -1);
	do_test ((-1 == qof_numeric_compare (a, b)), " 2 < 3 ");

	/* check for equality */
	a = qof_numeric_create (2, 1);
	b = qof_numeric_create (2, -1);
	do_test (qof_numeric_equal (a, b), " 2 == 2 ");

	/* check gnc_numeric_mul */
	a = qof_numeric_create (2, 1);
	b = qof_numeric_create (3, -1);
	ans = qof_numeric_mul (a, b, QOF_DENOM_RECIPROCAL (1), QOF_HOW_RND_NEVER);
	check_binary_op (qof_numeric_create (6, -1),
					 ans, a, b, "expected %s got %s = %s * %s for recirocal");

	/* check gnc_numeric_div */
	/* -60 / 20 = -3 */
	a = qof_numeric_create (-60, 1);
	b = qof_numeric_create (2, -10);
	ans = qof_numeric_div (a, b, QOF_DENOM_RECIPROCAL (1), QOF_HOW_RND_NEVER);
	check_binary_op (qof_numeric_create (-3, -1),
					 ans, a, b, "expected %s got %s = %s / %s for recirocal");

	/* 60 / 20 = 3 */
	a = qof_numeric_create (60, 1);
	b = qof_numeric_create (2, -10);
	ans = qof_numeric_div (a, b, QOF_DENOM_RECIPROCAL (1), QOF_HOW_RND_NEVER);
	check_binary_op (qof_numeric_create (3, -1),
					 ans, a, b, "expected %s got %s = %s / %s for recirocal");

}


/* ======================================================= */

static void
run_test (void)
{
	check_eq_operator ();
	check_reduce ();
	check_equality_operator ();
	check_rounding ();
	check_double ();
	check_neg ();
	check_add_subtract ();
	check_mult_div ();
	check_reciprocal ();
}

int
main (void)
{
	qof_init ();
	run_test ();

	print_test_results ();
	exit (get_rv ());
	qof_close ();
	return get_rv();
}

/* ======================== END OF FILE ====================== */
