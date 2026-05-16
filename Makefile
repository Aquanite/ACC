CC 	   		  := cc
CFLAGS 		  := -O2 -std=c11 -Wall -Wextra -Werror -Wpedantic
CFILES 		  := src/main.c src/token.c src/lexer.c src/parser.c src/backends/arm64.c src/backends/backend.c src/backends/x86_64.c lib/libstellar/src/memory.c
TARGET_FOLDER := build
TARGET 		  := $(TARGET_FOLDER)/acc
INCLUDES	  := -Iinclude -Ilib/libstellar/include

all: $(TARGET)

$(TARGET):
	mkdir $(TARGET_FOLDER)
	$(CC) $(INCLUDES) -o $(TARGET) $(CFILES) $(CFLAGS)

clean:
	rm -rf $(TARGET_FOLDER)

test: $(TARGET)
	./$(TARGET) -marm64 -o test/00/arm64.s test/00/00.c
	./$(TARGET) -mx86_64 -o test/00/x86_64.s test/00/00.c


.PHONY: clean test