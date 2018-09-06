( ForthMacs LOSE --- 6sep18ap )

: LOSE ( -- )
\G Abort execution.
   ." unDefined " ABORT ;

: DO-UNDEFINED ( caddr u -- i*x f )
\G Report unknown word, compile LOSE.
   ." unKnown " TYPE CR
   STATE @
   IF    ['] LOSE COMPILE, TRUE
   ELSE  FALSE
   THEN ;

' DO-UNDEFINED ADD-PARSE-STEP