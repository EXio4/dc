/* 
 * evaluate the dc language, from a FILE* or a string
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

/* This is the only module which knows about the dc input language */

#include "config.h"

#include <stdio.h>
#ifdef HAVE_STRING_H
# include <string.h>	/* memchr */
#else
# ifdef HAVE_MEMORY_H
#  include <memory.h>	/* memchr, maybe */
# else
#  ifdef HAVE_STRINGS_H
#   include <strings.h>	/* memchr, maybe */
#  endif
#endif
#endif
#include "dc.h"
#include "dc-proto.h"
#include "visitors.h"


/* passed as an argument to getnum */
int
DC::input_fil DC_DECLVOID()
{
	if (input_pushback != EOF){
		int c = input_pushback;
		input_pushback = EOF;
		return c;
	}
	return getc(input_fil_fp);
}

/* passed as an argument to getnum */
int
DC::input_str DC_DECLVOID()
{
	if (!*input_str_string)
		return EOF;
	return *input_str_string++;
}



/* takes a string and evals it; frees the string when done */
/* Wrapper around evalstr to avoid duplicating the free call
 * at all possible return points.
 */
int
DC::eval_and_free_str (
	
	int& n ,
	dc_data string )
{
	dc_status status;

	status = (dc_status)evalstr(n, string);
	p_visit<void>(string, FreeVar(*this));
	return status;
}


/* func does the grunt work of figuring out what each input
 * character means; used by both evalstr and evalfile
 *
 * c -> the "current" input character under consideration
 * peekc -> the lookahead input character
 * negcmp -> negate comparison test (for <,=,> commands)
 */
