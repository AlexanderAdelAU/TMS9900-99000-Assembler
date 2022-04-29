/*
	TMS9900/99105  Cross-Assembler  v. 1.0

	January, 1985

	Original 6800 version Copyright (c) 1980 William C. Colley, III.
	Modified for the TMS9900/99105  Series by Alexander Cameron.

File:	a99get.c

Routines to get source text from the disk and return it in
manageable chunks such as operators, labels, opcodes, etc.
*/

/*  Get Globals:  */
#include "a99gbl.h"

errmsg(char *msg)
{
    putls("error");


}

/*
void setmem(char *sympoint, int short size, char fill)
{
   sympoint= calloc(size, SYMBOLS * (SYMLEN + 2));


} */

void bdos(int short fd, char c)
{
    putls(c);


}
