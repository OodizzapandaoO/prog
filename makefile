SRC_DIR= ./src/
BIN_DIR= ./bin/

ALL:
	gcc -Wall \
$(SRC_DIR)main.c $(SRC_DIR)code_parser.c $(SRC_DIR)formatting_logic.c \
-o $(BIN_DIR)main.out

win:
	gcc \
$(SRC_DIR)main.c $(SRC_DIR)code_parser.c $(SRC_DIR)formatting_logic.c \
-o $(BIN_DIR)main.exe

test:
	$(BIN_DIR)main $(BIN_DIR)test.c stdout