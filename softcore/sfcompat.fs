\ SwiftForth compatibility words
.( loading SFCOMPAT -- SwiftForth ) cr

DECIMAL

( --- timers ------------------------------------------------- )

: COUNTER ( -- ms )
\G Return current milliseconds.
   GET-MSECS ;

: (TIMER) ( ms -- elapsed )
\G Return elapsed time since ms.
   GET-MSECS SWAP - ;

: TIMER ( ms -- )
\G Display milliseconds elapsed since ms.
   (TIMER) . ;


( --- aliases ------------------------------------------------ )

HIDE

' (BRANCH) 3 CELLS+ @ CONSTANT <branch>

14 CONSTANT <PRIMITIVE>
21 CONSTANT <INSTRUCTION-WORD>

: BUILT-IN? ( xt -- f )
   WORDKIND? 
   DUP  <PRIMITIVE>        =
   SWAP <INSTRUCTION-WORD> =  OR ;

: (SYNONYM) ( xt -- )
          IMMEDIATE ,
   DOES>  @  STATE @ 0=  OVER IMMEDIATE?  OR
          IF EXECUTE ELSE COMPILE, THEN
;

SET-CURRENT

: BRANCH, ( addr -- )
\G Assemble a branch to `addr'.
   HERE CELL+ -  /CELL /
   <branch> , , ;

: AKA ( "old" "new" -- )
\G Make alias `new' for `old'.
   ' CREATE (SYNONYM) ;

: SYNONYM ( "new" "old" -- )
\G Make alias `new' for `old'.
   CREATE +SMUDGE ' -SMUDGE (SYNONYM) ;

PREVIOUS


( --- enums -------------------------------------------------- )

0 Value #ENUM

: ENUM ( n "name" -- n+1 )
\G Define an enum with "name", increment number on stack.
   dup Constant 1+ ;

: ENUM4 ( n "name" -- n+4 )
\G Define an enum with "name", increment number on stack by 4.
   dup Constant 4 + ;


( --- NULL terminated strings -------------------------------- )

: ZLENGTH ( za -- u )
\G Length of NULL terminated string.
   strlen ;

: ZCOUNT ( za -- ca u )
\G Address and length of NULL terminated string.
   DUP ZLENGTH ;

: ZPLACE ( from n zto -- )
\G Place to NULL terminated string.
   2DUP + >R  SWAP MOVE  0 R> C! ;

: ZAPPEND ( from n zto -- )
\G Append to NULL terminated string.
   ZCOUNT + ZPLACE ;

: ZAPPEND-CHAR ( c zto -- )
\G Append char to NULL terminated string.
   ZCOUNT CHARS+ C!+  0 SWAP C! ;


( --- Increment/decrement ------------------------------------ )

: ++ ( addr -- )
\G Increment value at <addr>.
   1 SWAP +! ;

: -- ( addr -- )
\G Decrement value at <addr>.
   -1 SWAP +! ;


( --- WID create --------------------------------------------- )

: (WID-CREATE) ( c-addr u wid -- )
\G Create word <c-addr/u> in wordlist <wid>.
   GET-CURRENT >R
   SET-CURRENT
      SCREATE
   R> SET-CURRENT ;

: -ORDER ( wid' -- )
\G Remove wid from the search order.
   >R  GET-ORDER R> ( wid1 ... widn n wid' )
   OVER 0 ?DO
      I 2+ PICK OVER = IF
         I 2+ ROLL DROP  >R 1- R>
	 LEAVE
      THEN
   LOOP
   DROP  SET-ORDER ;

: +ORDER ( wid -- )
\G Add wid to the search order if not present.
   DUP -ORDER  >SEARCH ;


( --- FP display --------------------------------------------- )

HIDE

VARIABLE NPREC
VARIABLE 'N

SET-CURRENT

: N. ( r: r -- )
\G Display FP number as specified by FIX/SCI/ENG.
\G Default is fixed-point notation with 8 significant digits.
   PRECISION
      NPREC @ SET-PRECISION
      'N @ EXECUTE
   SET-PRECISION ;

: FIX ( n -- )
\G Select fixed-point notation as default FP display,
\G with <n> significant digits.
   NPREC ! [']  F. 'N ! ;

: SCI ( n -- )
\G Select scientific notation as default FP display,
\G with <n> significant digits.
   NPREC ! ['] FS. 'N ! ;

: ENG ( n -- )
\G Select engineering notation as default FP display,
\G with <n> significant digits.
   NPREC ! ['] FE. 'N ! ;

8 FIX

PREVIOUS

   

