\ B.Paysan Mini-OOF
.( loading Mini-OOF -- B.Paysan ) cr

: noop ( -- )
\G No operation.
   ;


: method ( m v "method" -- m' v )
\G Define method `method'.
   CREATE  over , swap cell+ swap
   DOES> ( ... o -- ... ) @ over @ + @ execute ;

: var ( m v size "var" -- m v' )
\G Define `var' with size.
   CREATE  over , +
   DOES> ( o -- addr )  @ + ;

: class ( superclass -- class methods vars )
\G Begin class definition with base class `superclass'.
\G Example:
\G    object class
\G       cell var mVar
\G       method :mymethod
\G    end-class myclass
   dup 2@ ;

: end-class ( class methods vars "classname" -- )
\G Ends class definition for class `classname'.
   CREATE  here >R , dup , 2 cells ?DO ['] noop , /cell +LOOP
   cell+ dup cell+ swap @ -2 cells+
   R> 2 cells+ swap move ;

: defines ( xt class "method" -- )
\G Define `xt' for `method' in `class'.
\G Example:
\G    :noname ( obj -- ) ... ; myclass defines :mymethod
   ' >body @ + ! ;

: new ( class -- o )
\G Allot space for instance of `class'.
\G Example:
\G    myclass new constant myobj
   here swap dup , @ -1 cells+ allot ;

: :: ( class "method" -- )
\G Compile xt for `method' of `class'.
\G Example:
\G    myobj [ myclass :: mymethod ] ...
   ' >body @ + @ compile, ;

CREATE object  1 cells , 2 cells ,

