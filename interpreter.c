#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"



extern ConsCell *tokens;
extern ConsCell *stack;




Value evaluationError() {
	printf("Evaluation error\n");
    cleanGarbage();
	exit(1);
}

Value makePrimitive(Value (*f)(Value)) {
    Value result;
    result.type = primitiveType;
    result.primitiveValue = f;
    return result;
}

Frame* makeTopFrame() {
    Frame* topFrame = (Frame *) malloc(sizeof(Frame));
    addToGarbage(topFrame);
    topFrame->parentPointer = NULL;
    topFrame->bindingPointer = NULL;

    //Add all primitive types binding into the top frame:

    //+
    Binding* plusBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(plusBinding);
    plusBinding->symbol = "+";
    plusBinding->bindingValue = makePrimitive(evalPlus);
    plusBinding->nextPointer = NULL;

    //null?
    Binding* nullBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(nullBinding);
    nullBinding->symbol = "null?";
    nullBinding->bindingValue = makePrimitive(evalNull);
    nullBinding->nextPointer = plusBinding;
    

    //car
    Binding* carBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(carBinding);
    carBinding->symbol = "car";
    carBinding->bindingValue = makePrimitive(evalCar);
    carBinding->nextPointer = nullBinding;
    

    //cdr
    Binding* cdrBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(cdrBinding);
    cdrBinding->symbol = "cdr";
    cdrBinding->bindingValue = makePrimitive(evalCdr);
    cdrBinding->nextPointer = carBinding;
    

    //cons
    Binding* consBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(consBinding);
    consBinding->symbol = "cons";
    consBinding->bindingValue = makePrimitive(evalCons);
    consBinding->nextPointer = cdrBinding;

    //*
    Binding* timesBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(timesBinding);
    timesBinding->symbol = "*";
    timesBinding->bindingValue = makePrimitive(evalTimes);
    timesBinding->nextPointer = consBinding;


    //-
    Binding* minusBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(minusBinding);
    minusBinding->symbol = "-";
    minusBinding->bindingValue = makePrimitive(evalMinus);
    minusBinding->nextPointer = timesBinding;

    ///
    Binding* divideBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(divideBinding);
    divideBinding->symbol = "/";
    divideBinding->bindingValue = makePrimitive(evalDivide);
    divideBinding->nextPointer = minusBinding;

    //modulo
    Binding* moduloBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(moduloBinding);
    moduloBinding->symbol = "modulo";
    moduloBinding->bindingValue = makePrimitive(evalModulo);
    moduloBinding->nextPointer = divideBinding;

    //<=
    Binding* lessEqualBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(lessEqualBinding);
    lessEqualBinding->symbol = "<=";
    lessEqualBinding->bindingValue = makePrimitive(evalLessEqual);
    lessEqualBinding->nextPointer = moduloBinding;

    //>=
    Binding* greaterEqualBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(greaterEqualBinding);
    greaterEqualBinding->symbol = ">=";
    greaterEqualBinding->bindingValue = makePrimitive(evalGreaterEqual);
    greaterEqualBinding->nextPointer = lessEqualBinding;

    //=
    Binding* equalBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(equalBinding);
    equalBinding->symbol = "=";
    equalBinding->bindingValue = makePrimitive(evalEqual);
    equalBinding->nextPointer = greaterEqualBinding;

    Binding* displayBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(displayBinding);
    displayBinding->symbol = "display";
    displayBinding->bindingValue = makePrimitive(evalDisplay);
    displayBinding->nextPointer = equalBinding;

    Binding* lengthBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(lengthBinding);
    lengthBinding->symbol = "string-length";
    lengthBinding->bindingValue = makePrimitive(evalStringLength);
    lengthBinding->nextPointer = displayBinding;

    Binding* equalEqualBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(equalEqualBinding);
    equalEqualBinding->symbol = "equal?";
    equalEqualBinding->bindingValue = makePrimitive(evalEqualEqual);
    equalEqualBinding->nextPointer = lengthBinding;

    topFrame->bindingPointer = equalEqualBinding;
    

    return topFrame;
}

Value resolveVariable(Value expr, Frame *env) {
    //Segfault on currying, due to consCell not symbol
	char* symbolValue = expr.symbolValue;
	while(env != NULL) {
		Binding *currentSet = env->bindingPointer;
		while(currentSet != NULL) {

			if(strcmp(currentSet->symbol, symbolValue) == 0) {
				return currentSet->bindingValue;
				break;
			}else{
				currentSet = currentSet->nextPointer;
			}
		}
	env = env->parentPointer;	
	}
    if (env == NULL) {
        //printf("resolveVariable\n");
        evaluationError();
    }
    
    return expr;
}

Value evalApply(Value expr, Frame *env) {
    Frame *top = env;
    while(top->parentPointer != NULL) {
        top = top->parentPointer;
    }

    Value closureValue = eval(expr.consValue->car, env);

    //Evaluates the arguments.
    Value args = evalEach(expr.consValue->cdr, env);
    

    //Apply the arguments to the function (which is a closure).
    return apply(closureValue, args);
}

