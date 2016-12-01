\ REDEFINE                                            ap 19may16

\ ACE-FORTH provided a word REDEFINE which enabled the user
\ to replace the definitions of old words in the dictionary.

\ I used the technique of Holon by W.Wejgaard, which patched
\ the old word to jump to the new definition.

\ REDEFINE ( <name> -- )
\ Patches <name> to execute the most recently defined word.

\ Arcane FiCL knowledge the XT is the LFA.

\ normal word (colon)
\  <name>--aligned--hashlink--/hash-flag-namelen/--/ptr-to-name/
\  46 2B ... prim/XT ... 2B

\ do-does
\ DOES>  4D ... prim/XT ...
\ the word created
\  4B--2B--<addr of does part>--<pfa>
\ the PFA contains the DOES> part address

ANEW <redefine.fth>

' (branch) ( ) 3 CELLS + @  CONSTANT <branch>-code
                    1 cells CONSTANT /cell

\ Header access.  
: NFA ( lfa -- nfa ) 2 cells + ;
: NFA>NAME ( nfa -- ca u ) dup @  swap 1- c@ ;
: CFA ( lfa -- cfa ) 3 cells + ;
: PFA ( lfa -- pfa ) 5 cells + ;
  
: .ID ( lfa -- ) nfa  nfa>name  type space ;


\ Prints a single hash-chain.
: .HASH-CHAIN ( lfa -- )
   BEGIN  dup
   WHILE  dup .id  @
   REPEAT drop ;


\ Patch old-lfa to jump to new-lfa.
: PATCH ( old-lfa new-lfa -- )
   pfa >R pfa                    \ old-pfa, R: new-pfa
   <branch>-code over ! cell+    \ old-dp
   R> over -  /cell /            \ old-dp disp
   swap ! ;

   
\ Answer true, if the word at LFA is the same as ca.
: SAME-NAME? ( ca u lfa -- f )
   nfa nfa>name  compare 0= ;
   
   
\ Find a previous definition of ca/u in the LFA chain.
\ When not found, returns ca 0, else the xt and -1.
: doFIND ( ca lfa -- ca 0 | xt n )
   >R  dup count              \ ca ca' u, R: lfa
   BEGIN  R@
   WHILE
      2dup R@ same-name? IF
         2drop drop           \ drop original ca, and ca/u
         R>  -1  EXIT
      THEN
      R> @ >R
   REPEAT  2drop R> ;

   
\ Redefine name to call most recent definition 
\ in dictionary.
: REDEFINE ( <name> -- )
   bl word                       \ get next word
   dup count last-word same-name? IF
      last-word @  doFIND
   ELSE FIND
   THEN
   IF    last-word  patch
   ELSE  DROP
   THEN ;


\ Calculate next LFA address, if ca/u would be
\ appended to the dictionary.
: NEXT-LFA ( ca u -- a-addr )
   swap drop 1+  here  +
   aligned ;
   
: OLD-COLON : ;

\ If the to be defined name is found in the dictionary
\ patch it to point to the new definition here.
: PATCH-COLON ( <name> -- )
   >in @ >R  bl word  R> >in !
   dup find IF   swap  ( xt ca ) count  next-lfa  patch
            ELSE 2drop
            THEN
   old-colon ;
   

\ tests
0 [IF]

: foo ." here is foo" cr ;
: bar ." here is bar, calling foo " foo ;
foo
bar
: lambada ." here is lambada" cr ;
lambada
redefine foo
foo
bar
: foo ." here is NEW foo" cr ;
redefine foo
foo
bar

[THEN]