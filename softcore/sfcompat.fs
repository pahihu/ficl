\ SwiftForth compatibility words
.( loading SwiftForth compatibility ) cr

( --- timers ------------------------------------------------- )

: COUNTER ( -- ms )
\G Return current milliseconds.
   GET-MSECS ;

: TIMER ( ms -- )
\G Display milliseconds elapsed since ms.
   GET-MSECS SWAP - . ;


( --- aliases ------------------------------------------------ )

HIDE

' (BRANCH) 3 CELLS + @ CONSTANT <branch>

SET-CURRENT

: BRANCH, ( addr -- )      \ assemble a branch to addr
   HERE CELL+ -  CELL /
   <branch> , , ;

: AKA ( old new -- )
\G Make alias `new' for `old'.
   ' DUP PRIMITIVE? IF        \ alias for primitive is
      >R : R>
      POSTPONE LITERAL        \ simple execution
      POSTPONE EXECUTE
   ELSE
      >BODY CELL-             \ get address of old
      >R : R>                 \ define new
      BRANCH,                 \ branch to old
   THEN
   POSTPONE ;                 \ close new def
;

PREVIOUS


( --- enums -------------------------------------------------- )

0 Value #ENUM

: ENUM  \ name ( n -- n+1 )
\G Define an enum with "name", increment number on stack.
   dup Constant 1+ ;

: ENUM4 \ name ( n -- n+4 )
\G Define an enum with "name", increment number on stack by 4.
   dup Constant 4 + ;


( --- NULL terminated strings -------------------------------- )

: ZLENGTH ( za -- u )
\G Length of NULL terminated string.
   DUP
   BEGIN COUNT WHILE REPEAT
   SWAP - 1- ;

: ZCOUNT ( za -- ca u )
\G Address and length of NULL terminated string.
   DUP ZLENGTH ;

: ZPLACE ( from n zto -- )
\G Place to NULL terminated string.
   2DUP + >R  SWAP MOVE  0 R> C! ;

: ZAPPEND ( from n zto -- )
\G Append to NULL terminated string.
   ZCOUNT + ZPLACE ;
   