Value evalBegin(Value value, Frame *env) {
    Value answer;
    while(value.consValue != NULL){
        answer = eval(value.consValue->car, env);
        value = value.consValue->cdr;
    }
    return answer;
}

Value evalCar(Value actualArguments) {
    if (actualArguments.consValue->car.consValue == NULL) {
        return evaluationError();
    }

    if (actualArguments.consValue->car.type != quoteType) {
        return evaluationError();
    }

    if (actualArguments.consValue->car.quoteValue->car.consValue == NULL) {
        return evaluationError();
    }
    actualArguments.consValue->car.quoteValue->car.consValue->cdr.consValue = NULL;
    return actualArguments.consValue->car;

}

Value evalCdr(Value actualArguments) {
    Value* returnQuote = (Value *) malloc(sizeof(Value));
    addToGarbage(returnQuote);
    ConsCell* newCons = (ConsCell *) malloc(sizeof(ConsCell));
    addToGarbage(newCons);
    returnQuote->type = quoteType;
    newCons->cdr.consValue = NULL;
    newCons->car.type = consType;

    if (actualArguments.consValue->car.quoteValue == NULL) {
        return evaluationError();
    }
    if (actualArguments.consValue->car.consValue->car.consValue->cdr.consValue == NULL) {
        
        newCons->car.consValue = NULL;
        returnQuote->quoteValue = newCons;
        return *returnQuote;
    }
    
    newCons->car = actualArguments.consValue->car.quoteValue->car.consValue->cdr;
    returnQuote->quoteValue = newCons;

    return *returnQuote;
    
}

Value evalCond(Value actualArguments, Frame *env) {
    
    ConsCell* firstCond = actualArguments.consValue;

    while(firstCond != NULL) {
        if (firstCond->cdr.type != consType) {
            return evaluationError();
        }
        if (firstCond->car.consValue->cdr.consValue->cdr.consValue != NULL) {
            return evaluationError();
        }
        Value booleanExpr = eval(firstCond->car.consValue->car, env);
        if(booleanExpr.booleanValue == true) {
            //printf("what?\n");
            if (firstCond->car.consValue->cdr.type != consType) {
                return eval(firstCond->car.consValue->cdr, env);
            } else {
                return eval(firstCond->car.consValue->cdr.consValue->car, env);
        }
    }
    firstCond = firstCond->cdr.consValue;
}
    Value* voidValue = (Value *) malloc(sizeof(Value));
    addToGarbage(voidValue);
    voidValue->type = voidType;
    return *voidValue;
        
}

Value evalCons(Value actualArguments) {
    Value* pairValue = (Value *) malloc(sizeof(Value));
    addToGarbage(pairValue);
    ConsCell* pairCons = (ConsCell *) malloc(sizeof(ConsCell));
    addToGarbage(pairCons);
    pairValue->type = quoteType;

    if (actualArguments.consValue->car.consValue == NULL) {
        return evaluationError();
    }
    if (actualArguments.consValue->cdr.consValue == NULL) {
        return evaluationError();
    }
    //Second argument could be:

    //if quote
    if (actualArguments.consValue->cdr.consValue->car.type == quoteType) {

        if (actualArguments.consValue->cdr.consValue->car.consValue->car.type == consType) {
            if(actualArguments.consValue->car.type != consType) {
                pairCons->cdr.consValue = NULL;
                pairCons->car = actualArguments;
                pairCons->car.consValue->cdr = actualArguments.consValue->cdr.consValue->car.quoteValue->car;
                pairValue->quoteValue = pairCons; 
            }

            else {

                pairCons->cdr.consValue = NULL;
                pairCons->car = actualArguments.consValue->car;
                pairCons->car.consValue->cdr = actualArguments.consValue->cdr.consValue->car.quoteValue->car;
                pairValue->quoteValue = pairCons;// with list on it
            }
        }
        else if(actualArguments.consValue->car.type != quoteType) {
            pairCons->car = actualArguments.consValue->car;
            pairCons->cdr = actualArguments.consValue->cdr.consValue->car.quoteValue->car;
            pairValue->quoteValue = pairCons;
        } else {
            pairCons->car = actualArguments.consValue->car.quoteValue->car;
            pairCons->cdr = actualArguments.consValue->cdr.consValue->car.quoteValue->car;
            pairValue->quoteValue = pairCons;
        }
    } else {
        pairCons->car = actualArguments.consValue->car;
        pairCons->cdr = actualArguments.consValue->cdr.consValue->car;
        pairValue->quoteValue = pairCons;
    }


    
    return *pairValue;
}

Value evalDefine(Value value, Frame *env) {
    Frame *top = env;
    while(top->parentPointer != NULL) {
        top = top->parentPointer;
    }

    ConsCell* symbolCell = value.consValue;
    Value variable = symbolCell->car;
    ConsCell* valueCell = symbolCell->cdr.consValue;
    Value valueValue = valueCell->car;
    Value symValue = eval(valueValue, env);
    
     //Create a binding
    Binding *currentBinding = (Binding *) malloc(sizeof(Binding));
    addToGarbage(currentBinding);
    currentBinding->symbol = variable.symbolValue;
    currentBinding->bindingValue = symValue;
    currentBinding->nextPointer = top->bindingPointer;
    top->bindingPointer = currentBinding;

    Value* voidValue = (Value *) malloc(sizeof(Value));
    addToGarbage(voidValue);
    voidValue->type = voidType;
    return *voidValue;
}

