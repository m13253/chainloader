.PHONY: all clean

CC = x86_64-w64-mingw32-gcc
RC = x86_64-w64-mingw32-windres
RM = rm -f

all: chainloader.exe

clean:
	$(RM) chainloader.exe chainloader.res

# Credits:
# https://stackoverflow.com/questions/42022132/how-to-create-tiny-pe-win32-executables-using-mingw
chainloader.exe: chainloader.c chainloader.res
	$(CC) -falign-functions=1 -falign-jumps=1 -falign-loops=1 -fno-asynchronous-unwind-tables -fno-ident -fno-stack-protector -fno-unwind-tables -fomit-frame-pointer -mpreferred-stack-boundary=3 -fstack-reuse=all -municode -mwindows -nolibc -nostdlib -s -std=c11 -Os -Wall -Wextra -Wl,--file-alignment,16,--gc-sections,--section-alignment,16,-T,tinygccpe.scr -o $@ $< chainloader.res -lcomctl32 -lkernel32 -luser32

chainloader.res: chainloader.rc
	$(RC) -O coff -o $@ $<
