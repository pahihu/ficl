.( loading OFCOMPAT ) cr

: pack ( from n to -- to )
\G Like place but returns <to>.
   dup >r  place  r> ;

: $save ( from n to -- to+1 n )
\G Place <from/n> to to, return as counted string.
   dup >r  place  r> count ;

: q>n ( q -- n )
\G Sign extend 32bit value.
   dup [ 31 bit ] literal and if  [ 32 bit bmask ] literal -  then ;

: <w@ ( w-addr -- n )
\G Fetch 16bit value, sign extend.
   w@ w>n ;

: w, ( w -- )
   here w!  2 allot ;

: q, ( q -- )
   here q!  4 allot ;

: words+ ( addr1 n -- addr2 )
   2* chars+ ;

: quads+ ( addr1 n -- addr2 )
   4 * chars+ ;

\ vim:ts=3:sw=3:et