Value evalDisplay(Value value) {
    if (value.consValue->cdr.consValue != NULL) {
        return evaluationError();
    }
    Value *returnString = (Value *) malloc(sizeof(Value));
    addToGarbage(returnString);
    if (value.consValue->car.type == integerType) {
        printf("%i", value.consValue->car.integerValue);
        
    } else if (value.consValue->car.type == realType) {
        //printf("saw real\n");
        printf("%f", value.consValue->car.realValue);

    } else if (value.consValue->car.type == stringType) {
        //printf("saw a string\n");
        printf("%s", unescape(value.consValue->car.stringValue));

    } else {
        return evaluationError();
    }
    Value* voidValue = (Value *) malloc(sizeof(Value));
    addToGarbage(voidValue);
    voidValue->type = voidType;
    //printf("at the return\n");
    return *voidValue;
} 

Value evalDivide(Value actualArguments) {
    ConsCell* args = actualArguments.consValue;
    ConsCell* countingArgs = actualArguments.consValue;
    Value *divideValue = (Value*) malloc(sizeof(Value));
    addToGarbage(divideValue);
    int argCount = 0;
    while(countingArgs != NULL) {
        argCount += 1;
        countingArgs = countingArgs->cdr.consValue;
    }
    if (argCount != 2) {
        return evaluationError();
    } 
    
    
    if (args->car.type == integerType && args->cdr.consValue->car.type == realType) { //0 / 0.00
        float intToFloat = (float) args->car.integerValue;
        divideValue->type = realType;
        divideValue->realValue = intToFloat / args->cdr.consValue->car.realValue;
        return *divideValue;

    } else if (args->car.type == realType && args->cdr.consValue->car.type == integerType) { //0.00 / 0
        float intToFloat = (float) args->cdr.consValue->car.integerValue;
        divideValue->type = realType;
        divideValue->realValue = args->car.realValue / intToFloat;
        return *divideValue;
    

    } else if (args->car.type == realType && args->cdr.consValue->car.type == realType) { //0.00 / 0.00
        divideValue->type = realType;
        divideValue->realValue = args->car.realValue / args->cdr.consValue->car.realValue;
        return *divideValue;

    } else if (args->car.type == integerType && args->cdr.consValue->car.type == integerType) { //0 / 0
        if (args->car.integerValue % args->cdr.consValue->car.integerValue != 0) {
            divideValue->type = realType;
            divideValue->realValue = (float) args->car.integerValue / (float) args->cdr.consValue->car.integerValue;
            return *divideValue;
        } else {
            divideValue->type = integerType;
            divideValue->integerValue = args->car.integerValue / args->cdr.consValue->car.integerValue;
            return *divideValue;
        }
    }
    return evaluationError();
}

Value evalEach(Value expr, Frame *env){
    ConsCell* evaluatedArgsCell = (ConsCell *) malloc(sizeof(ConsCell));
    addToGarbage(evaluatedArgsCell);
    ConsCell* argList = expr.consValue;
    

    Value* returnedArgs = (Value *) malloc(sizeof(Value));
    addToGarbage(returnedArgs);
    returnedArgs->type = consType;

    ConsCell* tempCell1 = NULL;
    ConsCell* tail = NULL;
    ConsCell* revisedList = NULL;
    while(argList != NULL) {
        ConsCell* evaluatedArgsCell = (ConsCell *) malloc(sizeof(ConsCell));
        addToGarbage(evaluatedArgsCell);
        evaluatedArgsCell->car = eval(argList->car, env);
        evaluatedArgsCell->cdr.type = consType;
        evaluatedArgsCell->cdr.consValue = NULL;
        tempCell1 = evaluatedArgsCell;

        if (revisedList == NULL) {
            revisedList = tempCell1;
            tail = tempCell1;
        } else {
            tail->cdr.consValue = tempCell1;
            tail = tempCell1;
        }
        argList = argList->cdr.consValue;
    }
    returnedArgs->consValue = revisedList;
    return *returnedArgs;
}

Value evalEqual(Value actualArguments) {
    ConsCell* args = actualArguments.consValue;
    ConsCell* countingArgs = actualArguments.consValue;
    Value *equalValue = (Value*) malloc(sizeof(Value));
    addToGarbage(equalValue);
    equalValue->type = booleanType;
    equalValue->booleanValue = false;
    int argCount = 0;

    while(countingArgs != NULL) {
        argCount += 1;

        countingArgs = countingArgs->cdr.consValue;
    }
    if (argCount != 2) {
        return evaluationError();
    }
        if (args->car.type == integerType && args->cdr.consValue->car.type == realType) { //0 <= 0.00
            if (args->car.integerValue == args->cdr.consValue->car.realValue) {
                equalValue->booleanValue = true;
                return *equalValue;
            }
        } else if (args->car.type == realType && args->cdr.consValue->car.type == integerType) { //0.00 <= 0
            if (args->car.realValue == args->cdr.consValue->car.integerValue) {
                equalValue->booleanValue = true;
                return *equalValue;
            }
        } else if (args->car.type == realType && args->cdr.consValue->car.type == realType) { //0.00 <= 0.00
            if (args->car.realValue == args->cdr.consValue->car.realValue) {
                equalValue->booleanValue = true;
                return *equalValue;
            }
            
        } else if (args->car.type == integerType && args->cdr.consValue->car.type == integerType) { //0 <= 0
            if (args->car.integerValue == args->cdr.consValue->car.integerValue) {
                equalValue->booleanValue = true;
                return *equalValue;
            }
        } else {
            equalValue->booleanValue = false;
        }
    return *equalValue;
}

