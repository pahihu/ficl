.( loading BITWISE -- M.W.Humphries ) CR

: 2** ( n -- 2^n )
\G Computes 2^<n>.
   1 SWAP LSHIFT ;

AKA 2** BIT ( # -- bit )
\G Converts bit number <#> to a <bit>.

AKA 1- BMASK ( bit -- mask )
\G Converts a <bit> to a <mask>.

: BSET ( mask addr -- )
\G Set <mask> bits in cell at <addr>.
   SWAP OVER @  OR  SWAP ! ;

: BRESET ( mask addr -- )
\G Reset <mask> bits in cell at <addr>.
   SWAP INVERT OVER @  AND  SWAP ! ;

: BTOGGLE ( mask addr -- )
\G Toggle <mask> bits in cell at <addr>.
   SWAP OVER @  XOR  SWAP ! ;
