\ SwiftForth COUNTER/TIMER
.( loading COUNTER/TIMER -- SwiftForth ) cr

: COUNTER ( -- ms )
\G Return current milliseconds.
   GET-MSECS ;

: TIMER ( ms -- )
\G Display milliseconds elapsed since ms.
   GET-MSECS SWAP - . ;

