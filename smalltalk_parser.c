#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

// Function to print AST nodes with indentation
void printAST(ASTNode* node, int indent) {
    if (node == NULL) return;
    
    char indentStr[128] = {0};
    for (int i = 0; i < indent; i++) {
        strcat(indentStr, "  ");
    }
    
    switch (node->type) {
        case AST_LITERAL_INTEGER: {
            ASTIntegerLiteral* intNode = (ASTIntegerLiteral*)node;
            printf("%sInteger: %lld\n", indentStr, intNode->value);
            break;
        }
        case AST_LITERAL_FLOAT: {
            ASTFloatLiteral* floatNode = (ASTFloatLiteral*)node;
            printf("%sFloat: %f\n", indentStr, floatNode->value);
            break;
        }
        case AST_LITERAL_SCALED: {
            ASTScaledLiteral* scaledNode = (ASTScaledLiteral*)node;
            printf("%sScaled: %f s%d\n", indentStr, scaledNode->value, scaledNode->scale);
            break;
        }
        case AST_LITERAL_CHARACTER: {
            ASTCharacterLiteral* charNode = (ASTCharacterLiteral*)node;
            printf("%sCharacter: '%c'\n", indentStr, charNode->value);
            break;
        }
        case AST_LITERAL_STRING: {
            ASTStringLiteral* stringNode = (ASTStringLiteral*)node;
            printf("%sString: '%s'\n", indentStr, stringNode->value);
            break;
        }
        case AST_LITERAL_SYMBOL: {
            ASTSymbolLiteral* symbolNode = (ASTSymbolLiteral*)node;
            printf("%sSymbol: #%s\n", indentStr, symbolNode->value);
            break;
        }
        case AST_LITERAL_ARRAY: {
            ASTArrayLiteral* arrayNode = (ASTArrayLiteral*)node;
            printf("%sArray: #(\n", indentStr);
            for (int i = 0; i < arrayNode->count; i++) {
                printAST(arrayNode->elements[i], indent + 1);
            }
            printf("%s)\n", indentStr);
            break;
        }
        case AST_LITERAL_BYTE_ARRAY: {
            ASTByteArrayLiteral* byteArrayNode = (ASTByteArrayLiteral*)node;
            printf("%sByteArray: #[\n", indentStr);
            for (int i = 0; i < byteArrayNode->count; i++) {
                printf("%s  %d\n", indentStr, byteArrayNode->bytes[i]);
            }
            printf("%s]\n", indentStr);
            break;
        }
        case AST_CONSTANT: {
            ASTConstantNode* constNode = (ASTConstantNode*)node;
            printf("%sConstant: ", indentStr);
            switch (constNode->type) {
                case TOKEN_NIL:
                    printf("nil\n");
                    break;
                case TOKEN_TRUE:
                    printf("true\n");
                    break;
                case TOKEN_FALSE:
                    printf("false\n");
                    break;
                default:
                    printf("unknown\n");
                    break;
            }
            break;
        }
        case AST_VARIABLE: {
            ASTVariableNode* varNode = (ASTVariableNode*)node;
            if (varNode->isPseudoVariable) {
                printf("%sPseudoVariable: %s\n", indentStr, varNode->name);
            } else {
                printf("%sVariable: %s\n", indentStr, varNode->name);
            }
            break;
        }
        case AST_ASSIGNMENT: {
            ASTAssignmentNode* assignNode = (ASTAssignmentNode*)node;
            printf("%sAssignment:\n", indentStr);
            printf("%s  Variable: %s\n", indentStr, assignNode->variable);
            printf("%s  Value:\n", indentStr);
            printAST(assignNode->value, indent + 2);
            break;
        }
        case AST_RETURN: {
            ASTReturnNode* returnNode = (ASTReturnNode*)node;
            printf("%sReturn:\n", indentStr);
            printAST(returnNode->expression, indent + 1);
            break;
        }
        case AST_MESSAGE_UNARY: {
            ASTUnaryMessageNode* msgNode = (ASTUnaryMessageNode*)node;
            printf("%sUnaryMessage:\n", indentStr);
            printf("%s  Selector: %s\n", indentStr, msgNode->selector);
            printf("%s  Receiver:\n", indentStr);
            printAST(msgNode->receiver, indent + 2);
            break;
        }
        case AST_MESSAGE_BINARY: {
            ASTBinaryMessageNode* msgNode = (ASTBinaryMessageNode*)node;
            printf("%sBinaryMessage:\n", indentStr);
            printf("%s  Selector: %s\n", indentStr, msgNode->selector);
            printf("%s  Receiver:\n", indentStr);
            printAST(msgNode->receiver, indent + 2);
            printf("%s  Argument:\n", indentStr);
            printAST(msgNode->argument, indent + 2);
            break;
        }
        case AST_MESSAGE_KEYWORD: {
            ASTKeywordMessageNode* msgNode = (ASTKeywordMessageNode*)node;
            printf("%sKeywordMessage:\n", indentStr);
            printf("%s  Selector: %s\n", indentStr, msgNode->selector);
            printf("%s  Receiver:\n", indentStr);
            printAST(msgNode->receiver, indent + 2);
            printf("%s  Arguments:\n", indentStr);
            for (int i = 0; i < msgNode->argumentCount; i++) {
                printAST(msgNode->arguments[i], indent + 2);
            }
            break;
        }
        case AST_CASCADE: {
            ASTCascadeNode* cascadeNode = (ASTCascadeNode*)node;
            printf("%sCascade:\n", indentStr);
            printf("%s  Receiver:\n", indentStr);
            printAST(cascadeNode->receiver, indent + 2);
            printf("%s  Messages:\n", indentStr);
            for (int i = 0; i < cascadeNode->messageCount; i++) {
                printAST(cascadeNode->messages[i], indent + 2);
            }
            break;
        }
        case AST_BLOCK: {
            ASTBlockNode* blockNode = (ASTBlockNode*)node;
            printf("%sBlock:\n", indentStr);
            if (blockNode->parameterCount > 0) {
                printf("%s  Parameters: [", indentStr);
                for (int i = 0; i < blockNode->parameterCount; i++) {
                    printf("%s", blockNode->parameters[i]);
                    if (i < blockNode->parameterCount - 1) {
                        printf(", ");
                    }
                }
                printf("]\n");
            }
            printf("%s  Statements:\n", indentStr);
            for (int i = 0; i < blockNode->statementCount; i++) {
                printAST(blockNode->statements[i], indent + 2);
            }
            break;
        }
        case AST_ARRAY_EXPRESSION: {
            ASTArrayExpressionNode* arrayNode = (ASTArrayExpressionNode*)node;
            printf("%sArrayExpression: {\n", indentStr);
            for (int i = 0; i < arrayNode->count; i++) {
                printAST(arrayNode->expressions[i], indent + 1);
            }
            printf("%s}\n", indentStr);
            break;
        }
        case AST_METHOD: {
            ASTMethodNode* methodNode = (ASTMethodNode*)node;
            printf("%sMethod:\n", indentStr);
            printf("%s  Selector: %s\n", indentStr, methodNode->selector);
            if (methodNode->parameterCount > 0) {
                printf("%s  Parameters: [", indentStr);
                for (int i = 0; i < methodNode->parameterCount; i++) {
                    printf("%s", methodNode->parameters[i]);
                    if (i < methodNode->parameterCount - 1) {
                        printf(", ");
                    }
                }
                printf("]\n");
            }
            if (methodNode->isPrimitive) {
                printf("%s  Primitive: %d\n", indentStr, methodNode->primitiveNumber);
            }
            printf("%s  Statements:\n", indentStr);
            for (int i = 0; i < methodNode->statementCount; i++) {
                printAST(methodNode->statements[i], indent + 2);
            }
            break;
        }
        default:
            printf("%sUnknown node type: %d\n", indentStr, node->type);
            break;
    }
}

