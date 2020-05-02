
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "parse.h"
#include "operator.h"
#include "expression.h"
#include "statement.h"
#include "error.h"

int8_t bodyPosGetCharacter(bodyPos_t *bodyPos) {
    return (bodyPos->script->body)[bodyPos->index];
}

void bodyPosSkipWhitespace(bodyPos_t *bodyPos) {
    int8_t tempIsEscaped = false;
    while (true) {
        int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
        int8_t tempShouldSkip = false;
        if (tempIsEscaped) {
            if (tempCharacter == '\n') {
                tempShouldSkip = true;
            }
            tempIsEscaped = false;
        } else {
            if (tempCharacter == '\\') {
                tempIsEscaped = true;
                tempShouldSkip = true;
            }
        }
        if (tempCharacter == ' ' || tempCharacter == '\t') {
            tempShouldSkip = true;
        }
        if (!tempShouldSkip) {
            break;
        }
        if (tempCharacter == '\n') {
            bodyPos->lineNumber += 1;
        }
        bodyPos->index += 1;
    }
}

int8_t bodyPosSeekNextLine(bodyPos_t *bodyPos) {
    int8_t tempIsEscaped = false;
    while (bodyPos->index < bodyPos->script->bodyLength) {
        int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
        bodyPos->index += 1;
        if (tempCharacter == '\n') {
            bodyPos->lineNumber += 1;
        }
        if (tempIsEscaped) {
            tempIsEscaped = false;
        } else {
            if (tempCharacter == '\n') {
                return true;
            }
            if (tempCharacter == '\\') {
                tempIsEscaped = true;
            }
        }
    }
    return false;
}

int8_t characterIsEndOfLine(int8_t character) {
    return (character == '\n' || character == 0 || character == '#');
}

int8_t isFirstIdentifierCharacter(int8_t character) {
    return ((character >= 'a' && character <= 'z')
            || (character >= 'A' && character <= 'Z')
            || character == '_');
}

int8_t isIdentifierCharacter(int8_t character) {
    return (isFirstIdentifierCharacter(character)
            || (character >= '0' && character <= '9'));
}

int8_t isNumberCharacter(int8_t character) {
    return ((character >= '0' && character <= '9')
            || character == '.');
}

int8_t escapeCharacter(int8_t character) {
    if (character == 'n') {
        return '\n';
    } else if (character == 't') {
        return '\t';
    } else {
        return character;
    }
}

void bodyPosSeekEndOfIdentifier(bodyPos_t *bodyPos) {
    while (true) {
        int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
        if (!isIdentifierCharacter(tempCharacter)) {
            break;
        }
        bodyPos->index += 1;
    }
}

void bodyPosSeekEndOfNumber(bodyPos_t *bodyPos) {
    while (true) {
        int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
        if (!isNumberCharacter(tempCharacter)) {
            break;
        }
        bodyPos->index += 1;
    }
}

int64_t getDistanceToBodyPos(bodyPos_t *startBodyPos, bodyPos_t *endBodyPos) {
    return endBodyPos->index - startBodyPos->index;
}

int8_t *getBodyPosPointer(bodyPos_t *bodyPos) {
    return bodyPos->script->body + bodyPos->index;
}

operator_t *bodyPosGetOperator(bodyPos_t *bodyPos, int8_t operatorArrangement) {
    int8_t *tempText = getBodyPosPointer(bodyPos);
    operator_t *output = getOperatorInText(tempText, operatorArrangement);
    if (output == NULL) {
        return NULL;
    }
    return output;
}

void bodyPosSkipOperator(bodyPos_t *bodyPos, operator_t *operator) {
    bodyPos->index += strlen((char *)(operator->text));
}

int8_t *parseIdentifier(parser_t *parser) {
    bodyPos_t *bodyPos = parser->bodyPos;
    int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
    if (!isFirstIdentifierCharacter(tempCharacter)) {
        THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Expected identifier.");
        return NULL;
    }
    bodyPos_t startBodyPos = *bodyPos;
    bodyPosSeekEndOfIdentifier(bodyPos);
    int8_t *tempText = getBodyPosPointer(&startBodyPos);
    int64_t tempLength = getDistanceToBodyPos(&startBodyPos, bodyPos);
    return mallocText(tempText, tempLength);
}

