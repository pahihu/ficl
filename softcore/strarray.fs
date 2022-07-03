( String arrays --- 15sep18ap )

\ ANEW -strarray

\ STRING-ARRAY
\   $" item0"
\   $" item1"
\   $" item2"
\ END-STRING-ARRAY ITEMS
\
\ Usage: ITEMS ( # -- caddr u )


: STRING-ARRAY ( -- )   0 ;
\G Begin definition of a literal string array.

: $" ( i*caI n "string" -- [i+1]*caI [n+1] )
\G Define a string item.
   [CHAR] " WORD COUNT  HERE >R  DUP C, STRING,
   R> SWAP 1+ ;

: END-STRING-ARRAY ( i*caddrI n "name" -- )
\G Close the definition of string array `name'.
\G Executing n `name' will return the nth string in the array
\G as caddr/u.
   CREATE DUP 1- , 0 ?DO , LOOP
   DOES>  ( # addr -- caddr u )
     @+ ( # addr2 n) ROT - CELLS+ @ COUNT ;

