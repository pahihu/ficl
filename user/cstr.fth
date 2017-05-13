\ Counted strings ----------------------------------- ap 21may16


: C+! ( char c-addr -- )
\ *G Adds char to the contents of c-addr.
   swap  over c@ +  swap c! ;

: PLACE ( c-addr1 u1 c-addr2  -- )
\ *G Place the string c-addr1/u1 as a counted string at c-addr2.
   2dup c!  char+ swap  chars  move ;

: APPEND ( c-addr1 u1 c-addr2 -- )
\ *G Append the string c-addr1/u1 to the counted string at c-addr2.
   dup  c@ >R  2dup c+! char+
   R>  chars + swap chars move ;

: +NULL ( c-addr -- )
\ *G Append a NULL to a counted string, but not included in count.
   count chars +  0 swap c! ;

: UPC? ( char -- flag )
\ *G Return true if upper case char.
   [char] A [char] Z 1+ within ;

: LWC? ( char -- flag )
\ *G Return true if lower case char.
   [char] a [char] z 1+ within ;

: UPC ( char1 -- char2 )
\ *G Convert char1 to upper case.
   dup [char] a [char] z 1+ within IF
      [ char a char A - ] literal -
   THEN ;

: LWC ( char1 -- char2 )
\ *G Convert char1 to lower case.
   dup [char] A [char] Z 1+ within IF
      [ char a char A - ] literal +
   THEN ;

: UCASE ( c-addr u -- )
\ *G Convert string to upper case in place.
   bounds DO
      I c@  upc  I c!
   LOOP ;

: LCASE ( c-addr u -- )
\ *G Convert string to lower case in place.
   bounds DO
      I c@  lwc  I c!
   LOOP ;
