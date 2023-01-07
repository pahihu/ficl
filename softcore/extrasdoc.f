: pwd ( -- )
\G Prints the current working directory.

: cd ( <path> -- )
\G Changes current working directory to <path>.

: clock ( -- u )
\G Returns the current UNIX clock value.

: get-msecs ( -- u )
\G Returns the current UNIX clock value in milliseconds.

: system ( <command> -- )
\G Execute <command> in the shell.

: load ( <file> -- )
\G Load a text file and execute it.

: spewhash ( -- )
\G Dump the contents of the dictionary hash table by hashcode.

: break ( -- )
\G A handy breakpoint in C.

: prepterm ( 0|1 -- )
\G Initialize/finish the terminal for raw terminal I/O.

: key ( -- c )
\G Returns the current key pressed.

: key? ( -- ff )
\G Returns TRUE if any key pressed.

: ekey ( -- code )
\G Reads extended key code.

: utime ( -- sec )
\G Returns the current UNIX time in seconds.

: now ( -- seconds minutes hours )
\G Returns the current time.

: today ( -- day mon year )
\G Returns the current date.

: time-zone ( -- minutes )
\G Returns the current time zone in minutes.

: dst? ( -- ff )
\G Returns TRUE if daylight saving time.

: (dlopen) ( ca u -- hnd )
\G Opens the shared library named by ca/u.

: (dlsym) ( ca u hnd -- fnAddr )
\G Returns the function address for the symbol ca/u
\G in the shared library identified by hnd.

: (c-call) ( arg8 ... arg1 N fnAddr -- ret )
\G Calls the C function <fnAddr> with <N> arguments
\G where N is less-or-equal to 8.

: (callback) ( xt nargs cbIndex -- ptr )
\G Returns a pointer for the <cbIndex>th 
\G C callback function executing <xt> with <nargs> arguments.
\G Max. 8 callback functions can be installed.

: /task ( -- tasksize )
\G Return the size of a task.

: construct ( addr -- )
\G Construct a task at <addr>.

: activate ( xt addr -- )
\G Activate task at <addr> to execute <xt>.

: pause ( -- )
\G Release processing time of a task.

: stop ( -- )
\G Stops the task. Use AWAKEN to continue.

: awaken ( task -- )
\G Awake task at <addr>.

: halt ( -- )
\G Halts the task.

: terminate ( addr -- )
\G Halts the task at <addr>.

: his ( task addr1 -- addr2 )
\G Returns the user variable address in task.

: /mutex ( -- n )
\G Returns the size of a mutex.

: mutex-init ( mutex -- )
\G Initialize mutex at <addr>.

: get ( mutex -- )
\G Locks mutex.

: release ( mutex -- )
\G Releases mutex.

: atom@ ( addr -- x )
\G Atomically reads cell at <addr>.

: atomic! ( x addr -- )
\G Atomically stores <x> at <addr>.

: atomic-xchg ( x1 addr -- x2 )
\G Test-and-set at <addr>.

: atomic-cas ( expected desired addr -- prev )
\G Compare-and-set at <addr>.

: atomic-op ( xt addr -- )
\G Execute <xt> atomically, xt consumes the value at <addr>
\G and produces a new value. The contents of <addr> is tested
\G with CAS.

: wflip ( uw1 -- uw2 )
\G Swaps the two halves of the 16bit word uw1.

: qflip ( uq1 -- uq2 )
\G Swaps the two halves of the 32bit quad-word uq1.

: xflip ( ux1 -- ux2 )
\G Swaps the two halves of the 64bit word ux1.

: flip ( u1 -- u2 )
\G Swaps the two halves of the cell u1.

: stick ( xu ... x0 x u -- x xu-1 .. xu )
\G Stores <u> at the uth stack location.

: wordkind? ( xt -- kind )
\G Returns the kind of word <xt>. See ficl.h for
\G the definiton of <kind> (ficlWordKind enum).

: errno ( -- #error )
\G Return the UNIX error number, errno.

: why ( #error - caddr u )
\G Return the error string corresponding to <#error>.

: @@ ( addr -- n )
\G Fetches the indirect pointer value stored at <addr>.
\G Equivalent to @ @.

: @! ( n addr -- )
\G Stores <n> thrugh the indirect pointer value at <addr>.
\G Equivalent to @ !.

: fuzzify ( pMemberfuncs pFuzzyInputs len systemInput -- )
\G Equivalent to CPU12 MEM instruction.

: defuzzify ( pFuzzyOuts pSingletons len -- systemOutput )
\G Equivalent to CPU12 WAV instruction.

: rulez ( pRules pFuzzyVars -- )
\G Equivalent to CPU12 REV instruction.

: wrulez ( pRules pWeights pFuzzyVars -- )
\G Equivalent to CPU12 REVW instruction.

: bit? ( ix c-addr -- ff )
\G Returns TRUE if <ix>th bit is set in bit-array at <c-addr>.

: bit-set ( ix c-addr -- )
\G Sets the <ix>th bit in bit-array at <c-addr>.

: bit-reset ( ix c-addr -- )
\G Resets the <ix>th bit in bit-array at <c-addr>.

: bit-flip ( ix c-addr -- )
\G Flips the <ix>th bit in bit-array at <c-addr>.
