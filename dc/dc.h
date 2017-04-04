/* 
 * Header file for dc routines
 *
 * Copyright (C) 1994, 1997, 1998 Free Software Foundation, Inc.
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
 *
 *    The Free Software Foundation, Inc.
 *    59 Temple Place, Suite 330
 *    Boston, MA 02111 USA
 */

#ifndef DC_DEFS_H
#define DC_DEFS_H

#include <iostream>
#include <memory>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include "../libs/inline_variant.hpp"
#include "../DC_Exc.h"

/* 'I' is a command, and bases 17 and 18 are quite
 * unusual, so we limit ourselves to bases 2 to 16
 */
#define DC_IBASE_MAX	16

#define DC_SUCCESS		0
#define DC_DOMAIN_ERROR	1
#define DC_FAIL			2	/* generic failure */


# define DC_DECLVOID()			(void)



typedef enum {DC_TOSS, DC_KEEP}   dc_discard;
typedef enum {DC_NONL, DC_WITHNL} dc_newline;


/* type discriminant for dc_data */
typedef enum {DC_UNINITIALIZED, DC_NUMBER, DC_STRING} value_type;

struct UNINITIALIZED {
};

struct bc_struct;
struct dc_string;

struct dc_num {
    bc_struct* d;
    dc_num() {};
    dc_num(bc_struct* d) : d(d) {};
};
struct dc_str {
    dc_string* s;
    dc_str() {};
    dc_str(dc_string* s) : s(s) {};
};

/* only numeric.c knows what dc_num's *really* look like */
//typedef struct bc_struct *dc_num;

/* only string.c knows what dc_str's *really* look like */
//typedef struct dc_string *dc_str;

// TODO: make dc_string its own fancy object (or maybe replace it with std::shared_ptr<std::string> later on, or smth)
/* here is the completion of the dc_string type: */

struct dc_string {
	char *s_ptr;  /* pointer to base of string */
	size_t s_len; /* length of counted string */
	int  s_refs;  /* reference count to cut down on memory use by duplicates */
};


/* except for the two implementation-specific modules, all
 * dc functions only know of this one generic type of object
 */
/*typedef struct {
	value_type type;	// discriminant for union
	union {
		dc_num number;
		dc_str string;
	} v;
} dc_data;

template <typename R,typename F1, typename F2>
inline R p_match(dc_data d, F1&& f1, F2&& f2) {
if (d.type == DC_NUMBER) {
return f1(d.v.number);
} else if (d.type == DC_STRING) {
return f2(d.v.string);
} else {
throw DC_Exc("Invalid type in stack");
}
}

template <>
inline dc_num dc_dget(dc_data d) {
if (d.type == DC_NUMBER) {
return d.v.number;
} else {
throw DC_Exc("Type Check error (expected number)");
}
}


template <>
inline dc_str dc_dget(dc_data d) {
if (d.type == DC_STRING) {
return d.v.string;
} else {
throw DC_Exc("Type Check error (expected string)");
}
}

template <typename R, typename Vis>
R p_visit(dc_data d, Vis vis) {
if (d.type == DC_NUMBER) {
return vis(d.v.number);
} else if (d.type == DC_STRING) {
return vis(d.v.string);
} else if (d.type = DC_UNINITIALIZED) {
return vis(UNINITIALIZED());
} else {
throw DC_Exc("Invalid type in stack");
}
}

*/

using dc_data = boost::variant<UNINITIALIZED, dc_num, dc_str>;

template <typename R, typename F1, typename F2>
inline R p_match(dc_data d, F1&& f1, F2&& f2) {
	return match(d, [f1](dc_num num)  -> R { return f1(num); },
					[f2](dc_str str)  -> R { return f2(str); },
					[](UNINITIALIZED) -> R { throw DC_Exc("Trying to use uninitialized value");
										});
}

template <typename R, typename Vis>
R p_visit(dc_data d, Vis&& vis) {
	return boost::apply_visitor(vis, d);
}


template <typename T>
inline T dc_dget(dc_data d);


template <>
inline dc_str dc_dget(dc_data d) {
	return match(d, [](dc_str x)      -> dc_str { return x; },
				    [](dc_num)        -> dc_str { throw DC_Exc("Type Error, expected string, got number");; },
			  	    [](UNINITIALIZED) -> dc_str { throw DC_Exc("Type Error, expected string, got unitiliazed value"); });
}


template <>
inline dc_num dc_dget(dc_data d) {
	return match(d, [](dc_num x)   -> dc_num { return x; },
				 [](dc_str)        -> dc_num { throw DC_Exc("Type Error, expected number, got string");; },
				 [](UNINITIALIZED) -> dc_num { throw DC_Exc("Type Error, expected number, got unitiliazed value"); });
}





#endif /* not DC_DEFS_H */
