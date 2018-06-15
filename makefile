SRC_DIR= ./src/

ALL:
	gcc -Wall \
$(SRC_DIR)main.c $(SRC_DIR)code_parser.c $(SRC_DIR)formatting_logic.c \
-o main.out

win:
	gcc \
$(SRC_DIR)main.c $(SRC_DIR)code_parser.c $(SRC_DIR)formatting_logic.c \
-o main.exe

test:
	./main test.c stdout