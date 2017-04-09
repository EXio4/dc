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
#include <string>
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

typedef enum {DC_NONL, DC_WITHNL} dc_newline;

struct UNINITIALIZED {
};

struct bc_struct;

struct dc_num {
    bc_struct* d;
    dc_num() {};
    dc_num(bc_struct* d) : d(d) {};
};
struct dc_str {
    std::shared_ptr<std::string> s;
    dc_str() {};
    dc_str(std::shared_ptr<std::string> s) : s(s) {};
};


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
