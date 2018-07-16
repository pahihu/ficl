\ SwiftForth ENUMs

.( loading ENUM -- SwiftForth ) cr

0 Value #ENUM

: ENUM  \ name ( n -- n+1 )
\G Define an enum with "name", increment number on stack.
   dup Constant 1+ ;

: ENUM4 \ name ( n -- n+4 )
\G Define an enum with "name", increment number on stack by 4.
   dup Constant 4 + ;
