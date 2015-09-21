CC = gcc -ansi -pedantic
FLAGS = 

tlisp : main.c
	$(CC) $(FLAGS) main.c -o tlisp.out
