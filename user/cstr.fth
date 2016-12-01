\ Counted strings ----------------------------------- ap 21may16


\  C+!      ( char c-addr -- )
\     Adds char to the contents of c-addr.

: C+! ( char c-addr -- )
   swap  over c@ +  swap c! ;


\  PLACE    ( c-addr1 u1 c-addr2 -- )
\     Place the string c-addr1/u1 as a counted string
\     at c-addr2.

: PLACE ( c-addr1 u1 c-addr2  -- )
   2dup c!  char+ swap  chars  move ;


\  APPEND   ( c-addr1 u1 c-addr2 -- )
\     Append the string c-addr1/u1 to the counted string
\     at c-addr2.

: APPEND ( c-addr1 u1 c-addr2 -- )
   dup  c@ >R  2dup c+! char+
   R>  chars + swap chars move ;


\  +NULL ( c-addr -- )
\     Append a NULL to a counted string, but not included
\     in count.

: +NULL ( c-addr -- )
   count chars +  0 swap c! ;


\  UPC      ( char1 -- char2 )
\     Convert char1 to upper case.

: UPC ( char1 -- char2 )
   dup [char] a [char] z between IF
      [ char a char A - ] literal -
   THEN ;


\  LWC      ( char1 -- char2 )
\     Convert char1 to lower case.

: LWC ( char1 -- char2 )
   dup [char] A [char] Z between IF
      [ char a char A - ] literal +
   THEN ;


\  UCASE    ( c-addr u -- )
\     Convert string to upper case in place.

: UCASE ( c-addr u -- )
   bounds DO
      I c@  upc  I c!
   LOOP ;


\  LCASE    ( c-addr u -- )
\     Convert string to lower case in place.

: LCASE ( c-addr u -- )
   bounds DO
      I c@  lwc  I c!
   LOOP ;
