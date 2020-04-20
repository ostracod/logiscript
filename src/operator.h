
#ifndef OPERATOR_HEADER_FILE
#define OPERATOR_HEADER_FILE

#include "value.h"

#define OPERATOR_ARRANGEMENT_UNARY 1
#define OPERATOR_ARRANGEMENT_BINARY 2

#define OPERATOR_DECLARE 1
#define OPERATOR_NAMESPACE 2
#define OPERATOR_ADD 3
#define OPERATOR_SUBTRACT 4
#define OPERATOR_NEGATE 5
#define OPERATOR_MULTIPLY 6
#define OPERATOR_DIVIDE 7
#define OPERATOR_MODULUS 8
#define OPERATOR_BITWISE_NOT 9
#define OPERATOR_BITWISE_OR 10
#define OPERATOR_BITWISE_AND 11
#define OPERATOR_BITWISE_XOR 12
#define OPERATOR_BITSHIFT_LEFT 13
#define OPERATOR_BITSHIFT_RIGHT 14
#define OPERATOR_LOGICAL_NOT 15
#define OPERATOR_LOGICAL_OR 16
#define OPERATOR_LOGICAL_AND 17
#define OPERATOR_LOGICAL_XOR 18
#define OPERATOR_EQUAL 19
#define OPERATOR_NOT_EQUAL 20
#define OPERATOR_IDENTICAL 21
#define OPERATOR_NOT_IDENTICAL 22
#define OPERATOR_GREATER 23
#define OPERATOR_GREATER_OR_EQUAL 24
#define OPERATOR_LESS 25
#define OPERATOR_LESS_OR_EQUAL 26

typedef struct operator {
    int8_t *text;
    int32_t number;
    int8_t arrangement;
    int8_t precedence;
} operator_t;

int8_t textMatchesOperator(int8_t *text, operator_t *operator);
operator_t *getOperatorInText(int8_t *text, int8_t operatorArrangement);
value_t calculateUnaryOperator(operator_t *operator, value_t operand);
value_t calculateBinaryOperator(operator_t *operator, value_t operand1, value_t operand2);

// OPERATOR_HEADER_FILE
#endif


