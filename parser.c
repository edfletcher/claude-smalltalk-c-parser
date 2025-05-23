#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void advance(Parser* parser) {
    parser->previous = parser->current;
    
    for (;;) {
        parser->current = nextToken(&parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;
        
        parserErrorAtCurrent(parser, parser->current.start);
    }
}

static void consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }
    
    parserErrorAtCurrent(parser, message);
}

static int check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

static int match(Parser* parser, TokenType type) {
    if (!check(parser, type)) return 0;
    // Debug to help diagnose issues with matching tokens
    printf("Matching token type: %d\n", type);
    advance(parser);
    return 1;
}

void parserError(Parser* parser, const char* message) {
    if (parser->panicMode) return;
    parser->panicMode = 1;
    
    fprintf(stderr, "[line %d, column %d] Error: %s\n", 
           parser->previous.line, parser->previous.column, message);
    parser->hadError = 1;
}

void parserErrorAtCurrent(Parser* parser, const char* message) {
    if (parser->panicMode) return;
    parser->panicMode = 1;
    
    fprintf(stderr, "[line %d, column %d] Error: %s\n", 
           parser->current.line, parser->current.column, message);
    parser->hadError = 1;
}

static char* extractTokenString(Token token) {
    char* str = (char*)malloc(token.length + 1);
    if (str == NULL) return NULL;
    
    memcpy(str, token.start, token.length);
    str[token.length] = '\0';
    
    return str;
}

// Forward declarations for parser functions
static ASTNode* expression(Parser* parser);
static ASTNode* statement(Parser* parser);
static ASTNode* blockBody(Parser* parser);
static ASTNode* primary(Parser* parser);
static ASTNode* parseMessageExpression(Parser* parser);

