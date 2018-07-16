\ OpenFirmware struct.fth

.( loading STRUCT -- OpenFirmware ) cr

: STRUCT ( -- initial-offset )
\G Begin definition of a struct.
   0 ;

: FIELD \ name ( offset size -- offset' )
\G Define a field with offset and size. 
   over >R
   : R> POSTPONE literal POSTPONE + POSTPONE ;
   +
;

: 2FIELD \ name name ( offset size -- offset' )
\G Create two name fields with the same offset and size
   2dup field drop
   field
;
