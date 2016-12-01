#include <stdlib.h>
#include <stdio.h>
#include <time.h>

extern void inner(long N);

void bench(long N)
{
	long i;
	for (i = 0; i < N; i++)
		inner(N);
}

long fib(long n)
{
   if (n < 2) return n;
   return fib(n-1) + fib(n-2);
}

long fib2(long n)
{
   long a,b,fib3;

   if (n < 2) return n;
   // a = fib(n-1);
   if (n < 3) a = n-1;
   else {
      fib3 = fib(n-3);        // n=3,4,5,6
      a    = fib(n-2) + fib3;
   }
   // b = fib(n-2);
   if (n < 4) b = n-2;
   else {                     // n=4,5,6
      b = fib3 + fib(n-4);
   }
   return a+b;
}

int main(int argc, char *argv[])
{
   long n,N;
	clock_t start, elapsed;

   if (2 != argc) {
      fprintf(stderr,"usage: bench <N>, where N at least 20000\n");
      exit(1);
   }
   N = atol(argv[1]);

	start = clock();
	bench(N);
	elapsed = (clock_t)((double)(clock() - start) /CLOCKS_PER_SEC * 1000.0);
	fprintf(stderr,"Mentink(20K) in  %ld ms.\n", elapsed);

   start = clock();
   n = fib(40);
	elapsed = (clock_t)((double)(clock() - start) /CLOCKS_PER_SEC * 1000.0);
	fprintf(stderr,"fib(40) = %ld in %ld ms.\n", n, elapsed);

   start = clock();
   n = fib2(40);
	elapsed = (clock_t)((double)(clock() - start) /CLOCKS_PER_SEC * 1000.0);
	fprintf(stderr,"fib2(40) = %ld in %ld ms.\n", n, elapsed);
	return 0;
}
