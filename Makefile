CC = gcc
OUT ?= a.out

CFLAGS = -Wall -Wextra -pedantic -Wfatal-errors -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fsanitize=pointer-subtract -fsanitize=pointer-compare -fno-omit-frame-pointer -fstack-protector-all -fstack-clash-protection -fcf-protection -g -o


shell: sh.c
	$(CC) $(CFLAGS) $(OUT) sh.c

clean: 
	rm $(OUT)
	rm $(OUT).id*
