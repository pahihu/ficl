( CCC3 PI demo --- ap 27jun2018 )

0 [IF]
================================================================
180627 AP	initial revision
180717 AP	JOIN renamed to WAIT

================================================================
[THEN]

anew -pi

include ficltask.f

: ~pi ( -- num denom )
   \ M.Williams for 64bit Forths 3jul18 
   \ assumes scaling operators using 128bit intermediate products
   $24baf15fe1658f99 $bb10cb777fb8137 ;

fvariable pi
mutex pi-lock

: pi! ( r -- )
   pi-lock get
      pi f+!
   pi-lock release ;

100000 value intervals
    10 constant thcount
fvariable width

#user
   cell +user IPROC
to #user

create tasks  thcount cells allot

: )task ( x -- addr )
   cells tasks + @ ;

: reset ( -- )
   0e pi f!
   1e intervals s>f f/  width f! ;

: init ( -- )
   thcount 0 do
      /task safe-allocate  i cells tasks +  !
      i )task construct
   loop  reset ;

: process ( -- )
   cr ." thread: " IPROC ?
   0e
   intervals IPROC @ DO
      i s>f 0.5e f- width f@ f*
      fdup f* 1e f+
      4e fswap f/
      f+
   thcount +LOOP
   width f@ f*

   pi!

   halt
;

: main ( -- )
   ." starting tasks..."
   thcount 0 DO
         i  i )task IPROC   his !
      ['] process i )task activate
   LOOP
   
   cr ." waiting for tasks..."
   thcount 0 DO
      i )task wait
   LOOP

   cr ." estimation of pi is: " pi f@ f.
;

init

