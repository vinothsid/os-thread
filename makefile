all:
	gcc -c DoubleLL.c
	gcc -c futex.c
	gcc -c mythread.c
	gcc -g userProgram.c DoubleLL.o futex.o mythread.o
