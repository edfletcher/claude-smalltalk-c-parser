#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* allocateNode(size_t size, ASTNodeType type, int line, int column) {
    ASTNode* node = (ASTNode*)malloc(size);
    if (node == NULL) return NULL;
    
    node->type = type;
    node->line = line;
    node->column = column;
    
    return node;
}

ASTNode* createIntegerLiteral(long long value, int line, int column) {
    ASTIntegerLiteral* node = (ASTIntegerLiteral*)allocateNode(sizeof(ASTIntegerLiteral), AST_LITERAL_INTEGER, line, column);
    if (node == NULL) return NULL;
    
    node->value = value;
    
    return (ASTNode*)node;
}

ASTNode* createFloatLiteral(double value, int line, int column) {
    ASTFloatLiteral* node = (ASTFloatLiteral*)allocateNode(sizeof(ASTFloatLiteral), AST_LITERAL_FLOAT, line, column);
    if (node == NULL) return NULL;
    
    node->value = value;
    
    return (ASTNode*)node;
}

ASTNode* createScaledLiteral(double value, int scale, int line, int column) {
    ASTScaledLiteral* node = (ASTScaledLiteral*)allocateNode(sizeof(ASTScaledLiteral), AST_LITERAL_SCALED, line, column);
    if (node == NULL) return NULL;
    
    node->value = value;
    node->scale = scale;
    
    return (ASTNode*)node;
}

ASTNode* createCharacterLiteral(char value, int line, int column) {
    ASTCharacterLiteral* node = (ASTCharacterLiteral*)allocateNode(sizeof(ASTCharacterLiteral), AST_LITERAL_CHARACTER, line, column);
    if (node == NULL) return NULL;
    
    node->value = value;
    
    return (ASTNode*)node;
}

ASTNode* createStringLiteral(const char* value, int line, int column) {
    ASTStringLiteral* node = (ASTStringLiteral*)allocateNode(sizeof(ASTStringLiteral), AST_LITERAL_STRING, line, column);
    if (node == NULL) return NULL;
    
    node->value = strdup(value);
    if (node->value == NULL) {
        free(node);
        return NULL;
    }
    
    return (ASTNode*)node;
}

ASTNode* createSymbolLiteral(const char* value, int line, int column) {
    ASTSymbolLiteral* node = (ASTSymbolLiteral*)allocateNode(sizeof(ASTSymbolLiteral), AST_LITERAL_SYMBOL, line, column);
    if (node == NULL) return NULL;
    
    node->value = strdup(value);
    if (node->value == NULL) {
        free(node);
        return NULL;
    }
    
    return (ASTNode*)node;
}

ASTNode* createArrayLiteral(ASTNode** elements, int count, int line, int column) {
    ASTArrayLiteral* node = (ASTArrayLiteral*)allocateNode(sizeof(ASTArrayLiteral), AST_LITERAL_ARRAY, line, column);
    if (node == NULL) return NULL;
    
    node->elements = (ASTNode**)malloc(sizeof(ASTNode*) * count);
    if (node->elements == NULL) {
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        node->elements[i] = elements[i];
    }
    node->count = count;
    
    return (ASTNode*)node;
}

ASTNode* createByteArrayLiteral(unsigned char* bytes, int count, int line, int column) {
    ASTByteArrayLiteral* node = (ASTByteArrayLiteral*)allocateNode(sizeof(ASTByteArrayLiteral), AST_LITERAL_BYTE_ARRAY, line, column);
    if (node == NULL) return NULL;
    
    node->bytes = (unsigned char*)malloc(count);
    if (node->bytes == NULL) {
        free(node);
        return NULL;
    }
    
    memcpy(node->bytes, bytes, count);
    node->count = count;
    
    return (ASTNode*)node;
}

ASTNode* createConstantNode(TokenType type, int line, int column) {
    ASTConstantNode* node = (ASTConstantNode*)allocateNode(sizeof(ASTConstantNode), AST_CONSTANT, line, column);
    if (node == NULL) return NULL;
    
    node->type = type;
    
    return (ASTNode*)node;
}

ASTNode* createVariableNode(const char* name, int isPseudoVariable, int line, int column) {
    ASTVariableNode* node = (ASTVariableNode*)allocateNode(sizeof(ASTVariableNode), AST_VARIABLE, line, column);
    if (node == NULL) return NULL;
    
    node->name = strdup(name);
    if (node->name == NULL) {
        free(node);
        return NULL;
    }
    
    node->isPseudoVariable = isPseudoVariable;
    
    return (ASTNode*)node;
}

ASTNode* createAssignmentNode(const char* variable, ASTNode* value, int line, int column) {
    ASTAssignmentNode* node = (ASTAssignmentNode*)allocateNode(sizeof(ASTAssignmentNode), AST_ASSIGNMENT, line, column);
    if (node == NULL) return NULL;
    
    node->variable = strdup(variable);
    if (node->variable == NULL) {
        free(node);
        return NULL;
    }
    
    node->value = value;
    
    return (ASTNode*)node;
}

