/* Common data structures for the Racket interpreter project. */

#include <stdbool.h>

typedef enum {
    booleanType,
    integerType,
    realType,
    openType,
    consType,
    closeType,
    symbolType,
    stringType,
    frameType,
    bindingType,
    quoteType,
    voidType,
    closureType,
    primitiveType,
} VALUE_TYPE;

typedef struct __Value {
    VALUE_TYPE type;
    union {
        bool booleanValue;
        int integerValue;
        double realValue;
        struct __ConsCell *consValue;
        struct __Frame *frameValue;
        struct __Binding *bindingValue;
        struct __Closure *closureValue;
        char openValue;
        char closeValue;
        char *symbolValue;
        char *stringValue;
        struct __ConsCell *quoteValue;
        struct __Value (*primitiveValue)(struct __Value args);

    };
} Value;

typedef struct __ConsCell {
    Value car;
    Value cdr;
} ConsCell;

typedef struct __Binding {
    char* symbol;
    Value bindingValue;
    struct __Binding* nextPointer;
} Binding;

typedef struct __Frame {
    Binding* bindingPointer;
    struct __Frame* parentPointer;
} Frame;

typedef struct __Closure {
    ConsCell* paramList;
    Value functionBody;
    Frame* env;
} Closure;




/* lexing */
int yylex();
int parser();
void ourPush();
void printQuote(ConsCell*);
void syntaxError();
ConsCell* ourPop();
void printValue(ConsCell*);
Value evalEach(Value, Frame*);
Value eval(Value, Frame*);
Value apply(Value, Value);
Frame* makeTopFrame();
Value evalPlus(Value);
Value evalNull(Value);
Value evalCar(Value);
Value evalCdr(Value);
Value evalCons(Value);
Value evalTimes(Value);
Value evalMinus(Value);
Value evalLessEqual(Value);
Value evalGreaterEqual(Value);
Value evalEqual(Value);
Value evalModulo(Value);
Value evalDivide(Value);
Value evalDisplay(Value);
Value evalStringLength(Value);
Value evalEqualEqual(Value);
char *unescape(char *);
Value equalHelper(Value, Value);
Value member(Value, Value);
void initGarbageBin();
void addToGarbage(void*);
void cleanGarbage();