Value evalEqualEqual(Value actualArguments) {
    Value* booleanReturn = (Value *) malloc(sizeof(Value));
    addToGarbage(booleanReturn);
    booleanReturn->type = booleanType;

    Value firstArgument = actualArguments.consValue->car;
    Value secondArgument = actualArguments.consValue->cdr.consValue->car;
    if (firstArgument.type != secondArgument.type) {
        booleanReturn->booleanValue = false;
        return *booleanReturn;
    }
    if(firstArgument.type != consType) {
        if (firstArgument.type == integerType) {
            if (firstArgument.integerValue == secondArgument.integerValue){
                booleanReturn->booleanValue = true;
                return *booleanReturn;
            } else {
                booleanReturn->booleanValue = false;
                return *booleanReturn;
            }
        } else if (firstArgument.type == realType) {
            if (firstArgument.realValue == secondArgument.realValue) {
                booleanReturn->booleanValue = true;
                return *booleanReturn;
            } else {
                booleanReturn->booleanValue = false;
                return *booleanReturn;
            }
        } else if (firstArgument.type == stringType) {
            if (strcmp(firstArgument.stringValue, secondArgument.stringValue) == 0) {
                booleanReturn->booleanValue = true;
                return *booleanReturn;
            } else {
                booleanReturn->booleanValue = false;
                return *booleanReturn;
            }
        } else if (firstArgument.type == booleanType) {
            if (firstArgument.booleanValue == secondArgument.booleanValue) {
                booleanReturn->booleanValue = true;
                return *booleanReturn;
            } else {
                booleanReturn->booleanValue = false;
                return *booleanReturn;
            }
        } else if (firstArgument.type == quoteType) {
            return equalHelper(firstArgument, secondArgument);
        } 
        else if (firstArgument.type == closureType) {
            if (&firstArgument.consValue->cdr == &secondArgument.consValue->cdr) {
                booleanReturn->booleanValue = true;
                return *booleanReturn;
            } else {
                booleanReturn->booleanValue = false;
                return *booleanReturn;
            }
            
        } 
    } else {
        return equalHelper(firstArgument, secondArgument);
    }
    return evaluationError();
}

Value equalHelper(Value first, Value second) {
    Value* booleanHelper = (Value *) malloc(sizeof(Value));
    addToGarbage(booleanHelper);
    booleanHelper->type = booleanType;
    

    if (first.consValue == NULL) {
        booleanHelper->booleanValue = true;
        return *booleanHelper;
    } else if ((member(first.consValue->car, second)).booleanValue == 1) {
        return equalHelper(first.consValue->cdr, second);
    } else {
        //printf("false 1\n");
        booleanHelper->booleanValue = false;
        return *booleanHelper;
    }
}

Value member(Value value, Value list) {
    Value* booleanHelper = (Value *) malloc(sizeof(Value));
    addToGarbage(booleanHelper);
    booleanHelper->type = booleanType;

    Value* pairValue = (Value *) malloc(sizeof(Value));
    addToGarbage(pairValue);
    ConsCell* pairCell = (ConsCell *) malloc(sizeof(ConsCell));
    addToGarbage(pairCell);
    pairCell->car = value;
    pairCell->cdr = list;
    pairValue->type = consType;
    pairValue->consValue = pairCell;
    if (list.consValue == NULL) {
        //printf("false 2\n");
        booleanHelper->booleanValue = false;
        return *booleanHelper;
    } else if ((evalEqualEqual(*pairValue)).booleanValue == 1) {
        booleanHelper->booleanValue = true;
        return *booleanHelper;
    } else {
        return member(value, list.consValue->cdr);
    }
}

