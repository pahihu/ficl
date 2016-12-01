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

int main(int argc, char *argv[])
{
   long N;
	clock_t start, elapsed;

   if (2 != argc) {
      fprintf(stderr,"usage: bench <N>, where N at least 20000\n");
      exit(1);
   }
   N = atol(argv[1]);

	start = clock();
	bench(N);
	elapsed = (clock_t)((double)(clock() - start) /CLOCKS_PER_SEC * 1000.0);
	fprintf(stderr,"Elapsed %ld\n", elapsed);
	return 0;
}
