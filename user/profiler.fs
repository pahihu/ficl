( PROFILER                                  D.Kuehling 17mar14 )

struct
   cell% field list-next
end-struct LIST%

list%
     cell% field def-name
     cell% field def-xt
     cell% field def-count
   double% field def-ms
end-struct DEF%

: NEXT ( a-addr1 -- a-addr2 ) list-next @ ;
: LINK ( a-addr1 a-addr2 -- ) \ link a-addr1 to a-addr2
   2dup @ swap list-next !  ! ;

: SORT-LIST ( xt a-addr -- ) \ sort list a-addr using xt
   { xt list }
   BEGIN
      true { finished? }
      list BEGIN ?dup WHILE   { item-ptr }
         item-ptr @ ?dup IF
            dup list-next @   { item next-item }
            next-item IF
               item next-item xt EXECUTE IF
                  next-item item-ptr !
                  next-item list-next @   { continuation }
                  item next-item list-next !
                  continuation item list-next !
                  false TO finished?
               THEN
            THEN
         THEN
         item-ptr @  list-next
      REPEAT
   finished? UNTIL ;

VARIABLE def-list    0 def-list !
0 VALUE curr-def
0 VALUE noname-def?

: NEW-DEF ( c-addr u -- a-addr )
   def% %allot dup >R def-list link
   0  R@ def-count  !
   0. R@ def-ms    2!
   HERE -rot string, R@ def-name !  R> ;

: DOES>PROFILE DOES> ( def-addr -- )
   @ dup >R def-xt @
   cputime 2drop 2>R  EXECUTE
   cputime 2drop 2R>  d- R@ def-ms dup >R 2@ d+  R> 2!
   1 R> def-count +! ;

: P: ( "name" -- xt colon-sys )
   bl SWORD new-def TO curr-def
   false TO noname-def? :NONAME ;

: P:NONAME ( -- xt colon-sys )
   s" (noname" new-def TO curr-def
   true TO noname-def?  :NONAME ;

: ;P ( colon-sys -- )
   POSTPONE ; noname-def? IF  dup  THEN  curr-def def-xt !
   curr-def def-name @ count
   nextname CREATE curr-def , does>profile ; IMMEDIATE

: N: ( "name" -- colon-sys ) : ;
: ;N ( colon-sys -- ) POSTPONE ; ;

: SORT-BY-COUNT ( a-addr1 a-addr2 -- flag )
   def-count @  swap  def-count @  > ;

: SORT-BY-MS ( a-addr1 a-addr2 -- flag )
   def-ms 2@  rot  def-ms 2@  d> ;

: SHOW-STAT ( -- )
   ['] sort-by-ms def-list sort-list
   def-list @ BEGIN ?dup WHILE >R
      CR R@ def-name  @ count dup >R    type  32 R> - spaces
         R@ def-count @               8 .r
         R@ def-ms    2@             12 d.r
         R> next
   REPEAT ;

n: : p: ;
n: :NONAME  p:noname ;
n: ;  POSTPONE ;p ;  IMMEDIATE

