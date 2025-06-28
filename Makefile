CC = gcc

CFLAGS = -Wall -Wextra -pedantic -Wfatal-errors -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fsanitize=pointer-subtract -fsanitize=pointer-compare -fno-omit-frame-pointer -fstack-protector-all -fstack-clash-protection -fcf-protection -g


shell: do.c
	$(CC) $(CFLAGS) do.c

clean: 
	rm a.out
	rm a.out.*