static ASTNode* primary(Parser* parser) {
    // Debug message to track token processing
    printf("Processing primary with token type: %d\n", parser->current.type);
    
    if (match(parser, TOKEN_LEFT_PAREN)) {
        ASTNode* expr = expression(parser);
        consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
        return expr;
    }
    
    if (match(parser, TOKEN_LEFT_BRACKET)) {
        // Parse a block
        
        // Parse parameters if present
        char** parameters = NULL;
        int parameterCount = 0;
        
        if (match(parser, TOKEN_COLON)) {
            // Block has parameters
            parameters = (char**)malloc(sizeof(char*) * 8); // Initial capacity
            if (parameters == NULL) {
                parserError(parser, "Out of memory.");
                return NULL;
            }
            
            // First parameter
            consume(parser, TOKEN_IDENTIFIER, "Expected parameter name after ':'.");
            parameters[parameterCount++] = extractTokenString(parser->previous);
            
            // Additional parameters
            while (match(parser, TOKEN_COLON)) {
                consume(parser, TOKEN_IDENTIFIER, "Expected parameter name after ':'.");
                
                // Grow the parameters array if needed
                if (parameterCount % 8 == 0) {
                    char** newParams = (char**)realloc(parameters, sizeof(char*) * (parameterCount + 8));
                    if (newParams == NULL) {
                        for (int i = 0; i < parameterCount; i++) free(parameters[i]);
                        free(parameters);
                        parserError(parser, "Out of memory.");
                        return NULL;
                    }
                    parameters = newParams;
                }
                
                parameters[parameterCount++] = extractTokenString(parser->previous);
            }
            
            // Block parameters are followed by a pipe
            consume(parser, TOKEN_PIPE, "Expected '|' after block parameters.");
        }
        
        // Parse the block body
        ASTNode** statements = NULL;
        int statementCount = 0;
        
        if (!check(parser, TOKEN_RIGHT_BRACKET)) {
            statements = (ASTNode**)malloc(sizeof(ASTNode*) * 8); // Initial capacity
            if (statements == NULL) {
                for (int i = 0; i < parameterCount; i++) free(parameters[i]);
                free(parameters);
                parserError(parser, "Out of memory.");
                return NULL;
            }
            
            // Parse statements
            do {
                statements[statementCount++] = statement(parser);
                
                // Grow the statements array if needed
                if (statementCount % 8 == 0) {
                    ASTNode** newStmts = (ASTNode**)realloc(statements, sizeof(ASTNode*) * (statementCount + 8));
                    if (newStmts == NULL) {
                        for (int i = 0; i < parameterCount; i++) free(parameters[i]);
                        free(parameters);
                        for (int i = 0; i < statementCount; i++) freeASTNode(statements[i]);
                        free(statements);
                        parserError(parser, "Out of memory.");
                        return NULL;
                    }
                    statements = newStmts;
                }
                
                // Statements are separated by periods
                if (!check(parser, TOKEN_PERIOD)) break;
                advance(parser); // Skip the period
            } while (!check(parser, TOKEN_RIGHT_BRACKET) && !check(parser, TOKEN_EOF));
        }
        
        consume(parser, TOKEN_RIGHT_BRACKET, "Expected ']' after block body.");
        
        return createBlockNode(parameters, parameterCount, statements, statementCount, 
                            parser->previous.line, parser->previous.column);
    }
    
    if (match(parser, TOKEN_LEFT_BRACE)) {
        // Parse an array expression
        
        ASTNode** expressions = NULL;
        int expressionCount = 0;
        
        if (!check(parser, TOKEN_RIGHT_BRACE)) {
            expressions = (ASTNode**)malloc(sizeof(ASTNode*) * 8); // Initial capacity
            if (expressions == NULL) {
                parserError(parser, "Out of memory.");
                return NULL;
            }
            
            // Parse expressions
            do {
                expressions[expressionCount++] = expression(parser);
                
                // Grow the expressions array if needed
                if (expressionCount % 8 == 0) {
                    ASTNode** newExprs = (ASTNode**)realloc(expressions, sizeof(ASTNode*) * (expressionCount + 8));
                    if (newExprs == NULL) {
                        for (int i = 0; i < expressionCount; i++) freeASTNode(expressions[i]);
                        free(expressions);
                        parserError(parser, "Out of memory.");
                        return NULL;
                    }
                    expressions = newExprs;
                }
                
                // Expressions are separated by periods
                if (!check(parser, TOKEN_PERIOD)) break;
                advance(parser); // Skip the period
            } while (!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF));
        }
        
        consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' after array expression.");
        
        return createArrayExpressionNode(expressions, expressionCount, 
                                    parser->previous.line, parser->previous.column);
    }
    
    // Handle array literals like #(1 2 3)
    if (match(parser, TOKEN_HASH_PAREN)) {
        ASTNode** elements = NULL;
        int elementCount = 0;
        
        if (!check(parser, TOKEN_RIGHT_PAREN)) {
            elements = (ASTNode**)malloc(sizeof(ASTNode*) * 8); // Initial capacity
            if (elements == NULL) {
                parserError(parser, "Out of memory.");
                return NULL;
            }
            
            // Parse array elements
            do {
                // Array literals can contain: integers, floats, strings, characters, and symbols
                if (match(parser, TOKEN_INTEGER)) {
                    elements[elementCount++] = createIntegerLiteral(
                        parser->previous.value.intValue,
                        parser->previous.line, parser->previous.column);
                } else if (match(parser, TOKEN_FLOAT)) {
                    elements[elementCount++] = createFloatLiteral(
                        parser->previous.value.floatValue,
                        parser->previous.line, parser->previous.column);
                } else if (match(parser, TOKEN_STRING)) {
                    char* str = extractTokenString(parser->previous);
                    // Remove the surrounding quotes
                    if (str[0] == '\'' && str[strlen(str) - 1] == '\'') {
                        memmove(str, str + 1, strlen(str) - 2);
                        str[strlen(str) - 2] = '\0';
                    }
                    elements[elementCount++] = createStringLiteral(str, 
                        parser->previous.line, parser->previous.column);
                } else if (match(parser, TOKEN_CHAR)) {
                    elements[elementCount++] = createCharacterLiteral(
                        parser->previous.value.charValue,
                        parser->previous.line, parser->previous.column);
                } else if (match(parser, TOKEN_SYMBOL)) {
                    char* str = extractTokenString(parser->previous);
                    // Remove the # prefix
                    if (str[0] == '#') {
                        memmove(str, str + 1, strlen(str));
                    }
                    elements[elementCount++] = createSymbolLiteral(str,
                        parser->previous.line, parser->previous.column);
                } else if (match(parser, TOKEN_IDENTIFIER)) {
                    // For keyword literals
                    char* str = extractTokenString(parser->previous);
                    elements[elementCount++] = createSymbolLiteral(str,
                        parser->previous.line, parser->previous.column);
                } else {
                    parserError(parser, "Expected literal value in array literal.");
                    for (int i = 0; i < elementCount; i++) freeASTNode(elements[i]);
                    free(elements);
                    return NULL;
                }
                
                // Grow the elements array if needed
                if (elementCount % 8 == 0) {
                    ASTNode** newElems = (ASTNode**)realloc(elements, sizeof(ASTNode*) * (elementCount + 8));
                    if (newElems == NULL) {
                        for (int i = 0; i < elementCount; i++) freeASTNode(elements[i]);
                        free(elements);
                        parserError(parser, "Out of memory.");
                        return NULL;
                    }
                    elements = newElems;
                }
                
                // Elements are separated by whitespace, no need for any separator token
            } while (!check(parser, TOKEN_RIGHT_PAREN) && !check(parser, TOKEN_EOF));
        }
        
        consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after array literal elements.");
        
        return createArrayLiteral(elements, elementCount,
                              parser->previous.line, parser->previous.column);
    }
    
    // Check for literals and variables
    if (match(parser, TOKEN_INTEGER) || match(parser, TOKEN_FLOAT) || 
        match(parser, TOKEN_SCALED) || match(parser, TOKEN_CHAR) || 
        match(parser, TOKEN_STRING) || match(parser, TOKEN_SYMBOL) ||
        match(parser, TOKEN_NIL) || match(parser, TOKEN_TRUE) || 
        match(parser, TOKEN_FALSE) || match(parser, TOKEN_SELF) || 
        match(parser, TOKEN_SUPER) || match(parser, TOKEN_THIS_CONTEXT)) {
        
        TokenType type = parser->previous.type;
        
        // Handle literals
        if (type == TOKEN_INTEGER) {
            return createIntegerLiteral(parser->previous.value.intValue, 
                                      parser->previous.line, parser->previous.column);
        } else if (type == TOKEN_FLOAT) {
            return createFloatLiteral(parser->previous.value.floatValue, 
                                    parser->previous.line, parser->previous.column);
        } else if (type == TOKEN_SCALED) {
            // Note: Scale information is not properly captured in the lexer yet
            return createScaledLiteral(parser->previous.value.floatValue, 0, 
                                     parser->previous.line, parser->previous.column);
        } else if (type == TOKEN_CHAR) {
            return createCharacterLiteral(parser->previous.value.charValue, 
                                        parser->previous.line, parser->previous.column);
        } else if (type == TOKEN_STRING) {
            char* str = extractTokenString(parser->previous);
            // Remove the surrounding quotes
            if (str[0] == '\'' && str[strlen(str) - 1] == '\'') {
                memmove(str, str + 1, strlen(str) - 2);
                str[strlen(str) - 2] = '\0';
            }
            return createStringLiteral(str, parser->previous.line, parser->previous.column);
        } else if (type == TOKEN_SYMBOL) {
            char* str = extractTokenString(parser->previous);
            // Remove the # prefix and, if present, surrounding quotes
            if (str[0] == '#') {
                if (str[1] == '\'') {
                    memmove(str, str + 2, strlen(str) - 3);
                    str[strlen(str) - 3] = '\0';
                } else {
                    memmove(str, str + 1, strlen(str));
                }
            }
            return createSymbolLiteral(str, parser->previous.line, parser->previous.column);
        } 
        
        // Handle constants and pseudo-variables
        else if (type == TOKEN_NIL || type == TOKEN_TRUE || type == TOKEN_FALSE) {
            return createConstantNode(type, parser->previous.line, parser->previous.column);
        } else if (type == TOKEN_SELF || type == TOKEN_SUPER || type == TOKEN_THIS_CONTEXT) {
            char* name = extractTokenString(parser->previous);
            return createVariableNode(name, 1, parser->previous.line, parser->previous.column);
        }
    }
    
    // Handle identifiers (variable references)
    if (match(parser, TOKEN_IDENTIFIER)) {
        char* name = extractTokenString(parser->previous);
        return createVariableNode(name, 0, parser->previous.line, parser->previous.column);
    }
    
    parserError(parser, "Expected expression.");
    return NULL;
}

