BOFNAME := ScreenshotBOF
COMINCLUDE := -I .common
LIBINCLUDE := 
CC_x64 := x86_64-w64-mingw32-gcc
CC_x86 := i686-w64-mingw32-gcc
CC=x86_64-w64-mingw32-clang

all:
	$(CC_x64) -o $(BOFNAME).x64.obj $(COMINCLUDE) -Os -c entry.cpp -DBOF 
	$(CC_x86) -o $(BOFNAME).x86.obj $(COMINCLUDE) -Os -c entry.cpp -DBOF
	mkdir -p $(BOFNAME) 
	mv $(BOFNAME)*.obj $(BOFNAME)

test:
	$(CC_x64) entry.c -g $(COMINCLUDE) $(LIBINCLUDE)  -o $(BOFNAME).x64.exe
	$(CC_x86) entry.c -g $(COMINCLUDE) $(LIBINCLUDE) -o $(BOFNAME).x86.exe

scanbuild:
	$(CC) entry.c -o $(BOFNAME).scanbuild.exe $(COMINCLUDE) $(LIBINCLUDE)

check:
	cppcheck --enable=all $(COMINCLUDE) --platform=win64 entry.c

clean:
	rm $(BOFNAME).*.exe