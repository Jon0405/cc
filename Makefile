cc: cc.c

test: cc
	./cc -test
	./test.sh

clean:
	rm -f cc *.o *~ tmp*
