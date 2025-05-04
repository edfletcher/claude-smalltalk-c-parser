# Smalltalk Parser

This is a parser for the Smalltalk programming language implemented in C. It parses Smalltalk code according to the grammar defined in the "Smalltalk in One Page" specification.

## Project Structure

- `token.h` - Token type definitions
- `lexer.h` / `lexer.c` - Lexical analyzer that converts source code into tokens
- `ast.h` / `ast.c` - Abstract Syntax Tree (AST) node definitions and functions
- `parser.h` / `parser.c` - Parser that builds an AST from tokens
- `smalltalk_parser.c` - Main program entry point
- `Makefile` - Build configuration
- `sample.st` - Sample Smalltalk program for testing

## Building

To build the parser, run:

```
make
```

This will produce an executable called `smalltalk_parser`.

## Running

To parse a Smalltalk source file and generate an AST:

```
./smalltalk_parser your_file.st
```

To display the tokens produced by the lexer:

```
./smalltalk_parser --tokens your_file.st
```

To run the parser on the included sample file:

```
make test
```

## Features

This parser supports most of the core Smalltalk syntax, including:

- Literals (integers, floats, scaled decimals, characters, strings, symbols, arrays)
- Variables and assignments
- Message sending (unary, binary, and keyword messages)
- Cascaded messages
- Blocks with parameters
- Return statements

## Limitations

The current implementation has the following limitations:

- No semantic analysis
- No symbol table or scope tracking
- No execution engine
- Limited error recovery
- No optimization

## Grammar

The parser implements the Smalltalk grammar as defined in the "Smalltalk in One Page" specification, which includes the full syntax for expressions, statements, and method definitions.

## License

This project is open source and free to use for any purpose.