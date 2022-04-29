/*
	9900 Cross-Assembler  v. 1.0


	January, 1985

	Original 6800 version Copyright (c) 1980 William C. Colley, III.
	Modified for the TMS9900/99105  Series by Alexander Cameron.

File:	a99.c

It all begins here.
*/

/*  Get globals:  */
#include <stdio.h>
#include "a99gbl.h"
#include "A99LIB.h"


#define _S_IFMT         0xF000          /* file type mask */
#define _S_IFDIR        0x4000          /* directory */
#define _S_IFCHR        0x2000          /* character special */
#define _S_IFIFO        0x1000          /* pipe */
#define _S_IFREG        0x8000          /* regular */
#define _S_IREAD        0x0100          /* read permission, owner */
#define _S_IWRITE       0x0080          /* write permission, owner */
#define _S_IEXEC        0x0040

/*  The assembler starts here.  */


int main(int argc, char *argv[])

{
	int short n, m;
	unsigned short u;
	putls("----------------------------------------------------\n");
	putls("TMS9900 / TMS99105 Cross-Assembler  v. 1.0\n");
	putls("Modified from William C. Colley, III's M6800 version\n");
	putls("CP/M Version by Alexander Cameron January, 1985\n");
	putls("MSDOS Version by  Alexander Cameron May 2010 to July, 2019\n");
	putls("----------------------------------------------------\n");
	setfiles(argc,argv);
	sympoint = symtbl;		/*  Initialize symbol table.	*/
	symend = &symtbl[SYMBOLS]; /*.symname; */
	memset(sympoint,'\0',(SYMBOLS * (SYMLEN + 2)));


	ifsp = 0;			/*  Initialise if stack.	*/
	ifstack[ifsp] = 0xffff;
	hxbytes = 0;			/*  Initialise hex generator.	*/
	flshhbf(0); 
	pc = errcount = 0;
	pass = 1;
	putls("Pass 1\n");
	while (pass != 3)	/*  The actual assembly starts here.	*/
	{
		errcode = ' ';
        if (!getlin()) 
		{
			strcpy(linbuf,"\tEND\t\t;You forgot this!\n");
			linptr = linbuf;
		 	markerr('*'); 
			ifstack[ifsp] = 0xffff;
		}
	
		
		asmline();		/*  Get binary from line.	*/


		if (pass > 1) {
			lineout();	/*  In pass 2, list line.	*/

			if (INTEL8) {
				hexout8();
			}

			else {
			    hexout16(); /*  In pass 2, build hex file.	*/
			}
		}
		pc += nbytes;
		
		if (pass == 0)	/*  This indicates end of pass 1.	*/
		{
			pass = 2;
			putls("Pass 2\n");
			source_rewind();


			ifsp = 0;
			pc = 0;
			errcount = 0;
				/* switch on flag at some stage */
			if (!INTEL8) {
				hexflg = EXTADDR;
			    hexout16(); 
			}
			hexflg =NOCODE;
		}
	}

		/*  print symbol table statistics  */

	u = sortsym(NOSORT)*100;
	u /= SYMBOLS;
	printf("\nSymbol table use factor = %d%s\n",u,"%");

	/*  List number of errors.	*/

	linptr = linbuf;
	*linptr++ = '\n';
	if (errcount == 0) strcpy(linptr,"No");
	else
	{
		putdec(errcount,&linptr);
		*linptr = '\0';
	}
	strcat(linbuf," error(s).\n");
	putls(linbuf);
	if (lstbuf.fd != CONO && lstbuf.fd != NOFILE)
	{
		putlin(linbuf,&lstbuf);
	/*	putchr('\f',&lstbuf); */
	}
	if (lstbuf.fd != NOFILE)		/*  If needed, sort and list
						symbol table.		*/
	{

		n = sortsym(SORT);
		sympoint = symtbl;
		while (n > 0)
		{
			linptr = linbuf;
			for (m = 0; m < 4; m++)
			{
				memcpy(linptr,sympoint->symname,SYMLEN);
				linptr += SYMLEN;
				*linptr++ = ' ';
				*linptr++ = ' ';
				puthex4(sympoint->symvalu,&linptr);


				*linptr++ = ' ';
				*linptr++ = ' ';
				*linptr++ = ' ';
				*linptr++ = ' ';
				sympoint++;
				if (--n <= 0) break;
			}
			linptr -= 4;
			*linptr++ = '\n';
			*linptr = '\0';
			putlin(linbuf,&lstbuf);
		}
		putchr(CPMEOF,&lstbuf);
	}
	flush(&lstbuf);
	close(sorbuf.fd);
	wipeout("\r");
}