static ASTNode* parseMessageExpression(Parser* parser) {
    ASTNode* receiver = primary(parser);
    
    // Parse any unary, binary, or keyword messages
    for (;;) {
        // Parse unary message
        if (match(parser, TOKEN_IDENTIFIER)) {
            char* selector = extractTokenString(parser->previous);
            receiver = createUnaryMessageNode(receiver, selector, 
                                           parser->previous.line, parser->previous.column);
        }
        // Parse binary message (specific tokens)
        else if (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS) || 
                 match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH) ||
                 match(parser, TOKEN_LESS) || match(parser, TOKEN_GREATER) ||
                 match(parser, TOKEN_EQUAL) || match(parser, TOKEN_AT) ||
                 match(parser, TOKEN_COMMA) || match(parser, TOKEN_TILDE) ||
                 match(parser, TOKEN_PERCENT) || match(parser, TOKEN_AMPERSAND) ||
                 match(parser, TOKEN_QUESTION) || match(parser, TOKEN_EXCLAMATION) ||
                 match(parser, TOKEN_BACKSLASH) || match(parser, TOKEN_BINARY_SELECTOR)) {
            
            char* selector = extractTokenString(parser->previous);
            ASTNode* argument = primary(parser);
            int selectorLine = parser->previous.line;
            int selectorColumn = parser->previous.column;
            
            receiver = createBinaryMessageNode(receiver, selector, argument, 
                                            selectorLine, selectorColumn);
        }
        // Parse keyword message
        else if (match(parser, TOKEN_KEYWORD)) {
            char selectorBuffer[256] = {0}; // Buffer for building the full selector
            ASTNode** arguments = (ASTNode**)malloc(sizeof(ASTNode*) * 8); // Initial capacity
            if (arguments == NULL) {
                parserError(parser, "Out of memory.");
                return NULL;
            }
            
            int argumentCount = 0;
            
            // First keyword and argument
            strncat(selectorBuffer, parser->previous.start, parser->previous.length);
            
            // Parse the argument
            ASTNode* argument = primary(parser);
            arguments[argumentCount++] = argument;
            
            // Additional keywords and arguments
            while (match(parser, TOKEN_KEYWORD)) {
                strncat(selectorBuffer, parser->previous.start, parser->previous.length);
                
                // Parse the argument
                argument = primary(parser);
                
                // Grow the arguments array if needed
                if (argumentCount % 8 == 0) {
                    ASTNode** newArgs = (ASTNode**)realloc(arguments, sizeof(ASTNode*) * (argumentCount + 8));
                    if (newArgs == NULL) {
                        for (int i = 0; i < argumentCount; i++) freeASTNode(arguments[i]);
                        free(arguments);
                        parserError(parser, "Out of memory.");
                        return NULL;
                    }
                    arguments = newArgs;
                }
                
                arguments[argumentCount++] = argument;
            }
            
            receiver = createKeywordMessageNode(receiver, selectorBuffer, arguments, argumentCount, 
                                            parser->previous.line, parser->previous.column);
        }
        // Handle cascade (semicolon)
        else if (match(parser, TOKEN_SEMICOLON)) {
            ASTNode* cascadeReceiver = receiver;
            ASTNode** messages = (ASTNode**)malloc(sizeof(ASTNode*) * 8); // Initial capacity
            if (messages == NULL) {
                parserError(parser, "Out of memory.");
                return NULL;
            }
            
            int messageCount = 0;
            
            // Parse cascade messages
            do {
                ASTNode* message = NULL;
                
                // Parse each message without receiver (it will be set in the cascade node)
                if (match(parser, TOKEN_IDENTIFIER)) {
                    char* selector = extractTokenString(parser->previous);
                    message = createUnaryMessageNode(NULL, selector, 
                                                  parser->previous.line, parser->previous.column);
                }
                else if (match(parser, TOKEN_PLUS) || match(parser, TOKEN_MINUS) || 
                         match(parser, TOKEN_STAR) || match(parser, TOKEN_SLASH) ||
                         match(parser, TOKEN_LESS) || match(parser, TOKEN_GREATER) ||
                         match(parser, TOKEN_EQUAL) || match(parser, TOKEN_AT) ||
                         match(parser, TOKEN_COMMA) || match(parser, TOKEN_TILDE) ||
                         match(parser, TOKEN_PERCENT) || match(parser, TOKEN_AMPERSAND) ||
                         match(parser, TOKEN_QUESTION) || match(parser, TOKEN_EXCLAMATION) ||
                         match(parser, TOKEN_BACKSLASH) || match(parser, TOKEN_BINARY_SELECTOR)) {
                    
                    char* selector = extractTokenString(parser->previous);
                    ASTNode* argument = primary(parser);
                    
                    message = createBinaryMessageNode(NULL, selector, argument, 
                                                   parser->previous.line, parser->previous.column);
                }
                else if (match(parser, TOKEN_KEYWORD)) {
                    // Handle keyword message in cascade
                    char selectorBuffer[256] = {0};
                    ASTNode** arguments = (ASTNode**)malloc(sizeof(ASTNode*) * 8);
                    if (arguments == NULL) {
                        for (int i = 0; i < messageCount; i++) freeASTNode(messages[i]);
                        free(messages);
                        parserError(parser, "Out of memory.");
                        return NULL;
                    }
                    
                    int argumentCount = 0;
                    
                    // First keyword and argument
                    strncat(selectorBuffer, parser->previous.start, parser->previous.length);
                    ASTNode* argument = primary(parser);
                    arguments[argumentCount++] = argument;
                    
                    // Additional keywords and arguments
                    while (match(parser, TOKEN_KEYWORD)) {
                        strncat(selectorBuffer, parser->previous.start, parser->previous.length);
                        argument = primary(parser);
                        
                        // Grow arguments if needed
                        if (argumentCount % 8 == 0) {
                            ASTNode** newArgs = (ASTNode**)realloc(arguments, sizeof(ASTNode*) * (argumentCount + 8));
                            if (newArgs == NULL) {
                                for (int i = 0; i < argumentCount; i++) freeASTNode(arguments[i]);
                                free(arguments);
                                for (int i = 0; i < messageCount; i++) freeASTNode(messages[i]);
                                free(messages);
                                parserError(parser, "Out of memory.");
                                return NULL;
                            }
                            arguments = newArgs;
                        }
                        
                        arguments[argumentCount++] = argument;
                    }
                    
                    message = createKeywordMessageNode(NULL, selectorBuffer, arguments, argumentCount, 
                                                    parser->previous.line, parser->previous.column);
                }
                else {
                    parserError(parser, "Expected message selector in cascade.");
                    for (int i = 0; i < messageCount; i++) freeASTNode(messages[i]);
                    free(messages);
                    return NULL;
                }
                
                // Store the message
                if (messageCount % 8 == 0 && messageCount > 0) {
                    ASTNode** newMsgs = (ASTNode**)realloc(messages, sizeof(ASTNode*) * (messageCount + 8));
                    if (newMsgs == NULL) {
                        for (int i = 0; i < messageCount; i++) freeASTNode(messages[i]);
                        free(messages);
                        parserError(parser, "Out of memory.");
                        return NULL;
                    }
                    messages = newMsgs;
                }
                
                messages[messageCount++] = message;
            } while (match(parser, TOKEN_SEMICOLON));
            
            // Create the cascade node
            return createCascadeNode(cascadeReceiver, messages, messageCount, 
                                 parser->previous.line, parser->previous.column);
        }
        else {
            // No more messages to parse
            break;
        }
    }
    
    return receiver;
}

