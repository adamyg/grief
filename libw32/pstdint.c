/*  A portable stdint.h
 ****************************************************************************
 *  BSD License:
 ****************************************************************************
 *
 *  Copyright (c) 2005-2011 Paul Hsieh
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *  ==nonotice==
 *
 ****************************************************************************
 *
 */
 

/* 
 *  Please compile with the maximum warning settings to make sure macros are not
 *  defined more than once.
 */
 
#define  _CRT_SECURE_NO_DEPRECATE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pstdint.h"

#define glue3_aux(x,y,z) x ## y ## z
#define glue3(x,y,z) glue3_aux(x,y,z)

#define DECLU(bits) glue3(uint,bits,_t) glue3(u,bits,=) glue3(UINT,bits,_C) (0);
#define DECLI(bits) glue3(int,bits,_t) glue3(i,bits,=) glue3(INT,bits,_C) (0);

#define DECL(us,bits) glue3(DECL,us,) (bits)

#define TESTUMAX(bits) glue3(u,bits,=) glue3(~,u,bits); if (glue3(UINT,bits,_MAX) glue3(!=,u,bits)) printf ("Something wrong with UINT%d_MAX\n", bits)
 
int main () {
	DECL(I,8)
	DECL(U,8)
	DECL(I,16)
	DECL(U,16)
	DECL(I,32)
	DECL(U,32)
#ifdef INT64_MAX
	DECL(I,64)
	DECL(U,64)
#endif
	intmax_t imax = INTMAX_C(0);
	uintmax_t umax = UINTMAX_C(0);
	char str0[256], str1[256];

	sprintf (str0, "%d %x\n", 0, ~0);
	
	sprintf (str1, "%d %x\n",  i8, ~0);
	if (0 != strcmp (str0, str1)) printf ("Something wrong with i8 : %s\n", str1);
	sprintf (str1, "%u %x\n",  u8, ~0);
	if (0 != strcmp (str0, str1)) printf ("Something wrong with u8 : %s\n", str1);
	sprintf (str1, "%d %x\n",  i16, ~0);
	if (0 != strcmp (str0, str1)) printf ("Something wrong with i16 : %s\n", str1);
	sprintf (str1, "%u %x\n",  u16, ~0);
	if (0 != strcmp (str0, str1)) printf ("Something wrong with u16 : %s\n", str1);
	sprintf (str1, "%" PRINTF_INT32_MODIFIER "d %x\n",  i32, ~0);
	if (0 != strcmp (str0, str1)) printf ("Something wrong with i32 : %s\n", str1);
	sprintf (str1, "%" PRINTF_INT32_MODIFIER "u %x\n",  u32, ~0);
	if (0 != strcmp (str0, str1)) printf ("Something wrong with u32 : %s\n", str1);
#ifdef INT64_MAX
	sprintf (str1, "%" PRINTF_INT64_MODIFIER "d %x\n",  i64, ~0);
	if (0 != strcmp (str0, str1)) printf ("Something wrong with i64 : %s\n", str1);
#endif
	sprintf (str1, "%" PRINTF_INTMAX_MODIFIER "d %x\n",  imax, ~0);
	if (0 != strcmp (str0, str1)) printf ("Something wrong with imax : %s\n", str1);
	sprintf (str1, "%" PRINTF_INTMAX_MODIFIER "u %x\n",  umax, ~0);
	if (0 != strcmp (str0, str1)) printf ("Something wrong with umax : %s\n", str1);
	
	TESTUMAX(8);
	TESTUMAX(16);
	TESTUMAX(32);
#ifdef INT64_MAX
	TESTUMAX(64);
#endif

	return EXIT_SUCCESS;
}
