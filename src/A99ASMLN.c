/*
 TMS9900/99105  Cross-Assembler  v. 1.0

 January, 1985


 Original 6800 version Copyright (c) 1980 William C. Colley, III.
 Modified for the TMS9900/99105  Series by Alexander Cameron.




 File:	a99asmln.c

 Line assembly routine.
 */

/*  Get globals:  */

#include <stdio.h>
#include "a99gbl.h"
#include "A99LIB.h"

/*
 This function is the workhorse of the assembler.  The routine
 gets any labels off the line and processes them, gets the opcode
 and builds the binary output as it evaluates the operand field.
 */

void asmline() {
	char c, d, label[2 * SYMLEN];
	int p;
	unsigned char opattr;
	unsigned short opcode;

	nbytes = 0;
	address = pc;

	directok = TRUE;
	evalerr = FALSE;
	hexflg = NOCODE;

	label[0] = '\0';
	binbuf[0] = binbuf[2] = binbuf[4] = 0x10;
	binbuf[1] = binbuf[3] = binbuf[5] = 0; /* NOP = 0x1000  */
	switch (getchr(&c, NOSKIP)) {
	case ALPHA:
		backchr;
		getname(label);
		if (chkoprat(label) != NO_OPR) {
			markerr('L');
			label[0] = '\0';
		}
		break;

	case END_LIN:
		return;

	case BLANK:
		break;

	default:
		markerr('L');
		while ((c = getchr(&c, NOSKIP)) != BLANK && c != END_LIN)
			;
		backchr;
	}
	d = getopcod(&opcode, &opattr);
	switch (d) {

	case 0:
		nbytes = 0;

	case -1:
		opcode = 255; /*  Set nil pseudo-op.	*/
		opattr = 0x80;
		/* label */
	case 1:
		if ((opattr & (PSOP | IFGROUP)) == (PSOP | IFGROUP)) {
			if (label[0] != '\0')
				markerr('L');
		} else {
			if (ifstack[ifsp] == 0)
				return;
			if (label[0] != '\0') {
				strcat(label, PADDING);
				if (pass == 1) {
					if (addsym(label)) {
						sympoint->symvalu = pc;

						if ((opattr & PSOP) && opcode == 0x000A) {
							sympoint->symname[1] |= 0x80;
						}
					}
					else
						sympoint->symflg |= DUPBIT;  /*Possible Duplicate */
				} else {
					if (sympoint->symflg & DUPBIT) markerr('D');
					if (slookup(label)) {
						markerr('P');
						label[0] = '\0';
					} else {
						if (sympoint->symvalu != pc
								|| (sympoint->symname[1] & 0x80)) {
							markerr('M');
						}
						sympoint->symname[0] &= 0x7f;
					}
				}
			}
		}
		hexflg = PUTCODE;
		if (opattr & PSOP)
			_psop(opcode, label);
		else
			_normop(opcode, opattr);
		return;
	}
}

/*
 Internal function to process pseudo opcodes.
 */

