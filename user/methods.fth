\ METHODS> Object-Oriented Extensions Redux
\ by Terry Rayburn, the same method is used for Holon    tgr 12aug87
\ FiCL port                                               ap 19may16

\           8bit  16bit 32bit
\ GET       C@    @     2@
\ PUT       C!    !     2!
\ VUE       C?    ?     2?

\ : HALF    CREATE 0 C,    DOES> EXIT C@ C! C? ;
\ : INTEGER CREATE 0 ,     DOES> EXIT  @  !  ? ;
\ : DOUBLE  CREATE 0 , 0 , DOES> EXIT 2@ 2! 2? ;

\ late binding and flexibility, early binding and efficiency

\ INTEGER CYCLES
\ : xxx ... CYCLES [GET] ... ;

\ 0 METHOD: GET   0 [METHOD]: [GET]
\ 1 METHOD: PUT   1 [METHOD]: [PUT]
\ 2 METHOD: VUE   2 [METHOD]: [VUE]

ANEW <methods.fth>

: PFA ( lfa -- pfa ) [ 5 cells ] literal + ;

: CELL- ( addr1 -- addr2 ) [ -1 cells ] literal + ;

\ methods
: METHODS>  POSTPONE DOES>  POSTPONE EXIT ; IMMEDIATE


\ early binding
: [METHOD]: ( idx <name> -- )
   CREATE cells , IMMEDIATE
   DOES>  ( a -- )
      HERE cell- @   \ get prev. word, which is CREATEd
      pfa @          \ get its PFA, which contains the DOES> part
      cell+          \ skip the EXIT
      swap @ +       \ add the index to it
      @ , ;          \ get the XT of the word, and inline


\ late binding
: METHOD: ( idx <name> -- )
   CREATE cells ,
   DOES>  ( obj a -- )
      @              \ obj idx
      over cell- @   \ obj idx does-addr
      cell+  +       \ skip EXIT, position to XT
      @ execute ;


\ tests

0 METHOD: GET     0 [METHOD]: [GET]
1 METHOD: PUT     1 [METHOD]: [PUT]
2 METHOD: VUE     2 [METHOD]: [VUE]

: INTEGER create 0 , methods> @ ! ? ;

0 [IF]
Integer n
n get
42 n put
n vue

: test-get n [get] . ;
: test-put 31415 n [put] test-get ;
: test-vue n [vue] ;

[THEN]

