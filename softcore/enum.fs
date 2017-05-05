\ SwiftForth ENUMs

.( loading ENUM -- SwiftForth ) cr

0 Value #ENUM

: ENUM  \ name ( n -- n+1 )
   dup Constant 1+ ;

: ENUM4 \ name ( n -- n+4 )
   dup Constant 4 + ;
