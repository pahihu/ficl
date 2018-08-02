( Wil Baden's COND ... THENS ------------------------ 2aug2018 )
.( loading COND -- N.Bawd ) CR

0 CONSTANT COND IMMEDIATE

: THENS ( CS: 0 orig_1 ... orig_N -- )
\G Close all open IFs up to COND.
   BEGIN ?DUP WHILE  POSTPONE THEN  REPEAT ; IMMEDIATE

: CASEOF ( n lit -- n )
\G Simple CASE branch.
   POSTPONE OVER POSTPONE = POSTPONE IF ; IMMEDIATE

0 [IF] =========================================================

COND
p IF
q IF
...
THENS

========================================================= [THEN]