static ASTNode* assignment(Parser* parser) {
    if (check(parser, TOKEN_IDENTIFIER)) {
        Token identifier = parser->current;
        advance(parser); // Consume the identifier
        
        if (match(parser, TOKEN_ASSIGNMENT)) {
            ASTNode* value = expression(parser);
            
            char* variableName = extractTokenString(identifier);
            return createAssignmentNode(variableName, value, 
                                     identifier.line, identifier.column);
        } else {
            // Not an assignment, so backtrack
            parser->current = identifier;
        }
    }
    
    return parseMessageExpression(parser);
}

static ASTNode* expression(Parser* parser) {
    if (match(parser, TOKEN_CARET)) {
        ASTNode* expr = expression(parser);
        return createReturnNode(expr, parser->previous.line, parser->previous.column);
    }
    
    ASTNode* expr = assignment(parser);
    
    // Debug print to see the current token type after expression parsing
    printf("After expression, current token type: %d\n", parser->current.type);
    
    return expr;
}

static ASTNode* statement(Parser* parser) {
    ASTNode* expr = expression(parser);
    
    // Debug print for statement parsing
    printf("Parsed statement, current token type: %d\n", parser->current.type);
    
    return expr;
}

static ASTNode* blockBody(Parser* parser) {
    ASTNode** statements = (ASTNode**)malloc(sizeof(ASTNode*) * 8); // Initial capacity
    if (statements == NULL) {
        parserError(parser, "Out of memory.");
        return NULL;
    }
    
    int statementCount = 0;
    
    while (!check(parser, TOKEN_EOF)) {
        // Skip any periods at the beginning (can happen with comments)
        while (match(parser, TOKEN_PERIOD));
        
        // If we've reached EOF after skipping periods, we're done
        if (check(parser, TOKEN_EOF)) break;
        
        // Parse the statement
        ASTNode* expr = statement(parser);
        statements[statementCount++] = expr;
        
        // Grow the statements array if needed
        if (statementCount % 8 == 0) {
            ASTNode** newStmts = (ASTNode**)realloc(statements, sizeof(ASTNode*) * (statementCount + 8));
            if (newStmts == NULL) {
                for (int i = 0; i < statementCount; i++) freeASTNode(statements[i]);
                free(statements);
                parserError(parser, "Out of memory.");
                return NULL;
            }
            statements = newStmts;
        }
        
        // After each statement, we require a period (unless EOF)
        if (check(parser, TOKEN_EOF)) break;
        
        // If next token is not a period, check if it's part of expression that needs period
        if (!check(parser, TOKEN_PERIOD)) {
            // Debug print to help diagnose the issue
            printf("Error: Expected period, but got token type: %d\n", parser->current.type);
            parserErrorAtCurrent(parser, "Expected '.' after statement.");
            break;
        }
        
        // Consume the period
        advance(parser);
    }
    
    // Create a block node without parameters
    return createBlockNode(NULL, 0, statements, statementCount, 
                        parser->previous.line, parser->previous.column);
}

void initParser(Parser* parser, const char* source) {
    initLexer(&parser->lexer, source);
    parser->hadError = 0;
    parser->panicMode = 0;
    
    advance(parser); // Prime the parser by loading the first token
}

ASTNode* parse(Parser* parser) {
    ASTNode* node = blockBody(parser);
    
    if (!parser->hadError) {
        consume(parser, TOKEN_EOF, "Expected end of expression.");
    }
    
    return node;
}