all:
	gcc -w -g -c DoubleLL.c -o DoubleLL.o
	gcc -w -g -c futex.c -o futex.o
	gcc -w -g -c mythread.c -o mythread.o
	ar rc mythread.a DoubleLL.o futex.o mythread.o
#	gcc  -w -g  testprogram.c mythread.o DoubleLL.o futex.o 
	gcc -g -w testprogram.c mythread.a -o mytest

clean:
	rm DoubleLL.o futex.o mythread.o mythread.a mytest


