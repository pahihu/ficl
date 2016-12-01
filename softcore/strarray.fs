\ String-arrays
.( loading STRINGS ) cr

: STRING-ARRAY ( "name" -- )
	CREATE here 0 dup ,
	DOES>  ( n addr -- ca u )
		@  swap 2* cells +  2@ ;

: $" ( aptr u1 "string" -- aptr ca u u2 )
	1+  [char] " word count
	here over allot ( ca u a)
	swap 2dup 2>R  move
	2R> rot ;

: END-STRING-ARRAY ( aptr ca1 u1 ... caN uN # -- )
	here >R  dup 2* cells allot
	here [ 2 cells ] literal -  swap 0 ?DO
		i 2* cells negate  over + ( ca u a a')
		swap >R  !+ !  R>
	LOOP  drop  R> swap ! ;


\ Return true if ca/u has zero len.
: empty$ ( ca u -- f )
   nip 0= ;

\ Return true if ca1/u1 = ca2/u2.
: $= ( ca1 u1 ca2 u2 -- f )
   compare 0= ;

\ Return true if ca1/u1 < ca2/u2.
: $< ( ca1 u1 ca2 u2 -- f )
   compare 0< ;

\ Return true if ca1/u1 > ca2/u2.
: $> ( ca1 u1 ca2 u2 -- f )
   compare 0> ;

\ Return the len of ca/u.
: len$ ( ca u -- u )
   nip ;

\ Return the left str of ca/u1.
: left$ ( ca u1 +n -- ca u2 )
   min ;

\ Return the right str of ca1/u.
: right$ ( ca1 u +n -- ca2 n )
   over min    \ restrict new length
   dup >R -    \ save new len, calc offset from str start
   0 max       \ no negative offset
   chars +     \ reposition str start
   R> ;        \ restore new len

\ Return the mid str of ca1/u from offs, running len. 
: mid$ ( ca1 u offs len -- ca2 u2 ) 
   >r             \ save len
   over - negate  \ calc new len for righ$
   0 max          \ restrict new len
   right$         \ take right
   r> left$ ;     \ restore len, take left

\ Return true if ca1/u1 has prefix ca2/u2.
: prefix$ ( ca1 u1 ca2 u2 -- f )
   tuck           \ put u2 under ca2
   2>r            \ save prefix str
   left$          \ take left of ca1/u1
   2r>            \ restore prefix str
   $= ;           \ compare it

\ Return true if ca1/u1 has postfix ca2/u2.
: postfix$ ( ca1 u1 ca2 u2 -- f )
   tuck           \ put u2 under ca2
   2>r            \ save postfix str
   right$         \ take right of ca1/u1
   2r>            \ restore prefix str
   $= ;           \ compare it
   
\ Convert c1 to upper case.
: ucase ( c1 -- c2 ) 
   dup [char] a [char] z between if
      [ char a char A - ] literal -
   then ;

\ Convert c1 to lower case.
: lcase ( c1 -- c2 )
   dup [char] A [char] Z between if
      [ char a  char A - ] literal +
   then ;

\ Convert ca1/u1 to upper case.
\ Returned string is at PAD.
: ucase$ ( ca1 u1 -- ca2 u2 )
   +PAD drop
   dup >R  0 ?DO
      i chars over + c@ ucase
      i chars PAD  + c!
   LOOP  drop PAD R> ;

\ Convert ca1/u1 to lowe case.
\ Returned string is at PAD.
: lcase$ ( ca1 u1 -- ca2 u2 )
   +PAD drop
   dup >R  0 ?DO
      i chars over + c@ lcase
      i chars PAD  + c!
   LOOP  drop PAD R> ;

\ Return the last char of ca/u.
: last$ ( ca +u -- c )
   1- chars +  c@ ;

\ Replace x with zero.
: zero ( x -- 0 )
   dup xor ;

: search ( ca1 u1 ca2 u2 -- ca3 u3 f )
   2>R 2dup                \ save search str on R stack, save original
   BEGIN
      2dup 2R@ prefix$ not \ not prefix of original
      over 0<> and         \ and original not empty
   WHILE
      1 /string            \ drop 1 char
   REPEAT
   2R> 2drop               \ drop search str
   \    match: ca1 u1 ca2 u2
   \ no match: ca1 u1 ca2 0
   ?dup IF    2>R 2drop 2R> true
        ELSE  zero
        THEN ;

