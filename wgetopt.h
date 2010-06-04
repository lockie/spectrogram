/* 
  Declarations for wgetopt.
  Copyright (C) 1989, 90, 91, 92, 93, 94 Free Software Foundation, Inc.

 This file is part of the GNU C Library.  Its master source is NOT part of
 the C library, however.  The master source lives in /gd/gnu/lib.

 The GNU C Library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public License as
 published by the Free Software Foundation; either version 2 of the
 License, or (at your option) any later version.

 The GNU C Library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with the GNU C Library; see the file COPYING.LIB.  If
 not, write to the Free Software Foundation, Inc., 675 Mass Ave,
 Cambridge, MA 02139, USA.  
*/

#ifndef WGETOPT_H
#define WGETOPT_H

#include <wchar.h>

#ifdef	__cplusplus
extern "C" {
#endif

extern wchar_t *woptarg;

extern int woptind;

extern int wopterr;

extern int woptopt;

struct woption
{
#if defined (__STDC__) && __STDC__
  const wchar_t *name;
#else
  wchar_t *name;
#endif
  int has_arg;
  int *flag;
  int val;
};

/* Names for the values of the `has_arg' field of `struct option'.  */
#define	no_argument		0
#define required_argument	1
#define optional_argument	2

#if defined (__STDC__) && __STDC__
#ifdef __GNU_LIBRARY__
extern int wgetopt (int argc, wchar_t *const *argv, const wchar_t *shortopts);
#else /* not __GNU_LIBRARY__ */
extern int wgetopt ();
#endif /* __GNU_LIBRARY__ */

extern int wgetopt_long (int argc, wchar_t *const *argv, const wchar_t *shortopts,
		        const struct woption *longopts, int *longind);

extern int wgetopt_long_only (int argc, wchar_t *const *argv,
			     const wchar_t *shortopts,
		             const struct woption *longopts, int *longind);

extern int _wgetopt_internal (int argc, wchar_t *const *argv,
			     const wchar_t *shortopts,
		             const struct woption *longopts, int *longind,
			     int long_only);
#else /* not __STDC__ */

extern int wgetopt();

extern int wgetopt_long();

extern int wgetopt_long_only();

extern int _wgetopt_internal();
#endif /* __STDC__ */

#ifdef	__cplusplus
}
#endif

#endif  // WGETOPT_H
