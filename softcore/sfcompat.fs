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
   ' >BODY CELL-           \ get address of old
   >R : R>                 \ define new
   BRANCH,                 \ branch to old
   POSTPONE ;              \ close new def
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

