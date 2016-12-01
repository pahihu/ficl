\ C.Jakeman    Forthwrite Jan02 Issue 115
\ MINI-OOF.FTH from Bernd Paysan

: Method ( m v -- m' v ) Create  over , swap cell+ swap
   DOES> ( ... o -- ... ) @ over @ + @ execute ;
: Var ( m v size -- m v' ) Create  over , +
   DOES> ( o -- addr ) @ + ;

create object  1 cells , 2 cells ,

: Class ( class -- class selectors vars ) dup 2@ ;
: UndefinedMethod
         true abort" undefined class method called"
;
: EndClass ( class methods vars -- )
   Create  here >r , dup , 2 cells ?DO
      ['] UndefinedMethod ,
   1 cells +LOOP
   cell+ dup cell+ r> rot @ 2 cells /string move ;
: Defines ( xt class "name" -- ) ' >body @ + ! ;
: New ( class -- o )  here over @ allot swap over ! ;
: :: ( class "name" -- ) ' >body @ + @ compile, ;


\ SAFER-OOF.FTH for debugging to check that method is appropriate for class.

: Class ( &Class -- &Class Key*m MethodOffset >Vars< )
   dup 2@ >r >r                  \ Save size of vars and methods
   r@ 2 cells /  1 ?DO  0  LOOP  \ Leave a dummy key value 0 for each method
   r> r>                         \ inherited from the parent class.
;
: CheckMethod ( key1 key2 -- )
   <> abort" Method not appropriate for class"
;
: Method ( MethodOffset >Vars< -- Key NewMethodOffset >Vars< )
   Create >r here                \ Key = HERE
      swap                       \ bury key under MethodOffset
      dup , over ,               \ compile Offset then key
      cell+ cell+ r>             \ adjust Offset for next method
   DOES> ( ... o -- ... ) 2@     \ -- object key offset
      2 pick @                   \ -- object key offset class
      +                          \ -- object key methodPointer
      2@ >r CheckMethod r> execute
;
: EndClass ( &Class Key*m MethodOffset >Vars< -- )
   Create  here >r , dup ,
   2 cells /  2 -
   0 swap  ?DO                \ Loop to compile keys, oldest key first
      ['] UndefinedMethod ,   \ Compile default method function
      i roll ,                \ Add method key (1 roll = swap, 0 roll = no-op)
   -1 +LOOP
                              \ Overwirte with contents of parent table
   cell+ dup cell+ r> rot @ 2 cells /string move
;
