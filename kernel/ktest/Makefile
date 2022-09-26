
CFLAGS += -I .


all: ktest.o bitree.o kfifo.o mutex.o rbtree.o

ktest.o: ktest.c
	gcc $(CFLAGS) -c ktest.c -o ktest.o

bitree.o: test-bitree.c
	gcc $(CFLAGS) -c test-bitree.c -o test-bitree.o

kfifo.o: test-kfifo.c
	gcc $(CFLAGS) -c test-kfifo.c -o test-kfifo.o

mutex.o: test-mutex.c
	gcc $(CFLAGS) -c test-mutex.c -o test-mutex.o

rbtree.o: test-rbtree.c
	gcc $(CFLAGS) -c test-rbtree.c -o test-rbtree.o