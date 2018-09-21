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

: CASE? ( n lit -- true | n false )
\G Alternative CASE branch.
   OVER = DUP IF  NIP  THEN ;

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

: [PUT_CHAR] ( char -- 0..3 )
         BL CASE? IF  1  EXIT THEN
   [CHAR] { CASE? IF  2  EXIT THEN
   [CHAR] } CASE? IF  3  EXIT THEN
   DROP 0 ;

: [PUT_CHAR] ( char -- 0..3 )
         BL CASE? IF  1  ELSE
   [CHAR] { CASE? IF  2  ELSE
   [CHAR] } CASE? IF  3  ELSE
   DROP 0  THEN THEN THEN ;

: [PUT_CHAR] ( char -- 0..3 )
   COND
            BL CASE? IF  1  ELSE
      [CHAR] { CASE? IF  2  ELSE
      [CHAR] } CASE? IF  3  ELSE
      DROP 0
   THENS ;

========================================================= [THEN]
