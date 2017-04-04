/* number.h: Arbitrary precision numbers header file. */
/*
    Copyright (C) 1991, 1992, 1993, 1994, 1997, 2000 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License , or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to:

      The Free Software Foundation, Inc.
      59 Temple Place, Suite 330
      Boston, MA 02111-1307 USA.


    You may contact the author by:
       e-mail:  philnelson@acm.org
      us-mail:  Philip A. Nelson
                Computer Science Department, 9062
                Western Washington University
                Bellingham, WA 98226-9062
       
*************************************************************************/

#ifndef _NUMBER_H_
#define _NUMBER_H_

#include <string>

typedef enum {PLUS, MINUS} sign;

typedef struct bc_struct *bc_num;

typedef struct bc_struct
    {
      sign  n_sign;
      int   n_len;	/* The number of digits before the decimal point. */
      int   n_scale;	/* The number of digits after the decimal point. */
      int   n_refs;     /* The number of pointers to this number. */
      bc_num n_next;	/* Linked list for available list. */
      char *n_ptr;	/* The pointer to the actual storage.
			   If NULL, n_value points to the inside of
			   another number (multiply...) and should
			   not be "freed." */
      char *n_value;	/* The number. Not zero char terminated.
			   May not point to the same place as n_ptr as
			   in the case of leading zeros generated. */
    } bc_struct;


/* The base used in storing the numbers in n_value above.
   Currently this MUST be 10. */

#define BASE 10

/*  Some useful macros and constants. */

#define CH_VAL(c)     (c - '0')
#define BCD_CHAR(d)   (d + '0')

#ifdef MIN
#undef MIN
#undef MAX
#endif
#define MAX(a,b)      ((a)>(b)?(a):(b))
#define MIN(a,b)      ((a)>(b)?(b):(a))
#define ODD(a)        ((a)&1)

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef LONG_MAX
#define LONG_MAX 0x7ffffff
#endif

/* Function Prototypes */

/* Define the  macro if it is needed. */

class BC {
private:
	
	bc_num _bc_Free_list = NULL;
	
	bc_num new_sub_num (int length, int scale, char* value);
	void pn (bc_num num);
	bc_num _bc_do_sub (bc_num n1, bc_num n2, int scale_min);
	void _bc_simp_mul (bc_num n1, int n1len, bc_num n2, int n2len, bc_num *prod,  int full_scale);
	bc_num _bc_do_add (bc_num n1, bc_num n2, int scale_min);
	void _bc_shift_addsub (bc_num accum, bc_num val, int shift, int sub);
	char* internal_num2str (bc_num num);
	void _bc_rec_mul (bc_num u, int ulen, bc_num v, int vlen, bc_num *prod, int full_scale);
	void bc_out_long (long val, int size, int space, std::string& output);
public:
	
	bc_num _zero_;
	bc_num _one_;
	bc_num _two_;
	
	BC();
	
	bc_num new_num (int length, int scale);
	void free_num(bc_num *num);
	
	bc_num copy_num (bc_num num);
	
	void init_num (bc_num *num);
	
	void str2num (bc_num *num, char *str, int scale);
	
	char *num2str (bc_num num);
	
	void int2num (bc_num *num, int val);
	
	long num2long (bc_num num);
	
	int compare (bc_num n1, bc_num n2);
	
	char is_zero (bc_num num);
	
	char is_near_zero (bc_num num, int scale);

	char is_neg (bc_num num);
	
	void add (bc_num n1, bc_num n2, bc_num *result, int scale_min);
	
	void sub (bc_num n1, bc_num n2, bc_num *result, int scale_min);
	
	void multiply (bc_num n1, bc_num n2, bc_num *prod, int scale);
	
	int divide (bc_num n1, bc_num n2, bc_num *quot, int scale);
	
	int modulo (bc_num num1, bc_num num2, bc_num *result,
							   int scale);
	
	int divmod (bc_num num1, bc_num num2, bc_num *quot,
							   bc_num *rem, int scale);
	
	int raisemod (std::ostream& irc, bc_num base, bc_num expo, bc_num mod,
								 bc_num *result, int scale);
	
	void raise (std::ostream& irc, bc_num num1, bc_num num2, bc_num *result,
							   int scale);
	
	int sqrt (std::ostream& irc, bc_num *num, int scale);
	
	void out_num (bc_num num, int o_base, std::string& output,
								 int leading_zero);
	
};


#endif
