.( loading TOOLBELT 2000 -- N.Bawd ) CR

FALSE CONSTANT [VOID] IMMEDIATE

( Stack Handling ============================================= )

: THIRD ( x y z -- x y z x )
\G Copy 3rd element to top of stack.
   2 PICK ;

: FOURTH ( w x y z -- w x y z w )
\G Copy 4th element to top of stack.
   3 PICK ;

: 3DROP ( x y z -- )
\G Drop top three elements from stack.
   DROP 2DROP ;

: 2NIP ( w x y z -- y z )
\G Drop 3rd and 4th element from stack.
   2>R 2DROP 2R> ;

: R'@ ( -- x1 ) ( R: x1 x2 -- x1 x2 )
\G 2nd element of return stack.
   S" 2R@ DROP" EVALUATE ; IMMEDIATE COMPILE-ONLY


( String Handling ============================================ )

255 CONSTANT /COUNTED-STRING

: CHOP ( a m b n -- b n a m-n )
\G Split string <a/m> at place given by <b/n>.
   DUP >R   2SWAP  R> - ;

: SPLIT ( str n c -- str+i n-i str i )
\G Divide a string at a given character.
   >R 2DUP R> SCAN  CHOP ;

: IS-WHITE ( char -- flag )
\G Test char for white space.
   33 - 0< ;

: TRIM ( str len -- str len-i )
\G Trim white space from end of string.
   BEGIN  DUP  WHILE
      1-  2DUP CHARS+ C@ IS-WHITE NOT
   UNTIL 1+ THEN ;

: END-WORD ( str n -- str+/str-i i )
\G Last word in a string after trimming.
   TRIM  2DUP BL BACK NIP /STRING ;

: BL-SCAN ( str len -- str+i len-i )
\G Look for white space from start.
   BEGIN  DUP  WHILE  OVER C@ IS-WHITE NOT
   WHILE  1 /STRING  REPEAT THEN ;

: BL-SKIP ( str len -- str+i len-i )
\G Skip over white space at start.
   BEGIN  DUP WHILE  OVER C@ IS-WHITE
   WHILE  1 /STRING  REPEAT THEN ;

: STARTS? ( str len pat len2 -- str len flag )
\G Check start of string.
   DUP >R  2OVER  R> MIN  COMPARE 0= ;

: ENDS? ( str len pat len2 -- str len flag )
\G Check end of string.
   DUP >R  2OVER  DUP R> - /STRING  COMPARE 0= ;

: CONTAINS? ( str1 n1 str2 n2 -- str1 n1 flag )
\G Test that str1 contains str2.
   2>R 2DUP 2R>  SEARCH  >R 2DROP R> ;

: EQUALS? ( str1 n1 str2 n2 -- str1 n1 flag )
\G Check for the string equal to the pattern.
   SWAP >R  OVER - ( str1 n1 n2-n1 )
   IF   R> DROP FALSE
   ELSE 2DUP R> OVER COMPARE 0=
   THEN ;

: SIMILAR? ( str1 n1 str2 n2 -- str1 n1 flag )
\G Check for the string equal to the pattern.
   SWAP >R  OVER - ( str1 n1 n2-n1 )
   IF   R> DROP FALSE
   ELSE 2DUP R> OVER COMPARE-INSENSITIVE 0=
   THEN ;

: HUNT ( str1 n1 pat n2 -- str+I n1-i )
\G Scan for a string pat/n2.
   SEARCH  IF EXIT THEN  ZERO ;

: JOIN ( dst n1 src n2 -- src n1+n2 )
\G Joint str1 and str2.
   2SWAP +STRING ;

: REPLACE-CHAR ( str n c1 c2 -- )
\G Replace c1 with c2 in string str.
   >R >R
   BEGIN  ?DUP  WHILE
      OVER C@  R@ =
      IF  OVER R'@ SWAP C!  THEN
      1 /STRING
   REPEAT  DROP 2R> 2DROP ;

: STRING/ ( str n i -- str+i/str-i i )
\G Get ending characters.
   OVER SWAP -  0 MAX  /STRING ;

: TH-WORD-FORWARD ( str n i -- str+I n-I )
\G The ith white-space delimited-substring of <str/n>
\G and what follows it.
   DUP 0= IF  DROP  EXIT THEN
   1 ?DO  BL-SKIP  BL-SCAN  LOOP
   BL-SKIP ;
	
: TH-WORD ( str n i -- str+I n-i )
\G The ith white-space delimited substring of str/n.
   ?DUP IF  TH-WORD-FORWARD  THEN
   2DUP BL-SCAN NIP - ;

