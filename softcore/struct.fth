\ OpenFirmware struct.fth

.( loading STRUCT -- OpenFirmware ) cr

: struct ( -- initial-offset )   0 ;

: field           \ name ( offset size -- offset' )
   create  immediate  over , +
   does>   ( base -- addr )
           state @ IF  @ POSTPONE literal POSTPONE +  EXIT  THEN
           @ +
;

\ Create two name fields with the same offset and size
: 2field          \ name name ( offset size -- offset' )
   2dup field drop
   field
;
