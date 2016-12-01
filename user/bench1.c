void inner(long N)
{
	long i;
	for (i = 0; i < N; i++)
		asm("nop");
}
