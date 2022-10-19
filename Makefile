.PHONY: all clean

CC = x86_64-w64-mingw32-gcc
RC = x86_64-w64-mingw32-windres
RM = rm -f

all: chainloader.exe

clean:
	$(RM) chainloader.exe chainloader.res

chainloader.exe: chainloader.c chainloader.res
	$(CC) -fno-asynchronous-unwind-tables -municode -mwindows -nolibc -nostdlib -s -std=c11 -Ofast -Wall -Wextra -o $@ $< chainloader.res -lcomctl32 -lkernel32 -luser32

chainloader.res: chainloader.rc
	$(RC) -O coff -o $@ $<
