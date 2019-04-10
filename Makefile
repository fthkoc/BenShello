all:
	gcc bunedu.c -c -pedantic -Wall
	gcc cat.c -c -pedantic -Wall
	gcc lsf.c -c -pedantic -Wall
	gcc pwd.c -c -pedantic -Wall
	gcc wc.c -c -pedantic -Wall
	gcc BenShello.c -c -pedantic -Wall
	gcc -o bunedu bunedu.o
	gcc -o cat cat.o
	gcc -o lsf lsf.o
	gcc -o pwd pwd.o
	gcc -o wc wc.o
	gcc -o BenShello BenShello.o

debug:
	gcc bunedu.c -c -pedantic -Wall -DDEBUG
	gcc cat.c -c -pedantic -Wall -DDEBUG
	gcc lsf.c -c -pedantic -Wall -DDEBUG
	gcc pwd.c -c -pedantic -Wall -DDEBUG
	gcc wc.c -c -pedantic -Wall -DDEBUG
	gcc BenShello.c -c -pedantic -Wall -DDEBUG
	gcc -o bunedu bunedu.o
	gcc -o cat cat.o
	gcc -o lsf lsf.o
	gcc -o pwd pwd.o
	gcc -o wc wc.o
	gcc -o BenShello BenShello.o

clean:
	rm *.o
