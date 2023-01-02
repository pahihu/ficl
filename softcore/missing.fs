\ Missing definitions from FiCL
\ 160507AP  /string fixed
\ 160603AP  /string fixed again, darwin?/winnt?/linux?
.( loading UTILS ) cr

\ ANEW -extras

decimal

Start-Prefixes
: % ( "%..." -- n )
\G Binary number prefix.
   2 __tempbase ; IMMEDIATE
: & ( "&..." -- n )
\G Octal number prefix.
   8 __tempbase ; IMMEDIATE

-WARNING
: # ( "#..." -- n )
\G Decimal number prefix.
   10 __tempbase ; IMMEDIATE
+WARNING

: $ ( "$..." -- n )
\G Hexadecimal number prefix.
   16 __tempbase ; IMMEDIATE
End-Prefixes

S" FICL-OS" ENVIRONMENT? [IF]
CONSTANT ficl-os
[ELSE]
0 CONSTANT ficl-os
[THEN]


: DARWIN?   ( -- ff )
\G True if Darwin.
   ficl-os 1 =
;

: LINUX?    ( -- ff )
\G True if Linux.
   ficl-os 2 =
;

: WINNT?    ( -- )
\G True if Windows.
   ficl-os 3 =
;

DECIMAL

: DIRECTIVE ( -- )
\G Make previous word immediate and compile only.
   IMMEDIATE COMPILE-ONLY ;

: XXX ( -- )
\G Abort with `not implemented' message.
   1 abort" not implemented" ;

: BINARY ( -- )
\G Set number base to binary.
   2 BASE ! ;
: OCTAL ( -- )
\G Set number base to octal.
   8 BASE ! ;
: ALPHA ( -- )
\G Set number base to 36.
   36 BASE ! ;

: DOT ( n base -- )
\G Display `n' in temporary base `base'.
   BASE @ >R  BASE !  U.  R> BASE ! ;
: .X ( n -- )
\G Display `n' in hexadecimal.
   16 DOT ;
: H. ( n -- )
\G Display `n' in hexadecimal.
   .X ;
: .B ( n -- )
\G Display `n' in binary.
   2 DOT ;
: .O ( n -- )
\G Display `n' in octal.
   8 DOT ;
: .D ( n -- )
\G Display `n' in decimal.
   10 DOT ;
: S. ( n -- )
\G Display `n' as signed number.
   DUP 0< IF [CHAR] - EMIT THEN  ABS . ;
: .BASE ( -- )
\G Display number base in decimal.
   BASE @  .D ;

: BLANK ( addr u -- )
\G Fill `u' bytes from `addr' with BL.
   bl fill ;
: TEXT ( delim -- )
\G Move following text delimited with `delim' to PAD.
   pad 258 blank  word count pad swap  move ;

: STRING, ( caddr u -- )
\G Store string `caddr/u' in dictionary space.
   here over chars allot  swap move ;
: ," ( "string" -- )
\G Store delimited string in dictionary space.
   [char] " word count string, ;
