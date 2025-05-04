#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer lexer;
    Token current;
    Token previous;
    int hadError;
    int panicMode;
} Parser;

void initParser(Parser* parser, const char* source);
ASTNode* parse(Parser* parser);
void parserError(Parser* parser, const char* message);
void parserErrorAtCurrent(Parser* parser, const char* message);

#endif /* PARSER_H */