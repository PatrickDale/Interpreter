#include <stdio.h>
#include <stdlib.h>
#include "common.h"

extern ConsCell *tokens;
ConsCell *stack = NULL;
int depthlevel = 0;
ConsCell *tempToken = NULL;
ConsCell *tail2 = NULL;


ConsCell* ourPop() {
    ConsCell *parenList = NULL;
    while (stack->car.type != openType) {
        ConsCell *current = stack;
        stack = stack->cdr.consValue;
        current->cdr.consValue = parenList;
        parenList = current;
    }
    ConsCell* tempParen = stack;
    stack = stack->cdr.consValue;
    free(tempParen);
    return parenList;
}

void ourPush() {

    ConsCell *valueToken = (ConsCell *) malloc(sizeof(ConsCell));
    addToGarbage(valueToken);
    valueToken->cdr.type = consType;
 

      switch(tokens->car.type){
            case openType:
                depthlevel += 1;
                valueToken->car.type = openType;
                valueToken->car.openValue = tokens->car.openValue;
                break;
            case booleanType:
                valueToken->car.type = booleanType;
                valueToken->car.booleanValue = tokens->car.booleanValue;
                break;
            case integerType:
                valueToken->car.type = integerType;
                valueToken->car.integerValue = tokens->car.integerValue;
                break;
            case realType:
                valueToken->car.type = realType;
                valueToken->car.realValue = tokens->car.realValue;
                break;
            case closeType:
                depthlevel -= 1;
                if (depthlevel < 0) {
                    //printf("%i\n", depthlevel);
                    syntaxError();
                }
                valueToken->car.type = consType;
                valueToken->car.consValue = ourPop();
                break;
            case symbolType:
                valueToken->car.type = symbolType;
                valueToken->car.symbolValue = tokens->car.symbolValue;
                break;
            case stringType:
                valueToken->car.type = stringType;
                valueToken->car.stringValue = tokens->car.stringValue;
                break;

            default:
                break;
            }

        valueToken->cdr.consValue = stack;
        stack = valueToken;
    
    //if the car is a closed paren pop untill closed paren then push that onto stack. that
    //should then be a conscell wtih constype for car and cdr. car points to list
    //being pushed on and cdr will point to next item that will be pushed on.
}



int parser() {
    while (tokens != NULL) {
        ourPush();
        ConsCell *tempToken = tokens;
        tokens = tokens->cdr.consValue;
        free(tempToken);
    }
    return depthlevel;

}

