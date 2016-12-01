\ Monitor for HolonX                                   ap 160519
\ Based on W.Wejgaard's Tcl Monitor

Hide

  Variable lastModTime
  
  : holon.mon ( -- ca u )   S" holon.mon" ;
  
  : modTime ( -- mtime )   holon.mon FILE-MTIME ;
  : changed? ( mtime -- f )   lastModTime @  - ;
    
  : doEval ( -- )   holon.mon INCLUDED ;
  : eval ( -- )
     ['] doEval CATCH
     ?dup IF   ." Error: " .  THEN ;
     
  : doHMon ( -- )
     BEGIN
       modTime dup changed? IF
         lastModTime !  eval
       THEN
       1000 ms
     AGAIN
  ;

Set-Current

  : HMON ( -- )   modTime lastModTime !  doHMon ;

Previous
