CC = clang
CFLAGS = -Wall -Wextra -I./raylib-5.0_linux_amd64/include/
LIB_PATH = -L./raylib-5.0_linux_amd64/lib/
LIBS = -l:libraylib.a -lm -ldl -lpthread

BIN_DIR = ./bin
SRC_DIR = ./src

first: 
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) ./breakout.c  -o $(BIN_DIR)/breakout $(LIB_PATH) $(LIBS) && ./bin/breakout


clean:
	rm -rf $(BIN_DIR)

.PHONY: clean $(BIN_DIR)
