#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }
    
    // Read the file
    FILE* file = fopen(argv[1], "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", argv[1]);
        return 1;
    }
    
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    
    char* source = (char*)malloc(fileSize + 1);
    if (source == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", argv[1]);
        fclose(file);
        return 1;
    }
    
    size_t bytesRead = fread(source, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", argv[1]);
        free(source);
        fclose(file);
        return 1;
    }
    
    source[bytesRead] = '\0';
    fclose(file);
    
    // Initialize lexer
    Lexer lexer;
    initLexer(&lexer, source);
    
    // Print header
    printf("%-20s %-30s %-5s %-5s %-5s\n", "Token Type", "Value", "Line", "Col", "Type#");
    printf("-------------------------------------------------------------------\n");
    
    // Print all tokens
    Token token;
    do {
        token = nextToken(&lexer);
        
        char tokenValue[32] = {0};
        if (token.length < 30) {
            strncpy(tokenValue, token.start, token.length);
            tokenValue[token.length] = '\0';
        } else {
            strncpy(tokenValue, token.start, 27);
            strcat(tokenValue, "...");
        }
        
        char tokenTypeName[32] = {0};
        switch (token.type) {
            case TOKEN_EOF: strcpy(tokenTypeName, "EOF"); break;
            case TOKEN_ERROR: strcpy(tokenTypeName, "ERROR"); break;
            case TOKEN_IDENTIFIER: strcpy(tokenTypeName, "IDENTIFIER"); break;
            case TOKEN_KEYWORD: strcpy(tokenTypeName, "KEYWORD"); break;
            case TOKEN_INTEGER: strcpy(tokenTypeName, "INTEGER"); break;
            case TOKEN_FLOAT: strcpy(tokenTypeName, "FLOAT"); break;
            case TOKEN_SCALED: strcpy(tokenTypeName, "SCALED"); break;
            case TOKEN_CHAR: strcpy(tokenTypeName, "CHAR"); break;
            case TOKEN_STRING: strcpy(tokenTypeName, "STRING"); break;
            case TOKEN_SYMBOL: strcpy(tokenTypeName, "SYMBOL"); break;
            case TOKEN_HASH_PAREN: strcpy(tokenTypeName, "HASH_PAREN"); break;
            case TOKEN_NIL: strcpy(tokenTypeName, "NIL"); break;
            case TOKEN_TRUE: strcpy(tokenTypeName, "TRUE"); break;
            case TOKEN_FALSE: strcpy(tokenTypeName, "FALSE"); break;
            case TOKEN_SELF: strcpy(tokenTypeName, "SELF"); break;
            case TOKEN_SUPER: strcpy(tokenTypeName, "SUPER"); break;
            case TOKEN_THIS_CONTEXT: strcpy(tokenTypeName, "THIS_CONTEXT"); break;
            case TOKEN_BINARY_SELECTOR: strcpy(tokenTypeName, "BINARY_SELECTOR"); break;
            case TOKEN_PERIOD: strcpy(tokenTypeName, "PERIOD"); break;
            case TOKEN_SEMICOLON: strcpy(tokenTypeName, "SEMICOLON"); break;
            case TOKEN_LEFT_PAREN: strcpy(tokenTypeName, "LEFT_PAREN"); break;
            case TOKEN_RIGHT_PAREN: strcpy(tokenTypeName, "RIGHT_PAREN"); break;
            case TOKEN_LEFT_BRACKET: strcpy(tokenTypeName, "LEFT_BRACKET"); break;
            case TOKEN_RIGHT_BRACKET: strcpy(tokenTypeName, "RIGHT_BRACKET"); break;
            case TOKEN_LEFT_BRACE: strcpy(tokenTypeName, "LEFT_BRACE"); break;
            case TOKEN_RIGHT_BRACE: strcpy(tokenTypeName, "RIGHT_BRACE"); break;
            case TOKEN_CARET: strcpy(tokenTypeName, "CARET"); break;
            case TOKEN_PIPE: strcpy(tokenTypeName, "PIPE"); break;
            case TOKEN_ASSIGNMENT: strcpy(tokenTypeName, "ASSIGNMENT"); break;
            case TOKEN_HASH: strcpy(tokenTypeName, "HASH"); break;
            case TOKEN_DOLLAR: strcpy(tokenTypeName, "DOLLAR"); break;
            case TOKEN_COLON: strcpy(tokenTypeName, "COLON"); break;
            case TOKEN_MINUS: strcpy(tokenTypeName, "MINUS"); break;
            case TOKEN_PLUS: strcpy(tokenTypeName, "PLUS"); break;
            case TOKEN_STAR: strcpy(tokenTypeName, "STAR"); break;
            case TOKEN_SLASH: strcpy(tokenTypeName, "SLASH"); break;
            case TOKEN_LESS: strcpy(tokenTypeName, "LESS"); break;
            case TOKEN_GREATER: strcpy(tokenTypeName, "GREATER"); break;
            case TOKEN_EQUAL: strcpy(tokenTypeName, "EQUAL"); break;
            case TOKEN_AT: strcpy(tokenTypeName, "AT"); break;
            case TOKEN_COMMA: strcpy(tokenTypeName, "COMMA"); break;
            case TOKEN_UNDERSCORE: strcpy(tokenTypeName, "UNDERSCORE"); break;
            case TOKEN_TILDE: strcpy(tokenTypeName, "TILDE"); break;
            case TOKEN_PERCENT: strcpy(tokenTypeName, "PERCENT"); break;
            case TOKEN_AMPERSAND: strcpy(tokenTypeName, "AMPERSAND"); break;
            case TOKEN_QUESTION: strcpy(tokenTypeName, "QUESTION"); break;
            case TOKEN_EXCLAMATION: strcpy(tokenTypeName, "EXCLAMATION"); break;
            case TOKEN_BACKSLASH: strcpy(tokenTypeName, "BACKSLASH"); break;
            default: strcpy(tokenTypeName, "UNKNOWN"); break;
        }
        
        printf("%-20s %-30s %-5d %-5d %-5d\n", tokenTypeName, tokenValue, token.line, token.column, token.type);
        
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);
    
    free(source);
    return 0;
}