dc_status
DC::func (
	
	int& ix ,
	int c ,
	int peekc ,
	int negcmp )
{
	if (ix <= 0) {
		out << "Timeout" << std::endl;
		return DC_QUIT;
	}
	ix--;
	/* we occasionally need these for temporary data */
	/* Despite the GNU coding standards, it is much easier
	 * to have these declared once here, since this function
	 * is just one big switch statement.
	 */
	dc_data datum;
	int tmpint;

	switch (c){
	case '_': case '.':
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	case '8': case '9': case 'A': case 'B':
	case 'C': case 'D': case 'E': case 'F':
		return DC_INT;
	case ' ':
	case '\t':
	case '\n':
		/* standard command separators */
		break;

	case '+':	/* add top two stack elements */
		binop([this](dc_num p2, dc_num p3, int p4, dc_num& p5) { return add(p2,p3,p4,p5); }, scale);
		break;
	case '-':	/* subtract top two stack elements */
		binop([this](dc_num p2, dc_num p3, int p4, dc_num& p5) { return sub(p2,p3,p4,p5); }, scale);
		break;
	case '*':	/* multiply top two stack elements */
		binop([this](dc_num p2, dc_num p3, int p4, dc_num& p5) { return mul(p2,p3,p4,p5); }, scale);
		break;
	case '/':	/* divide top two stack elements */
		binop([this](dc_num p2, dc_num p3, int p4, dc_num& p5) { return div(p2,p3,p4,p5); }, scale);
		break;
	case '%':
		/* take the remainder from division of the top two stack elements */
		binop([this](dc_num p2, dc_num p3, int p4, dc_num& p5) { return rem(p2,p3,p4,p5); }, scale);
		break;
	case '~':
		/* Do division on the top two stack elements.  Return the
		 * quotient as next-to-top of stack and the remainder as
		 * top-of-stack.
		 */
		binop2([this](dc_num p2, dc_num p3, int p4, dc_num& p5, dc_num& p6) { return divrem(p2,p3,p4,p5,p6); }, scale);
		break;
	case '|':
		/* Consider the top three elements of the stack as (base, exp, mod),
		 * where mod is top-of-stack, exp is next-to-top, and base is
		 * second-from-top. Mod must be non-zero, exp must be non-negative,
		 * and all three must be integers. Push the result of raising
		 * base to the exp power, reduced modulo mod. If we had base in
		 * register b, exp in register e, and mod in register m then this
		 * is conceptually equivalent to "lble^lm%", but it is implemented
		 * in a more efficient manner, and can handle arbritrarily large
		 * values for exp.
		 */
		triop([this](dc_num p2, dc_num p3, dc_num p4, int p5, dc_num& p6) { return modexp(p2,p3,p4,p5,p6); }, scale);
		break;
	case '^':	/* exponientiation of the top two stack elements */
		binop([this](dc_num p2, dc_num p3, int p4, dc_num& p5) { return exp(p2,p3,p4,p5); }, scale);
		break;
	case '<':
		/* eval register named by peekc if
		 * less-than holds for top two stack elements
		 */
		if (peekc == EOF)
			return DC_EOF_ERROR;
		if ( (cmpop() <  0) == !negcmp )
			if (register_get(peekc, &datum) == DC_SUCCESS)
				if (eval_and_free_str(ix, datum) == DC_QUIT)
					return DC_QUIT;
		return DC_EATONE;
	case '=':
		/* eval register named by peekc if
		 * equal-to holds for top two stack elements
		 */
		if (peekc == EOF)
			return DC_EOF_ERROR;
		if ( (cmpop() == 0) == !negcmp )
			if (register_get(peekc, &datum) == DC_SUCCESS)
				if (eval_and_free_str(ix, datum) == DC_QUIT)
					return DC_QUIT;
		return DC_EATONE;
	case '>':
		/* eval register named by peekc if
		 * greater-than holds for top two stack elements
		 */
		if (peekc == EOF)
			return DC_EOF_ERROR;
		if ( (cmpop() >  0) == !negcmp )
			if (register_get(peekc, &datum) == DC_SUCCESS)
				if (eval_and_free_str(ix, datum) == DC_QUIT)
					return DC_QUIT;
		return DC_EATONE;
#ifdef REAL_DC
	case '?':	/* read a line from standard-input and eval it */
		if (stdin_lookahead != EOF){
			ungetc(stdin_lookahead, stdin);
			stdin_lookahead = EOF;
		}
		if (eval_and_free_str(readstring(stdin, '\n', '\n')) == DC_QUIT)
			return DC_QUIT;
		return DC_OKAY;
#endif
	case '[':	/* read to balancing ']' into a dc_str */
		return DC_STR;

	case '!':	/* read to newline and call system() on resulting string */
		if (peekc == '<' || peekc == '=' || peekc == '>')
			return DC_NEGCMP;
#ifdef REAL_DC
		return DC_SYSTEM;
#endif
	case '#':	/* comment; skip remainder of current line */
		return DC_COMMENT;

	case 'a':	/* Convert top of stack to an ascii character. */
		if (pop(&datum) == DC_SUCCESS){
			char tmps;
			p_match<void>(datum,
						  [this, &tmps](dc_num num) {
							  tmps = (char) dc_num2int(num, DC_TOSS);
						  },
						  [this, &tmps](dc_str str) {
							  tmps = *dc_str2charp(str);
							  free_str(&str);							
						  });
			push(makestring(&tmps, 1));
		}
		break;
	case 'c':	/* clear whole stack */
		clear_stack();
		break;
	case 'd':	/* duplicate the datum on the top of stack */
		if (top_of_stack(&datum) == DC_SUCCESS)
			push(dup(datum));
		break;
	case 'f':	/* print list of all stack items */
		printall(obase);
		break;
	case 'i':	/* set input base to value on top of stack */
		if (pop(&datum) == DC_SUCCESS){
			tmpint = 0;

			p_match<void>(datum,
						  [this, &tmpint](dc_num num) {
								tmpint = dc_num2int(num, DC_TOSS);
						  },
						  [](dc_str str) {});
			if ( ! (2 <= tmpint  &&  tmpint <= DC_IBASE_MAX) )
				out << "input base must be a number between 2 and 16 (" << DC_IBASE_MAX << ") inclusive" << std::endl;
			else
				ibase = tmpint;
		}
		break;
	case 'k':	/* set scale to value on top of stack */
		if (pop(&datum) == DC_SUCCESS){
			tmpint = -1;
			p_match<void>(datum,
						  [this, &tmpint](dc_num num) {
				tmpint = dc_num2int(num, DC_TOSS);
			},
										  [](dc_str str) {});
			if ( ! (tmpint >= 0) )
				out << "scale must be a non negative number" << std::endl;
			else
				scale = tmpint;
		}
		break;
	case 'l':	/* "load" -- push value on top of register stack named
				 * by peekc onto top of evaluation stack; does not
				 * modify the register stack
				 */
		if (peekc == EOF)
			return DC_EOF_ERROR;
		if (register_get(peekc, &datum) == DC_SUCCESS)
			push(datum);
		return DC_EATONE;
	case 'n':	/* print the value popped off of top-of-stack;
				 * do not add a trailing newline
				 */
		if (pop(&datum) == DC_SUCCESS)
			print(datum, obase, DC_NONL, DC_TOSS);
		break;
	case 'o':	/* set output base to value on top of stack */
		if (pop(&datum) == DC_SUCCESS){
			tmpint = 0;
			p_match<void>(datum,
						  [this, &tmpint](dc_num num) {
				tmpint = dc_num2int(num, DC_TOSS);
			},
										  [](dc_str str) {});
			if ( ! (tmpint > 1) )
				out << "output base must be a number greater than 1" << std::endl;
			else
				obase = tmpint;
		}
		break;
	case 'p':	/* print the datum on the top of stack,
				 * with a trailing newline
				 */
		if (top_of_stack(&datum) == DC_SUCCESS)
			print(datum, obase, DC_WITHNL, DC_KEEP);
		break;
	case 'q':	/* quit two levels of evaluation, posibly exiting program */
		unwind_depth = 1; /* the return below is the first level of returns */
		unwind_noexit = DC_FALSE;
		return DC_QUIT;
	case 'r':	/* rotate (swap) the top two elements on the stack
				 */
		if (pop(&datum) == DC_SUCCESS) {
			dc_data datum2;
			int two_status;
			two_status = pop(&datum2);
			push(datum);
			if (two_status == DC_SUCCESS)
				push(datum2);
		}
		break;
	case 's':	/* "store" -- replace top of register stack named
				 * by peekc with the value popped from the top
				 * of the evaluation stack
				 */
		if (peekc == EOF)
			return DC_EOF_ERROR;
		if (pop(&datum) == DC_SUCCESS)
			register_set(peekc, datum);
		return DC_EATONE;
	case 'v':	/* replace top of stack with its square root */
		if (pop(&datum) == DC_SUCCESS){
			dc_num tmpnum;
			p_match<void>(datum,
						  [this, &datum, &tmpnum](dc_num num) {
								if (sqrt(num, scale, tmpnum) == DC_SUCCESS){
											push(dup_num(num));
											free_num(&num);
								}
						  },
						  [this](dc_str str) {
							  out << "square root of nonnumeric attempted" << std::endl;
						  });
		}
		break;
	case 'x':	/* eval the datum popped from top of stack */
		if (pop(&datum) == DC_SUCCESS){
			dc_status x = (dc_status)p_match<int>(datum,
						  [this,&datum](dc_num num) {
								push(datum);
								return (int)DC_SUCCESS;
						  },
						  [this,&ix,&datum](dc_str str) {
							  return (int)(eval_and_free_str(ix, datum));
						  });
			if (x == DC_QUIT) return DC_QUIT;
		}
		break;
	case 'z':	/* push the current stack depth onto the top of stack */
		push(int2data(tell_stackdepth()));
		break;

	case 'I':	/* push the current input base onto the stack */
		push(int2data(ibase));
		break;
	case 'K':	/* push the current scale onto the stack */
		push(int2data(scale));
		break;
	case 'L':	/* pop a value off of register stack named by peekc
				 * and push it onto the evaluation stack
				 */
		if (peekc == EOF)
			return DC_EOF_ERROR;
		if (register_pop(peekc, &datum) == DC_SUCCESS)
			push(datum);
		return DC_EATONE;
	case 'O':	/* push the current output base onto the stack */
		push(int2data(obase));
		break;
	case 'P':
		/* Pop the value off the top of a stack.  If it is
		 * a number, dump out the integer portion of its
		 * absolute value as a "base UCHAR_MAX+1" byte stream;
		 * if it is a string, just print it.
		 * In either case, do not append a trailing newline.
		 */
		if (pop(&datum) == DC_SUCCESS){
			p_match<void>(datum,
						  [this](dc_num num) {
								dump_num(num, DC_TOSS);
						  },
						  [this](dc_str str) {
								out_str(str, DC_NONL, DC_TOSS);
						  });
		}
		break;
	case 'Q':	/* quit out of top-of-stack nested evals;
				 * pops value from stack;
				 * does not exit program (stops short if necessary)
				 */
		if (pop(&datum) == DC_SUCCESS){
			unwind_depth = 0;
			unwind_noexit = DC_TRUE;
			unwind_depth = dc_num2int(dc_dget<dc_num>(datum), DC_TOSS);
			if (unwind_depth-- > 0)
				return DC_QUIT;
			unwind_depth = 0;	/* paranoia */
			out << "Q command requires a number >= 1" << std::endl;
		}
		break;
#if 0
	case 'R':	/* pop a value off of the evaluation stack,;
				 * rotate the top
				 remaining stack elements that many
				 * places forward (negative numbers mean rotate
				 * backward).
				 */
		if (pop(&datum) == DC_SUCCESS){
			tmpint = dc_num2int(dc_dget<dc_num>(datum), DC_TOSS);
			stack_rotate(tmpint);
		}
		break;
#endif
	case 'S':	/* pop a value off of the evaluation stack
				 * and push it onto the register stack named by peekc
				 */
		if (peekc == EOF)
			return DC_EOF_ERROR;
		if (pop(&datum) == DC_SUCCESS)
			register_push(peekc, datum);
		return DC_EATONE;
	case 'X':	/* replace the number on top-of-stack with its scale factor */
		if (pop(&datum) == DC_SUCCESS){
			tmpint = tell_scale(dc_dget<dc_num>(datum), DC_TOSS);
			push(int2data(tmpint));
		}
		break;
	case 'Z':	/* replace the datum on the top-of-stack with its length */
		if (pop(&datum) == DC_SUCCESS)
			push(int2data(tell_length(datum, DC_TOSS)));
		break;

	case ':':	/* store into array */
		if (peekc == EOF)
			return DC_EOF_ERROR;
		if (pop(&datum) == DC_SUCCESS){
			tmpint = dc_num2int(dc_dget<dc_num>(datum), DC_TOSS);
			if (pop(&datum) == DC_SUCCESS){
				if (tmpint < 0)
					out << "array index must be a nonnegative integer" << std::endl;
				else
					dc_array_set(peekc, tmpint, datum);
			}
		}
		return DC_EATONE;
	case ';':	/* retreive from array */
		if (peekc == EOF)
			return DC_EOF_ERROR;
		if (pop(&datum) == DC_SUCCESS){
			tmpint = dc_num2int(dc_dget<dc_num>(datum), DC_TOSS);
			if (tmpint < 0)
				out << "array index must be a non negative integer" << std::endl;
			else
				push(dc_array_get(peekc, tmpint));
		}
		return DC_EATONE;

	default:	/* What did that user mean? */
		{ 
		  out << c << " unimplemented" << std::endl;
		}
		break;
	}
	return DC_OKAY;
}