/*
Function to set up the file structure.  Routine is called with
the original argc and argv from main().
*/

setfiles(int argc, char *argv[])


{
	char sorfname[24],lstfname[24],hexfname[24], *tptr;
	if (--argc == 0) wipeout("\nNo file info supplied.\n");
	argv++;
	sorbuf.fd = lstbuf.fd = hexbuf.fd = hexbuf.fd = NOFILE;
	lstbuf.pointr = lstbuf.space;
	hexbuf.pointr = hexbuf.space;
	sorfname[0] = curdrive + 'A';
	sorfname[1] = ':';
	sorfname[2] = '\0';

	strcat(sorfname,*argv++);
	for (tptr = sorfname; *tptr != '\0'; tptr++) if (*tptr == '.') *tptr = '\0';
	strcpy(lstfname,sorfname);
	strcpy(hexfname,lstfname);
	strcat(sorfname,".A99");
	strcat(lstfname,".L99");
	strcat(hexfname,".H99");

	if (--argc == 0) goto defsorf;
	while (**argv != '\0')
	{
		switch (*(*argv)++)
		{
			case 'S':	switch (*(*argv)++)
					{
						case 'A':
						case 'B':
						case 'C':
						case 'D':	sorfname[0] = *(*argv - 1);

						case '-':	if((sorbuf.fd = open(sorfname,0)) == -1)
									wipeout("\nCan't open source.\n");
								    source_rewind(); 
								    break;

						default:	goto badcomnd;
					}
					break;

			case 'L':	switch (*(*argv)++)
					{
						case 'A':
						case 'B':
						case 'C':
						case 'D':	lstfname[0] = *(*argv - 1);
						case '-':	if ((lstbuf.fd = creat(lstfname,_S_IREAD | _S_IWRITE )) == -1)
									wipeout("\nCan't open list.\n");
								break;

						case 'X':	lstbuf.fd = CONO;
								break;

						case 'Y':	lstbuf.fd = LST;
								break;

						default:	goto badcomnd;
					}
					break;

					case 'H':	switch(*(*argv)++)
							{
								case 'A':
								case 'B':
								case 'C':
								case 'D': hexfname[0] = *(*argv - 1);
								case '-': if (( hexbuf.fd = creat(hexfname,_S_IREAD | _S_IWRITE )) == -1)
										  	  wipeout("\n Can't open hex.\n");
									   /*   if (( hexbufU.fd = creat(hexfnameU,_S_IREAD | _S_IWRITE )) == -1)
										  wipeout("\n Can't open hex.\n");
										  if (( hexbufL.fd = creat(hexfnameL,_S_IREAD | _S_IWRITE )) == -1)
										  wipeout("\n Can't open hex.\n"); */
									      break;

								case 'X':	hexbuf.fd = CONO;
								break;

					case 'Y':	hexbuf.fd = LST;
								break;

						default:	goto badcomnd;
					}
					break;

			badcomnd:
			default:	wipeout("\nIllegal command line.\n");
		}
	}
	/* if (sorbuf.fd != NOFILE) return 0; */

	defsorf:

	if ((sorbuf.fd = open(sorfname,0)) == -1) wipeout("\nCan't open source.\n");
	source_rewind();

}


