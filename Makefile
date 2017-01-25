INCLUDES=-I lua/
CFLAGS=$(INCLUDES) -Wall -Werror -pedantic
LIBFLAGS=-Llua -llua -lm -ldl
OBJ=objs/luat_file.o

LUA_VERSION=5.2.4

default: libgoatee.a
debug: CFLAGS += -g -O0 -D__DEBUG
debug: libgoatee.a

# lua 5.3.3
lua:
	curl -R -O http://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz
	tar zxf lua-$(LUA_VERSION).tar.gz
	rm lua-$(LUA_VERSION).tar.gz
	mv lua-$(LUA_VERSION) lua
	cd lua; make generic
	cp lua/src/*.a lua/
	cp lua/src/luac lua/

mkobjs:
	mkdir -p objs

# requires packages liblua5.2-dev and lua5.2 (5.2 or any version)
objs/goatee.o: src/goatee.lua
	mkdir -p objs
	lua/luac -o objs/goatee.luac src/goatee.lua
	xxd -i objs/goatee.luac src/goatee.c
	$(CC) -c -o objs/goatee.o src/goatee.c

objs/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(EXTRA)

libgoatee.a: mkobjs $(OBJ)
	$(CC) -o $(OUTPUT) $(OBJ) $(LIBFLAGS) $(CFLAGS)
	-rm -f $(OBJ) objs/luat_file.luac

clean:
	-rm -f $(OBJ) objs/luat_file.luac src/luat_file.c
	-rm -f $(OUTPUT)