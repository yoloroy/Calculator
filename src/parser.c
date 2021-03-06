#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "parser.h"

#define DArray(type, name) typedef struct { type *array; int size, count;} name;
#define scase(var) if (strlen(#var) == i && strncmp(scaseval, #var, i) == 0)
#define elif else if
#define throw_error(message) fprintf(stderr, "Error: "#message); exit(-1)

DArray(ComplexNumber, ComplexNumbersDArray)
DArray(Operation, OperationsDArray)

void AddToComplexNumbersDArray(ComplexNumbersDArray *dArray, ComplexNumber element);

void AddToOperationsDArray(OperationsDArray *dArray, Operation element);

int IsComma(char chr);

int IsDigit(char chr);

int IsSymbol(char chr);

int IsAlpha(char chr);

int IsFunction(char *startStr);

ComplexNumber GetNumber(char **startStr);

ComplexNumber GetDefined(char **startStr);

Operation GetOperation(char **startStr);

Operation GetSymbolOperation(char chr);

DictEntire ParseDefinition(char *line) {
    DictEntire result;

    char *p = line;
    while (*++p != '=' && *p != ' ');

    result.key = calloc(p - line + 1, sizeof(char));
    strncpy(result.key, line, p - line);
    result.key[p - line] = '\0';

    while (*++p == '=' || *p == ' ');

    ParseExpression(p, &result.value.numc, &result.value.numv, &result.value.opc, &result.value.opv);

    return result;
}

void ParseExpression(char *line, int *numc, ComplexNumber **numv, int *opc, Operation **opv) {
    ComplexNumbersDArray dNArray = { NULL, 0, 0 };
    OperationsDArray dOArray = { NULL, 0, 0 };

    for (char *pc = line; *pc != '\0'; ++pc) {
        if (*pc == ' ') {
        } elif (IsComma(*pc)) {
            AddToOperationsDArray(&dOArray, EComma);
        } elif (IsDigit(*pc)) {
            AddToComplexNumbersDArray(&dNArray, GetNumber(&pc));
        } elif (IsAlpha(*pc)) {
            if (IsFunction(pc)) {
                AddToOperationsDArray(&dOArray, GetOperation(&pc));
            } else {
                AddToComplexNumbersDArray(&dNArray, GetDefined(&pc));
            }
        } elif (IsSymbol(*pc)) {
            Operation operation = GetSymbolOperation(*pc);

            if (operation == EMinus &&
                    (((dOArray.count == 0) && (dNArray.count == 0)) || (dOArray.count != 0) && (
                    dOArray.array[dOArray.count - 1] == EOpenParenthesis ||
                    dOArray.array[dOArray.count - 1] == EPlus ||
                    dOArray.array[dOArray.count - 1] == EMinus ||
                    dOArray.array[dOArray.count - 1] == EMultiply ||
                    dOArray.array[dOArray.count - 1] == EDivide))
            ) {
                AddToOperationsDArray(&dOArray, EUnaryMinus);
            } else {
                AddToOperationsDArray(&dOArray, operation);
            }
        }
    }

    *numc = dNArray.count;
    *numv = calloc(sizeof(ComplexNumber), *numc);
    memcpy(*numv, dNArray.array, sizeof(ComplexNumber) * *numc);
    free(dNArray.array);

    *opc = dOArray.count;
    *opv = calloc(sizeof(Operation), *opc);
    memcpy(*opv, dOArray.array, sizeof(Operation) * *opc);
    free(dOArray.array);
}

int IsDigit(char chr) {
    return ((chr >= '0')&&(chr <= '9'));
}

int IsAlpha(char chr) {
    return (((chr >= 'a')&&(chr <= 'z'))||((chr >= 'A')&&(chr <= 'Z'))) || (chr == '_');
}

int IsSymbol(char chr) {
    return ((!IsAlpha(chr))&&(!IsDigit(chr))&&(chr != ' '));
}