/* takes a string and evals it */
int
DC::evalstr (
	
	int& n ,
	dc_data string )
{
	const char *s;
	const char *end;
	const char *p;
	size_t len;
	int c;
	int peekc;
	int count;
	int negcmp;
	int next_negcmp = 0;

	dc_str string_s = dc_dget<dc_str>(string);
	
	s = dc_str2charp(string_s);
	end = s + dc_strlen(string_s);
	while (s < end){
		c = *(const unsigned char *)s++;
		peekc = EOF;
		if (s < end)
			peekc = *(const unsigned char *)s;
		negcmp = next_negcmp;
		next_negcmp = 0;
		switch (func(n, c, peekc, negcmp)){
		case DC_OKAY:
			break;
		case DC_EATONE:
			if (peekc != EOF)
				++s;
			break;
		case DC_QUIT:
			if (unwind_depth > 0){
				--unwind_depth;
				return DC_QUIT;
			}
			return DC_OKAY;

		case DC_INT:
			input_str_string = s - 1;
			push(getnum([this]() { return input_str(); }, ibase, &peekc));
			s = input_str_string;
			if (peekc != EOF)
				--s;
			break;
		case DC_STR:
			count = 1;
			for (p=s; p<end && count>0; ++p)
				if (*p == ']')
					--count;
				else if (*p == '[')
					++count;
			len = p - s;
			push(makestring(s, len-1));
			s = p;
			break;
		case DC_SYSTEM:
			s = system(s);
		case DC_COMMENT:
			s = (const char*)memchr((void*)s, '\n', (size_t)(end-s));
			if (!s)
				s = end;
			else
				++s;
			break;
		case DC_NEGCMP:
			next_negcmp = 1;
			break;

		case DC_EOF_ERROR:
			out << "unexpected EOS" << std::endl;
			return DC_OKAY;
		}
	}
	return DC_OKAY;
}


