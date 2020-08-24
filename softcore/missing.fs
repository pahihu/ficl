\ Missing definitions from FiCL
\ 160507AP  /string fixed
\ 160603AP  /string fixed again, darwin?/winnt?/linux?
.( loading UTILS ) cr

\ ANEW -extras

decimal

Start-Prefixes
: %  2 __tempbase ; IMMEDIATE
: &  8 __tempbase ; IMMEDIATE

-WARNING
: # 10 __tempbase ; IMMEDIATE
+WARNING

: $ 16 __tempbase ; IMMEDIATE
End-Prefixes

S" FICL-OS" ENVIRONMENT? [IF]
CONSTANT ficl-os
[ELSE]
0 CONSTANT ficl-os
[THEN]


: DARWIN?   \ -- t|f ; true if Darwin
   ficl-os 1 =
;

: LINUX?    \ -- t|f ; true if Linux
   ficl-os 2 =
;

: WINNT?    \ -- t|f ; true if Windows
   ficl-os 3 =
;

DECIMAL

: DIRECTIVE ( -- )   IMMEDIATE COMPILE-ONLY ;

: XXX ( -- ) 1 abort" not implemented" ;

: BINARY 2 BASE ! ;
: OCTAL	 8 BASE ! ;
: ALPHA  36 BASE ! ;

: DOT ( n base -- )
   BASE @ >R  BASE !  U.  R> BASE ! ;
: .X ( n -- )  16 DOT ;
: H. ( n -- )  .X ;
: .B ( n -- )   2 DOT ;
: .O ( n -- )   8 DOT ;
: .D ( n -- )  10 DOT ;
: S. ( n -- )  DUP 0< IF [CHAR] - EMIT THEN  ABS . ;
: .BASE ( -- )  BASE @  .D ;

: BLANK ( a u -- ) bl fill ;
: TEXT ( delim -- ) pad 258 blank  word count pad swap  move ;

: STRING, ( ca n -- )   here over chars allot  swap move ;
: ," ( "string" -- )   [char] " word count string, ;
: /STRING ( ca1 u1 +n -- ca2 u2 ) \ remove +n chars from sc
   over
   IF 0 max  over min
      swap over - >R  +  R>
   ELSE drop
   THEN
;

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

: 3DUP ( a b c -- a b c a b c ) dup 2over rot ;
: BETWEEN ( n lo hi -- f ) 1+ within ;
: BOUNDS ( lo count -- hi lo ) over + swap ;

: $DUMP ( ca n -- ca n ) 2dup cr ." [" type ." ]" ;

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

: DABS ( d -- d )   DUP 0< if  DNEGATE  THEN ;
: (UD.) ( ud -- ca n )   <# #S #> ;
: (D.) ( d -- ca n )   DUP >R DABS  <# #S R> SIGN #> ;
: (.)  ( n -- ca n )   S>D (D.) ;
: (U.) ( n -- ca n )   0 (UD.) ;
: (U.R) ( u m -- ca #)   >R (U.) R> RJUSTIFY ;
: U.R ( u m -- )   (U.R) TYPE ;
: (.R) ( n m -- ca #)   >R (.) R> RJUSTIFY ;
: .R ( n m -- )   (.R) TYPE ;

-WARNING
: . ( n -- )   (.) TYPE SPACE ;
+WARNING

: UD. ( u -- )   (UD.) TYPE SPACE ;
: D. ( d -- )   (D.) TYPE SPACE ;
: (UD.R) ( d m -- ca # )   >R (UD.) R> RJUSTIFY ;
: UD.R ( ud m -- )   (UD.R) TYPE ;
: (D.R) ( d m -- ca n )   >R (D.) R> RJUSTIFY ;
: D.R ( d m -- )   (D.R) TYPE ;

: UMAX ( u1 u2 -- u ) 2dup u< IF swap THEN drop ;
: UM+ ( u1 u2 -- ud ) over +  dup rot  u< abs ;
: D+ ( d1 d2 -- d ) rot + >r um+ r> + ;
: D- ( d1 d2 -- d ) dnegate d+ ;
: D0= ( d -- f ) or 0= ;
: D0< ( d -- f ) 0< nip ;
: D= ( d1 d2 -- f ) d- d0= ;
: D<> ( d1 d2 -- f ) d= 0= ;

: M+ ( d1 n -- d2 ) s>d d+ ;
: <= ( n1 n2 -- ff ) > 0= ;
: >= ( n1 n2 -- ff ) < 0= ;
: \\ ( -- ) BEGIN  refill 0=  UNTIL ;

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
: AT-XY ( col row -- ) 1+ SWAP 1+ SWAP cup ;
: HOME ( -- ) 0 0 at-xy ;
: PAGE ( -- ) home cls ;

[ENDIF]

: /TERM ( -- ) 1 PREPTERM ;
: -TERM ( -- ) 0 PREPTERM ;

\ Array definitions
: ARRAY ( n "name" -- )
   CREATE CELLS ALLOT
   DOES>  ( n addr1 -- addr2 ) SWAP CELLS+ ;

: CARRAY ( n "name" -- )
   CREATE CHARS ALLOT
   DOES>  ( n addr1 -- addr2 ) SWAP CHARS+ ;

: BUFFER: ( n "name" -- )   CREATE CHARS ALLOT ;



\ DUMP
: ?PRINTABLE ( c -- f ) 32 128 within ;

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
	base @ >r hex
	>r ( 16*k)  r> 15 + ( 16*k)  bounds DO
		i dump-line
	16 +LOOP  r> base ! ;
	
+WARNING

previous

\ P.Falth, W.Baden
: .. ( i*x -- )   DEPTH IF  .S  DEPTH 0 DO DROP LOOP  THEN ;

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

: SOURCE! ( ca u -- ) source.fs $PLACE ;
: IN ( -- ) \ include source.fs
	S" include "  PAD $PLACE
	source.fs $COUNT  PAD $APPEND
	PAD $COUNT  EVALUATE ;

: EDITED ( ca u -- ) \ edit name
	S" system vim " PAD $PLACE  PAD $APPEND
	PAD $COUNT  EVALUATE ;

: ED ( -- ) \ edit source.fs
	source.fs $COUNT  edited ;

: EDIT ( "filename" -- ) bl word count  edited ;

: LS ( -- ) S" system ls" EVALUATE ;
: RM ( name -- )
	s" system rm " PAD $PLACE
	bl word count  PAD $APPEND
	PAD $COUNT  EVALUATE ;

previous

\ vim:ts=3:sw=3:et
