all: test

test: clean
	$(CC) -pthread -g RDCSS_Test.c RDCSS.c -o test

clean:
	rm -rf test