int IsFunction(char *startStr) {
    bool result = 0;
    int len = 0;
    while (IsAlpha(*(startStr + len++)));
    char line[len];
    memset(line, 0, len);
    strncpy(line, startStr, --len);

    if (len == 2) {
        if ((strcmp(line, "tg") == 0) ||
            (strcmp(line, "ln") == 0)) {
            result = 1;
        }
    } elif (len == 3) { // TODO: ADD file for operations names
        if ((strcmp(line, "cos") == 0) ||
            (strcmp(line, "sin") == 0) ||
            (strcmp(line, "pow") == 0) ||
            (strcmp(line, "abs") == 0) ||
            (strcmp(line, "exp") == 0) ||
            (strcmp(line, "mag") == 0) ||
            (strcmp(line, "log") == 0)
        ) {
            result = 1;
        }
    } elif (len == 4) {
        if ((strcmp(line, "real") == 0) ||
            (strcmp(line, "imag") == 0) ||
            (strcmp(line, "sqrt") == 0)
        ) {
            result = 1;
        }
    } elif (len == 5) {
        if (strcmp(line, "phase") == 0) {
            result = 1;
        }
    }

    return result;
}

ComplexNumber GetNumber(char **startStr) {
    ComplexNumber result = { 0, NULL };
    double number = 0;
    int afterDotCount = 0;
    bool isAfterDot = false;
    bool isFake = false;

    char *pc;
    for (pc = *startStr; IsDigit(*pc) || *pc == 'i' || *pc == '.'; ++pc) {
        if (IsAlpha(*pc)) {
            isFake = *pc == 'i';
            break;
        } elif (IsDigit(*pc)) {
            number *= 10;
            number += *pc - '0';

            if (isAfterDot)
                afterDotCount++;
        } elif (*pc == '.') {
            isAfterDot = true;
        }
    }

    number /= pow(10, afterDotCount);
    if (!isFake) {
        result.number = number;
    } else {
        result.number = number * I;
    }

    *startStr = pc - 1;
    if (**startStr == ' ') (*startStr)++;
    if (isFake) (*startStr)++;

    return result;
}

ComplexNumber GetDefined(char **startStr) {
    ComplexNumber result = { 0, NULL };
    int size = 0;

    int i = 0;
    for (;;i++) {
        if (IsAlpha((*startStr)[i]) || IsDigit((*startStr)[i])) {
            size++;
        } else {
            break;
        }
    }

    if (size == 1 && (*startStr)[0] == 'i') {
        result.number = 1 * I;
    } else {
        result.definedName = calloc(i + 1, sizeof(char));
        strncpy(result.definedName, *startStr, i);
        *startStr += i - 1;
    }

    return result;
}

Operation GetOperation(char **startStr) {
    int i = 0;
    while (IsAlpha((*startStr)[i++]));
    i--;

    char strval[i];

    strncpy(strval, *startStr, i);
    *startStr += i - 1;

    if (i == 1) return GetSymbolOperation(**startStr);

#define scaseval strval
    scase(tg) return ETg;
    scase(ln) return ELn;
    scase(cos) return ECos;
    scase(sin) return ESin;
    scase(pow) return EPow;
    scase(abs) return EAbs;
    scase(exp) return EExp;
    scase(mag) return EMag;
    scase(log) return ELog;
    scase(real) return EReal;
    scase(imag) return EImag;
    scase(sqrt) return ESqrt;
    scase(phase) return EPhase;

    throw_error("it is not operation(GetOperation)");
}

Operation GetSymbolOperation(char chr) {
    switch (chr) {
        case '+': return EPlus;
        case '-': return EMinus;
        case '/': return EDivide;
        case '*': return EMultiply;
        case '(': return EOpenParenthesis;
        case ')': return ECloseParenthesis;
        case '^': return EPow;
        default: throw_error("it is not operation(GetSymbolOperation)");
    }
}

#define AddToDArray(type, name) \
void AddTo##name(name *dArray, type element) { \
    if (dArray->size == dArray->count) {                                              \
        type *newArray = calloc(dArray->size * 2 + 1, sizeof(type));\
        for (int i = 0; i < dArray->count; ++i) {                                     \
            newArray[i] = dArray->array[i];                                           \
        }                       \
        free(dArray->array);    \
        dArray->array = newArray;              \
        dArray->size = dArray->size * 2 + 1;\
    }                           \
                                \
    dArray->array[dArray->count++] = element;                                         \
}

AddToDArray(ComplexNumber, ComplexNumbersDArray)
AddToDArray(Operation, OperationsDArray)

int IsComma(char chr) {
    return chr == ',';
}