void _psop(unsigned short opcode, char *label) {
	char *bptr, d, c;
	int cflag;
	unsigned short t;
	struct symbtbl *sptr;
	sptr = sympoint;
	bptr = binbuf;

	switch (opcode) {
	case 0: /* AORG */
		t = eval(START);
		if (evalerr)
			return;
		address = pc = t;
		hexflg = FLUSH;
		if (label[0] == '\0')
			goto chkargs;
		backitem(t, VALUE);
		backchr;
		break;

	case 1:
		if (label[0] == '\0') /* EQU.	*/
		{
			markerr('L');
			return;
		}

		/* MSB set for multiple declarations */
		if (!(sptr->symname[1] & 0x80) && errcode == 'M') {
			errcode = ' ';
			errcount--;
		}

		t = eval(START);

		if (evalerr) {
			return;
		}

		address = sptr->symvalu;
		sptr->symvalu = t;

		if (!directok)
			markerr('P');
		if (address != t)
			markerr('M'); /* Lable defined multiple times */
		goto chkargs;

	case 2: /* BYTE	*/

		cflag = FALSE;
		while ((c = getitem(&t, SMALST)) != END_LIN) {
			switch (c) {
			case COMMA:
				if (cflag == TRUE) {
					cflag = FALSE;
					break;
				}
				*bptr++ = 0;
				nbytes += 1;
				break;

			case END_LIN:
				return;

			case OPERATR:
			case VALUE:
				backitem(t, c);
				/* Note eval does not gobbles up COMMA */
				cflag = TRUE;
				t = eval(START);
				if ((evalerr || t > 0x00ff) && (t < 0xff80))
					markerr('V');
				*bptr++ = t;
				nbytes += 1;
				break;

			default:
				markerr('S');
				break;
			}
		}
		return;

	case 3: /* WORD	*/
		cflag = FALSE;
		while ((c = getitem(&t, SMALST)) != END_LIN) {
			switch (c) {
			case COMMA:
				if (cflag == TRUE) {
					cflag = FALSE;
					break;
				}
				*bptr++ = 0;
				*bptr++ = 0;
				nbytes += 2;
				break;

			case END_LIN:
				return;

			case OPERATR:
			case VALUE:

				backitem(t, c);
				/* Note eval does not gobbles up COMMA */
				cflag = TRUE;
				t = eval(START);
				*bptr++ = t >> 8;
				*bptr++ = t;

				nbytes += 2;
				break;

			default:
				markerr('S');
				break;
			}

		}
		return;

	case 4: /* TEXT	*/
		while ((c = getchr(&d, SKIP)) != END_LIN) {
			if (c == COMMA)
				continue;
			if (c != QUOTE) {
				markerr('"');
				/*return;*/
			}
			while ((c = *linptr++) != d) {
				if (c == '\n') {
					markerr('"');
					return;
				}
				*bptr++ = c;
				nbytes++;
			}
		}
		return;

	case 5: /* BSS	*/
		t = eval(START);

		pc += t;
		hexflg = FLUSH;
		goto chkargs;

	case 6:
		if (pass == 1)
			pass--; /* END.	*/
		else {
			pass++;
			if (ifsp != 0)
				markerr('I');
		}
		hexflg = NOMORE;
		goto chkargs;

	case 7: /*  ELSE.	*/
	case 8:
		hexflg = NOCODE; /*  ENDI.	*/
		if (ifsp == 0) {
			markerr('I');
			return;
		}
		if (opcode == 8)
			ifsp--;
		else
			ifstack[ifsp] = ~ifstack[ifsp];
		goto chkargs;

	case 9:
		t = eval(START); /* IF.	*/
		if (evalerr || !directok) {
			markerr('P');
			t = 0xffff;
		}
		if (ifsp == 16)
			wipeout("\nIf stack overflow.\n");
		ifstack[++ifsp] = address = t;
		goto chkargs;

	case 10:
		if (label[0] == '\0') /* SET.	*/
		{
			markerr('L');
			return;
		}
		if ((sptr->symname[1] & 0x80) && errcode == 'M') {
			errcode = ' ';
			errcount--;
		}

		t = eval(START);
		if (evalerr)
			return;
		address = sptr->symvalu = t;
		if (!directok)
			markerr('P');
		goto chkargs;

	case 11:
		if (pc & 1) /* EVEN */
		{
			nbytes++;
			*bptr = 0;

		}
		goto chkargs;

	case 12:
		hexflg = NOCODE; /* DXOP */

		if (getchr(&c, SKIP) != ALPHA) {
			markerr('S');
			return;
		}
		backchr;
		getname(label); /* name of DXOP */
		strcat(label, PADDING);
		getitem(&t, SMALST); /* skip to number */
		t = eval(START); /* DXOP number */
		if (evalerr || t > 15) {
			markerr('S');
			return;
		}
		if (pass == 1)
			if (addsym(label))
				sympoint->symvalu = (0x2C00 | t << 6); /* XOP opcode */
		return;
		/* check to see if it is END_LIN.  It should be otherwise superfolous */
		chkargs: /*if (getitem(&c,SMALST) != END_LIN) markerr('T');  likely bug should be unsighed */

		if (getitem(&t, SMALST) != END_LIN)
			markerr('T');
		return;

	case 255: /*  NO OPCODE.	*/
		if (nbytes == 0)
			hexflg = NOCODE;
		return;
	}
}

/*
 Internal function to process normal (non-pseudo) opcodes.
 */

void _normop(unsigned short opcode, char attr)

