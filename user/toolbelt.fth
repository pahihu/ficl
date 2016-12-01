\  TOOLBELT                                      Wil Baden  2002-07-13
\  *********************************************************************
\  *                                                                   *
\  *                         Minimum Toolbelt                          *
\  *                                                                   *
\  *********************************************************************

\  These are common tools used in several source files.  Most of them 
\  are well known.  They are given in one place to avoid duplicate 
\  definitions.  Comment out those that you already have or are 
\  enhancing.  Many of them should be CODE definitions.

\  In any Forth system, the definitions should be optimized.  The 
\  definitions here are in Standard Forth for portability.

\  I hope that readers will submit environmental optimizations.
\  mailto: neil...@earthlink.net

\  Definitions in Standard Forth by Wil Baden. Any similarity with
\  anyone else's code is coincidental, historical, or inevitable.


\     #Chars/Line     C#              Memory-Check    R'@
\     (.)             C+!             Next-Word       Rewind-File
\     ,"              COUNTER         NOT             STRING,
\     2NIP            EMPTY           nth             TEMP
\     3DROP           EXPIRED         OFF             THIRD
\     3dup            File-Check      OFFSET:         TIMER
\     ANDIF           FLAG            ON              VOCABULARY
\     APPEND          FOURTH          ORIF            [DEFINED]
\     BOUNDS          H#              OUT             [UNDEFINED]
\     BUFFER:         Line-Terminator PLACE


\  *********************************************************************
\  *         Forth Programmer's Handbook, Conklin and Rather           *
\  *********************************************************************


\  NOT                          ( x -- flag )
\     Identical to `0=`, used for program clarity to reverse the 
\     result of a previous test.

: NOT  ( x -- flag )  S" 0= " EVALUATE ; IMMEDIATE


\  [DEFINED]                    ( "name" -- flag )
\     Search the dictionary for _name_. If _name_ is found, return
\     TRUE; otherwise return FALSE. Immediate for use in definitions.

: [DEFINED]                 ( "name" -- flag )
    BL WORD FIND NIP 0<> ; IMMEDIATE


\  [UNDEFINED]                  ( "name" -- flag )
\     Search the dictionary for _name_. If _name_ is found, return
\     FALSE; otherwise return TRUE. Immediate for use in definitions.

: [UNDEFINED]               ( "name" -- flag )
    BL WORD FIND NIP 0= ; IMMEDIATE


\  C+!                          ( n addr -- )
\     Add the low-order byte of _n_ to the byte at _addr_, removing
\     both from the stack.

: C+!  ( n addr -- )  dup >R  C@ +  R> C! ;


\  POSSIBLY                        ( "name" -- )
\     Execute _name_ if it exists; otherwise, do nothing. Useful
\     implementation factor of `ANEW`.

: POSSIBLY  ( "name" -- )  BL WORD FIND  ?dup AND IF  EXECUTE  THEN ;

          
\  ANEW                            ( "name" -- )( Run: -- )  
\     Compiler directive used in the form: `ANEW _name_`. If the word 
\     _name_ already exists, it and all subsequent words are 
\     forgotten from the current dictionary, then a `MARKER` word 
\     _name_ is created. This is usually placed at the start of a 
\     file. When the code is reloaded, any prior version is 
\     automatically pruned from the dictionary. 
\
\     Executing _name_ will also cause it to be forgotten, since
\     it is a `MARKER` word.
\
\     Useful implementation factor of `EMPTY`.

: ANEW  ( "name" -- )( Run: -- )  >IN @ POSSIBLY  >IN ! MARKER ;


\  EMPTY                        ( -- )
\     Reset the dictionary to a predefined golden state, discarding 
\     all definitions and releasing all allocated data space beyond 
\     that state.

\  This `EMPTY` uses `--WORKSPACE--` to separate kernel words and user
\  words.  Rename `--WORKSPACE--` if you wish.

: EMPTY                         ( "name" -- )
    S" ANEW --WORKSPACE-- DECIMAL  ONLY FORTH DEFINITIONS "
    EVALUATE ;


