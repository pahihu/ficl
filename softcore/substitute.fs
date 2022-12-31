\ ANEW -substitute

.( loading REPLACES & SUBSTITUTE -- Forth200x ) CR

\ https://forth-standard.org/standard/string/REPLACES

HIDE

: "/COUNTED-STRING" S" /COUNTED-STRING" ; 
"/COUNTED-STRING" ENVIRONMENT? 0= [IF] 256 [THEN] 
CHARS CONSTANT string-max

WORDLIST CONSTANT wid-subst 
\ Wordlist ID of the wordlist used to hold substitution names and replacement text.

: makeSubst \ c-addr len -- c-addr 
   wid-subst (WID-CREATE)            \ like CREATE but takes c-addr/len/wid 
   HERE string-max ALLOT 0 OVER C! \ create buffer space 
; 

: findSubst \ c-addr len -- xt flag | 0 
\ Given a name string, find the substitution. 
\ Return xt and flag if found, or just zero if not found. 
\ Some systems may need to perform case conversion here. 
   wid-subst SEARCH-WORDLIST 
;

SET-CURRENT

: REPLACES ( text tlen name nlen -- )
\G Define the string text/tlen as the text to substitute for the substitution named name/nlen.
\G If the substitution does not exist it is created.
   2DUP findSubst IF 
     NIP NIP EXECUTE    \ get buffer address 
   ELSE 
     makeSubst 
   THEN 
   place                  \ copy as counted string 
;

PREVIOUS


\ https://forth-standard.org/standard/string/SUBSTITUTE

HIDE

CHAR % CONSTANT delim     \ Character used as the substitution name delimiter. 
string-max BUFFER: Name \ Holds substitution name as a counted string. 
VARIABLE DestLen           \ Maximum length of the destination buffer. 
2VARIABLE Dest             \ Holds destination string current length and address. 
VARIABLE SubstErr          \ Holds zero or an error code.

: addDest \ char -- 
\ Add the character to the destination string. 
   Dest @ DestLen @ < IF 
     Dest 2@ + C! 1 CHARS Dest +! 
   ELSE 
     DROP -1 SubstErr ! 
   THEN 
;

: formName \ c-addr len -- c-addr' len' 
\ Given a source string pointing at a leading delimiter, place the name string in the name buffer. 
   1 /STRING 2DUP delim scan >R DROP \ find length of residue 
   2DUP R> - DUP >R Name place        \ save name in buffer 
   R> 1 CHARS + /STRING                 \ step over name and trailing % 
;

: >dest \ c-addr len -- 
\ Add a string to the output string. 
   bounds ?DO 
     I C@ addDest 
   1 CHARS +LOOP 
;

: processName \ -- flag 
\ Process the last substitution name. Return true if found, 0 if not found. 
   Name COUNT findSubst DUP >R IF 
     EXECUTE COUNT >dest 
   ELSE 
     delim addDest Name COUNT >dest delim addDest 
   THEN 
   R> 
;

SET-CURRENT

: SUBSTITUTE ( src slen dest dlen -- dest dlen' n )
\G Expand the source string using substitutions.
\G Note that this version is simplistic, performs no error checking,
\G and requires a global buffer and global variables.
   Destlen ! 0 Dest 2! 0 -rot \ -- 0 src slen 
   0 SubstErr ! 
   BEGIN 
     DUP 0 > 
   WHILE 
     OVER C@ delim <> IF                \ character not % 
       OVER C@ addDest 1 /STRING 
     ELSE 
       OVER 1 CHARS + C@ delim = IF    \ %% for one output % 
         delim addDest 2 /STRING       \ add one % to output 
       ELSE 
         formName processName IF 
           ROT 1+ -rot                    \ count substitutions 
         THEN 
       THEN 
     THEN 
   REPEAT 
   2DROP Dest 2@ ROT SubstErr @ IF 
     DROP SubstErr @ 
   THEN 
;

PREVIOUS
