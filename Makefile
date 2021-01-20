# Broadcom Flash Writer Makefile
INCLUDES=-I include "-DUNUSED(x)=(void)(x)"
INDENT_FLAGS=-br -ce -i4 -bl -bli0 -bls -c4 -cdw -ci4 -cs -nbfda -l100 -lp -prs -nlp -nut -nbfde -npsl -nss

OBJS = \
	bin/main.o \
	bin/board_api.o \
	bin/info.o \
	bin/leds.o \
	bin/flash.o \
	bin/firmware.o \
	bin/crc32.o

all: host

internal: prepare
	@echo "  CC    src/main.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/main.c -o bin/main.o
	@echo "  CC    src/board_api.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/board_api.c -o bin/board_api.o
	@echo "  CC    src/info.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/info.c -o bin/info.o
	@echo "  CC    src/leds.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/leds.c -o bin/leds.o
	@echo "  CC    src/flash.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/flash.c -o bin/flash.o
	@echo "  CC    src/firmware.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/firmware.c -o bin/firmware.o
	@echo "  CC    src/crc32.c"
	@$(CC) $(CFLAGS) $(INCLUDES) src/crc32.c -o bin/crc32.o
	@echo "  LD    bin/brfw"
	@$(LD) -o bin/brfw $(OBJS) $(LDFLAGS)

prepare:
	@mkdir -p bin

host:
	@make internal \
		CC=gcc \
		LD=gcc \
		CFLAGS='-c -Wall -Wextra -Os -ffunction-sections -fdata-sections -Wstrict-prototypes -D_mmap=mmap' \
		LDFLAGS='-s -Wl,--gc-sections -Wl,--relax'

mipsel:
	@make internal \
		CC=mips-unknown-linux-gnu-gcc \
		LD=mips-unknown-linux-gnu-gcc \
		CFLAGS='-c $(MIPSEL_CFLAGS) -I $(ESLIB_INC) -Os -EL' \
		LDFLAGS='$(MIPSEL_LDFLAGS) -L $(ESLIB_DIR) -les-mipsel-Os -EL'

mipseb:
	@make internal \
		CC=mips-unknown-linux-gnu-gcc \
		LD=mips-unknown-linux-gnu-gcc \
		CFLAGS='-c $(MIPSEB_CFLAGS) -I $(ESLIB_INC) -Os -EB' \
		LDFLAGS='$(MIPSEB_LDFLAGS) -L $(ESLIB_DIR) -les-mipseb-Os -EB'

arm:
	@make internal \
		CC=arm-linux-gnueabi-gcc \
		LD=arm-linux-gnueabi-gcc \
		CFLAGS='-c $(ARM_CFLAGS) -I $(ESLIB_INC) -Os' \
		LDFLAGS='$(ARM_LDFLAGS) -L $(ESLIB_DIR) -les-arm-Os'

install:
	cp -v bin/brfw /usr/bin/brfw

uninstall:
	rm -fv /usr/bin/brfw

post:
	@echo "  STRIP brfw"
	@sstrip bin/brfw
	@echo "  UPX   brfw"
	@upx bin/brfw
	@echo "  LCK   brfw"
	@perl -pi -e 's/UPX!/EsNf/g' bin/brfw
	@echo "  AEM   brfw"
	@nogdb bin/brfw

post2:
	@echo "  STRIP brfw"
	@sstrip bin/brfw
	@echo "  AEM   brfw"
	@nogdb bin/brfw
	@echo "  STRIP brfw"
	@sstrip bin/brfw
	@echo "  AEM   brfw"
	@nogdb bin/brfw

indent:
	@indent $(INDENT_FLAGS) ./*/*.h
	@indent $(INDENT_FLAGS) ./*/*.c
	@rm -rf ./*/*~

clean:
	@echo "  CLEAN ."
	@rm -rf bin

analysis:
	@scan-build make
	@cppcheck --force */*.h
	@cppcheck --force */*.c

gendoc:
	@doxygen aux/doxygen.conf
