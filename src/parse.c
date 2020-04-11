
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

baseExpression_t *parseExpression(parser_t *parser, int8_t precedence) {
    // TODO: Implement.
    
    return NULL;
}

// If endCharacter is -1, then this function will parse
// expressions until the end of a line.
void parseExpressionList(vector_t *destination, parser_t *parser, int8_t endCharacter) {
    // TODO: Implement.
    
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


