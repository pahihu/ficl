\ OpenFirmware struct.fth

.( loading STRUCT -- OpenFirmware ) cr

: STRUCT ( -- initial-offset )
\G Begin definition of a struct.
   0 ;

: FIELD ( offset size "name" -- offset' )
\G Define a field with offset and size. 
   over >R
   : R> POSTPONE literal POSTPONE + POSTPONE ;
   +
;

: 2FIELD ( offset size "name1" "name2" -- offset' )
\G Create two name fields with the same offset and size
   2dup field drop
   field
;

.( loading BEGIN-STRUCTURE -- Forth2012 ) cr

: BEGIN-STRUCTURE ( <name> -- addr offs )
\G Begin the definition of the structure <name>.
   create here 0 0 ,
    does> @ ;

: END-STRUCTURE ( addr offs -- )
\G End the definition of a structure.
   swap ! ;

: +FIELD ( addr offs size <name> -- addr offs2 )
\G Add the field definition <name> with <size>.
   create over , +
    does> @ + ;

: FIELD: ( addr offs <name> -- addr offs2 )
\G Add aligned cell field.
   aligned /cell +field ;

: CFIELD: ( addr offs <name> -- addr offs2 )
\G Add aligned character field.
   [ 1 chars ] literal +field ;

: FFIELD: ( addr offs <name> -- addr offs2 )
\G Add aligned float field.
   faligned [ 1 floats ] literal +field ;

: SFFIELD: ( addr offs <name> -- addr offs2 )
\G Add aligned single float field.
   sfaligned [ 1 sfloats ] literal +field ;

: DFFIELD: ( addr offs <name> -- addr offs2 )
\G Add aligned double float field.
   dfaligned [ 1 dfloats ] literal +field ;

: 2ALIGNED ( addr1 -- addr2 )
\G Align addr1 to double cell boundary.
   >cells  1+ 2/  cells 2* ;

: 2FIELD: ( addr offs <name> -- addr offs2 )
\G Add aligned double cell field.
   2aligned [ 2 cells ] literal +field ;

