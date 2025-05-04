CC = gcc
CFLAGS = -Wall -Wextra -g
OBJECTS = lexer.o parser.o ast.o smalltalk_parser.o

all: smalltalk_parser

smalltalk_parser: $(OBJECTS)
	$(CC) $(CFLAGS) -o smalltalk_parser $(OBJECTS)

lexer.o: lexer.c lexer.h token.h
	$(CC) $(CFLAGS) -c lexer.c

parser.o: parser.c parser.h lexer.h ast.h
	$(CC) $(CFLAGS) -c parser.c

ast.o: ast.c ast.h token.h
	$(CC) $(CFLAGS) -c ast.c

smalltalk_parser.o: smalltalk_parser.c lexer.h parser.h ast.h
	$(CC) $(CFLAGS) -c smalltalk_parser.c

clean:
	rm -f *.o smalltalk_parser

test: smalltalk_parser
	./smalltalk_parser sample.st

tokens: smalltalk_parser
	./smalltalk_parser --tokens sample.st