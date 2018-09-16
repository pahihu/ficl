\ B.Paysan Mini-OOF
.( loading Mini-OOF -- B.Paysan ) cr

: noop ( -- ) ;


: method ( m v -- m' v )
   CREATE  over , swap cell+ swap
   DOES> ( ... o -- ... ) @ over @ + @ execute ;

: var ( m v size -- m v' )
   CREATE  over , +
   DOES> ( o -- addr )  @ + ;

: class ( class -- class methods vars )
   dup 2@ ;

: end-class ( class methods vars -- )
   CREATE  here >R , dup , 0 ?DO ['] noop , LOOP
   cell+ dup cell+ swap @ -2 cells+
   R> 2 cells+ swap move ;

: defines ( xt class -- )
   ' >body @ + ! ;

: new ( class -- o )
   here swap dup , @ -1 cells+ allot ;

: :: ( class "name" -- )
   ' >body @ + @ compile, ;

CREATE object  1 cells , 2 cells ,

