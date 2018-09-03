\ Missing definitions from FiCL
\ 160507AP  /string fixed
\ 160603AP  /string fixed again, darwin?/winnt?/linux?
.( loading UTILS ) cr

ANEW -extras

decimal

Start-Prefixes
: %  2 __tempbase ; IMMEDIATE
: &  8 __tempbase ; IMMEDIATE
: # 10 __tempbase ; IMMEDIATE
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
\ FiCL NOTES
\ - only DNEGATE present from the double operators

: XXX ( -- ) 1 abort" not implemented" ;

: BINARY 2 BASE ! ;
: OCTAL	 8 BASE ! ;
: ALPHA  36 BASE ! ;

: (BASE.) ( n base -- )
   BASE @ >R  BASE !  U.  R> BASE ! ;
: H. ( n -- )  16 (BASE.) ;
: B. ( n -- )   2 (BASE.) ;
: O. ( n -- )   8 (BASE.) ;
: .BASE ( -- )  BASE @  10 (BASE.) ;


: BLANK ( a u -- ) bl fill ;
: TEXT ( delim -- ) pad 258 blank  word count pad swap  move ;

: $, ( ca n -- ) here over allot  swap move ;
: ," ( "string" -- ) [char] " word count $, ;
: /STRING ( ca1 u1 +n -- ca2 u2 ) \ remove +n chars from sc
   over
   IF 0 max  over min
      swap over - >R  +  R>
   ELSE drop
   THEN
;

: EOL? ( c -- f )
\G Answer TRUE if char is CR/LF.
   13 OVER =  SWAP 10 = OR ;

: SPACE? ( c -- f )
\G Answer TRUE if char is white-space (space or tab).
   BL OVER =  SWAP 9 = OR ;

: -TRAILING ( a u1 -- ca u2 )
\G Strip trailing whitespace.
	BEGIN
		dup  0<> >r 				   \ u1 <> 0 ?
		2dup 1- chars + c@  SPACE?	\ last is space ?
		r>   and
	WHILE	1 chars -
	REPEAT ;

: -LEADING ( ca u1 -- ca u2 )
\G Strip leading whitespace.
   BEGIN
      OVER C@ SPACE?
      OVER 0= NOT    AND
   WHILE  1 /STRING  REPEAT ;

: 3DUP ( a b c -- a b c a b c ) dup 2over rot ;
: BETWEEN ( n lo hi -- f ) 1+ within ;
: BOUNDS ( lo count -- hi lo ) over + swap ;

: $DUMP ( ca n -- ca n ) 2dup cr ." [" type ." ]" ;

: DABS ( d -- d ) dup 0< IF  dnegate  THEN ;
: (UD.) ( ud -- ca n ) <# #s #> >pad ;
: (D.) ( d -- ca n ) dup >r dabs <# #s r> sign #> >pad ;
: (.)  ( n -- ca n ) s>d (d.) ;
: (U.) ( n -- ca n ) 0 (ud.) ;
: TYPE.R ( ca n m -- ) over - 0 max spaces  type ;
: U.R ( u m -- ) >r (u.) r> type.r ;
: .R ( n m -- ) >r (.) r> type.r ;

: UD. ( u -- ) (ud.) type space ;
: D. ( d -- ) (d.) type space ;
: UD.R ( d m -- ) >r (ud.) r> type.r ;
: D.R ( d m -- ) >r (d.) r> type.r ;
\ tuck dabs <# ... rot sign #>

: UMAX ( u1 u2 -- u ) 2dup u< IF swap THEN drop ;
: UM+ ( u1 u2 -- ud ) over +  dup rot  u< abs ;
: D+ ( d1 d2 -- d ) rot + >r um+ r> + ;
: D- ( d1 d2 -- d ) dnegate d+ ;
: D0= ( d -- f ) or 0= ;
: D0< ( d -- f ) 0< nip ;

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
: AT-XY ( col row -- ) cup ;
: HOME ( -- ) 1 1 at-xy ;
: PAGE ( -- ) home cls ;

[ENDIF]

\ Array definitions
: TH ( a1 n -- a2 ) cells + ;
: cTH ( ca1 n -- ca2 ) chars + ;

: ARRAY ( n "name" -- )
	CREATE cells allot
	DOES>  ( n addr1 -- addr2 ) swap th ;

: CARRAY ( n "name" -- )
	CREATE chars allot
	DOES>  ( n addr1 -- addr2 ) swap cth ;

: BUFFER: ( n "name" -- ) CREATE chars allot ;



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
		dup i cTH c@  hh. \ " hh"
	LOOP
	2 spaces
	16 0 DO
		dup i cTH c@  printable emit \ "c"
	LOOP  drop cr ;

set-current

: DUMP ( addr u -- )
	base @ >r hex
	>r ( 16*k)  r> 15 + ( 16*k)  bounds DO
		i dump-line
	16 +LOOP  r> base ! ;

previous

\ P.Falth
: .. ( i*x -- )   .S  DEPTH 0 ?DO  DROP  LOOP ;

\ strings
: $VARIABLE ( len "name" -- ) CREATE 0 , allot ;

: /$ ( sa -- ) 0 ! ; \ clear str
: $! ( ca u sa -- ) 2dup !  cell+ swap move ; \ store str
: $@ ( sa -- ca u ) dup cell+ swap @ ; \ str to ca n
: $+! ( ca u sa -- ) \ append str
	dup >r		( ca u sa -- sa)
	$@ chars +	( ca u da' -- sa)
	swap dup 	( ca da' u u -- sa)
	r> +!  move ;
: $. ( sa -- ) $@  type ; \ print str

\ temporary string at PAD
: $PAD! ( -- ) pad $! ;
: $PAD+! ( ca u -- ) pad $+! ;
: $PAD ( -- ca u ) pad $@ ;


\ development support
hide

80 $VARIABLE source.fs

set-current

: SOURCE! ( ca u -- ) source.fs $! ;
: IN ( -- ) \ include source.fs
	s" include "  $pad!
	source.fs $@  $pad+!
	$pad evaluate ;

: EDITED ( ca u -- ) \ edit name
	s" system vim " $pad!  $pad+!
	$pad evaluate ;

: ED ( -- ) \ edit source.fs
	source.fs $@  edited ;

: EDIT ( "filename" -- ) bl word count  edited ;

: LS ( -- ) s" system ls" evaluate ;
: RM ( name -- )
	s" system rm " $pad!
	bl word count  $pad+!
	$pad evaluate ;

previous
