all:
	gcc -g -c DoubleLL.c
	gcc -g -c futex.c
##	gcc -g -c mythread.c
	gcc -g  mythread.c DoubleLL.o futex.o 