Value evalGreaterEqual(Value actualArguments) {
    ConsCell* args = actualArguments.consValue;
    ConsCell* countingArgs = actualArguments.consValue;
    Value *greaterEqualValue = (Value*) malloc(sizeof(Value));
    addToGarbage(greaterEqualValue);
    greaterEqualValue->type = booleanType;
    greaterEqualValue->booleanValue = false;

    int argCount = 0;

    while(countingArgs != NULL) {
        argCount += 1;

        countingArgs = countingArgs->cdr.consValue;
    }
    if (argCount != 2) {
        return evaluationError();
    }
        if (args->car.type == integerType && args->cdr.consValue->car.type == realType) { //0 <= 0.00
            if ((args->car.integerValue > args->cdr.consValue->car.realValue) || (args->car.integerValue == args->cdr.consValue->car.realValue)) {
                greaterEqualValue->booleanValue = true;
                return *greaterEqualValue;
            }
        } else if (args->car.type == realType && args->cdr.consValue->car.type == integerType) { //0.00 <= 0
            if ((args->car.realValue > args->cdr.consValue->car.integerValue) || (args->car.realValue == args->cdr.consValue->car.integerValue)) {
                greaterEqualValue->booleanValue = true;
                return *greaterEqualValue;
            }
        } else if (args->car.type == realType && args->cdr.consValue->car.type == realType) { //0.00 <= 0.00
            if ((args->car.realValue > args->cdr.consValue->car.realValue) || (args->car.realValue == args->cdr.consValue->car.realValue)) {
                greaterEqualValue->booleanValue = true;
                return *greaterEqualValue;
            }
            
        } else if (args->car.type == integerType && args->cdr.consValue->car.type == integerType) { //0 <= 0
            if ((args->car.integerValue > args->cdr.consValue->car.integerValue) || (args->car.integerValue == args->cdr.consValue->car.integerValue)) {
                greaterEqualValue->booleanValue = true;
                return *greaterEqualValue;
            }
        } else {
            greaterEqualValue->booleanValue = false;
        }
    return *greaterEqualValue;
}

Value evalIf(Value value, Frame *env) {
	int count = 0;
	ConsCell *counterValue = value.consValue;
	
	while (counterValue != NULL) {
		count += 1;
		counterValue = counterValue->cdr.consValue;
	}
	if (count != 3) {
                //rintf("evalIf\n");

		return evaluationError();
	}
	Value boolean = eval(value.consValue->car, env);
	value = value.consValue->cdr;

	if (boolean.booleanValue == 1) {
		return eval(value.consValue->car,env);
	} else if (boolean.booleanValue == 0) {
		value = value.consValue->cdr;
		return eval(value.consValue->car,env);
	}
            //printf("evalIf2\n");

	return evaluationError();
}

Value evalLambda(Value value, Frame *env) {
    //printf("In evalLambda\n");
    ConsCell* paramList = value.consValue->car.consValue;
    Value body = value.consValue->cdr.consValue->car;   
    
    

    Value* lambdaValue = (Value*) malloc(sizeof(Value));
    addToGarbage(lambdaValue);
    Closure* lambdaClosure = (Closure *) malloc(sizeof(Closure));
    addToGarbage(lambdaClosure);
    
    
    lambdaClosure->paramList = paramList;
    lambdaClosure->functionBody = body;
    lambdaClosure->env = env;

    lambdaValue->type = closureType;
    lambdaValue->closureValue = lambdaClosure;

    return *lambdaValue;
}

Value evalLessEqual(Value actualArguments) {
    ConsCell* args = actualArguments.consValue;
    ConsCell* countingArgs = actualArguments.consValue;
    Value *lessEqualValue = (Value*) malloc(sizeof(Value));
    addToGarbage(lessEqualValue);
    lessEqualValue->type = booleanType;
    lessEqualValue->booleanValue = false;
    int argCount = 0;

    while(countingArgs != NULL) {
        argCount += 1;

        countingArgs = countingArgs->cdr.consValue;
    }
    if (argCount != 2) {
        return evaluationError();
    }
        if (args->car.type == integerType && args->cdr.consValue->car.type == realType) { //0 <= 0.00
            if ((args->car.integerValue < args->cdr.consValue->car.realValue) || (args->car.integerValue == args->cdr.consValue->car.realValue)) {
                lessEqualValue->booleanValue = true;
            }
        } else if (args->car.type == realType && args->cdr.consValue->car.type == integerType) { //0.00 <= 0
            if ((args->car.realValue < args->cdr.consValue->car.integerValue) || (args->car.realValue == args->cdr.consValue->car.integerValue)) {
                lessEqualValue->booleanValue = true;
            }
        } else if (args->car.type == realType && args->cdr.consValue->car.type == realType) { //0.00 <= 0.00
            if ((args->car.realValue < args->cdr.consValue->car.realValue) || (args->car.realValue == args->cdr.consValue->car.realValue)) {
                lessEqualValue->booleanValue = true;
            }
            
        } else if (args->car.type == integerType && args->cdr.consValue->car.type == integerType) { //0 <= 0
            if ((args->car.integerValue < args->cdr.consValue->car.integerValue) || (args->car.integerValue == args->cdr.consValue->car.integerValue)) {
                lessEqualValue->booleanValue = true;
            }
        } else {
            lessEqualValue->booleanValue = false;
        }
    return *lessEqualValue;
}

