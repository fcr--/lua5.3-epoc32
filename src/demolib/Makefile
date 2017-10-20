# makefile for Lua demo libraries

LUA= ../..

CC= arm-epoc-pe-gcc -Znoemx
CXX= $(CC)
LD= arm-epoc-pe-gcc -Znoemx
CFLAGS= -O2 -Wall -fomit-frame-pointer
CXXFLAGS += -I..
DLLTOOL= arm-epoc-pe-dlltool

all: demolib.dll screenlib.dll

%.dll: %.o %.def
	$(CC) -shared -o $@ $^ -llua53 -loplr -lestlib -lcone -lws32 \
		-L.. -uid2 0x4c756121 -uid3 "0x10`echo $@ | md5sum | cut -c1-6`"

%.def: %.o
	$(DLLTOOL) -z $@ $^

clean:
	rm -f *.dll *.o *.def