ASTNode* createReturnNode(ASTNode* expression, int line, int column) {
    ASTReturnNode* node = (ASTReturnNode*)allocateNode(sizeof(ASTReturnNode), AST_RETURN, line, column);
    if (node == NULL) return NULL;
    
    node->expression = expression;
    
    return (ASTNode*)node;
}

ASTNode* createUnaryMessageNode(ASTNode* receiver, const char* selector, int line, int column) {
    ASTUnaryMessageNode* node = (ASTUnaryMessageNode*)allocateNode(sizeof(ASTUnaryMessageNode), AST_MESSAGE_UNARY, line, column);
    if (node == NULL) return NULL;
    
    node->receiver = receiver;
    
    node->selector = strdup(selector);
    if (node->selector == NULL) {
        free(node);
        return NULL;
    }
    
    return (ASTNode*)node;
}

ASTNode* createBinaryMessageNode(ASTNode* receiver, const char* selector, ASTNode* argument, int line, int column) {
    ASTBinaryMessageNode* node = (ASTBinaryMessageNode*)allocateNode(sizeof(ASTBinaryMessageNode), AST_MESSAGE_BINARY, line, column);
    if (node == NULL) return NULL;
    
    node->receiver = receiver;
    
    node->selector = strdup(selector);
    if (node->selector == NULL) {
        free(node);
        return NULL;
    }
    
    node->argument = argument;
    
    return (ASTNode*)node;
}

ASTNode* createKeywordMessageNode(ASTNode* receiver, const char* selector, ASTNode** arguments, int argumentCount, int line, int column) {
    ASTKeywordMessageNode* node = (ASTKeywordMessageNode*)allocateNode(sizeof(ASTKeywordMessageNode), AST_MESSAGE_KEYWORD, line, column);
    if (node == NULL) return NULL;
    
    node->receiver = receiver;
    
    node->selector = strdup(selector);
    if (node->selector == NULL) {
        free(node);
        return NULL;
    }
    
    node->arguments = (ASTNode**)malloc(sizeof(ASTNode*) * argumentCount);
    if (node->arguments == NULL) {
        free(node->selector);
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < argumentCount; i++) {
        node->arguments[i] = arguments[i];
    }
    node->argumentCount = argumentCount;
    
    return (ASTNode*)node;
}

ASTNode* createCascadeNode(ASTNode* receiver, ASTNode** messages, int messageCount, int line, int column) {
    ASTCascadeNode* node = (ASTCascadeNode*)allocateNode(sizeof(ASTCascadeNode), AST_CASCADE, line, column);
    if (node == NULL) return NULL;
    
    node->receiver = receiver;
    
    node->messages = (ASTNode**)malloc(sizeof(ASTNode*) * messageCount);
    if (node->messages == NULL) {
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < messageCount; i++) {
        node->messages[i] = messages[i];
    }
    node->messageCount = messageCount;
    
    return (ASTNode*)node;
}

ASTNode* createBlockNode(char** parameters, int parameterCount, ASTNode** statements, int statementCount, int line, int column) {
    ASTBlockNode* node = (ASTBlockNode*)allocateNode(sizeof(ASTBlockNode), AST_BLOCK, line, column);
    if (node == NULL) return NULL;
    
    node->parameters = (char**)malloc(sizeof(char*) * parameterCount);
    if (node->parameters == NULL) {
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < parameterCount; i++) {
        node->parameters[i] = strdup(parameters[i]);
        if (node->parameters[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(node->parameters[j]);
            }
            free(node->parameters);
            free(node);
            return NULL;
        }
    }
    node->parameterCount = parameterCount;
    
    node->statements = (ASTNode**)malloc(sizeof(ASTNode*) * statementCount);
    if (node->statements == NULL) {
        for (int i = 0; i < parameterCount; i++) {
            free(node->parameters[i]);
        }
        free(node->parameters);
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < statementCount; i++) {
        node->statements[i] = statements[i];
    }
    node->statementCount = statementCount;
    
    return (ASTNode*)node;
}

ASTNode* createArrayExpressionNode(ASTNode** expressions, int count, int line, int column) {
    ASTArrayExpressionNode* node = (ASTArrayExpressionNode*)allocateNode(sizeof(ASTArrayExpressionNode), AST_ARRAY_EXPRESSION, line, column);
    if (node == NULL) return NULL;
    
    node->expressions = (ASTNode**)malloc(sizeof(ASTNode*) * count);
    if (node->expressions == NULL) {
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        node->expressions[i] = expressions[i];
    }
    node->count = count;
    
    return (ASTNode*)node;
}