: /STRING ( caddr1 u1 +n -- caddr2 u2 )
\G Remove +n chars from `caddr1/u'.
   swap over - >R + R> ;

( Character constants ---------------------------------------- )

 8 CONSTANT BS
 9 CONSTANT TAB
10 CONSTANT LINEFEED
13 CONSTANT CARRIAGE

: ?NEWLINE ( c -- f )
\G Answer TRUE if char is CR/LF.
   CARRIAGE OVER = IF  DROP TRUE EXIT  THEN
   LINEFEED      = ;

: ?BLANK ( c -- f )
\G Answer TRUE if char is white-space (space or tab) or newline.
         BL OVER = IF  DROP TRUE EXIT  THEN
        TAB OVER = IF  DROP TRUE EXIT  THEN
   ?NEWLINE ;

: CO ( -- )
\G Suspend current, return to caller. When caller exits
\G continue with current. [CI]
   R> R>  SWAP  >R >R ;

: EXCHANGE ( n1 addr -- n2 )
\G Store <n1> at <addr>, return old contents. [MAX]
   DUP >R  !  R> ;
  
: SCAN ( ca1 u1 b -- ca2 u2 )
\G Search for <b> in ca1/u1.
   >R
   BEGIN  OVER C@ R@  <>
	  OVER       0<> AND
   WHILE  1- >R CHAR+ R>
   REPEAT R> DROP ;

: SKIP ( ca1 u1 b -- ca2 u2 )
\G Skip <b> chars in ca1/u1.
   >R
   BEGIN  OVER C@ R@  =
          OVER       0<> AND
   WHILE  1- >R CHAR+ R>
   REPEAT R> DROP ;

: END-C@ ( ca u -- b )
\G Last char of string ca/u.
   DUP IF 1- CHARS THEN + C@ ;

: -SKIP ( ca u1 b -- ca u2 )
\G Remove trailing <b> chars from ca/u1.
   >R
   BEGIN  2DUP END-C@ R@  =
          OVER           0<> AND
   WHILE  1-
   REPEAT R> DROP ;

: BACK ( ca u1 b -- ca u2 )
\G Scan backwards for char <b> in ca/u1.
   >R
   BEGIN  2DUP END-C@ R@   <>
          OVER           0<> AND
   WHILE  1-
   REPEAT R> DROP ;

: -TRAILING ( ca u1 -- ca u2 )
\G Strip trailing whitespace.
   BEGIN 2DUP END-C@ ?BLANK
         OVER       0<>    AND
   WHILE 1-
   REPEAT ;

: -LEADING ( ca1 u1 -- ca2 u2 )
\G Strip leading whitespace.
   BEGIN
      OVER C@ ?BLANK
      OVER    0<>    AND
   WHILE 1- >R CHAR+ R>
   REPEAT ;

: -TRIM ( ca1 u1 -- ca2 u2 )
\G Trim, strip leading and trailing spaces.
   -LEADING  -TRAILING ;

: 3DUP ( a b c -- a b c a b c )
\G Duplicate top three data stack items.
   dup 2over rot ;
: BETWEEN ( n lo hi -- f )
\G Returns true if lo <= n <= hi.
   1+ within ;
: BOUNDS ( lo count -- hi lo )
\G Convert lo/count to boundary.
   over + swap ;

: $DUMP ( ca n -- ca n )
\G Display string on output device as "[string]".
   2dup cr ." [" type ." ]" ;

: HOLDS ( ca u -- )
\G Hold string in conversion buffer.
   DUP >R  1- CHARS+  R>
   BEGIN  ?DUP  WHILE
      OVER C@ HOLD
      1- SWAP CHAR- SWAP
   REPEAT  DROP ;

: $MOVUP ( ca n m -- )
\G Move string <ca/n> m chars up.
   SWAP >R           ( ca m )( R: n )
   OVER SWAP CHARS+  ( ca ca+m)( R: n )
   R>  MOVE ;

: RJUSTIFY ( ca n m -- ca n+?m )
\G Right align string <ca/n> in a field of <m>.
\G Modifies buffer <ca> in place!
   OVER - 0 MAX ( ca n #bl)
   DUP
   IF   3DUP $MOVUP
        SWAP >R 2DUP BLANK R>
   THEN
   + ;

: DABS ( d -- ud )
\G Convert double cell number to its absolute value.
   DUP 0< if  DNEGATE  THEN ;
: (UD.) ( ud -- ca n )
\G Convert unsigned double number to string.
   <# #S #> ;
: (D.) ( d -- ca n )
\G Convert double number to string.
   DUP >R DABS  <# #S R> SIGN #> ;
: (.)  ( n -- ca n )
\G Convert number to string.
   S>D (D.) ;
: (U.) ( u -- ca n )
\G Convert unsigned number to string.
   0 (UD.) ;
: (U.R) ( u m -- ca #)
\G Convert unsigned number to string in a field right-justified.
   >R (U.) R> RJUSTIFY ;
: U.R ( u m -- )
\G Display unsigned number in a field right-justified.
   (U.R) TYPE ;
: (.R) ( n m -- ca #)
\G Convert number to string in a field right-justified.
   >R (.) R> RJUSTIFY ;
: .R ( n m -- )
\G Display number in a field right-justified.
   (.R) TYPE ;
: U> ( u1 u2 -- ff )
\G True if u1 is greater than u2.
   SWAP U< ;

-WARNING
: . ( n -- )
\G Display number.
   (.) TYPE SPACE ;
+WARNING

: UD. ( ud -- )
\G Display unsigned double number.
   (UD.) TYPE SPACE ;
: D. ( d -- )
\G Display double number.
   (D.) TYPE SPACE ;
: (UD.R) ( ud m -- ca # )
\G Convert unsigned double number to string in a field right-justified.
   >R (UD.) R> RJUSTIFY ;
: UD.R ( ud m -- )
\G Display unsigned double number in a field right-justified.
   (UD.R) TYPE ;
: (D.R) ( d m -- ca n )
\G Convert double number to string in a field right-justified.
   >R (D.) R> RJUSTIFY ;
: D.R ( d m -- )
\G Display double number in a field right-justified.
   (D.R) TYPE ;

: UMAX ( u1 u2 -- u )
\G Return the maximum of two unsigned numbers.
   2dup u< IF swap THEN drop ;
: UM+ ( u1 u2 -- ud )
\G Add two unsigned number producing an unsigned double result.
   over +  dup rot  u< abs ;
: D+ ( d1 d2 -- d )
\G Add two double numbers.
   rot + >r um+ r> + ;
: D- ( d1 d2 -- d )
\G Subtract two double numbers.
   dnegate d+ ;
: D0= ( d -- f )
\G Return true if double number is zero.
   or 0= ;
: D0< ( d -- f )
\G Return true if double number is less-than zero.
   0< nip ;
: D= ( d1 d2 -- f )
\G Return true if two double numbers are equal.
   d- d0= ;
: D<> ( d1 d2 -- f )
\G Returns true if two double numbers are not equal.
   d= 0= ;
: 2ROT ( d1 d2 d3 -- d2 d3 d1 )
\G Rotate top three double elements to left.
   2>r 2swap 2r> 2swap ;
: D>S ( d -- s )
\G Convert double number to single.
   drop ;
: DMIN ( d1 d2 -- d )
\G Return the minimum of d1 and d2.
   2over 2over d< if 2drop else 2swap 2drop then ;
: DMAX ( d1 d2 -- d )
\G Return the maximum of d1 and d2.
   2over 2over d< if  2swap 2drop  else  2drop  then ;

: M+ ( d1 n -- d2 )
\G Add double and single number producing a double result.
   s>d d+ ;
: <= ( n1 n2 -- ff )
\G Return true if `n1' is less-or-equal to `n2'.
   > 0= ;
