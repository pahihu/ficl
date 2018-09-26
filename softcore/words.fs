\ Search for words by substrings                      ap 13may17
\ ANEW -words

HIDE

: CONTAINS-STRING ( ca1 u1 nt -- ff )
   DUP >R NAME>STRING 2OVER SEARCH-NC
   IF    R> NAME>STRING  >PAD 2DUP UPPER  10 $.R
   ELSE  R> DROP  THEN
   2DROP TRUE ;

: (WORDS:) ( <substr> -- )
\G List those word names in the current word list that partly
\G match name.
   ['] CONTAINS-STRING  GET-CURRENT  TRAVERSE-WORDLIST
   2DROP CR ;

AKA WORDS (WORDS)

SET-CURRENT

-WARNING

: WORDS ( ["name"] -- )
   PARSE-WORD DUP
   IF         (WORDS:)
   ELSE 2DROP (WORDS)
   THEN ;
   
+WARNING

: WORDS: ( "name" -- )
\G iForth variant.
   PARSE-WORD (WORDS:) ;

PREVIOUS