// If endCharacter is -1, then this function will parse
// identifiers until the end of a line.
void parseCommaSeparatedValues(
    vector_t *destination,
    parser_t *parser,
    int8_t endCharacter,
    void (*parseElement)(vector_t *, parser_t *)
) {
    bodyPos_t *bodyPos = parser->bodyPos;
    createEmptyVector(destination, sizeof(void *));
    while (true) {
        bodyPosSkipWhitespace(bodyPos);
        int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
        if (endCharacter < 0) {
            if (characterIsEndOfLine(tempCharacter)) {
                break;
            }
        } else {
            if (tempCharacter == endCharacter) {
                bodyPos->index += 1;
                break;
            }
        }
        if (destination->length > 0) {
            if (tempCharacter == ',') {
                bodyPos->index += 1;
                bodyPosSkipWhitespace(bodyPos);
            } else {
                if (endCharacter < 0) {
                    THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Expected ',' or end of line.");
                } else {
                    THROW_BUILT_IN_ERROR(
                        DATA_ERROR_CONSTANT,
                        "Expected ',' or '%c'.",
                        endCharacter
                    );
                }
                return;
            }
        }
        parseElement(destination, parser);
        if (hasThrownError) {
            return;
        }
    }
}

void parseIdentifierListHelper(vector_t *destination, parser_t *parser) {
    int8_t *tempIdentifier = parseIdentifier(parser);
    if (hasThrownError) {
        return;
    }
    pushVectorElement(destination, &tempIdentifier);
}

void parseIdentifierList(vector_t *destination, parser_t *parser, int8_t endCharacter) {
    parseCommaSeparatedValues(
        destination,
        parser,
        endCharacter,
        parseIdentifierListHelper
    );
}

baseExpression_t *parseStringConstantExpression(parser_t *parser) {
    bodyPos_t *bodyPos = parser->bodyPos;
    heapValue_t *tempHeapValue = createHeapValue(VALUE_TYPE_STRING);
    vector_t *tempText = &(tempHeapValue->vector);
    createEmptyVector(tempText, 1);
    int8_t tempIsEscaped = false;
    while (true) {
        int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
        if (characterIsEndOfLine(tempCharacter)) {
            THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Expected '\"'.");
            return NULL;
        }
        bodyPos->index += 1;
        if (tempIsEscaped) {
            tempCharacter = escapeCharacter(tempCharacter);
            pushVectorElement(tempText, &tempCharacter);
            tempIsEscaped = false;
        } else {
            if (tempCharacter == '"') {
                break;
            } else if (tempCharacter == '\\') {
                tempIsEscaped = true;
            } else {
                pushVectorElement(tempText, &tempCharacter);
            }
        }
    }
    int8_t tempCharacter = 0;
    pushVectorElement(tempText, &tempCharacter);
    value_t tempValue = createValueFromHeapValue(tempHeapValue);
    return createConstantExpression(tempValue);
}

baseExpression_t *parseCustomFunctionExpression(parser_t *parser) {
    customFunction_t *lastCustomFunction = parser->customFunction;
    vector_t identifierList;
    parseIdentifierList(&identifierList, parser, -1);
    if (hasThrownError) {
        return NULL;
    }
    bodyPosSeekNextLine(parser->bodyPos);
    int32_t argumentAmount = (int32_t)(identifierList.length);
    customFunction_t *customFunction = malloc(sizeof(customFunction_t));
    customFunction->base.type = FUNCTION_TYPE_CUSTOM;
    customFunction->base.argumentAmount = argumentAmount;
    scope_t *tempScope = &(customFunction->scope);
    tempScope->parentScope = &(lastCustomFunction->scope);
    tempScope->parentVariableAmount = 0;
    createEmptyVector(&(tempScope->variableList), sizeof(scopeVariable_t *));
    for (int32_t index = 0; index < argumentAmount; index++) {
        int8_t *tempIdentifier;
        getVectorElement(&tempIdentifier, &identifierList, index);
        scopeAddVariable(tempScope, tempIdentifier, -1, false);
    }
    if (hasThrownError) {
        return NULL;
    }
    cleanUpVector(&identifierList);
    parser->customFunction = customFunction;
    parseStatementList(&(customFunction->statementList), parser, '}');
    if (hasThrownError) {
        return NULL;
    }
    parser->customFunction = lastCustomFunction;
    return createCustomFunctionExpression(customFunction);
}

void parseExpressionList(vector_t *destination, parser_t *parser, int8_t endCharacter);

