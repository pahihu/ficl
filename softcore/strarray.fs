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

: $" ( i*caI n "string" -- [i+1]*caI [n+1] )
   '" WORD COUNT  HERE >R  DUP C, $,
   R> SWAP 1+ ;

: END-STRING-ARRAY ( i*caddrI n "name" -- )
   CREATE DUP 1- , 0 ?DO , LOOP
   DOES>  ( # addr -- caddr u )
     @+ ( # addr2 n) ROT - CELLS + @ COUNT ;

