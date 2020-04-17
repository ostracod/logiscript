
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "parse.h"
#include "expression.h"
#include "statement.h"

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

baseExpression_t *createIdentifierExpression(int8_t *name, int64_t length) {
    identifierExpression_t *output = malloc(sizeof(identifierExpression_t));
    output->base.type = EXPRESSION_TYPE_IDENTIFIER;
    output->name = mallocText(name, length);
    return (baseExpression_t *)output;
}

baseExpression_t *createConstantExpression(value_t value) {
    constantExpression_t *output = malloc(sizeof(constantExpression_t));
    output->base.type = EXPRESSION_TYPE_CONSTANT;
    output->value = value;
    return (baseExpression_t *)output;
}

baseExpression_t *parseExpression(parser_t *parser, int8_t precedence) {
    bodyPos_t *bodyPos = parser->bodyPos;
    bodyPosSkipWhitespace(bodyPos);
    int8_t firstCharacter = bodyPosGetCharacter(bodyPos);
    if (isFirstIdentifierCharacter(firstCharacter)) {
        bodyPos_t startBodyPos = *bodyPos;
        bodyPosSeekEndOfIdentifier(bodyPos);
        int8_t *tempText = getBodyPosPointer(&startBodyPos);
        int64_t tempLength = getDistanceToBodyPos(&startBodyPos, bodyPos);
        return createIdentifierExpression(tempText, tempLength);
    }
    if (isNumberCharacter(firstCharacter)) {
        bodyPos_t startBodyPos = *bodyPos;
        bodyPosSeekEndOfNumber(bodyPos);
        // TODO: Detect malformed numbers.
        int64_t tempLength = getDistanceToBodyPos(&startBodyPos, bodyPos);
        int8_t tempText[tempLength + 1];
        memcpy(tempText, getBodyPosPointer(&startBodyPos), tempLength);
        tempText[tempLength] = 0;
        double tempNumber;
        sscanf((char *)tempText, "%lf", &tempNumber);
        value_t tempValue;
        tempValue.type = VALUE_TYPE_NUMBER;
        tempValue.numberValue = tempNumber;
        return createConstantExpression(tempValue);
    }
    // TODO: Handle more types of expressions.
    
    return NULL;
}

// If endCharacter is -1, then this function will parse
// expressions until the end of a line.
void parseExpressionList(vector_t *destination, parser_t *parser, int8_t endCharacter) {
    bodyPos_t *bodyPos = parser->bodyPos;
    createEmptyVector(destination, sizeof(baseExpression_t *));
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
            } else {
                // TODO: Report parsing errors.
                
                return;
            }
        }
        baseExpression_t *tempExpression = parseExpression(parser, 99);
        pushVectorElement(destination, &tempExpression);
    }
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
    output->function = parseExpression(parser, 99);
    bodyPosSkipWhitespace(bodyPos);
    parseExpressionList(&(output->argumentList), parser, -1);
    // TODO: Report parsing errors.
    
    *hasReachedEnd = !bodyPosSeekNextLine(bodyPos);
    return (baseStatement_t *)output;
}

void parseStatementList(vector_t *destination, parser_t *parser) {
    createEmptyVector(destination, sizeof(baseStatement_t *));
    while (true) {
        int8_t tempHasReachedEnd;
        baseStatement_t *baseStatement = parseStatement(&tempHasReachedEnd, parser);
        if (baseStatement != NULL) {
            pushVectorElement(destination, &baseStatement);
        }
        if (tempHasReachedEnd) {
            break;
        }
    }
}


