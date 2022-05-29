.( loading TOFCOMPAT -- B.Eckert ) CR

: R>DROP ( -- )
\G Drop item from return stack.
   S" R> DROP" EVALUATE ; DIRECTIVE

: 0>= ( n -- f )
\G True if n >= 0.
   0< 0= ;

: 0<= ( n -- f )
\G True if n <= 0.
   0> 0= ;

: UNDER+ ( a b c -- a+c b )
\G Add TOS to under NOS.
   ROT + SWAP ;

: LAY ( ca1 b -- ca2 )
\G Lay down next char in string.
   OVER C! CHAR+ ;

: INCR ( addr -- )
\G Increment value at <addr>.
   1 SWAP +! ;

: DECR ( addr -- )
\G Decrement value at <addr>.
   -1 SWAP +! ;

: ?EXIT ( f -- )
\G Exit if <f> is true.
   S" IF EXIT THEN" EVALUATE ; DIRECTIVE

: D0<> ( d -- f )
\G True if <d> <> 0.
   D0= NOT ;

: ON ( addr -- )
\G Set flag at <addr>.
   TRUE SWAP ! ;

: OFF ( addr -- )
\G Clear flag at <addr>.
   FALSE SWAP ! ;

: ON? ( addr -- f )
\G Flag at <addr> set ?
   @ 0= NOT ;

: OFF? ( addr -- f )
\G Flag at <addr> clear ?
   @ 0= ;

: UMIN ( u1 u2 -- u )
\G Minimum of u1 and u2.
   2DUP U< IF DROP ELSE NIP THEN ;

: 0MAX ( x -- u )
\G Limit top to x.
   0 MAX ;

: EMITS ( n char -- )
\G Emit <n> <char>s.
   SWAP 0 ?DO DUP EMIT LOOP  DROP ;

: TYPE. ( ca u -- )
\G Type string and a space.
   TYPE SPACE ;

: MULTI ( -- )
\G Execute loop until TOR is not negative.
\G Example: n >R MULTI ... REPEAT R>DROP 
   S" BEGIN R@ 0< 0= WHILE R> 1- >R" EVALUATE ; DIRECTIVE

: C>N ( c -- x)
\G Sign extend char to cell.
   DUP 128 AND IF  256 -  THEN ;

: W>N ( w -- x )
\G Sign extend 16bit word to cell.
   DUP 32768 AND IF 65536 - THEN ;

: CLEAR ( i*x -- )
\G Clear parameter stack.
   DEPTH 0 ?DO  DROP  LOOP ;

: UNDER1+ ( x1 x2 -- x1' x2 )
\G Add 1 to NOS.
   >R 1+ R> ;

\ ---

: UNDER1- ( x1 x2 -- x1' x2 )
\G Subtract 1 from NOS.
   >R 1- R> ;

: BSPLIT ( w -- bl bh )
\G Split 16bit word to 8bit bytes.
   DUP   255 AND  SWAP  8 RSHIFT    255 AND ;

: WSPLIT ( n -- wl wh )
\G Split 32bit word to 16bit words.
   DUP 65535 AND  SWAP 16 RSHIFT  65535 AND ;

\\
TRUE 32 RSHIFT CONSTANT BITMASK32

: QSPLIT ( n -- ql qh )
\G Split 64bit word to 32bit words.
   DUP BITMASK32 AND  SWAP 32 RSHIFT  BITMASK32 AND ;

: BJOIN ( cl ch -- n )
\G Join lo and hi 8bit bytes.
   8 LSHIFT + ;

: WJOIN ( wl wh -- n )
\G Join lo and hi 16bit words.
   16 LSHIFT + ;

: QJOIN ( ql qh -- n )
\G Join lo and hi 32bit words.
   32 LSHIFT + ;

: RETRY ( -- )
\G Jump back to the beginning of a word. (W.Baden)
   LAST-WORD >BODY CELL-  BRANCH, ; DIRECTIVE

