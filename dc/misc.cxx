/* 
 * misc. functions for the "dc" Desk Calculator language.
 *
 * Copyright (C) 1994, 1997, 1998, 2000 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can either send email to this
 * program's author (see below) or write to:
 *   The Free Software Foundation, Inc.
 *   59 Temple Place, Suite 330
 *   Boston, MA 02111 USA
 */

/* This module contains miscelaneous functions that have no
 * special knowledge of any private data structures.
 * They could all be moved to their own separate modules, but
 * are agglomerated here for convenience.
 */

#include "config.h"

#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#include <ctype.h>
#ifndef isgraph
# ifndef HAVE_ISGRAPH
#  define isgraph isprint
# endif
#endif
#include <getopt.h>
#include "dc.h"
#include "dc-proto.h"

#ifndef EXIT_FAILURE	/* C89 <stdlib.h> */
# define EXIT_FAILURE	1
#endif


/* print an "out of memory" diagnostic and exit program */
void
memfail ()
{
	printf("dc/out of memory");
	exit(EXIT_FAILURE);
}

/* malloc or die */
void *
DC::malloc (
	size_t len )
{
	void *result = ::malloc(len);

	if (!result)
		memfail();
	return result;
}


/* print the id in a human-understandable form
 *  fp is the output stream to place the output on
 *  id is the name of the register (or command) to be printed
 *  suffix is a modifier (such as "stack") to be printed
 */
void
DC::show_id (
	FILE *fp ,
	int id ,
	const char *suffix )
{
	if (isgraph(id))
		fprintf(fp, "'%c' (%#o)%s", id, id, suffix);
	else
		fprintf(fp, "%#o%s", id, suffix);
}


/* report that corrupt data has been detected;
 * use the msg and regid (if nonnegative) to give information
 * about where the garbage was found,
 *
 * will abort() so that a debugger might be used to help find
 * the bug
 */
/* If this routine is called, then there is a bug in the code;
 * i.e. it is _not_ a data or user error
 */
void
DC::garbage (
	
	const char *msg ,
	int regid )
{
	if (regid < 0) {
		out << "garbage " << msg << std::endl;
	} else {
		out << "register" << msg << std::endl;
	}
	abort();
}


/* call system() with the passed string;
 * if the string contains a newline, terminate the string
 * there before calling system.
 * Return a pointer to the first unused character in the string
 * (i.e. past the '\n' if there was one, to the '\0' otherwise).
 */
const char *
DC::system (
	const char *s )
{
#ifndef REAL_DC
	return NULL;
#endif
	const char *p;
	char *tmpstr;
	size_t len;

	p = strchr(s, '\n');
	if (p) {
		len = p - s;
		tmpstr = (char*)malloc(len + 1);
		strncpy(tmpstr, s, len);
		tmpstr[len] = '\0';
		system(tmpstr);
		free(tmpstr);
		return p + 1;
	}
	system(s);
	return s + strlen(s);
}


/* print out the indicated value */
void DC::print (dc_data value, int obase, dc_newline newline_p)
{
		
	p_match<void>(value,
			   [this, obase, newline_p](dc_num n) {
					out_num(n, obase, newline_p);
			   },
			   [this, obase, newline_p](dc_str s) {
					out_str(s, newline_p);
			   });
}

/* return a duplicate of the passed value, regardless of type */
dc_data
DC::dup (
	
	dc_data value )
{
	return p_match<dc_data>(value,
				      [this](dc_num n) {
							return (dc_data)dup_num(n);
   					  },
					  [this](dc_str s) {
							return (dc_data)dup_str(s);
					  });
}