baseExpression_t *parseExpression(parser_t *parser, int8_t precedence) {
    bodyPos_t *bodyPos = parser->bodyPos;
    bodyPosSkipWhitespace(bodyPos);
    baseExpression_t *output = NULL;
    operator_t *tempOperator = bodyPosGetOperator(bodyPos, OPERATOR_ARRANGEMENT_UNARY);
    if (tempOperator == NULL) {
        int8_t firstCharacter = bodyPosGetCharacter(bodyPos);
        if (isFirstIdentifierCharacter(firstCharacter)) {
            int8_t *tempIdentifier = parseIdentifier(parser);
            if (hasThrownError) {
                return NULL;
            }
            output = createIdentifierExpression(tempIdentifier);
        } else if (isNumberCharacter(firstCharacter)) {
            bodyPos_t startBodyPos = *bodyPos;
            bodyPosSeekEndOfNumber(bodyPos);
            int64_t tempLength = getDistanceToBodyPos(&startBodyPos, bodyPos);
            int8_t tempText[tempLength + 1];
            memcpy(tempText, getBodyPosPointer(&startBodyPos), tempLength);
            tempText[tempLength] = 0;
            int8_t decimalPointCount = 0;
            for (int64_t index = 0; index < tempLength; index++) {
                int8_t tempCharacter = tempText[index];
                if (tempCharacter == '.') {
                    decimalPointCount += 1;
                    if (decimalPointCount > 1) {
                        THROW_BUILT_IN_ERROR(
                            DATA_ERROR_CONSTANT,
                            "Malformed number literal."
                        );
                        return NULL;
                    }
                }
            }
            double tempNumber;
            int32_t tempResult = sscanf((char *)tempText, "%lf", &tempNumber);
            if (tempResult < 1) {
                THROW_BUILT_IN_ERROR(
                    DATA_ERROR_CONSTANT,
                    "Malformed number literal."
                );
                return NULL;
            }
            value_t tempValue;
            tempValue.type = VALUE_TYPE_NUMBER;
            tempValue.numberValue = tempNumber;
            output = createConstantExpression(tempValue);
        } else {
            switch (firstCharacter) {
                case '\'':
                {
                    bodyPos->index += 1;
                    int8_t tempNumber = bodyPosGetCharacter(bodyPos);
                    if (characterIsEndOfLine(tempNumber)) {
                        THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Malformed character.");
                        return NULL;
                    }
                    bodyPos->index += 1;
                    if (tempNumber == '\\') {
                        int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
                        if (characterIsEndOfLine(tempCharacter)) {
                            THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Malformed character.");
                            return NULL;
                        }
                        bodyPos->index += 1;
                        tempNumber = escapeCharacter(tempCharacter);
                    }
                    int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
                    if (tempCharacter != '\'') {
                        THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Malformed character.");
                        return NULL;
                    }
                    bodyPos->index += 1;
                    value_t tempValue;
                    tempValue.type = VALUE_TYPE_NUMBER;
                    tempValue.numberValue = tempNumber;
                    output = createConstantExpression(tempValue);
                    break;
                }
                case '"':
                {
                    bodyPos->index += 1;
                    output = parseStringConstantExpression(parser);
                    break;
                }
                case '[':
                {
                    bodyPos->index += 1;
                    vector_t tempList;
                    parseExpressionList(&tempList, parser, ']');
                    output = createListExpression(&tempList);
                    break;
                }
                case '{':
                {
                    bodyPos->index += 1;
                    output = parseCustomFunctionExpression(parser);
                    break;
                }
                default:
                {
                    THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Expected expression.");
                    return NULL;
                }
            }
            if (hasThrownError) {
                return NULL;
            }
        }
    } else {
        bodyPosSkipOperator(bodyPos, tempOperator);
        baseExpression_t *tempOperand = parseExpression(parser, 0);
        if (hasThrownError) {
            return NULL;
        }
        if (tempOperator->number == OPERATOR_DECLARE) {
            if (tempOperand->type != EXPRESSION_TYPE_IDENTIFIER) {
                THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Expected identifier.");
                return NULL;
            }
            identifierExpression_t *identifierExpression = (identifierExpression_t *)tempOperand;
            scopeAddVariable(
                &(parser->customFunction->scope),
                identifierExpression->name,
                -1,
                true
            );
            output = tempOperand;
        } else {
            output = createUnaryExpression(tempOperator, tempOperand);
        }
    }
    while (true) {
        bodyPosSkipWhitespace(bodyPos);
        int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
        if (tempCharacter == '[' && precedence > 3) {
            bodyPos->index += 1;
            baseExpression_t *tempOperand = parseExpression(parser, 99);
            if (hasThrownError) {
                return NULL;
            }
            bodyPosSkipWhitespace(bodyPos);
            int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
            if (tempCharacter != ']') {
                THROW_BUILT_IN_ERROR(DATA_ERROR_CONSTANT, "Expected ']'.");
                return NULL;
            }
            bodyPos->index += 1;
            output = createIndexExpression(output, tempOperand);
            continue;
        }
        if (tempCharacter == '(' && precedence > 3) {
            bodyPos->index += 1;
            vector_t tempList;
            parseExpressionList(&tempList, parser, ')');
            output = createInvocationExpression(output, &tempList);
            continue;
        }
        tempOperator = bodyPosGetOperator(bodyPos, OPERATOR_ARRANGEMENT_BINARY);
        if (tempOperator != NULL && tempOperator->precedence < precedence) {
            if (tempOperator->number == OPERATOR_NAMESPACE) {
                // Since the namespace operator is a period, we need
                // to have special logic to handle number literals.
                bodyPos_t tempBodyPos = *bodyPos;
                bodyPosSkipOperator(&tempBodyPos, tempOperator);
                int8_t tempCharacter = bodyPosGetCharacter(&tempBodyPos);
                if (isNumberCharacter(tempCharacter)) {
                    break;
                }
                *bodyPos = tempBodyPos;
            } else {
                bodyPosSkipOperator(bodyPos, tempOperator);
            }
            baseExpression_t *tempOperand = parseExpression(parser, tempOperator->precedence);
            if (hasThrownError) {
                return NULL;
            }
            output = createBinaryExpression(tempOperator, output, tempOperand);
        } else {
            break;
        }
    }
    return output;
}