Value evalLet(Value value, Frame *env) {
        //Labels the body of the let statement
        Value body = value.consValue->cdr.consValue->car;

        Binding *clusterPointer = NULL;
        
        //Counts the number of variables in the let expression.
        Value varSet = value.consValue->car;
        int numberOfVar = 0;
        while(varSet.consValue != NULL) {
                numberOfVar += 1;
                varSet = varSet.consValue->cdr;

        }
        
        //Creates a new environment and sets its parent environment to env.
        Frame *newEnv = (Frame *) malloc(sizeof(Frame));
        addToGarbage(newEnv);
        Frame *frameame = newEnv;
        frameame->parentPointer = env;
        
        //
        ConsCell* temporaryCell = value.consValue->car.consValue;


        for(int i=0; i<numberOfVar; i++) {
            ConsCell* symbolCell = temporaryCell->car.consValue;
            Value variable = symbolCell->car;
            ConsCell* variableCell = symbolCell->cdr.consValue;
            Value variableValue = variableCell->car;
            Value value = eval(variableValue, env);

             //Create a binding
            Binding *currentBinding = (Binding *) malloc(sizeof(Binding));
            addToGarbage(currentBinding);
            currentBinding->symbol = variable.symbolValue;
            currentBinding->bindingValue = value;
            currentBinding->nextPointer = clusterPointer;
            clusterPointer = currentBinding;
            newEnv->bindingPointer = currentBinding;

            temporaryCell = temporaryCell->cdr.consValue;
        }

        return eval(body, newEnv); 
      
}

Value evalLetRec(Value value, Frame *env) {
    //Labels the body of the let statement
        Value body = value.consValue->cdr.consValue->car;

        Binding *clusterPointer = NULL;
        
        //Counts the number of variables in the let expression.
        Value varSet = value.consValue->car;
        int numberOfVar = 0;
        while(varSet.consValue != NULL) {
                numberOfVar += 1;
                varSet = varSet.consValue->cdr;

        }
        
        //Creates a new environment and sets its parent environment to env.
        Frame *newEnv = (Frame *) malloc(sizeof(Frame));
        addToGarbage(newEnv);
        Frame *frameame = newEnv;
        frameame->parentPointer = env;
        
        //
        ConsCell* temporaryCell = value.consValue->car.consValue;
        Value *booleanFalseValue = (Value *) malloc(sizeof(Value));
        addToGarbage(booleanFalseValue);
        booleanFalseValue->type = booleanType;
        booleanFalseValue->booleanValue = false;

        for(int i=0; i<numberOfVar; i++) {
            ConsCell* symbolCell = temporaryCell->car.consValue;
            Value variable = symbolCell->car;
            ConsCell* variableCell = symbolCell->cdr.consValue;
            Value variableValue = variableCell->car;
            Value value = *booleanFalseValue;

             //Create a binding
            Binding *currentBinding = (Binding *) malloc(sizeof(Binding));
            addToGarbage(currentBinding);
            currentBinding->symbol = variable.symbolValue;
            currentBinding->bindingValue = value;
            currentBinding->nextPointer = clusterPointer;
            clusterPointer = currentBinding;
            newEnv->bindingPointer = currentBinding;

            temporaryCell = temporaryCell->cdr.consValue;
        }

        temporaryCell = value.consValue->car.consValue;

        for(int i=0; i<numberOfVar; i++) {
            ConsCell* symbolCell = temporaryCell->car.consValue;
            Value variable = symbolCell->car;
            ConsCell* variableCell = symbolCell->cdr.consValue;
            Value variableValue = variableCell->car;
            Value value = eval(variableValue, newEnv);

             //Create a binding
            Binding *currentBinding = (Binding *) malloc(sizeof(Binding));
            addToGarbage(currentBinding);
            currentBinding->symbol = variable.symbolValue;
            currentBinding->bindingValue = value;
            currentBinding->nextPointer = clusterPointer;
            clusterPointer = currentBinding;
            newEnv->bindingPointer = currentBinding;

            temporaryCell = temporaryCell->cdr.consValue;
        }

        return eval(body, newEnv); 
}

Value evalLetStar(Value value, Frame *env) {
        //Labels the body of the let statement
        Value body = value.consValue->cdr.consValue->car;

        Binding *clusterPointer = NULL;
        
        //Counts the number of variables in the let expression.
        Value varSet = value.consValue->car;
        int numberOfVar = 0;
        while(varSet.consValue != NULL) {
                numberOfVar += 1;
                varSet = varSet.consValue->cdr;

        }
        
        //Creates a new environment and sets its parent environment to env.
        Frame *newEnv = (Frame *) malloc(sizeof(Frame));
        addToGarbage(newEnv);
        Frame *frameame = newEnv;
        frameame->parentPointer = env;
        
        //
        ConsCell* temporaryCell = value.consValue->car.consValue;


        for(int i=0; i<numberOfVar; i++) {
            ConsCell* symbolCell = temporaryCell->car.consValue;
            Value variable = symbolCell->car;
            ConsCell* variableCell = symbolCell->cdr.consValue;
            Value variableValue = variableCell->car;
            Value value = eval(variableValue, newEnv);

             //Create a binding
            Binding *currentBinding = (Binding *) malloc(sizeof(Binding));
            addToGarbage(currentBinding);
            currentBinding->symbol = variable.symbolValue;
            currentBinding->bindingValue = value;
            currentBinding->nextPointer = clusterPointer;
            clusterPointer = currentBinding;
            newEnv->bindingPointer = currentBinding;

            temporaryCell = temporaryCell->cdr.consValue;
        }

        return eval(body, newEnv); 
      
}


