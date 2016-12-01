#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static volatile int g = 0;

void inner()
{
	int i;
	for (i = 0; i < 10000; i++)
		g = 34;
}

void bench()
{
	int i;
	for (i = 0; i < 10000; i++)
		inner();
}

int main(int argc, char *argv[])
{
	clock_t start, elapsed;

	start = clock();
	bench();
	elapsed = clock() - start;
	fprintf(stderr,"Elapsed %ld\n", elapsed);
	return 0;
}
