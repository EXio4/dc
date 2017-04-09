/*
 * prototypes of all externally visible dc functions
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

#ifndef DC_PROTO
#define DC_PROTO

#include "dc-regdef.h"
#include "number.h"
#include <map>
#include <list>
#include <ostream>
#include <functional>

typedef enum {DC_FALSE, DC_TRUE} boolean;

typedef enum {
	DC_OKAY = DC_SUCCESS, /* no further intervention needed for this command */
		DC_EATONE,		/* caller needs to eat the lookahead char */
		DC_QUIT,		/* quit out of unwind_depth levels of evaluation */
		
		/* with the following return values, the caller does not have to 
		* fret about stdin_lookahead's value
		*/
		DC_INT,			/* caller needs to parse a dc_num from input stream */
		DC_STR,			/* caller needs to parse a dc_str from input stream */
		DC_SYSTEM,		/* caller needs to run a system() on next input line */
		DC_COMMENT,		/* caller needs to skip to the next input line */
		DC_NEGCMP,		/* caller needs to re-call func() with `negcmp' set */
		
		DC_EOF_ERROR	/* unexpected end of input; abort current eval */
} dc_status;


/* what's most useful: quick access or sparse arrays? */
/* I'll go with sparse arrays for now (ie, map) */
using dc_array = std::map<int, dc_data>;



/* simple linked-list implementaion suffices: */
/*struct dc_list {
	dc_data value;
	struct dc_array *array;	// opaque
	struct dc_list *link;
};
typedef struct dc_list dc_list;*/

struct dc_node {
    dc_data value;
    std::shared_ptr<dc_array> array;
    dc_node() {
        array = NULL;
    }
    dc_node(dc_data value, std::shared_ptr<dc_array> array) : value(value), array(array) {};
};

extern void memfail (void);
extern void out_of_memory();

class DC {
private:
	
	std::ostream& out;
	
	BC bc;
	int ibase=10;		/* input base, 2 <= ibase <= DC_IBASE_MAX */
	int obase=10;		/* output base, 2 <= obase */
	int scale=0;		/* scale (see user documentaton) */
	
	/* for Quitting evaluations */
	int unwind_depth=0;
	
	/* if true, active Quit will not exit program */
	boolean unwind_noexit=DC_FALSE;
	
	/*
	* Used to synchronize lookahead on stdin for '?' command.
	* If set to EOF then lookahead is used up.
	*/
	int stdin_lookahead=EOF;

		/* input_fil and input_str are passed as arguments to getnum */
		
		/* used by the input_* functions: */
	FILE *input_fil_fp;
	const char *input_str_string;
	
	/* Since we have a need for two characters of pushback, and
	* ungetc() only guarantees one, we place the second pushback here
	*/
    int input_pushback;

	std::list<dc_node> stack;
	std::list<dc_node> registers[DC_REGCOUNT];
	int eval_and_free_str(int&, dc_data);
	dc_status func(int&, int, int, int);
	int input_fil();
	int input_str();
	void register_init (void);
	void dc_array_init (void);
public:
	DC(std::ostream& out) : out(out) {
		register_init();
		dc_array_init();
	}
	
	
	const char *dc_str2charp (dc_str);
	const char *system (const char *);
	void *malloc (size_t);
	std::shared_ptr<dc_array> get_stacked_array (int);

	void dc_array_set (int, int, dc_data);
	dc_data dc_array_get ( int, int);
			
	void binop (std::function<int( dc_num, dc_num, int, dc_num&)>, int);
	void binop2 (std::function<int( dc_num, dc_num, int, dc_num&, dc_num&)>, int);
	void triop (std::function<int( dc_num, dc_num, dc_num, int, dc_num&)>, int);
	void clear_stack ();
	void dump_num(dc_num);
	void free_num (dc_num *);
	void garbage (const char *, int);
	
	void out_num (dc_num, int, dc_newline);
	void out_str (dc_str, dc_newline);
	void print (dc_data, int, dc_newline);
	void printall (int);
	void push (dc_data);
	void register_push (int, dc_data);
	void register_set (int, dc_data);
	void set_stacked_array (int, std::shared_ptr<dc_array>);
	void show_id (FILE *, int, const char *);

	int  cmpop ();
	int  compare (dc_num, dc_num);
	int  evalfile ( int&, FILE *);
	int  evalstr ( int&, dc_data);
	int  dc_num2int (dc_num);
	int  dc_numlen (dc_num);
	int  pop ( dc_data *);
	int  register_get ( int, dc_data *);
	int  register_pop ( int, dc_data *);
	int  tell_length ( dc_data);
	int  tell_scale (dc_num);
	int  tell_stackdepth (void);
	int  top_of_stack ( dc_data *);

	size_t dc_strlen (dc_str);

	dc_data dup ( dc_data);
	dc_data dup_num (dc_num);
	dc_data dup_str (dc_str);
	dc_data getnum (std::function<int(void)>, int, int *);
	dc_data int2data (int);
	dc_data makestring (const char *, size_t);
	dc_data readstring (FILE *, int , int);

	int add ( dc_num, dc_num, int, dc_num&);
	int div ( dc_num, dc_num, int, dc_num&);
	int divrem ( dc_num, dc_num, int, dc_num&, dc_num&);
	int exp ( dc_num, dc_num, int, dc_num&);
	int modexp ( dc_num, dc_num, dc_num, int, dc_num&);
	int mul ( dc_num, dc_num, int, dc_num&);
	int rem ( dc_num, dc_num, int, dc_num&);
	int sub ( dc_num, dc_num, int, dc_num&);
	int sqrt ( dc_num, int, dc_num&);
	
};

#endif
