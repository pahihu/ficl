SRCS= bench1.c bench.c
OBJS= $(SRCS:.c=.o)
# CFLAGS= -m32 -O2 -mcpu=power5+
CFLAGS= -m32 -Ofast

bench: $(OBJS)
	$(CC) -m32 -o $@ $(OBJS)

test: bench
	./bench 20000

clean:
	$(RM) bench $(OBJS)
