# TMS9900/TMS99000/TMS99105A-Cross-Assembler
This is an Cross Assembler for the TMS9900 and TMS99000 CPUs

This is a assembler for the TMS9900/90000 processor that produces non relocatable (absolute) code for .A99 assembler code.   The relocatable assembler is based on this assembler.   A test programme located in the Tests folder provide an example of a TMS 9900 Monitor (TIMON) source code, TIMON-V2.A99 and HEX (TIMON-V2.H99) and Listing file (TIMON-V2.L99).  Eclipse project files are also included.

### An Example of a List File Produced By the A99 Cross Assembler.

<pre>
                        ;
                        ;---BOOT MODULE
                        ;
                        
                        	DXOP	CALL,6
                        	DXOP	RET,7
                        	DXOP	PUSH,8
                        	DXOP	POP,9
                        	DXOP	WRITE,12
                        	DXOP	READ,13
                        	DXOP	MESG,14
                        ;
  0000                  R0	EQU	0
  0001                  R1	EQU	1
  0002                  R2	EQU	2
  0003                  R3	EQU	3
  0004                  R4	EQU	4
  0005                  R5	EQU	5
  0006                  R6	EQU	6
  0007                  R7	EQU	7
  0008                  R8	EQU	8
  0009                  R9	EQU	9
  000A                  R10	EQU	10
  000B                  R11	EQU	11
  000C                  R12	EQU	12
  000D                  R13	EQU	13
  000E                  R14	EQU	14
  000F                  R15	EQU	15
 ;
  C400                  	AORG	0C400H
   ;
  C400   1000           	JMP	BOOT 
  ;
  ;	MONITOR EQUATES	
 ;
  C600                  LOADER	EQU	0C600H
  E600                  MONITOR EQU	0E600H
  F000                  TIMON	EQU	0F000H
  E624                  WBOOT	EQU	MONITOR+9*4		;WARM BOOT
  E60C                  SELDSK  EQU	MONITOR+3*4		;SELECT A DISK
  E610                  RECAL	EQU	MONITOR+4*4		;HOME THE DRIVE
  E614                  SEEK	EQU	MONITOR+5*4		;SEEK A TRACK
  E618                  RDREC	EQU	MONITOR+6*4		;READ A SECTOR
  E61C                  WRREC	EQU	MONITOR+7*4		;WRITE A SECTOR
  E620                  RDID	EQU	MONITOR+8*4
                        ;
                        ;
                        ;---NOW TRY TO BOOT FROM TRACK 0 SECTOR 1
                        ;
  C402   02E0 C462      BOOT	LWPI	WORKSP			;ALL PROGRAMMES HAVE THEIR OWN WP
  C406   020A C4A2      	LI	R10,STACKP
  C40A   0209 C482      	LI	R9,STACKL
  C40E   0420 E600      	BLWP	@MONITOR		;NEED TO SET UP MONITOR
  C412   2FA0 C450      	MESG	@MSG2
  C416   04C2           	CLR	R2
  C418   2DA0 E60C      	CALL	@SELDSK			;SELECT DRIVE A
  C41C   160B           	JNE	BOOT2
  C41E   2DA0 E610      	CALL	@RECAL		 	;RECAL DRIVE A
  C422   0203 0001      	LI	R3,1			;BOOT SECTOR
  C426   0204 C600      	LI	R4,LOADER		;USE THIS AREA TO BOOT
  C42A   2DA0 E618      	CALL	@RDREC
  C42E   1602           	JNE	BOOT2
  C430   0460 C600      	B	@LOADER			;BRANCH HERE TO LOAD THE SYSTEM
                        ;
                        ; --BOOT ERROR
                        ;
  C434   C801 C460      BOOT2	MOV	R1,@STAT
  C438   2FA0 C440      	MESG	@MSG1
  C43C   0460 F000      	B	@TIMON
                        ;
  C440   2D2D 426F      MSG1	TEXT	'--Boot error'
  C444   6F74 2065      
  C448   7272 6F72      
  C44C   0D0A 00        	BYTE	0DH,0AH,0
  C44F   00             	EVEN
  C450   2D2D 426F      MSG2	TEXT	'--Booting....'
  C454   6F74 696E      
  C458   672E 2E2E      
  C45C   2E             
  C45D   0D0A 00        	BYTE	0DH,0AH,0
  C460                  	EVEN
  C460   0000           STAT	WORD	0
                        	
                        ;
                        ;--BUFFERS ETC
                        ;
                        ;	EVEN
  C462                  WORKSP	BSS	32
  C482                  STACKL	BSS	32
  C4A2                  STACKP	EQU	$+0
                        ;
                        
  C4A2                  	END

No error(s).
R13               000D    R2                0002    CALL              2D80    R3                0003
STACKL            C482    R4                0004    POP               2E40    R5                0005
RET               2DC0    R6                0006    R14               000E    R7                0007
RECAL             E610    R8                0008    WRREC             E61C    R9                0009
RDREC             E618    SELDSK            E60C    BOOT              C402    R11               000B
R0                0000    LOADER            C600    STAT              C460    SEEK              E614
WRITE             2F00    READ              2F40    RDID              E620    PUSH              2E00
R15               000F    BOOT2             C434    WORKSP            C462    R12               000C
STACKP            C4A2    TIMON             F000    R1                0001    MESG              2F80
R10               000A    MSG1              C440    MSG2              C450    WBOOT             E624
MONITOR           E600
  </pre>