/* This is the main function of the whole DC program.
 * Reads the file described by fp, calls func to do
 * the dirty work, and takes care of func's shortcomings.
 */
int
DC::evalfile (
	
	int& n ,
	FILE *fp )
{
	int c;
	int peekc;
	int negcmp;
	int next_negcmp = 0;
	dc_data datum;

	stdin_lookahead = EOF;
	for (c=getc(fp); c!=EOF; c=peekc){
		peekc = getc(fp);
		/*
		 * The following if() is the only place where ``stdin_lookahead''
		 * might be set to other than EOF:
		 */
		if (fp == stdin)
			stdin_lookahead = peekc;
		negcmp = next_negcmp;
		next_negcmp = 0;
		switch (func(n, c, peekc, negcmp)){
		case DC_OKAY:
			if (stdin_lookahead != peekc  &&  fp == stdin)
				peekc = getc(fp);
			break;
		case DC_EATONE:
			peekc = getc(fp);
			break;
		case DC_QUIT:
			if (unwind_noexit != DC_TRUE)
				return DC_SUCCESS;
			out << "Q command argument exceeded string execution depth" << std::endl;
			if (stdin_lookahead != peekc  &&  fp == stdin)
				peekc = getc(fp);
			break;

		case DC_INT:
			input_fil_fp = fp;
			input_pushback = c;
			ungetc(peekc, fp);
			push(getnum([this]() { return input_fil(); }, ibase, &peekc));
			break;
		case DC_STR:
			ungetc(peekc, fp);
			datum = readstring(fp, '[', ']');
			push(datum);
			peekc = getc(fp);
			break;
		case DC_SYSTEM:
			{
				ungetc(peekc, fp);
				datum = readstring(stdin, '\n', '\n');
				dc_str str = dc_dget<dc_str>(datum);
				(void)system(dc_str2charp(str));
				free_str(&str);
				peekc = getc(fp);
			}
			break;
		case DC_COMMENT:
			while (peekc!=EOF && peekc!='\n')
				peekc = getc(fp);
			if (peekc != EOF)
				peekc = getc(fp);
			break;
		case DC_NEGCMP:
			next_negcmp = 1;
			break;

		case DC_EOF_ERROR:
			out << "unexpected EOF" << std::endl;
			return DC_FAIL;
		}
	}
	return DC_SUCCESS;
}
