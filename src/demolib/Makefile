# makefile for Lua demo libraries

LUA= ../..

CC= arm-epoc-pe-gcc -Znoemx
CXX= $(CC)
LD= arm-epoc-pe-gcc -Znoemx
CFLAGS= -O2 -Wall -fomit-frame-pointer
AR= arm-epoc-pe-ar rcu
RANLIB= arm-epoc-pe-ranlib
CXXFLAGS += -I..
DLLTOOL= arm-epoc-pe-dlltool

T= libluaopx.a

all: demolib.dll

%.a: %.o
	$(AR) $@ $^
	$(RANLIB) $@

%.dll: %.a %.def
	$(CC) -shared -o $@ $^ -llua53 -loplr -L.. -uid2 0x4c756121 -uid3 0x10481a89 -lestlib

%.def: %.a
	$(DLLTOOL) -z $@ $^

clean:
	rm -f *.dll *.o *.a *.def
