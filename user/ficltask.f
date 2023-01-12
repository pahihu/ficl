( FiCL tasking --- ap 29jun2018 )

anew -ficltask

#user
   cell +user active
to #user

-warning

: activate ( xt tid)
   dup ACTIVE his  TRUE swap atomic!
   activate ;

: halt ( -- )
   FALSE ACTIVE atomic!  halt ;

+warning

: active? ( tid - t)
   ACTIVE his atomic@ ;

: wait ( tid)
   BEGIN dup active?
   WHILE pause REPEAT  drop ;

: ?! ( addr prev new -- bool )
   over >r  rot atomic-cas  r> = 0= ;

