.PHONY: all clean

CC = x86_64-w64-mingw32-gcc
RC = x86_64-w64-mingw32-windres
RM = rm -f

all: exewrapper.exe

clean:
	$(RM) exewrapper.exe exewrapper.res

exewrapper.exe: exewrapper.c exewrapper.res
	$(CC) -municode -mwindows -s -std=c99 -Ofast -Wextra -o $@ $< exewrapper.res

exewrapper.res: exewrapper.rc
	$(RC) -O coff -o $@ $<
