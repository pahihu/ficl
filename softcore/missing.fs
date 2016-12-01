\ Missing definitions from FiCL
\ 160507AP  /string fixed
.( loading UTILS ) cr

ANEW -extras

DECIMAL
\ FiCL NOTES
\ - NUMBER cannot read double numbers larger than 2^32-1
\ - only DNEGATE present from the double operators
\ - in colon definitions cannot embed double numbers
\	 workaround: [ d. ] 2literal

: XXX ( -- ) 1 abort" not implemented" ;

: BINARY	 2 BASE ! ;
: OCTAL	 8 BASE ! ;
: ALPHA  36 BASE ! ;

: .D ( n -- ) base @ >r  decimal u. r> base ! ;
: .X ( n -- ) base @ >r  hex     u. r> base ! ;
: .B ( n -- ) base @ >r  binary  u. r> base ! ;
: .BASE ( -- ) base @ .D ;


: BLANK ( a u -- ) bl fill ;
: TEXT ( delim -- ) pad 258 blank  word count pad swap  move ;

: $, ( ca n -- ) here over allot  swap move ;
: ," ( "string" -- ) [char] " word count $, ;
: /STRING ( ca1 u1 +n -- ca2 u2 ) \ remove +n chars from sc
   over IF
      2dup - 0 max >R
      min chars +  R>
   THEN  drop ;
: -TRAILING ( a u1 -- ca u2 )
	BEGIN
		dup  0<> >r 				\ u1 <> 0 ?
		2dup 1- chars + c@  bl =	\ last is space ?
		r>   and
	WHILE	1 chars -
	REPEAT ;

: !+ ( n a1 -- a2 ) tuck ! cell+ ;
: 3DUP ( a b c -- a b c a b c ) dup 2over rot ;
: BETWEEN ( n lo hi -- f ) 1+ within ;
: BOUNDS ( lo count -- hi lo ) over + swap ;

: $DUMP ( ca n -- ca n ) 2dup cr ." [" type ." ]" ;

: (u.) ( n -- ca n ) <# 0 #s #> +pad, ;
: TYPE.R ( ca n m -- ) over - 0 max spaces  type ;
: U.R ( u m -- ) >r (u.) r> type.r ;

: DABS ( d -- d ) dup 0< IF  dnegate  THEN ;
: UD. ( u -- ) <# #s #>  type space ;
: D. ( d -- ) dup >r dabs <# #s r> sign #>  type space ;
\ tuck dabs <# ... rot sign #>

: UMAX ( u1 u2 -- u ) 2dup u< IF swap THEN drop ;
: UM+ ( u1 u2 -- d ) 2dup umax >r + dup r> ( sum umax) u< abs ;
: D+ ( d1 d2 -- d ) rot + >r um+ r> + ;
: D- ( d1 d2 -- d ) dnegate d+ ;
: D0= ( d -- f ) or 0= ;
: D0< ( d -- f ) 0< nip ;

\ ANSI colors
0 CONSTANT black
1 CONSTANT red
2 CONSTANT green
3 CONSTANT yellow
4 CONSTANT blue
5 CONSTANT magenta
6 CONSTANT cyan
7 CONSTANT white

[UNDEFINED] PAGE [IF]

\ ANSI escape codes
: CSI ( -- ) 27 emit [char] [ emit ;
: .CSI ( n -- ) 0 u.r ;
: CUP ( row col -- ) swap  csi .csi [char] ; emit .csi [char] H emit ;
: SGR ( attr -- ) csi .csi [char] m emit ;
: CLS ( -- ) csi 2 .csi [char] J emit ;

: SET-FG ( color -- ) 30 +  sgr ;
: SET-BG ( color -- ) 40 +  sgr ;
: RESET-ATTRS ( -- ) 0 sgr ;
: AT-XY ( x y -- ) cup ;
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
: PRINTABLE ( c -- c' )
	dup ?printable 0= IF  drop [char] .  THEN ;

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

: 16*K ( n -- 16*k ) 4 rshift  4 lshift ;
: DUMP ( addr u -- )
	base @ >r hex
	>r 16*k  r> 15 + 16*k  bounds DO
		i dump-line
	16 +LOOP  r> base ! ;

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
80 $VARIABLE source.fs
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
