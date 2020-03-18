all: test

test: clean
	$(CC) -O2 -Wall -std=c11 -pthread RDCSS_Test.c RDCSS.c -o test

clean:
	rm -rf test