\  VOCABULARY                   ( "name" -- )
\     Create a word list _name_. Subsequent execution of _name_ 
\     replaces the first word list in the search order with _name_. 
\     When _name_ is made the compilation word list, new definitions 
\     will be added to _name_'s list.

\ : Do-Vocabulary               ( -- )

\     \  From Standard Forth Rationale A.16.6.2.0715.
\     DOES>  @ >R               ( )( R: widnew)
\         GET-ORDER  SWAP DROP  ( wid_n ... wid_2 n)
\     R> SWAP SET-ORDER ;

\ : VOCABULARY                  ( "name" -- )

\     WORDLIST CREATE ,  Do-Vocabulary ;


\  *********************************************************************
\  *       Environmentally Dependent Values                            *
\  *********************************************************************


\  #Chars/Line                   ( -- n )
\     Preferred width of line in source files.  User defined.

72 VALUE     #Chars/Line


\  LINE-TERMINATOR              ( -- char )
\     The end-of-line character. 13 for Mac and DOS, 10 for Unix

13 CONSTANT  Line-Terminator


\  *********************************************************************
\  *       Common Use                                                  *
\  *********************************************************************


\  (.)                          ( n -- str len )
\     Convert _n_ to characters, without punctuation, as for `.` 
\     (dot), returning the address and length of the resulting 
\     string.

: (.)  ( n -- str len )  dup ABS 0 <# #S ROT SIGN #> ;


\  APPEND                       ( str len addr -- )
\     Catenate the string at _str_, whose length is _len_, to the 
\     counted string already existing at _addr_.  Does not check 
\     whether space is allocated for the final string.  AKA `+PLACE`.

: APPEND                    ( str len addr -- )
    2dup 2>R  COUNT chars +  SWAP chars MOVE ( ) 2R> C+! ;


\  BOUNDS                       ( str len -- str+len str )
\     Convert _str len_ to range for DO-loop.

: BOUNDS  ( str len -- str+len str )  over + SWAP ;


\  C#                           ( -- addr )
\     Variable for character count for display.

VARIABLE  C#  \  Should be USER variable.


\  Next-Word                    ( -- str len )
\     Get the next word in the input stream as a character string - 
\     extending the search across line breaks as necessary, until the 
\     end-of-file is reached - and return its address and length. 
\     Returns a string length of 0 at the end of the file.

: Next-Word                 ( -- str len )
    BEGIN  PARSE-WORD               ( str len)
        dup IF EXIT THEN
        REFILL
    WHILE  2DROP ( ) REPEAT ;       ( str len)


\  `OFF` and `ON` are already defined in most implementations.


\  OFF                          ( addr  -- )
\     Set the flag at _addr_ to true.

\ : OFF  ( addr -- )  FALSE SWAP ! ;


\  ON                           ( addr -- )
\     Set the flag at _addr_ to false.

\ : ON  ( addr -- )  TRUE SWAP ! ;


\  PLACE                        ( str len addr -- )
\     Copy the string at _str_, whose length is _len_, to _addr_, 
\     formatting it as a counted string, i.e., the length is in the 
\     first byte.  Does not check whether space is allocated for the 
\     final string.

: PLACE                     ( str len addr -- )
    2dup 2>R  char+  SWAP chars MOVE  2R> C! ;


\  STRING,                      ( str len -- )
\     Store a string in data space as a counted string.

: STRING,                   ( str len -- )
    HERE  over 1+ chars ALLOT  PLACE ;


\  ,"                           ( "<ccc><quote>" -- )
\     Store a quote-delimited string in data space as a counted string.

: ," [char] " PARSE  STRING, ; IMMEDIATE

\  *********************************************************************
\  *       Stack Handling.  These should be CODE definitions.          *
\  *********************************************************************


\  THIRD                        ( x y z -- x y z x )
\     Copy third element on the stack onto top of stack.

: THIRD  ( x y z -- x y z x )      2 PICK ;    \  Should be CODE defn.


\  FOURTH                       ( w x y z -- w x y z w )
\     Copy fourth element on the stack onto top of stack.

: FOURTH ( w x y z -- w x y z w )  3 PICK ;    \  Should be CODE defn.


\  3DUP                         ( x y z -- x y z x y z )
\     Copy top three elements on the stack onto top of stack.

