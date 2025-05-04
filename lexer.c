#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static int isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}

static char advance(Lexer* lexer) {
    lexer->column++;
    return *lexer->current++;
}

static char peek(Lexer* lexer) {
    return *lexer->current;
}

static char peekNext(Lexer* lexer) {
    if (isAtEnd(lexer)) return '\0';
    return lexer->current[1];
}

static int match(Lexer* lexer, char expected) {
    if (isAtEnd(lexer)) return 0;
    if (*lexer->current != expected) return 0;
    lexer->current++;
    lexer->column++;
    return 1;
}

void lexerError(Lexer* lexer, const char* message) {
    fprintf(stderr, "[line %d, column %d] Error: %s\n", lexer->line, lexer->column, message);
    lexer->hadError = 1;
}

static void skipWhitespace(Lexer* lexer) {
    for (;;) {
        char c = peek(lexer);
        switch (c) {
            case ' ':
            case '\t':
                advance(lexer);
                break;
            case '\n':
                lexer->line++;
                lexer->column = 0;
                advance(lexer);
                break;
            case '"': { // Comment
                advance(lexer); // Consume opening quote
                while (peek(lexer) != '"' && !isAtEnd(lexer)) {
                    if (peek(lexer) == '\n') {
                        lexer->line++;
                        lexer->column = 0;
                    }
                    advance(lexer);
                }
                
                if (isAtEnd(lexer)) {
                    lexerError(lexer, "Unterminated comment.");
                    return;
                }
                
                advance(lexer); // Consume closing quote
                break;
            }
            default:
                return;
        }
    }
}

// Removed unused checkKeyword function

static TokenType identifierType(const char* start, int length) {
    // Check for reserved words and pseudo-variables
    switch (start[0]) {
        case 'f':
            if (length == 5 && memcmp(start, "false", 5) == 0) return TOKEN_FALSE;
            break;
        case 'n':
            if (length == 3 && memcmp(start, "nil", 3) == 0) return TOKEN_NIL;
            break;
        case 's':
            if (length == 4 && memcmp(start, "self", 4) == 0) return TOKEN_SELF;
            if (length == 5 && memcmp(start, "super", 5) == 0) return TOKEN_SUPER;
            break;
        case 't':
            if (length == 4 && memcmp(start, "true", 4) == 0) return TOKEN_TRUE;
            if (length == 12 && memcmp(start, "thisContext", 12) == 0) return TOKEN_THIS_CONTEXT;
            break;
    }
    
    return TOKEN_IDENTIFIER;
}

static Token makeToken(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column - token.length;
    return token;
}