// Function to read a file into a string
char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        return NULL;
    }
    
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        fclose(file);
        return NULL;
    }
    
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    buffer[bytesRead] = '\0';
    
    fclose(file);
    return buffer;
}

void printUsage(char* programName) {
    printf("Usage: %s [options] <file>\n", programName);
    printf("Options:\n");
    printf("  -h, --help     Display this help message\n");
    printf("  --tokens       Display tokens only\n");
    printf("  --ast          Display AST only (default)\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Expected a source file argument.\n");
        printUsage(argv[0]);
        return 1;
    }
    
    int showTokens = 0;
    int showAST = 1;
    char* filePath = NULL;
    
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--tokens") == 0) {
            showTokens = 1;
            showAST = 0;
        } else if (strcmp(argv[i], "--ast") == 0) {
            showAST = 1;
        } else {
            filePath = argv[i];
        }
    }
    
    if (filePath == NULL) {
        fprintf(stderr, "No source file specified.\n");
        printUsage(argv[0]);
        return 1;
    }
    
    char* source = readFile(filePath);
    if (source == NULL) {
        return 1;
    }
    
    if (showTokens) {
        // Initialize lexer and print all tokens
        Lexer lexer;
        initLexer(&lexer, source);
        
        printf("Tokens from %s:\n", filePath);
        printf("%-20s %-30s %-5s %-5s\n", "Token Type", "Value", "Line", "Col");
        printf("------------------------------------------------------------\n");
        
        for (;;) {
            Token token = nextToken(&lexer);
            
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
            
            printf("%-20s %-30s %-5d %-5d\n", tokenTypeName, tokenValue, token.line, token.column);
            
            if (token.type == TOKEN_EOF) break;
            if (token.type == TOKEN_ERROR) {
                fprintf(stderr, "Error: %s\n", tokenValue);
                break;
            }
        }
    }
    
    if (showAST) {
        // Initialize parser and parse the source
        Parser parser;
        initParser(&parser, source);
        
        ASTNode* ast = parse(&parser);
        
        if (!parser.hadError && ast != NULL) {
            printf("Abstract Syntax Tree for %s:\n", filePath);
            printAST(ast, 0);
            freeASTNode(ast);
        } else {
            fprintf(stderr, "Failed to parse %s.\n", filePath);
        }
    }
    
    free(source);
    return 0;
}