CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cc: $(OBJS)
	$(CC) -o cc $(OBJS) $(LDFLAGS)

$(OBJS): cc.h

test: cc
	./cc -test
	./test.sh

clean:
	rm -f cc *.o *~ tmp*
