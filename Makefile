all:
	gcc -o test test.c fifo.c -lpthread -lrt
