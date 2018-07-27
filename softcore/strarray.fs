\ String-arrays
.( loading STRINGS ) cr

\ STRING-ARRAY <name>
\  $" str1"
\  $" str2"
\  ...
\ END-STRING-ARRAY

: STRING-ARRAY ( "name" -- )
\G Define a string array by `name'.
\G At run-time `name' ( n -- ca u ) returns the nth entry.
   CREATE here 0 dup ,
   DOES>  ( n addr -- ca u )
          @  swap [ 1 2* cells ] literal *  +  2@ ;

: $" ( aptr u1 "string" -- aptr ca u u2 )
\G Add an entry to the string array.
   1+  [char] " word count
   here over allot ( ca u a)
   swap 2dup 2>R  move
   2R> rot ;

: END-STRING-ARRAY ( aptr ca1 u1 ... caN uN # -- )
\G End the definition of string array.
   here >R  dup 2* cells allot
   here [ 2 cells ] literal -  swap 0 ?DO
      i 2* cells negate  over + ( ca u a a')
      swap >R  !+ !  R>
   LOOP  drop  R> swap ! ;

: $empty ( ca u -- f )
\G Return true if ca/u has zero len.
   nip 0= ;

: $= ( ca1 u1 ca2 u2 -- f )
\G Return true if ca1/u1 = ca2/u2.
   compare 0= ;

: $~= ( ca1 u1 ca2 u2 -- f )
\G Return true if ca1/u1 = ca2/u2 case insensitive.
   compare-insensitive 0= ;        

: $< ( ca1 u1 ca2 u2 -- f )
\G Return true if ca1/u1 < ca2/u2.
   compare 0< ;

: $> ( ca1 u1 ca2 u2 -- f )
\G Return true if ca1/u1 > ca2/u2.
   compare 0> ;

: $len ( ca u -- u )
\G Return the len of ca/u.
   nip ;

: $left ( ca u1 +n -- ca u2 )
\G Return the left str of ca/u1.
   min ;

: $right ( ca1 u +n -- ca2 n )
\G Return the right str of ca1/u.
   over min    \ restrict new length
   dup >R -    \ save new len, calc offset from str start
   0 max       \ no negative offset
   chars +     \ reposition str start
   R> ;        \ restore new len

: $mid ( ca1 u offs len -- ca2 u2 ) 
\G Return the mid str of ca1/u from offs, running len. 
   >r             \ save len
   over - negate  \ calc new len for $right
   0 max          \ restrict new len
   $right         \ take right
   r> $left ;     \ restore len, take left

: $prefix ( ca1 u1 ca2 u2 -- f )
\G Return true if ca1/u1 has prefix ca2/u2.
   tuck           \ put u2 under ca2
   2>r            \ save prefix str
   $left          \ take left of ca1/u1
   2r>            \ restore prefix str
   $= ;           \ compare it

: $postfix ( ca1 u1 ca2 u2 -- f )
\G Return true if ca1/u1 has postfix ca2/u2.
   tuck           \ put u2 under ca2
   2>r            \ save postfix str
   $right         \ take right of ca1/u1
   2r>            \ restore prefix str
   $= ;           \ compare it
   
: ucase ( c1 -- c2 ) 
\G Convert c1 to upper case.
   dup [char] a [char] z between if
      [ char a char A - ] literal -
   then ;

: lcase ( c1 -- c2 )
\G Convert c1 to lower case.
   dup [char] A [char] Z between if
      [ char a  char A - ] literal +
   then ;

: $ucase ( ca1 u1 -- ca2 u2 )
\G Convert ca1/u1 to upper case. Returned string is at PAD.
   +PAD drop
   dup >R  0 ?DO
      i chars over + c@ ucase
      i chars PAD  + c!
   LOOP  drop PAD R> ;

: $lcase ( ca1 u1 -- ca2 u2 )
\G Convert ca1/u1 to lowe case. Returned string is at PAD.
   +PAD drop
   dup >R  0 ?DO
      i chars over + c@ lcase
      i chars PAD  + c!
   LOOP  drop PAD R> ;

: $last ( ca +u -- c )
\G Return the last char of ca/u.
   1- chars +  c@ ;

: zero ( x -- 0 )
\G Replace x with zero.
   dup xor ;

: search ( ca1 u1 ca2 u2 -- ca3 u3 f )
   2>R 2dup                \ save search str on R stack, save original
   BEGIN
      2dup 2R@ $prefix not \ not prefix of original
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

: $prefix-nc ( ca1 u1 ca2 u2 -- f )
   tuck           \ put u2 under ca2
   2>r            \ save prefix str
   $left          \ take left of ca1/u1
   2r>            \ restore prefix str
   $~= ;          \ compare it

: search-nc ( ca1 u1 ca2 u2 -- ca3 u3 f )
   2>R 2dup                \ save search str on R stack, save original
   BEGIN
      2dup 2R@ $prefix-nc not \ not prefix of original
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

: $.R ( ca u n -- )
\G Display ca/u in a field of n chars width.
   over - >R  type  R> spaces ;