: >= ( n1 n2 -- ff )
\G Return true if `n1' is greater-or-equal to `n2'.
   < 0= ;
: \\ ( -- )
\G Skip remaining text in a file.
   BEGIN  refill 0=  UNTIL ;

\ ANSI colors
0 CONSTANT black
1 CONSTANT red
2 CONSTANT green
3 CONSTANT yellow
4 CONSTANT blue
5 CONSTANT magenta
6 CONSTANT cyan
7 CONSTANT white

WINNT? 0=
[IF]

\ ANSI escape codes
: CSI ( -- ) 27 emit [char] [ emit ;
: .CSI ( n -- ) 0 u.r ;
: CUP ( col row -- ) csi .csi [char] ; emit .csi [char] H emit ;
: SGR ( attr -- ) csi .csi [char] m emit ;
: CLS ( -- ) csi 2 .csi [char] J emit ;

: SET-FG ( color -- ) 30 +  sgr ;
: SET-BG ( color -- ) 40 +  sgr ;
: RESET-ATTRS ( -- ) 0 sgr ;
: AT-XY ( col row -- )
\G Position cursor to col/row.
   1+ SWAP 1+ SWAP cup ;
: HOME ( -- )
\G Position cursor to top left corner.
   0 0 at-xy ;
: PAGE ( -- )
\G Clear page.
   home cls ;

[ENDIF]

: /TERM ( -- )
\G Prepare terminal for raw terminal input.
   1 PREPTERM ;
: -TERM ( -- )
\G Turn off raw terminal input.
   0 PREPTERM ;

\ Array definitions
: ARRAY ( n "name" -- )
\G Define an array `name' with n cells.
\G Usage: name ( n -- addr )
\G Return the address of the nth cell.
   CREATE CELLS ALLOT
   DOES>  ( n addr1 -- addr2 ) SWAP CELLS+ ;

: CARRAY ( n "name" -- )
\G Define an array `name' with n chars.
\G Usage: name ( n -- caddr )
\G Return the address of the nth char.
   CREATE CHARS ALLOT
   DOES>  ( n addr1 -- addr2 ) SWAP CHARS+ ;

: BUFFER: ( n "name" -- )
\G Allot a buffer `name' with `n' chars in dictionary space.
   CREATE CHARS ALLOT ;



\ DUMP
: ?PRINTABLE ( c -- f )
\G Return true if char is printable.
   32 128 within ;

hide

: PRINTABLE ( c -- c' )
	dup ?printable 0= IF  drop [char] .  THEN ;

: 16*K ( n -- 16*k ) 4 rshift  4 lshift ;

: HH. ( u -- ) 0 <# # # #> type ;

: DUMP-LINE ( addr -- )
	dup  8 u.r [char] : emit \ "hhhhhhhh:"
	16 0 DO
		space
		i ?dup IF
			8 mod dup  0= IF [char] - emit space THEN
			4 = IF space THEN
		THEN
		dup i chars+ c@  hh. \ " hh"
	LOOP
	2 spaces
	16 0 DO
		dup i chars+ c@  printable emit \ "c"
	LOOP  drop cr ;

set-current

-WARNING

: DUMP ( addr u -- )
\G Dump memory `u' bytes from `addr' in hexadecimal.
	base @ >r hex
	>r ( 16*k)  r> 15 + ( 16*k)  bounds DO
		i dump-line
	16 +LOOP  r> base ! ;
	
+WARNING

previous

\ P.Falth, W.Baden
: .. ( i*x -- )
\G Display the contents of the data stack and clear the stack.
   DEPTH IF  .S  DEPTH 0 DO DROP LOOP  THEN ;

\ strings
: $VARIABLE ( len "name" -- )
\G Create string variable of length <len>.
   CREATE 0 , ALLOT ;

: $COUNT ( sa -- ca u )
\G Length of string <sa>.
   @+ ;

: $PLACE ( ca u sa -- )
\G Overwrite string <sa> with ca/u.
   2DUP !  CELL+ SWAP MOVE ;

: $APPEND ( ca u sa -- )
\G Append ca/u to string <sa>.
   DUP >R		( ca u sa -- sa)
   $COUNT CHARS +	( ca u da' -- sa)
   SWAP DUP 		( ca da' u u -- sa)
   R> +!  MOVE ;

: $PRINT ( sa -- )
\G Print string.
   $COUNT TYPE ;

: $APPEND-CHAR ( c sa -- )
\G Append char <c> to string <sa>.
   1 OVER +!  $COUNT 1- CHARS+ C! ;

: $SPLIT ( ca1 u1 c -- ca2 u2 ca3 u3 )
\G Split <ca1/u1> at char <c>.
   2>R DUP 2R>  SCAN ( ca1 ca3 u3)
   ?DUP IF ( found?)
      >R DUP >R OVER -
      R> R> 1 /STRING
   ELSE
      OVER -  2>R  0 0  2R>
   THEN ;
   

\ development support
hide

80 $VARIABLE source.fs

set-current

: SOURCE! ( ca u -- )
\G Set source file name to `ca/u'.
   source.fs $PLACE ;
: IN ( -- )
\G Include current source file.
	S" include "  PAD $PLACE
	source.fs $COUNT  PAD $APPEND
	PAD $COUNT  EVALUATE ;

: EDITED ( ca u -- )
\G Edit file named `ca/u'.
	S" system vim " PAD $PLACE  PAD $APPEND
	PAD $COUNT  EVALUATE ;

: ED ( -- ) 
\G Edit current source.
	source.fs $COUNT  edited ;

: EDIT ( "filename" -- )
\G Edit file named `filename'.
   bl word count  edited ;

: LS ( -- )
\G List current directory.
   S" system ls" EVALUATE ;
: RM ( "name" -- )
\G Remove file named `name'.
	s" system rm " PAD $PLACE
	bl word count  PAD $APPEND
	PAD $COUNT  EVALUATE ;

previous

\ vim:ts=3:sw=3:et
