# Broadcom Flash Writer Makefile
INCLUDES=-I include "-DUNUSED(x)=(void)(x)"
INDENT_FLAGS=-br -ce -i4 -bl -bli0 -bls -c4 -cdw -ci4 -cs -nbfda -l100 -lp -prs -nlp -nut -nbfde -npsl -nss

OBJS = \
	release/main.o \
	release/board_api.o \
	release/info.o \
	release/leds.o \
	release/flash.o \
	release/firmware.o \
	release/crc32.o

all: host

internal: prepare
	@echo "  CC    src/main.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/main.c -o release/main.o
	@echo "  CC    src/board_api.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/board_api.c -o release/board_api.o
	@echo "  CC    src/info.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/info.c -o release/info.o
	@echo "  CC    src/leds.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/leds.c -o release/leds.o
	@echo "  CC    src/flash.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/flash.c -o release/flash.o
	@echo "  CC    src/firmware.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/firmware.c -o release/firmware.o
	@echo "  CC    src/crc32.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/crc32.c -o release/crc32.o
	@echo "  LD    release/brfw"
	@$(LD) -o release/brfw $(OBJS) $(LDFLAGS)

prepare:
	@mkdir -p release

host:
	@make internal \
		CC=gcc \
		LD=gcc \
		CFLAGS='-c -Wall -Wextra -Os -ffunction-sections -fdata-sections -Wstrict-prototypes -D_mmap=mmap' \
		LDFLAGS='-s -Wl,--gc-sections -Wl,--relax'

install:
	cp -v release/brfw /usr/bin/brfw

uninstall:
	rm -fv /usr/bin/brfw

indent:
	@indent $(INDENT_FLAGS) ./*/*.h
	@indent $(INDENT_FLAGS) ./*/*.c
	@rm -rf ./*/*~

clean:
	@echo "  CLEAN ."
	@rm -rf release

analysis:
	@scan-build make
	@cppcheck --force */*.h
	@cppcheck --force */*.c

gendoc:
	@doxygen aux/doxygen.conf
