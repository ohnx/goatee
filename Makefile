INCLUDES=-Iinclude/ -Ilibs/lua/src -Ilibs/libstring/include
CFLAGS=$(INCLUDES) -Wall -Werror -pedantic
LUA_VERSION=5.2.4
LIBFLAGS=-Llibs/ -lstring -llua -lm -ldl

OBJ=objs/goatee_gen.o objs/goatee_run.o
OUTPUT=libgoatee.a

default: $(OUTPUT)
debug: CFLAGS += -g -O0 -D__DEBUG
debug: $(OUTPUT)


################################################################################
#                                LIBRARY STUFF                                 #
################################################################################

# lua 5.3.3
lua: libs/liblua.a

libs/liblua.a:
	@echo "Building lua...";
	@mkdir -p libs; \
	cd libs; \
	if [ ! -d "lua" ]; then \
	curl -R -O http://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz; \
	tar zxf lua-$(LUA_VERSION).tar.gz; \
	rm lua-$(LUA_VERSION).tar.gz; \
	mv lua-$(LUA_VERSION) lua; \
	cd lua; make generic; cd ..; \
	cp lua/src/*.a .; \
	cp lua/src/luac .; \
	cd ../; \
	rm -f luac; \
	fi;
	@cd libs/; \
	cp lua/src/*.a .; \
	cp lua/src/luac .;

libstring: libs/libstring.a

libs/libstring.a:
	@echo "Building libstring...";
	@mkdir -p libs; \
	cd libs; \
	git clone https://github.com/ohnx/libstring; \
	cd libstring; \
	make; \
	cp libstring.a ..;
	@cd libs; \
	cp libstring/libstring.a .; \

libs: lua libstring

cleanlibs: 
	rm -f libs/liblua.a libs/libstring.a


################################################################################
#                                SOURCES STUFF                                 #
################################################################################

objs/%.o: src/%.c
	@mkdir -p objs/
	$(CC) -c -o $@ $< $(CFLAGS) $(EXTRA)

$(OUTPUT): libs $(OBJ)
	ar rcs $(OUTPUT) $(OBJ)

clean:
	-rm -f $(OBJ)
	-rm -f $(OUTPUT)
	-rm -f tests/runner tests/runner.o


################################################################################
#                                 TESTS STUFF                                  #
################################################################################

tests/runner: debug tests/runner.c
	$(CC) -c tests/runner.c -o tests/runner.o $(CFLAGS) -g
	$(CC) tests/runner.o -L. -lgoatee $(LIBFLAGS) -o tests/runner

test: $(OUTPUT) tests/runner
	tests/runner tests/enginetest.in