: 3dup   ( x y z -- x y z x y z )  third third third ;


\  3DROP                        ( x y z -- )
\     Drop the top three elements from the stack.

: 3DROP  ( x y z -- )            2DROP DROP ;  \  Should be CODE defn.


\  2NIP                         ( w x y z -- y z )
\     Drop the third and fourth elements from the stack.

: 2NIP   ( w x y z -- y z )      2SWAP 2DROP ; \  Should be CODE defn.


\  R'@                          ( -- x )( R: x y -- x y )
\     The second element on the return stack.

: R'@    S" 2R@ DROP " EVALUATE ; IMMEDIATE    \  Should be CODE defn.


\  *********************************************************************
\  *       Short-Circuit Conditional                                   *
\  *********************************************************************


\  ANDIF                        ( p ... -- flag )
\     Given `p ANDIF q THEN`:
\     If _p_ is 0 then _q_ will not be performed and the result
\     will be _p_, i.e. 0;
\     If _p_ is not 0 then the result will be _q_.

: ANDIF  S" DUP IF DROP " EVALUATE ; IMMEDIATE


\  ORIF                         ( p ... -- flag )
\     Given `p ORIF q THEN`:
\     If _p_ is 0 then the result will be _q_;
\     If _p_ is not 0 then _q_ will not be performed and the result
\     will be _p_.

: ORIF   S" DUP 0= IF DROP " EVALUATE ; IMMEDIATE


\  *********************************************************************
\  *     Promiscuous Variables Available in Any Task                   *
\  *********************************************************************


\  TEMP                         ( -- addr )
\     Promiscuous variable available in any task.

VARIABLE  TEMP   \  Should be USER variable.


\  OUT                          ( -- addr )
\     Promiscuous variable available in any task.

VARIABLE  OUT    \  Should be USER variable.


\  FLAG                         ( -- addr )
\     Promiscuous variable available in any task.

VARIABLE  FLAG   \  Should be USER variable.


\  *********************************************************************
\  *       Error Checking                                              *
\  *********************************************************************


\  File-Check                   ( n -- )
\     Check for file access error.


\  Memory-Check                 ( n -- )
\     Check for memory allocation error.


\  *********  These words should be tailored for your system.  *********

: File-Check      ( n -- )  ABORT" File Access Error " ;
: Memory-Check    ( n -- )  ABORT" Memory Allocation Error " ;

\ : File-Check    ( n -- )  THROW ;
\ : Memory-Check  ( n -- )  THROW ;

\ : File-Check    ( n -- )  SHOWERROR ;  \  PMF
\ : Memory-Check  ( n -- )  SHOWERROR ;  \  PMF


\  *********************************************************************
\  *        Interval Timing                                            *
\  *********************************************************************


\  COUNTER                      ( -- ms )
\     Return the current value of the millisecond timer.


\  TIMER                        ( u -- )
\     Repeat COUNTER, then subtract the two values and display the
\     interval between the two in milliseconds.


\  EXPIRED                      ( u -- flag )
\     Return true if the current millisecond timer reading has passed 
\     _u_.

[DEFINED] _TickCount [IF]  \  Environment Dependent

    60 CONSTANT  Ticks-per-Second

: COUNTER  ( -- u )  _TickCount  1000 Ticks-per-Second */ ;

: TIMER                     ( u -- )
    COUNTER SWAP -
        0 <#  # # # [char] . HOLD  #S  #> TYPE SPACE
    ;

: EXPIRED ( u -- flag ) COUNTER - 0< ;

[THEN]


\  *********************************************************************
\  *       Miscellaneous                                               *
\ 
*********************************************************************


\  `BL WORD COUNT` may be used instead of `PARSE-WORD`.


\  **************************  Miscellaneous  **************************


\  BUFFER:                      ( n "name" -- )
\     Create buffer _name_ of length _n_ in data space.

: BUFFER:                   ( n "name" -- )
    CREATE  ALLOT ;