{
	unsigned short regmask, regval, t, value, c;
	short int operand;
	short int disp, negindex;
	char d;
	unsigned char shift, parse, opdcount, attr1, attr2;

	nbytes = 2;
	if (pc & 0x01) /* make sure code is on an even boundary */
	{
		pc++;
		address++;
		markerr('A');
	}
	operand = 0;
	attr1 = attr & 0x07; /*  mask 3 most least significant bits */
	attr2 = (attr & 0x38) >> 3; /* next 3 bits represent attr2 */

	attr = attr1;
	opdcount = 1;
	negindex = 0;

	while (TRUE) {
		parse = TRUE;
		addrmode = WREG; /* default address mode */
		switch (attr) {

		case 0:
			if (attr1 || attr2)
				markerr('S');
			binbuf[0] = opcode >> 8;
			binbuf[1] = opcode;

			if ((c = getitem(&value, SMALST)) != END_LIN)
				(markerr('T'));
			return;

			/* Format 1 - S,D  */
			/* detect N,*N,*N+,@X,@X(N) */

		case 1:
			while (parse) {
				switch (c = getitem(&value, SMALST)) {

				case OPERATR:
					if (value == '*') {
						if (addrmode == INDIRECT || addrmode == SYMBOLIC)
							markerr('S'); /* Can't re-enter these modes */
						addrmode = INDIRECT;
					}
					if (value == '@') {
						if (addrmode == INDIRECT || addrmode == SYMBOLIC)
							markerr('S'); /* Can't re-enter these modes */
						addrmode = SYMBOLIC;
						nbytes += 2;
					}
					if (value == '-') {
						negindex = 1;
					}
					if (value == '+') {
						negindex = 0;
					}
					break;

				case VALUE:
					regval = 0;
					backitem(value, c);
					t = eval(START);

					if (evalerr)
						markerr('S');
					switch (addrmode) {
					case WREG:
						regval = t;
						regmask = 0;
						break;
					case INDIRECT:
						regval = t;
						regmask = 1;
						break;
					case AUTOINC:
						regval = t;
						regmask = 3;
						break;
					case SYMBOLIC:
						regmask = 2;
						binbuf[nbytes - 2] = t >> 8;
						binbuf[nbytes - 1] = t;
						break;
					case INDEXED:
						if (negindex)
							t = -t;
						regmask = 2;
						binbuf[nbytes - 2] = t >> 8;
						binbuf[nbytes - 1] = t;
						regval = eval(START);
						if (evalerr)
							markerr('E');
					}
					if (regval > 15)
						markerr('R');

					if (opdcount == 2)
						shift = 6;
					else
						shift = 0;
					operand |= (regval << shift);
					operand |= (regmask << (shift + 4));
					break;

				case COMMA:
					opdcount++;

					if (opdcount > 3) {
						markerr('T');
						break;
					}
					attr = attr2;
					parse = FALSE;
					break;

				case END_LIN:
					opcode |= operand;
					binbuf[0] = opcode >> 8;
					binbuf[1] = opcode;

					return;
				}
			}
			break;

			/*  case 2 handles signed displacement */

		case 2:
			while (TRUE) {
				switch (c = getitem(&value, SMALST))

				{
				case OPERATR:
				case VALUE:
					backitem(value, c);
					operand = eval(START);
					break;

				case END_LIN:

					operand -= (pc + 2);
					disp = operand / 2;
					if (disp < -128 || disp > 127)
						markerr('B'); //FIXED 12/6/2019 [AC]  disp and operand declared as int short was if(disp > 128 && disp < -127) markerr('B');
					opcode |= (disp & 0xff);
					binbuf[0] = opcode >> 8;
					binbuf[1] = opcode;
					return;

				default:
					markerr('S');
				}
			}

			/*  Destination register only - D and also C   */

		case 3:
			regval = eval(START);
			if (evalerr)
				markerr('S');
			if (regval > 15)
				markerr('R');
			if (getitem(&value, SMALST) != END_LIN)
				markerr('T');
			if (opdcount != 2)
				markerr('S');
			operand |= regval << 6;
			opcode |= operand;
			binbuf[0] = opcode >> 8;
			binbuf[1] = opcode;

			return;

			/*  Bit field operands	*/

		case 4:
			disp = eval(START);
			if (evalerr)
				markerr('S');
			opcode |= disp & 0xff;
			binbuf[0] = opcode >> 8;

			binbuf[1] = opcode;

			if (getitem(&value, SMALST) != END_LIN)
				markerr('T');
			return;

			/*  Workspace register - W  or shift count C  */

		case 5:
			while (parse) {
				switch (c = getitem(&value, SMALST)) {
				case VALUE:
				case OPERATR:
					backitem(value, c);
					regval = eval(START);
					if (regval > 15)
						markerr('R');
					if (opdcount == 2)
						operand |= regval << 4;
					else
						operand = regval;
					break;

				case COMMA:
					opdcount++;

					if (opdcount > 3) {
						markerr('T');
						break;
					}
					parse = FALSE;
					attr = attr2;
					break;

				case END_LIN:
					opcode |= operand;
					binbuf[0] = opcode >> 8;
					binbuf[1] = opcode;

					return;

				}
			}
			break;

			/*  Immediate operation - IOP */

		case 6:
			nbytes += 2;
			t = eval(START);
			opcode |= operand;

			binbuf[0] = (char) (opcode >> 8);
			binbuf[1] = (char) opcode;
			binbuf[2] = t >> 8;
			binbuf[3] = t;
			if (getitem(&value, SMALST) != END_LIN)
				markerr('T');
			return;

			/* Bit Manipulation Instructions - for the 99105 only
			 this instruction has a very similar format to the CRU
			 Bit Manipulation set and but the opcode addressing modes
			 are in the second word i.e.	[     opcode     ]
			 [ format3|format1]
			 hence calling _normop recursively we can then easily
			 swap the words around to produce the correct code   */

		case 7:
			_normop(0x0000, 0x19); /* call with opcode 0 and format 3 and format 1  */
			nbytes += 2; /* binbuf[0,1,2,3] now hold the 2nd and 3rd word */
			binbuf[4] = binbuf[2]; /* shift data down by 1 word to */
			binbuf[5] = binbuf[3]; /* allow for opcode             */
			binbuf[2] = binbuf[0];
			binbuf[3] = binbuf[1];
			binbuf[0] = opcode >> 8; /* now place original opcode in 1st word  */
			binbuf[1] = opcode;
			return; /* END_LIN_ comes from recursion */

		default:
			wipeout("bad operand format -- aborting \n");
			exit(-1); /* why -1? */

		}
	}
}
