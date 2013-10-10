//Written by: Sarah LeBlanc, Pat Dale, Thor LaackVeeder

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include <unistd.h>


extern ConsCell *tokens;
extern ConsCell *stack;


ConsCell* getParseTree() {
	return stack;
}


void freeLinkedList(ConsCell* list) {
    if(list) {
        if(list->car.type == consType) {
            freeLinkedList(list->car.consValue);
        }
    ConsCell* tempValue = list;
    list = list->cdr.consValue;
    free(tempValue);
    }
}


void syntaxError() {
    freeLinkedList(stack);
    freeLinkedList(tokens);
	printf("Syntax error\n");
    cleanGarbage();
    exit(1);
}



void printCell(Value result) {
    switch(result.type){
        case booleanType:
            if (result.booleanValue == 1) {
                printf("true\n");
            } else {
                printf("false\n");
            }
            
            break;
        case integerType:
            printf("%i ",result.integerValue);
            break;
        case realType:
            printf("%f ",result.realValue);
            break;
        case closeType:
            printf("%c ",result.closeValue);
            break;
        case symbolType:
            printf("%s ",result.symbolValue);
            break;
        case stringType:
            printf("%s ",result.stringValue);
            break;
        case consType:
            printf("(");
            printValue(result.consValue);
            printf(")");
            break;
        case quoteType:
            printf("'");

            if(result.quoteValue->cdr.consValue == NULL){
                printQuote(result.quoteValue);
                break;

            }else if(result.quoteValue->car.consValue == NULL && result.quoteValue->cdr.consValue == NULL) {
                printf("(");
                printf(")");
                break;
            }
            
            else {
                printf("(");
                printCell(result.quoteValue->car);
                printf(" . ");
                printCell(result.quoteValue->cdr);
                printf(")");
                break;
            }
            
            //printf("Twice\n");
            
            break;
        default:
            break;
        }
}

void printQuote(ConsCell* quotedList) {
    Value quoteCell;
    if(quotedList->car.type == consType) {
        while (quotedList != NULL) {
            quoteCell = quotedList->car;
            printCell(quoteCell);
            quotedList = quotedList->cdr.consValue;
        }
    }
    else {
        //printf("else\n");
        printCell(quotedList->car);
    }
}



void printValue(ConsCell* tree) {
        while (tree != NULL) {
            Value treeValue = tree->car;
            printCell(treeValue);
            Value tempTokens = treeValue;
            //free(tempTokens.consValue);
            tree = tree->cdr.consValue;
        }
    }



int main() {
    initGarbageBin();
    printf("\n");
    int depth = 0;
    Frame* topFrame = makeTopFrame();

    if (isatty(fileno(stdin)) == false) {


        while (yylex()) {

            if (tokens == NULL) {

                // empty line, skip, 
                continue;
            }
            depth = parser();
            ConsCell* parseTree = getParseTree();

            if (depth == 0) {
                printf("\n");
                printCell(eval(parseTree->car, topFrame));


            }
        }
        if (depth != 0) {
            syntaxError();
        }
        printf("\n");
        return 1;
    } else {
        printf("> ");

        while (yylex()) {

            if (tokens == NULL) {
                printf("> ");

                // empty line, skip, 
                continue;
            }
            depth = parser();
            ConsCell* parseTree = getParseTree();
    
            
    
            if (depth == 0) {
                printCell(eval(parseTree->car, topFrame));
                printf("\n");

            }
            if (depth != 0) {
                printf(". ");
            } else {
                printf("> ");
            }
        }

        if (depth != 0) {
            syntaxError();
        }

        return 1;

    }
    cleanGarbage();
}

        
