ls.o: ls.c 
	gcc -o ls.o ls.c -lbsd -lm
clear:
	rm -rf ls.o

