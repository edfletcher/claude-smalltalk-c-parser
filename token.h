#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    /* Basic tokens */
    TOKEN_EOF,
    TOKEN_ERROR,
    
    /* Identifiers and references */
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,      /* Identifier with colon */
    
    /* Literals */
    TOKEN_INTEGER,      /* Integer literal */
    TOKEN_FLOAT,        /* Floating point literal */
    TOKEN_SCALED,       /* Scaled decimal literal */
    TOKEN_CHAR,         /* Character literal */
    TOKEN_STRING,       /* String literal */
    TOKEN_SYMBOL,       /* Symbol literal */
    
    /* Constants */
    TOKEN_NIL,          /* nil */
    TOKEN_TRUE,         /* true */
    TOKEN_FALSE,        /* false */
    
    /* Pseudo-variables */
    TOKEN_SELF,         /* self */
    TOKEN_SUPER,        /* super */
    TOKEN_THIS_CONTEXT, /* thisContext */
    
    /* Binary selectors */
    TOKEN_BINARY_SELECTOR,
    
    /* Punctuation */
    TOKEN_PERIOD,       /* . */
    TOKEN_SEMICOLON,    /* ; */
    TOKEN_LEFT_PAREN,   /* ( */
    TOKEN_RIGHT_PAREN,  /* ) */
    TOKEN_LEFT_BRACKET, /* [ */
    TOKEN_RIGHT_BRACKET,/* ] */
    TOKEN_LEFT_BRACE,   /* { */
    TOKEN_RIGHT_BRACE,  /* } */
    TOKEN_CARET,        /* ^ */
    TOKEN_PIPE,         /* | */
    TOKEN_ASSIGNMENT,   /* := */
    TOKEN_HASH,         /* # */
    TOKEN_DOLLAR,       /* $ */
    TOKEN_COLON,        /* : */
    TOKEN_MINUS,        /* - */
    TOKEN_PLUS,         /* + */
    TOKEN_STAR,         /* * */
    TOKEN_SLASH,        /* / */
    TOKEN_LESS,         /* < */
    TOKEN_GREATER,      /* > */
    TOKEN_EQUAL,        /* = */
    TOKEN_AT,           /* @ */
    TOKEN_COMMA,        /* , */
    TOKEN_UNDERSCORE,   /* _ */
    TOKEN_TILDE,        /* ~ */
    TOKEN_PERCENT,      /* % */
    TOKEN_AMPERSAND,    /* & */
    TOKEN_QUESTION,     /* ? */
    TOKEN_EXCLAMATION,  /* ! */
    TOKEN_BACKSLASH     /* \ */
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
    int column;
    union {
        long long intValue;
        double floatValue;
        char charValue;
    } value;
} Token;

#endif /* TOKEN_H */