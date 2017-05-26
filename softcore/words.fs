\ Search for words by substrings                      ap 13may17
ANEW -words

: contains-string ( ca1 u1 nt -- ff )
   dup >R name>string 2over search-nc
   IF    R> name>string 20 $.f
   ELSE  R> drop  THEN
   2drop true ;

: words: ( <substr> -- )
\ *G List those word names in the current word list that partly
\ *G match name.
   parse-word
   ['] contains-string  get-current  traverse-wordlist CR ;

