# makefile for Lua OPX

LUA= ../..

CC= arm-epoc-pe-gcc -Znoemx
CXX= $(CC)
LD= arm-epoc-pe-gcc -Znoemx
CFLAGS= -O2 -Wall -fomit-frame-pointer
AR= arm-epoc-pe-ar rcu
RANLIB= arm-epoc-pe-ranlib

OBJS= luaopx.o
OPX= Lua53.opx
SRCS= luaopx.cpp
CXXFLAGS += -I..

T= libluaopx.a

$(OPX): $T

all: $(OPX)

$T: $(OBJS)
	$(AR) $@ $(OBJS)
	$(RANLIB) $@

$(OPX): $T
	$(CC) -shared -o Lua53opx.dll libluaopx.a opx.def -llua53 -loplr -L.. -uid2 0x1000005d -uid3 0x105DD51D -lestlib
	mv Lua53opx.dll Lua53.opx

clean:
	rm -f $(OBJS) $T $(OPX)