Value evalMinus(Value actualArguments) {
    ConsCell* args = actualArguments.consValue;
    ConsCell* countingArgs = actualArguments.consValue;
    Value *minusValue = (Value*) malloc(sizeof(Value));
    addToGarbage(minusValue);
    int argCount = 0;
    bool seenReal = false;
    while(countingArgs != NULL) {
        argCount += 1;
        if (countingArgs->car.type == realType) {
            //printf("seen a real\n");
            seenReal = true;
        }
        countingArgs = countingArgs->cdr.consValue;
    }
    if (argCount != 2) {
        return evaluationError();
    } 
    float floatCount = 0.0;
    if(seenReal) {
        minusValue->type = realType;
        
        if (args->car.type == integerType && args->cdr.consValue->car.type == realType) {
            floatCount = args->car.integerValue - args->cdr.consValue->car.realValue;
            minusValue->realValue = floatCount;
            return *minusValue;
            //0 - 0.00
        } else if (args->car.type == realType && args->cdr.consValue->car.type == integerType) {
            floatCount = args->car.realValue - args->cdr.consValue->car.integerValue;
            minusValue->realValue = floatCount;
            return *minusValue;
            //0.00 - 0
        } else if (args->car.type == realType && args->cdr.consValue->car.type == realType) {
            floatCount = args->car.realValue - args->cdr.consValue->car.realValue;
            minusValue->realValue = floatCount;
            return *minusValue;
            //0.00 - 0.00
        } 
    }
    //0 - 0
    floatCount = (int) (args->car.integerValue - args->cdr.consValue->car.integerValue);
    minusValue->type = integerType;
    minusValue->integerValue = floatCount;
    return *minusValue;

    
}

Value evalModulo(Value actualArguments){
    ConsCell* args = actualArguments.consValue;
    ConsCell* countingArgs = actualArguments.consValue;
    Value *moduloValue = (Value*) malloc(sizeof(Value));
    addToGarbage(moduloValue);
    int argCount = 0;
    int modValue;
    while(countingArgs != NULL) {
        argCount += 1;
        countingArgs = countingArgs->cdr.consValue;
    }
    if (argCount != 2) {
        return evaluationError();
    } 

    modValue = args->car.integerValue % args->cdr.consValue->car.integerValue;
    moduloValue->type = integerType;
    moduloValue->integerValue = modValue;
    return *moduloValue;
}


Value evalNull(Value actualArguments){
    //printf("in eval null\n");
    if(actualArguments.consValue->cdr.consValue != NULL) {
        return evaluationError();
    }

    Value* boolValue = (Value*) malloc(sizeof(Value));
    addToGarbage(boolValue);
    boolValue->type = booleanType;

    if (actualArguments.consValue->car.type != quoteType) {
        boolValue->booleanValue = false;
    }

    if (actualArguments.consValue->car.type == quoteType) {
        ConsCell* afterQuoteCell = actualArguments.consValue->car.quoteValue->car.quoteValue;

        if (afterQuoteCell == NULL) {
            boolValue->booleanValue = true;
        } else {
            boolValue->booleanValue = false;
        }
    }

    return *boolValue;
}

Value evalPlus(Value actualArguments){
    ConsCell* args = actualArguments.consValue;
    Value* countValue = (Value*) malloc(sizeof(Value)); 
    addToGarbage(countValue);
    double floatCount = 0.0;
    bool seenReal = false;
    while(args != NULL){
        if (args->car.type == realType){
            floatCount += args->car.realValue;
            seenReal = true;
        }
        else if (args->car.type == integerType){
            floatCount += args->car.integerValue;
        }
        else{
            return evaluationError();
        }
        args = args->cdr.consValue;  
    }

    if (seenReal) {
        countValue->type = realType;
        countValue->realValue = floatCount;
        return *countValue;
    }
    countValue->type = integerType;
    countValue->integerValue = (int) floatCount;
    return *countValue;
}

Value evalQuote(Value value, Frame *env) {
	Value *quote = (Value *) malloc(sizeof(Value));
    addToGarbage(quote);
	quote->type = quoteType;
	quote->quoteValue = value.consValue;
	return *quote;

}



char *unescape(char *escaped) {
    int size = strlen(escaped)-1;
    int count = 0;
    int i = 0;
    
    while (escaped[i] != 0) {
        char currentchar = escaped[i];
        if (currentchar == '\\') {
            count = count + 1;
        }
        i = i + 1;
    }
    
    char *unescaped = (char *) malloc((size - count) * sizeof(char));
    addToGarbage(unescaped);
    i = 1;
    int j = 0;
    int temporaryCount = 0;
    
    while (escaped[i] != 0) {
        temporaryCount += 1;
        char currentchar = escaped[i];
        char nextchar = escaped[i+1];
        if (currentchar == '\\') {
            if (nextchar == 't') {
                unescaped[j] = '\t';
                i = i + 2;
            }
            if (nextchar == 'n') {
                unescaped[j] = '\n';
                i = i + 2;
            }
            if (nextchar == '\\') {
                unescaped[j] = '\\';
                i = i + 2;
            }
            
            if (nextchar == '"') {
                unescaped[j] = '"';
                i = i + 2;
            }
            if (nextchar == '\'') {
                unescaped[j] = '\'';
                i = i + 2;
            }

        }
         else {
            unescaped[j] = escaped[i];
            i = i + 1;
        }
        j = j + 1;
    }
    //free(unescaped);
    unescaped[temporaryCount-1] = '\0';
    return unescaped;
}