: TH-WORD-BACK ( str n i -- str+I n-I )
\G All before the ith white-space delimited substring 
\G of <str/n>.
   >R 2DUP R> TH-WORD-FORWARD NIP - ;


( Character Tests ============================================ )

: IS-DIGIT ( char -- flag )
\G Test char for digit [0-9].
   [CHAR] 0 -  10 U< ;

: IS-ALPHA ( char -- flag )
\G Test char for alphabetic [A-Za-z].
   32 OR  [CHAR] a -  26 U< ;

: IS-ALNUM ( char -- flag )
\G Test char for alphanumeric [A-Za-z0-9].
   DUP IS-ALPHA  ORELSE  DUP IS-DIGIT  THEN  NIP ;

: IS-LOWER ( char -- flag )
\G Test for lowercase letter.
   [CHAR] a [CHAR] z  BETWEEN ;

: IS-UPPER ( char -- flag )
\G Test for uppercase letter.
   [CHAR] A [CHAR] Z  BETWEEN ;

: .LOWER ( str n -- )
\G Prints str/n in lower case.
   BEGIN  ?DUP  WHILE
      OVER C@ LCASE EMIT
      1 /STRING
   REPEAT  DROP ;

: .UPPER ( str n -- )
\G Prints str/n in upper case.
   BEGIN  ?DUP  WHILE
      OVER C@ UPCASE EMIT
      1 /STRING
   REPEAT  DROP ;

: >LOWER ( str n -- )
\G Convert string to lowercase.
   BEGIN  ?DUP  WHILE
      OVER DUP C@ LCASE  SWAP C!
      1 /STRING
   REPEAT  DROP ;

: >UPPER ( str n -- )
\G Convert string to uppercase.
   BEGIN  ?DUP  WHILE
      OVER DUP C@ UPCASE  SWAP C!
      1 /STRING
   REPEAT  DROP ;

   
( Common Constants =========================================== )

 8 CONSTANT #BACKSPACE-CHAR
 9 CONSTANT #TAB-CHAR
10 CONSTANT #LINEFEED-CHAR
13 CONSTANT #CARRIAGE-CHAR

64 VALUE    #CHARS/LINE
13 VALUE    #EOL-CHAR

TRUE 1 RSHIFT        CONSTANT MAX-N
TRUE 1 RSHIFT INVERT CONSTANT SIGN-BIT

 1 CELLS CONSTANT  CELL
-1 CELLS CONSTANT -CELL


( Filter Handling ============================================ )

: SPLIT-NEXT-LINE ( src . -- src' . str len )
\G Split the next line from the string.
   2DUP #EOL-CHAR SCAN
   DUP >R  1 /STRING  2SWAP R> - ;

: VIEW-NEXT-LINE ( src . str len -- src . str len str2 len2 )
\G Copy next line above current line.
   2OVER 2DUP #EOL-CHAR SCAN NIP - ;


( Input Stream =============================================== )

: NEXT-WORD ( -- str len )
\G Get next word across line breaks. len is 0 at EOF.
   BEGIN  BL WORD COUNT    ( str len)
      DUP IF EXIT THEN
      REFILL
   WHILE  2DROP ( ) REPEAT ;

: LEXEME ( "name" -- str len )
\G Get next word on the line. If it's a single character, use
\G it as the delimiter to get a phrase.
   BL WORD ( addr) DUP C@ 1 =
      IF  CHAR+ C@ WORD  THEN
   COUNT ;


( Error Checking ============================================= )

: FILE-CHECK ( n -- )
\G Check for file access error.
   ABORT" File Access Error" ;

: MEMORY-CHECK ( n -- )
\G Check for memory allocation error.
   ABORT" Memory Allocation Error" ;


( Interval Timing ============================================ )

: EXPIRED ( u -- flag )
\G Return true if time passed.
   COUNTER - 0< ;

( Miscellaneous ============================================== )

AKA CELLS+ TH

: 'TH ( n "addr" -- &addr[n] )
\G Address of cell <n> at array <addr>.
   S" CELLS " EVALUATE
   BL WORD COUNT EVALUATE
   S" + " EVALUATE ; IMMEDIATE

: HIWORD ( xxyy -- xx )
\G High half of value.
\64 QSPLIT
\32 WSPLIT
   NIP ;

: LOWORD ( xxyy -- yy )
\G Low half of value.
\64 QSPLIT
\32 WSPLIT
   DROP ;

: REWIND-FILE ( fid -- ior )
\G Rewind the file.
   0 0 ROT REPOSITION-FILE ;


\ vim:ts=3:sw=3:et
