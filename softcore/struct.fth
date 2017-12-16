\ OpenFirmware struct.fth

.( loading STRUCT -- OpenFirmware ) cr

: struct ( -- initial-offset )   0 ;

: field           \ name ( offset site -- offset' )
   over >R
   : R> POSTPONE literal POSTPONE + POSTPONE ;
   +
;

\ Create two name fields with the same offset and size
: 2field          \ name name ( offset size -- offset' )
   2dup field drop
   field
;
