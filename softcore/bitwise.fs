.( loading BITWISE -- M.W.Humphries ) CR

: 2** ( n -- 2^n )
\G Computes 2^<n>.
   1 SWAP LSHIFT ;

AKA 2** BIT ( # -- bit )
\G Converts bit number <#> to a <bit>.

AKA 1- BITMASK ( bit -- mask )
\G Converts a <bit> to a <mask>.

: SET-BITS ( mask addr -- )
\G Set <mask> bits in cell at <addr>.
   SWAP OVER @  OR  SWAP ! ;

: CLEAR-BITS ( mask addr -- )
\G Reset <mask> bits in cell at <addr>.
   SWAP INVERT OVER @  AND  SWAP ! ;

: TOGGLE-BITS ( mask addr -- )
\G Toggle <mask> bits in cell at <addr>.
   SWAP OVER @  XOR  SWAP ! ;

: MUX ( a b mask -- out )
\G Cellwide MUX, 1 in <mask> selects the corrsponding bit from <a>,
\G 0 selects the bit from <b>.
   SWAP OVER INVERT AND >R AND R> OR ;
