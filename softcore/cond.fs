( Wil Baden's COND ... THENS ------------------------ 2aug2018 )
.( loading COND -- N.Bawd ) CR

0 CONSTANT COND IMMEDIATE

: THENS ( CS: 0 orig_1 ... orig_N -- )
\G Close all open IFs up to COND.
   BEGIN ?DUP WHILE  POSTPONE THEN  REPEAT ; IMMEDIATE

: ANDIF ( f -- )
\G Shortcut logical AND.
   S" DUP IF DROP" EVALUATE ; IMMEDIATE

: ORELSE ( f -- )
\G Shortcut logical OR.
   S" DUP 0= IF DROP" EVALUATE ; IMMEDIATE

: CASEOF ( n lit -- n )
\G Simple CASE branch.
   S" OVER = IF" EVALUATE ; IMMEDIATE

0 [IF] =========================================================

COND
p IF
q IF
...
THENS

COND
   p ANDIF / ORELSE
   q ANDIF / ORELSE
   r
THENS
IF
ELSE
THEN

========================================================= [THEN]