void parseExpressionListHelper(vector_t *destination, parser_t *parser) {
    baseExpression_t *tempExpression = parseExpression(parser, 99);
    if (hasThrownError) {
        return;
    }
    pushVectorElement(destination, &tempExpression);
}

void parseExpressionList(vector_t *destination, parser_t *parser, int8_t endCharacter) {
    parseCommaSeparatedValues(
        destination,
        parser,
        endCharacter,
        parseExpressionListHelper
    );
}

baseStatement_t *parseStatement(int8_t *hasReachedEnd, parser_t *parser) {
    bodyPos_t *bodyPos = parser->bodyPos;
    bodyPos_t startBodyPos = *bodyPos;
    bodyPosSkipWhitespace(bodyPos);
    int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
    if (characterIsEndOfLine(tempCharacter)) {
        *hasReachedEnd = !bodyPosSeekNextLine(bodyPos);
        return NULL;
    }
    // TODO: Parse import statements.
    
    invocationStatement_t *output = malloc(sizeof(invocationStatement_t));
    output->base.type = STATEMENT_TYPE_INVOCATION;
    output->base.bodyPos = startBodyPos;
    output->function = parseExpression(parser, 3);
    if (hasThrownError) {
        return NULL;
    }
    bodyPosSkipWhitespace(bodyPos);
    parseExpressionList(&(output->argumentList), parser, -1);
    if (hasThrownError) {
        return NULL;
    }
    *hasReachedEnd = !bodyPosSeekNextLine(bodyPos);
    return (baseStatement_t *)output;
}

// If endCharacter is -1, then this function will parse
// expressions until the end of the file.
void parseStatementList(vector_t *destination, parser_t *parser, int8_t endCharacter) {
    bodyPos_t *bodyPos = parser->bodyPos;
    createEmptyVector(destination, sizeof(baseStatement_t *));
    while (true) {
        bodyPosSkipWhitespace(bodyPos);
        int8_t tempCharacter = bodyPosGetCharacter(bodyPos);
        if (tempCharacter == endCharacter) {
            bodyPos->index += 1;
            break;
        }
        int8_t tempHasReachedEnd;
        baseStatement_t *baseStatement = parseStatement(&tempHasReachedEnd, parser);
        if (hasThrownError) {
            return;
        }
        if (baseStatement != NULL) {
            pushVectorElement(destination, &baseStatement);
        }
        if (tempHasReachedEnd) {
            if (endCharacter >= 0) {
                THROW_BUILT_IN_ERROR(
                    DATA_ERROR_CONSTANT,
                    "Expected '%c'.",
                    endCharacter
                );
                return;
            }
            break;
        }
    }
}