ASTNode* createMethodNode(const char* selector, char** parameters, int parameterCount, 
                         ASTNode** statements, int statementCount, int isPrimitive, 
                         int primitiveNumber, int line, int column) {
    ASTMethodNode* node = (ASTMethodNode*)allocateNode(sizeof(ASTMethodNode), AST_METHOD, line, column);
    if (node == NULL) return NULL;
    
    node->selector = strdup(selector);
    if (node->selector == NULL) {
        free(node);
        return NULL;
    }
    
    node->parameters = (char**)malloc(sizeof(char*) * parameterCount);
    if (node->parameters == NULL) {
        free(node->selector);
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < parameterCount; i++) {
        node->parameters[i] = strdup(parameters[i]);
        if (node->parameters[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(node->parameters[j]);
            }
            free(node->parameters);
            free(node->selector);
            free(node);
            return NULL;
        }
    }
    node->parameterCount = parameterCount;
    
    node->statements = (ASTNode**)malloc(sizeof(ASTNode*) * statementCount);
    if (node->statements == NULL) {
        for (int i = 0; i < parameterCount; i++) {
            free(node->parameters[i]);
        }
        free(node->parameters);
        free(node->selector);
        free(node);
        return NULL;
    }
    
    for (int i = 0; i < statementCount; i++) {
        node->statements[i] = statements[i];
    }
    node->statementCount = statementCount;
    
    node->isPrimitive = isPrimitive;
    node->primitiveNumber = primitiveNumber;
    
    return (ASTNode*)node;
}

void freeASTNode(ASTNode* node) {
    if (node == NULL) return;
    
    switch (node->type) {
        case AST_LITERAL_STRING: {
            ASTStringLiteral* stringNode = (ASTStringLiteral*)node;
            free(stringNode->value);
            break;
        }
        case AST_LITERAL_SYMBOL: {
            ASTSymbolLiteral* symbolNode = (ASTSymbolLiteral*)node;
            free(symbolNode->value);
            break;
        }
        case AST_LITERAL_ARRAY: {
            ASTArrayLiteral* arrayNode = (ASTArrayLiteral*)node;
            for (int i = 0; i < arrayNode->count; i++) {
                freeASTNode(arrayNode->elements[i]);
            }
            free(arrayNode->elements);
            break;
        }
        case AST_LITERAL_BYTE_ARRAY: {
            ASTByteArrayLiteral* byteArrayNode = (ASTByteArrayLiteral*)node;
            free(byteArrayNode->bytes);
            break;
        }
        case AST_VARIABLE: {
            ASTVariableNode* variableNode = (ASTVariableNode*)node;
            free(variableNode->name);
            break;
        }
        case AST_ASSIGNMENT: {
            ASTAssignmentNode* assignmentNode = (ASTAssignmentNode*)node;
            free(assignmentNode->variable);
            freeASTNode(assignmentNode->value);
            break;
        }
        case AST_RETURN: {
            ASTReturnNode* returnNode = (ASTReturnNode*)node;
            freeASTNode(returnNode->expression);
            break;
        }
        case AST_MESSAGE_UNARY: {
            ASTUnaryMessageNode* messageNode = (ASTUnaryMessageNode*)node;
            freeASTNode(messageNode->receiver);
            free(messageNode->selector);
            break;
        }
        case AST_MESSAGE_BINARY: {
            ASTBinaryMessageNode* messageNode = (ASTBinaryMessageNode*)node;
            freeASTNode(messageNode->receiver);
            free(messageNode->selector);
            freeASTNode(messageNode->argument);
            break;
        }
        case AST_MESSAGE_KEYWORD: {
            ASTKeywordMessageNode* messageNode = (ASTKeywordMessageNode*)node;
            freeASTNode(messageNode->receiver);
            free(messageNode->selector);
            for (int i = 0; i < messageNode->argumentCount; i++) {
                freeASTNode(messageNode->arguments[i]);
            }
            free(messageNode->arguments);
            break;
        }
        case AST_CASCADE: {
            ASTCascadeNode* cascadeNode = (ASTCascadeNode*)node;
            freeASTNode(cascadeNode->receiver);
            for (int i = 0; i < cascadeNode->messageCount; i++) {
                freeASTNode(cascadeNode->messages[i]);
            }
            free(cascadeNode->messages);
            break;
        }
        case AST_BLOCK: {
            ASTBlockNode* blockNode = (ASTBlockNode*)node;
            for (int i = 0; i < blockNode->parameterCount; i++) {
                free(blockNode->parameters[i]);
            }
            free(blockNode->parameters);
            for (int i = 0; i < blockNode->statementCount; i++) {
                freeASTNode(blockNode->statements[i]);
            }
            free(blockNode->statements);
            break;
        }
        case AST_ARRAY_EXPRESSION: {
            ASTArrayExpressionNode* arrayNode = (ASTArrayExpressionNode*)node;
            for (int i = 0; i < arrayNode->count; i++) {
                freeASTNode(arrayNode->expressions[i]);
            }
            free(arrayNode->expressions);
            break;
        }
        case AST_METHOD: {
            ASTMethodNode* methodNode = (ASTMethodNode*)node;
            free(methodNode->selector);
            for (int i = 0; i < methodNode->parameterCount; i++) {
                free(methodNode->parameters[i]);
            }
            free(methodNode->parameters);
            for (int i = 0; i < methodNode->statementCount; i++) {
                freeASTNode(methodNode->statements[i]);
            }
            free(methodNode->statements);
            break;
        }
        default:
            break;
    }
    
    free(node);
}