Value evalStringLength(Value value) {
    if(value.consValue->car.type != stringType) {
        return evaluationError();
    }
    char* escaped = unescape(value.consValue->car.stringValue);
    int size = strlen(escaped); 
    Value* strlength = (Value *) malloc(sizeof(Value));
    addToGarbage(strlength);
    strlength->type = integerType;
    strlength->integerValue = size;
    return *strlength;
}

Value evalTimes(Value actualArguments) {
    ConsCell* args = actualArguments.consValue;
    Value* timesValue = (Value*) malloc(sizeof(Value));
    addToGarbage(timesValue);
    double floatCount = 1.0;
    bool seenReal = false;
    while(args != NULL) {
        if (args->car.type == realType) {
            floatCount *= args->car.realValue;
            seenReal = true;
        }
        else if (args->car.type == integerType){
            floatCount *= args->car.integerValue;
        }
        else {
            return evaluationError();
        }
        args = args->cdr.consValue;
    }
    if (seenReal) {
        timesValue->type = realType;
        timesValue->realValue = floatCount;
        return *timesValue;
    }
    timesValue->type = integerType;
    timesValue->integerValue = (int) floatCount;
    return *timesValue;
}


Value apply(Value function, Value actualArguments){
    //printf("in apply\n");
    if (function.type == primitiveType) {
        return function.primitiveValue(actualArguments);
    }
    if (function.type == closureType) {
        Frame *newEnv = (Frame *) malloc(sizeof(Frame));
        addToGarbage(newEnv);
        newEnv->parentPointer = function.closureValue->env;
        //Check 0 argument functions, set to NULL
        newEnv->bindingPointer = NULL;

        ConsCell* paramCell = function.closureValue->paramList;
        
        ConsCell* argsCell = actualArguments.consValue;
        Binding* clusterPointer = NULL;

        while ((argsCell != NULL) && (paramCell != NULL)) {
            Binding* newBinding = (Binding *) malloc(sizeof(Binding));
            addToGarbage(newBinding);
            newBinding->symbol = paramCell->car.symbolValue;
            newBinding->bindingValue = argsCell->car;
            newBinding->nextPointer = clusterPointer;
            clusterPointer = newBinding;
            newEnv->bindingPointer = newBinding;

            argsCell = argsCell->cdr.consValue;
            paramCell = paramCell->cdr.consValue;

        }

        if ((paramCell != NULL) || (argsCell != NULL)) {
            return evaluationError();
        }

        
        //Change functionBody to Value
        Value functionBodyValue = function.closureValue->functionBody;

        return eval(functionBodyValue, newEnv);

    }
    //printf("apply\n");
    return evaluationError();

}

Value eval(Value expr, Frame *env) {
    //printf("Going through eval\n");
    switch (expr.type) {

		case openType:
            break;

        case booleanType:
            return expr;
            break;

        case integerType:
            //printf("%i\n", expr.integerValue);
            return expr;
			break;

        case realType:
        	return expr;
            break;

        case quoteType:
            return expr;
            break;

        case closeType:
            break;

        case symbolType:

             /* resolve the variable */
        	//printf("foreval\n");
            return resolveVariable(expr, env);
			break;

        case stringType:
        	return expr;
            break;

        case consType:{
            if(expr.consValue->car.type == symbolType) {
            	char *keyword = expr.consValue->car.symbolValue;
            	if (strcmp(keyword, "if") == 0) {
                	return evalIf(expr.consValue->cdr, env);
                	break;
            	} else if (strcmp(keyword, "let*") == 0) {
                    return evalLetStar(expr.consValue->cdr, env);
                    
                    break;
                }else if (strcmp(keyword, "let") == 0) {
            		return evalLet(expr.consValue->cdr, env);
                    
            		break;
            	} else if (strcmp(keyword, "quote") == 0) {
            		return evalQuote(expr.consValue->cdr, env);
                 
            		break;
                }else if (strcmp(keyword, "begin") == 0) {
                    return evalBegin(expr.consValue->cdr, env);
                    break;
                } else if (strcmp(keyword, "define") == 0) {
                    //printf("found define\n");
                    return evalDefine(expr.consValue->cdr, env);
                    break;
                } else if (strcmp(keyword, "lambda") == 0) {
                    //printf("found lambda\n");
                    return evalLambda(expr.consValue->cdr, env);
                 
                    break;
            	
                }else if (strcmp(keyword, "cond") == 0) {
                    return evalCond(expr.consValue->cdr, env);
                    break;
                }else if (strcmp(keyword, "letrec") == 0) {
                    return evalLetRec(expr.consValue->cdr, env);
                    break;
                }else {
                    //printf("found other symbol\n");
                    //printf("%s\n", keyword);
                    return evalApply(expr, env);
                    break;
                }
            } else {
                return evalApply(expr, env);
                break;
            }
        }
        default:
            break;
        }

        //printf("eval2\n");
        return evaluationError();
   	

    }



 

