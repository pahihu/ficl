0 value DebugStr

HIDE

: .str ( ca # -- ca # )
   [char] [ emit 2dup type [char] ] emit
   over h. [char] @ emit dup . ;

SET-CURRENT

: $debug ( ca # -- ca # )
\G If DebugStr is true, show string ca/#.
   DebugStr IF
      cr .str
   THEN ;

: $debugmsg ( ca # msg # -- ca # )
\G If DebugStr is true, show string ca/#.
   DebugStr IF
      cr type space .str
   ELSE  2drop
   THEN ;

PREVIOUS
