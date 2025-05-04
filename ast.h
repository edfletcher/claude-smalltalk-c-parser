#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include "token.h"

typedef enum {
    AST_LITERAL_INTEGER,
    AST_LITERAL_FLOAT,
    AST_LITERAL_SCALED,
    AST_LITERAL_CHARACTER,
    AST_LITERAL_STRING,
    AST_LITERAL_SYMBOL,
    AST_LITERAL_ARRAY,
    AST_LITERAL_BYTE_ARRAY,
    AST_CONSTANT,
    AST_VARIABLE,
    AST_ASSIGNMENT,
    AST_RETURN,
    AST_MESSAGE_UNARY,
    AST_MESSAGE_BINARY,
    AST_MESSAGE_KEYWORD,
    AST_CASCADE,
    AST_BLOCK,
    AST_ARRAY_EXPRESSION,
    AST_METHOD
} ASTNodeType;

typedef struct ASTNode ASTNode;

/* Base AST node structure */
struct ASTNode {
    ASTNodeType type;
    int line;
    int column;
};

/* Literal node types */
typedef struct {
    ASTNode base;
    long long value;
} ASTIntegerLiteral;

typedef struct {
    ASTNode base;
    double value;
} ASTFloatLiteral;

typedef struct {
    ASTNode base;
    double value;
    int scale;
} ASTScaledLiteral;

typedef struct {
    ASTNode base;
    char value;
} ASTCharacterLiteral;

typedef struct {
    ASTNode base;
    char* value;
} ASTStringLiteral;

typedef struct {
    ASTNode base;
    char* value;
} ASTSymbolLiteral;

typedef struct {
    ASTNode base;
    ASTNode** elements;
    int count;
} ASTArrayLiteral;

typedef struct {
    ASTNode base;
    unsigned char* bytes;
    int count;
} ASTByteArrayLiteral;

/* Variable and constant reference nodes */
typedef struct {
    ASTNode base;
    TokenType type;  /* TOKEN_NIL, TOKEN_TRUE, TOKEN_FALSE */
} ASTConstantNode;

typedef struct {
    ASTNode base;
    char* name;
    int isPseudoVariable;  /* 1 for self, super, thisContext; 0 otherwise */
} ASTVariableNode;

/* Assignment node */
typedef struct {
    ASTNode base;
    char* variable;
    ASTNode* value;
} ASTAssignmentNode;

/* Return node */
typedef struct {
    ASTNode base;
    ASTNode* expression;
} ASTReturnNode;

/* Message nodes */
typedef struct {
    ASTNode base;
    ASTNode* receiver;
    char* selector;
} ASTUnaryMessageNode;

typedef struct {
    ASTNode base;
    ASTNode* receiver;
    char* selector;
    ASTNode* argument;
} ASTBinaryMessageNode;

typedef struct {
    ASTNode base;
    ASTNode* receiver;
    char* selector;
    ASTNode** arguments;
    int argumentCount;
} ASTKeywordMessageNode;

/* Cascade node */
typedef struct {
    ASTNode base;
    ASTNode* receiver;
    ASTNode** messages;
    int messageCount;
} ASTCascadeNode;

/* Block node */
typedef struct {
    ASTNode base;
    char** parameters;
    int parameterCount;
    ASTNode** statements;
    int statementCount;
} ASTBlockNode;

/* Array expression node */
typedef struct {
    ASTNode base;
    ASTNode** expressions;
    int count;
} ASTArrayExpressionNode;

/* Method node */
typedef struct {
    ASTNode base;
    char* selector;
    char** parameters;
    int parameterCount;
    ASTNode** statements;
    int statementCount;
    int isPrimitive;
    int primitiveNumber;
} ASTMethodNode;

/* AST node creation functions */
ASTNode* createIntegerLiteral(long long value, int line, int column);
ASTNode* createFloatLiteral(double value, int line, int column);
ASTNode* createScaledLiteral(double value, int scale, int line, int column);
ASTNode* createCharacterLiteral(char value, int line, int column);
ASTNode* createStringLiteral(const char* value, int line, int column);
ASTNode* createSymbolLiteral(const char* value, int line, int column);
ASTNode* createArrayLiteral(ASTNode** elements, int count, int line, int column);
ASTNode* createByteArrayLiteral(unsigned char* bytes, int count, int line, int column);
ASTNode* createConstantNode(TokenType type, int line, int column);
ASTNode* createVariableNode(const char* name, int isPseudoVariable, int line, int column);
ASTNode* createAssignmentNode(const char* variable, ASTNode* value, int line, int column);
ASTNode* createReturnNode(ASTNode* expression, int line, int column);
ASTNode* createUnaryMessageNode(ASTNode* receiver, const char* selector, int line, int column);
ASTNode* createBinaryMessageNode(ASTNode* receiver, const char* selector, ASTNode* argument, int line, int column);
ASTNode* createKeywordMessageNode(ASTNode* receiver, const char* selector, ASTNode** arguments, int argumentCount, int line, int column);
ASTNode* createCascadeNode(ASTNode* receiver, ASTNode** messages, int messageCount, int line, int column);
ASTNode* createBlockNode(char** parameters, int parameterCount, ASTNode** statements, int statementCount, int line, int column);
ASTNode* createArrayExpressionNode(ASTNode** expressions, int count, int line, int column);
ASTNode* createMethodNode(const char* selector, char** parameters, int parameterCount, 
                         ASTNode** statements, int statementCount, int isPrimitive, 
                         int primitiveNumber, int line, int column);

/* AST management functions */
ASTNode* allocateNode(size_t size, ASTNodeType type, int line, int column);
void freeASTNode(ASTNode* node);

#endif /* AST_H */