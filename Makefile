#Makefile for httprique
CC = gcc
CFLAGS=-Wl,-rpath,/home/yongdeng/libevent/lib/ -L/home/yongdeng/libevent/lib/ -levent -I/home/yongdeng/libevent/include/ -lpthread

OBJS=diskalloc.o pqnode.o priorque.o httprique.o

httprique: $(OBJS)
	$(CC) -o httprique $(OBJS) $(CFLAGS)
	@echo ""
	@echo "httprique build complete."
	@echo ""
httprique.o:httprique.c 
	$(CC) -c httprique.c $(CFLAGS)
priorque.o:priorque.c priorque.h pqnode.h
	$(CC) -c priorque.c
pqnode.o:pqnode.c pqnode.h priorque.h
	$(CC) -c pqnode.c
diskalloc.o:diskalloc.c diskalloc.h
	$(CC) -c diskalloc.c
clean:
	rm -f httprique $(OBJS)
install:httprique
	install -m 0755 httprique /usr/local/bin
