/* Tokenizer for the Scheme interpreter. */
/* Sarah LeBlanc, Pat Dale, Thor Laack-Veeder */
%{

/* DEFINITIONS - This section contains C code to be included before
   yylex(). You can use it to include libraries, define helper
   functions, etc. */

#include <stdio.h>

#include "common.h"

ConsCell *tokens = NULL;
Value currentToken;
ConsCell *tail = NULL;
ConsCell *tempToken1 = NULL;


void addCurrentToken() {
    /* Add currentToken to the list. */
    ConsCell *token = (ConsCell *) malloc(sizeof(ConsCell));
    token->car = currentToken;
    token->cdr.type = consType;
    token->cdr.consValue = NULL;
    tempToken1 = token;

    if (tokens == NULL) {
        tokens = tempToken1;
        tail = tempToken1;
    } else {
        tail->cdr.consValue = tempToken1;
        tail = tempToken1;
    }
}



%}

/* OPTIONS - Comment these lines out in order to get the yywrap, input
   or unput functions. */
%option noyywrap
%option nounput
%option noinput

%%

 /* RULES */

\( {
    /* open paren */
    currentToken.type = openType;
    currentToken.openValue = *yytext;
    addCurrentToken();
}

\) {
    /* closed paren */
    currentToken.type = closeType;
    currentToken.closeValue = *yytext;
    addCurrentToken();
    }

(\+|\-)?[0-9]+ {
    /* integer */
    currentToken.type = integerType;
    currentToken.integerValue = atoi(yytext);
    addCurrentToken();
}

\"([^\"]|\\\")*\" {
    /* string */
    char* string = strdup(yytext);
    currentToken.type = stringType;
    currentToken.stringValue = string;
    addCurrentToken();
    
}


\#t|\#f {
    /* boolean */
    char* boolean = strdup(yytext);
    currentToken.type = booleanType;
    if (strcmp(boolean, "#t") == 0){
        currentToken.booleanValue = 1;
    } else if (strcmp(boolean, "#f") == 0) {
        currentToken.booleanValue = 0;
    }
    addCurrentToken();
    free(boolean);
}

(\+|\-)?\.[0-9]+|(\+|\-)?[0-9]+\.[0-9]* {
    /* real */
    currentToken.type = realType;
    currentToken.realValue = atof(yytext);
    addCurrentToken();
}


((([a-z]|[A-Z])|\!|\$|\%|\&|\*|\/|\:|\<|\=|\>|\?|\~|\_|\^)((([a-z]|[A-Z])|\!|\$|\%|\&|\*|\/|\:|\<|\=|\>|\?|\~|\_|\^)|[0-9]|\.|\+|\-)*)|\+|\- {
    /* symbol */
    char* symbol = strdup(yytext);
    currentToken.type = symbolType;
    currentToken.symbolValue = symbol;
    addCurrentToken();
    
}




" "|\t {

}

\n {
    /* end of line */
    return 1;
}

";"[^\n]* {

    
}


. {
    /* other */
    printf("syntax error\n");
    exit(1);
}