static Token errorToken(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

static Token identifier(Lexer* lexer) {
    while (isalnum(peek(lexer)) || peek(lexer) == '_') {
        advance(lexer);
    }
    
    // Check if it's a keyword (identifier followed by a colon)
    if (peek(lexer) == ':' && peekNext(lexer) != '=') {
        advance(lexer); // Consume the colon
        return makeToken(lexer, TOKEN_KEYWORD);
    }
    
    TokenType type = identifierType(lexer->start, lexer->current - lexer->start);
    return makeToken(lexer, type);
}

static Token number(Lexer* lexer) {
    int isNegative = 0;
    if (lexer->start[0] == '-') {
        isNegative = 1;
        advance(lexer); // Skip the minus sign
    }
    
    // Parse the integer part
    while (isdigit(peek(lexer))) {
        advance(lexer);
    }
    
    // Check for radix notation (e.g., 16r1A)
    if (peek(lexer) == 'r' && isdigit(lexer->start[0])) {
        // Get the radix
        char* endptr;
        long radix = strtol(lexer->start, &endptr, 10);
        if (radix < 2 || radix > 36) {
            return errorToken(lexer, "Invalid radix. Must be between 2 and 36.");
        }
        
        advance(lexer); // Skip 'r'
        
        // Parse the base-N integer
        while (isalnum(peek(lexer))) {
            char c = peek(lexer);
            int digit;
            
            if (isdigit(c)) {
                digit = c - '0';
            } else if (islower(c)) {
                digit = c - 'a' + 10;
            } else if (isupper(c)) {
                digit = c - 'A' + 10;
            } else {
                break;
            }
            
            if (digit >= radix) {
                return errorToken(lexer, "Digit out of range for specified radix.");
            }
            
            advance(lexer);
        }
        
        Token token = makeToken(lexer, TOKEN_INTEGER);
        // Compute the value
        token.value.intValue = strtoll(lexer->start, NULL, (int)radix);
        if (isNegative) token.value.intValue = -token.value.intValue;
        return token;
    }
    
    // Check for decimal point (floating point or scaled decimal)
    if (peek(lexer) == '.') {
        // In Smalltalk, we treat a trailing period after a number as a statement terminator,
        // not as a decimal point, if it's followed by whitespace or EOF
        if (peekNext(lexer) == ' ' || peekNext(lexer) == '\t' || 
            peekNext(lexer) == '\n' || peekNext(lexer) == '\0') {
            // Return the integer value without consuming the period
            Token token = makeToken(lexer, TOKEN_INTEGER);
            token.value.intValue = strtoll(lexer->start, NULL, 10);
            if (isNegative) token.value.intValue = -token.value.intValue;
            return token;
        }
        
        advance(lexer); // Skip the decimal point
        
        // Parse the fractional part
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
        
        // Check for scaled decimal (e.g., 123.45s2)
        if (peek(lexer) == 's') {
            advance(lexer); // Skip 's'
            
            // Parse the scale
            // Parse the scale (nothing to do with scaleStart)
            while (isdigit(peek(lexer))) {
                advance(lexer);
            }
            
            Token token = makeToken(lexer, TOKEN_SCALED);
            // TODO: Handle proper scaled decimal conversion
            token.value.floatValue = strtod(lexer->start, NULL);
            return token;
        }
        
        // Check for exponent (e.g., 123.45e6)
        if (peek(lexer) == 'e' || peek(lexer) == 'd' || peek(lexer) == 'q') {
            advance(lexer); // Skip the exponent indicator
            
            // Handle optional sign
            if (peek(lexer) == '+' || peek(lexer) == '-') {
                advance(lexer);
            }
            
            // Parse the exponent
            if (!isdigit(peek(lexer))) {
                return errorToken(lexer, "Expected digits after exponent.");
            }
            
            while (isdigit(peek(lexer))) {
                advance(lexer);
            }
        }
        
        Token token = makeToken(lexer, TOKEN_FLOAT);
        token.value.floatValue = strtod(lexer->start, NULL);
        return token;
    }
    
    // Check for exponent without decimal point (e.g., 123e6)
    if (peek(lexer) == 'e' || peek(lexer) == 'd' || peek(lexer) == 'q') {
        advance(lexer); // Skip the exponent indicator
        
        // Handle optional sign
        if (peek(lexer) == '+' || peek(lexer) == '-') {
            advance(lexer);
        }
        
        // Parse the exponent
        if (!isdigit(peek(lexer))) {
            return errorToken(lexer, "Expected digits after exponent.");
        }
        
        while (isdigit(peek(lexer))) {
            advance(lexer);
        }
        
        Token token = makeToken(lexer, TOKEN_FLOAT);
        token.value.floatValue = strtod(lexer->start, NULL);
        return token;
    }
    
    // Simple integer
    Token token = makeToken(lexer, TOKEN_INTEGER);
    token.value.intValue = strtoll(lexer->start, NULL, 10);
    if (isNegative) token.value.intValue = -token.value.intValue;
    return token;
}

static Token string(Lexer* lexer) {
    while (peek(lexer) != '\'' && !isAtEnd(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 0;
        }
        
        // Handle escape sequence for apostrophe
        if (peek(lexer) == '\'' && peekNext(lexer) == '\'') {
            advance(lexer); // Skip the first apostrophe
        }
        
        advance(lexer);
    }
    
    if (isAtEnd(lexer)) {
        return errorToken(lexer, "Unterminated string.");
    }
    
    advance(lexer); // Closing apostrophe
    return makeToken(lexer, TOKEN_STRING);
}

static Token character(Lexer* lexer) {
    if (isAtEnd(lexer)) {
        return errorToken(lexer, "Expected character after '$'.");
    }
    
    advance(lexer); // Consume the character
    
    Token token = makeToken(lexer, TOKEN_CHAR);
    token.value.charValue = *(lexer->current - 1);
    return token;
}

static Token symbol(Lexer* lexer) {
    // Remember the current position (to revert in case of errors)
    const char* startPos = lexer->current;
    
    // Skip the '#' (already consumed in nextToken)
    
    // Handle array literals like #(1 2 3)
    if (peek(lexer) == '(') {
        advance(lexer); // Skip the opening parenthesis
        return makeToken(lexer, TOKEN_HASH_PAREN);
    }
    // If followed by string literal, it's a #'symbol' syntax
    else if (peek(lexer) == '\'') {
        advance(lexer); // Skip the opening quote
        
        while (peek(lexer) != '\'' && !isAtEnd(lexer)) {
            if (peek(lexer) == '\n') {
                lexer->line++;
                lexer->column = 0;
            }
            
            // Handle escape sequence for apostrophe
            if (peek(lexer) == '\'' && peekNext(lexer) == '\'') {
                advance(lexer); // Skip the first apostrophe
            }
            
            advance(lexer);
        }
        
        if (isAtEnd(lexer)) {
            return errorToken(lexer, "Unterminated symbol string.");
        }
        
        advance(lexer); // Closing apostrophe
    } 
    else {
        // Otherwise it's a normal symbol (#symbol)
        if (!(isalpha(peek(lexer)) || peek(lexer) == '_' || 
              strchr("~!@%&*-+=|\\<>,?/", peek(lexer)) != NULL)) {
            // Invalid character after #, revert and return an error
            lexer->current = startPos;
            return errorToken(lexer, "Expected identifier, binary selector, single quote, or opening parenthesis after '#'.");
        }
        
        if (isalpha(peek(lexer)) || peek(lexer) == '_') {
            // Symbol is an identifier or keyword
            while (isalnum(peek(lexer)) || peek(lexer) == '_') {
                advance(lexer);
            }
            
            // Check if it's a keyword (ends with colon)
            if (peek(lexer) == ':') {
                advance(lexer); // Consume the colon
                
                // Handle multi-keyword selectors
                while (isalpha(peek(lexer)) || peek(lexer) == '_') {
                    while (isalnum(peek(lexer)) || peek(lexer) == '_') {
                        advance(lexer);
                    }
                    
                    if (peek(lexer) == ':') {
                        advance(lexer); // Consume the colon
                    } else {
                        break;
                    }
                }
            }
        } else {
            // Symbol is a binary selector
            while (strchr("~!@%&*-+=|\\<>,?/", peek(lexer)) != NULL) {
                advance(lexer);
                
                // Binary selectors are at most 2 characters
                if (lexer->current - lexer->start > 3) { // +1 for the '#' prefix
                    break;
                }
            }
        }
    }
    
    return makeToken(lexer, TOKEN_SYMBOL);
}

static Token binarySelector(Lexer* lexer) {
    // First binary character is already consumed
    
    // Check for second binary character, if any
    if (strchr("~!@%&*-+=|\\<>,?/", peek(lexer)) != NULL) {
        advance(lexer);
    }
    
    return makeToken(lexer, TOKEN_BINARY_SELECTOR);
}

void initLexer(Lexer* lexer, const char* source) {
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->hadError = 0;
}

Token nextToken(Lexer* lexer) {
    skipWhitespace(lexer);
    
    lexer->start = lexer->current;
    
    if (isAtEnd(lexer)) return makeToken(lexer, TOKEN_EOF);
    
    char c = advance(lexer);
    
    // Handle identifiers
    if (isalpha(c) || c == '_') {
        return identifier(lexer);
    }
    
    // Handle numbers
    if (isdigit(c) || (c == '-' && isdigit(peek(lexer)))) {
        return number(lexer);
    }
    
    switch (c) {
        case '(': return makeToken(lexer, TOKEN_LEFT_PAREN);
        case ')': return makeToken(lexer, TOKEN_RIGHT_PAREN);
        case '[': return makeToken(lexer, TOKEN_LEFT_BRACKET);
        case ']': return makeToken(lexer, TOKEN_RIGHT_BRACKET);
        case '{': return makeToken(lexer, TOKEN_LEFT_BRACE);
        case '}': return makeToken(lexer, TOKEN_RIGHT_BRACE);
        case '^': return makeToken(lexer, TOKEN_CARET);
        case '.': return makeToken(lexer, TOKEN_PERIOD);
        case ';': return makeToken(lexer, TOKEN_SEMICOLON);
        case '|': return makeToken(lexer, TOKEN_PIPE);
        case '#':
            // Remember current position for the symbol token
            lexer->start = lexer->current - 1; // -1 to include the # character
            return symbol(lexer);
        case '$': return character(lexer);
        case '\'': return string(lexer);
        case ':':
            if (match(lexer, '=')) {
                return makeToken(lexer, TOKEN_ASSIGNMENT);
            }
            return makeToken(lexer, TOKEN_COLON);
        case ',': return makeToken(lexer, TOKEN_COMMA);
        case '_': return makeToken(lexer, TOKEN_UNDERSCORE);
        
        // Binary selectors
        case '~': return makeToken(lexer, TOKEN_BINARY_SELECTOR);
        case '!': return makeToken(lexer, TOKEN_BINARY_SELECTOR);
        case '@': return makeToken(lexer, TOKEN_BINARY_SELECTOR);
        case '%': return makeToken(lexer, TOKEN_BINARY_SELECTOR);
        case '&': return makeToken(lexer, TOKEN_BINARY_SELECTOR);
        case '*': return makeToken(lexer, TOKEN_STAR);
        case '-': return makeToken(lexer, TOKEN_MINUS);
        case '+': return makeToken(lexer, TOKEN_PLUS);
        case '=': return makeToken(lexer, TOKEN_EQUAL);
        case '\\': return makeToken(lexer, TOKEN_BACKSLASH);
        case '<': return makeToken(lexer, TOKEN_LESS);
        case '>': return makeToken(lexer, TOKEN_GREATER);
        case '?': return makeToken(lexer, TOKEN_QUESTION);
        case '/': return makeToken(lexer, TOKEN_SLASH);
    }
    
    return errorToken(lexer, "Unexpected character.");
}