SRCPATHS = \
	src/init/*.c \
	src/render/*.c \
	src/util/*.c \
	src/game/*.c \

SRC = $(wildcard $(SRCPATHS))
OBJECTS = $(SRC:src/%.c=build/%.o)

EXECUTABLE_NAME = infection

CC = gcc
LD = gcc

INCLUDEPATHS = -Isrc
DEBUGFLAGS = -g -rdynamic
ASANFLAGS = -fsanitize=undefined -fsanitize=address
CFLAGS = -std=c2x -MD -D_XOPEN_SOURCE=700 -fwrapv \
		 -fno-delete-null-pointer-checks -fno-strict-overflow -fno-strict-aliasing \
		 -Wall -Wno-format -Wno-unused -Werror=incompatible-pointer-types -Wno-discarded-qualifiers \
		 -Isrc/ -lncurses -Wno-char-subscripts

OPT = -O0

FILE_NUM = 0

build/%.o: src/%.c
	$(eval FILE_NUM=$(shell echo $$(($(FILE_NUM)+1))))
	$(shell echo 1>&2 -e "\e[0m[\e[32m$(FILE_NUM)/$(words $(SRC))\e[0m]\t Compiling \e[1m$<\e[0m")
	@$(CC) -c -o $@ $< $(INCLUDEPATHS) $(CFLAGS) $(OPT)

build: $(OBJECTS)
	@echo Linking with $(LD)...
	@$(LD) $(OBJECTS) -o $(EXECUTABLE_NAME) $(CFLAGS) -lm
	@echo Successfully built: $(EXECUTABLE_NAME)

debug: CFLAGS += $(DEBUGFLAGS)
debug: OPT = -O0
debug: build

clean:
	@rm -rf build/
	@mkdir build/
	@mkdir -p $(dir $(OBJECTS))

cleanbuild: clean build

run:
	./desertbus


-include $(OBJECTS:.o=.d)