\  H#                           ( "hexnumber" -- n )
\     Get the next word in the input stream as an unsigned hex
\     single-number literal. (Adopted from Open Firmware.)
\
\     "The best way to manage BASE is to establish a global default, 
\     e.g. DECIMAL. If you change to another, e.g. HEX, you assume 
\     responsibility for changing back when you're done with it. The 
\     scope of a changed region can be part of a file or all of a 
\     file, but shouldn't span files. Always return to your default 
\     at the end of a file _if_ you've changed it. This minimizes 
\     fussy saving & restoring, and you always know where you are." 
\     -- Elizabeth D. Rather

: H#  ( "hexnumber" -- u )  \  Simplified for easy porting.
    0 0 PARSE-WORD                  ( 0 0 str len)
    BASE @ >R  HEX  >NUMBER  R> BASE !
        ABORT" Not Hex " 2DROP      ( u)
    STATE @ IF  postpone LITERAL  THEN
    ; IMMEDIATE


\  nth                          ( n "addr" -- &addr[n] )
\     The address of `n CELLS addr +`.

: nth                       ( n "addr" -- &addr[n] )
    S" CELLS " EVALUATE
    PARSE-WORD EVALUATE
    S" + "     EVALUATE
    ; IMMEDIATE


\  OFFSET:                      ( n "name" -- )( Run: addr -- addr+n )
\     Field-defining word.  Creates a word which adds a constant to 
\     the number (usually an address) on top of the stack.  Used  in 
\     the form  _n OFFSET: <field name>_  to create a field definition 
\     <field name>.  AKA `FIELD`.  (`FIELD` has been given other 
\     conflicting definitions, so `OFFSET:` hopes to avoid conflict.)

: OFFSET:                   ( n "name" -- )( Run: addr -- addr+n )
    S" : " PAD PLACE
    PARSE-WORD  PAD APPEND
    S"  " PAD APPEND
    dup 0= IF DROP
        S" ; IMMEDIATE " PAD APPEND
    ELSE
        (.) PAD APPEND
        S"  + ; " PAD APPEND
    THEN
    PAD COUNT EVALUATE ;


\  REWIND-FILE                  ( file-id -- ior )
\     Reposition the file at its beginning.

: Rewind-File               ( file-id -- ior )
    0 0 ROT REPOSITION-FILE ;


\  *************************  End of TOOLBELT  ************************

\  \\                           ( "...<eof>" -- )
\     During an INCLUDE operation, treat anything following this word 
\     as a comment; i.e., everything after `\\` is interpreted in a 
\     source file will not be interpreted or compiled.

: \\                          ( "...<eof>" -- )
   BEGIN  -1 PARSE  2DROP  REFILL 0= UNTIL ;


\  SPLIT/
\     To allow to write `SPLIT/ <scan-for-something> /SPLIT`.

: SPLIT/ 2DUP ;


\  /SPLIT                     ( a m a+i m-i -- a+i m-i a i )
\     Split a character string _a m_ at place given by _a+i m-i_.

: /SPLIT ( a m b n -- b n a m-n )
   DUP >R 2SWAP R> - ;


\  SPLIT                      ( addr len char -- addr+i len-i addr i )
\     Divide a string at a given character. The first part of the
\     string is on top, the remaining part underneath. The remaining
\     part begins with the scanned-for character.

: SPLIT     ( addr len char -- addr+i len-i addr i )
   >R 2DUP R> SCAN 2SWAP THIRD - ;


\  STARTS?                    ( str len str2 len2 -- str len flag )
\     Check start of string.

: STARTS?                     ( str len str2 len2 -- str len flag )
   2OVER THIRD MIN COMPARE 0= ;


\  ENDS?                      ( str len pattern len2 -- str len flag )
\     Check end of string.

: ENDS?                       ( str len pattern len2 -- str len flag )
   2OVER DUP FOURTH - /STRING COMPARE 0= ;


\  UNEQUAL                    ( addr1 addr2 -- flag )
\     Compare counted strings addr1 and addr2.

: UNEQUAL ( addr1 addr2 -- flag )
   >R COUNT R> COUNT COMPARE ( 0<> ) ;


\  >PAD                       ( str len -- str' )
\     Move string to PAD.

: >PAD ( str len -- str' )
   PAD PLACE PAD ;


\\  ********************  End of Minimum Toolbelt  ********************
