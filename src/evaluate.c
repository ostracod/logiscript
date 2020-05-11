
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "utilities.h"
#include "evaluate.h"
#include "function.h"
#include "expression.h"
#include "operator.h"
#include "value.h"
#include "error.h"
#include "script.h"

int32_t markAndSweepDelay = 0;
int32_t maximumMarkAndSweepDelay = 10000;

hyperValue_t accessSequenceElement(value_t sequenceValue, value_t indexValue) {
    hyperValue_t output;
    output.type = HYPER_VALUE_TYPE_VALUE;
    output.value.type = VALUE_TYPE_VOID;
    if (sequenceValue.type != VALUE_TYPE_STRING && sequenceValue.type != VALUE_TYPE_LIST) {
        THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Expected sequence value.");
        return output;
    }
    if (indexValue.type != VALUE_TYPE_NUMBER) {
        THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Expected number value.");
        return output;
    }
    output.type = HYPER_VALUE_TYPE_ALIAS;
    output.alias.container = sequenceValue.heapValue;
    output.alias.index = (int64_t)(indexValue.numberValue);
    return output;
}

hyperValue_t evaluateExpression(heapValue_t *frame, baseExpression_t *expression);
value_t evaluateAndResolveExpression(heapValue_t *frame, baseExpression_t *expression);

// Output will be locked.
hyperValue_t invokeFunctionWithExpressions(
    heapValue_t *frame,
    baseExpression_t *functionExpression,
    vector_t *argumentExpressionList,
    int8_t shouldAddDestinationArgument
) {
    hyperValue_t output;
    output.type = HYPER_VALUE_TYPE_VALUE;
    output.value.type = VALUE_TYPE_VOID;
    int32_t tempOffset;
    if (shouldAddDestinationArgument) {
        tempOffset = 1;
    } else {
        tempOffset = 0;
    }
    int32_t tempLength = (int32_t)(argumentExpressionList->length) + tempOffset;
    hyperValue_t tempValueArray[tempLength];
    hyperValueList_t argumentList;
    argumentList.valueArray = tempValueArray;
    argumentList.length = tempLength;
    for (int32_t index = 0; index < tempLength; index++) {
        hyperValue_t *tempValue = tempValueArray + index;
        tempValue->type = HYPER_VALUE_TYPE_VALUE;
        tempValue->value.type = VALUE_TYPE_VOID;
    }
    value_t functionValue = evaluateAndResolveExpression(frame, functionExpression);
    if (hasThrownError) {
        unlockFunctionInvocationValues(
            &functionValue,
            &argumentList,
            shouldAddDestinationArgument
        );
        return output;
    }
    for (int32_t index = tempOffset; index < tempLength; index++) {
        baseExpression_t *tempExpression;
        getVectorElement(&tempExpression, argumentExpressionList, index - tempOffset);
        tempValueArray[index] = evaluateExpression(frame, tempExpression);
        if (hasThrownError) {
            unlockFunctionInvocationValues(
                &functionValue,
                &argumentList,
                shouldAddDestinationArgument
            );
            return output;
        }
    }
    output = invokeFunction(functionValue, &argumentList, shouldAddDestinationArgument);
    unlockFunctionInvocationValues(
        &functionValue,
        &argumentList,
        shouldAddDestinationArgument
    );
    return output;
}

// Output will be locked.
hyperValue_t evaluateExpression(heapValue_t *frame, baseExpression_t *expression) {
    hyperValue_t output;
    output.type = HYPER_VALUE_TYPE_VALUE;
    output.value.type = VALUE_TYPE_VOID;
    switch (expression->type) {
        case EXPRESSION_TYPE_CONSTANT:
        {
            constantExpression_t *constantExpression = (constantExpression_t *)expression;
            output.value = copyValue(constantExpression->value);
            lockHyperValue(&output);
            break;
        }
        case EXPRESSION_TYPE_LIST:
        {
            listExpression_t *listExpression = (listExpression_t *)expression;
            vector_t tempValueList;
            createEmptyVector(&tempValueList, sizeof(value_t));
            for (int64_t index = 0; index < listExpression->expressionList.length; index++) {
                baseExpression_t *tempElementExpression;
                getVectorElement(
                    &tempElementExpression,
                    &(listExpression->expressionList),
                    index
                );
                value_t tempValue = evaluateAndResolveExpression(
                    frame,
                    tempElementExpression
                );
                if (hasThrownError) {
                    unlockValuesInVector(&tempValueList);
                    cleanUpVector(&tempValueList);
                    return output;
                }
                pushVectorElement(&tempValueList, &tempValue);
            }
            heapValue_t *tempHeapValue = createHeapValue(VALUE_TYPE_LIST);
            tempHeapValue->vector = tempValueList;
            addValueReferencesInVector(&tempValueList);
            unlockValuesInVector(&tempValueList);
            output.value = createValueFromHeapValue(tempHeapValue);
            lockHyperValue(&output);
            break;
        }
        case EXPRESSION_TYPE_VARIABLE:
        {
            variableExpression_t *variableExpression = (variableExpression_t *)expression;
            baseScopeVariable_t *tempVariable = variableExpression->variable;
            int32_t tempScopeIndex = tempVariable->scopeIndex;
            heapValue_t *tempFrame;
            if (tempVariable->type == SCOPE_VARIABLE_TYPE_IMPORT
                    || tempVariable->type == SCOPE_VARIABLE_TYPE_NAMESPACE) {
                tempFrame = expression->script->globalFrame;
            } else {
                tempFrame = frame;
            }
            output = getFrameVariableLocation(tempFrame, tempScopeIndex);
            lockHyperValue(&output);
            break;
        }
        case EXPRESSION_TYPE_UNARY:
        {
            unaryExpression_t *unaryExpression = (unaryExpression_t *)expression;
            operator_t *tempOperator = unaryExpression->operator;
            value_t tempOperand = evaluateAndResolveExpression(
                frame,
                unaryExpression->operand
            );
            if (hasThrownError) {
                return output;
            }
            output.value = calculateUnaryOperator(tempOperator, tempOperand);
            lockHyperValue(&output);
            unlockValue(&tempOperand);
            break;
        }
        case EXPRESSION_TYPE_BINARY:
        {
            binaryExpression_t *binaryExpression = (binaryExpression_t *)expression;
            operator_t *tempOperator = binaryExpression->operator;
            value_t tempOperand1 = evaluateAndResolveExpression(
                frame,
                binaryExpression->operand1
            );
            if (hasThrownError) {
                return output;
            }
            value_t tempOperand2 = evaluateAndResolveExpression(
                frame,
                binaryExpression->operand2
            );
            if (hasThrownError) {
                unlockValue(&tempOperand1);
                return output;
            }
            output.value = calculateBinaryOperator(tempOperator, tempOperand1, tempOperand2);
            lockHyperValue(&output);
            unlockValue(&tempOperand1);
            unlockValue(&tempOperand2);
            break;
        }
        case EXPRESSION_TYPE_FUNCTION:
        {
            heapValue_t *tempHandle = createFunctionHandle(
                frame,
                ((customFunctionExpression_t *)expression)->customFunction
            );
            output.value = createValueFromHeapValue(tempHandle);
            lockHyperValue(&output);
            break;
        }
        case EXPRESSION_TYPE_INDEX:
        {
            indexExpression_t *indexExpression = (indexExpression_t *)expression;
            value_t sequenceValue = evaluateAndResolveExpression(
                frame,
                indexExpression->sequence
            );
            if (hasThrownError) {
                return output;
            }
            value_t indexValue = evaluateAndResolveExpression(
                frame,
                indexExpression->index
            );
            if (hasThrownError) {
                unlockValue(&sequenceValue);
                return output;
            }
            output = accessSequenceElement(sequenceValue, indexValue);
            lockHyperValue(&output);
            unlockValue(&sequenceValue);
            unlockValue(&indexValue);
            break;
        }
        case EXPRESSION_TYPE_INVOCATION:
        {
            invocationExpression_t *invocationExpression = (invocationExpression_t *)expression;
            output = invokeFunctionWithExpressions(
                frame,
                invocationExpression->function,
                &(invocationExpression->argumentList),
                true
            );
            // output is already locked here.
            break;
        }
        default:
        {
            break;
        }
    }
    return output;
}

value_t evaluateAndResolveExpression(heapValue_t *frame, baseExpression_t *expression) {
    hyperValue_t tempHyperValue = evaluateExpression(frame, expression);
    if (hasThrownError) {
        value_t output;
        output.type = VALUE_TYPE_VOID;
        return output;
    }
    value_t output = readValueFromHyperValue(tempHyperValue);
    lockValue(&output);
    unlockHyperValue(&tempHyperValue);
    return output;
}

script_t *importScriptWithExpression(heapValue_t *frame, baseExpression_t *pathExpression) {
    value_t pathValue = evaluateAndResolveExpression(frame, pathExpression);
    if (hasThrownError) {
        return NULL;
    }
    if (pathValue.type != VALUE_TYPE_STRING) {
        THROW_BUILT_IN_ERROR(TYPE_ERROR_CONSTANT, "Expected string value.");
        unlockValue(&pathValue);
        return NULL;
    }
    int8_t *moduleDirectory = pathExpression->script->moduleDirectory;
    int8_t *scriptPath = pathValue.heapValue->vector.data;
    script_t *output = importScript(moduleDirectory, scriptPath);
    unlockValue(&pathValue);
    return output;
}

void populateImportVariables(
    script_t *destinationScript,
    vector_t *variableList,
    script_t *sourceScript
) {
    scope_t *sourceScope = &(sourceScript->topLevelFunction->scope);
    heapValue_t *sourceFrame = sourceScript->globalFrame;
    heapValue_t *destinationFrame = destinationScript->globalFrame;
    for (int32_t index = 0; index < variableList->length; index++) {
        baseScopeVariable_t *destinationVariable;
        getVectorElement(&destinationVariable, variableList, index);
        int8_t *tempName = destinationVariable->name;
        baseScopeVariable_t *sourceVariable = scopeFindVariable(sourceScope, tempName, NULL);
        if (sourceVariable == NULL) {
            THROW_BUILT_IN_ERROR(
                PARSE_ERROR_CONSTANT,
                "Could not find \"%s\" in imported script.",
                tempName
            );
            return;
        }
        hyperValue_t tempValue = getFrameVariableLocation(
            sourceFrame,
            sourceVariable->scopeIndex
        );
        if (hasThrownError) {
            return;
        }
        hyperValue_t *tempValueArray = destinationFrame->frameVariableList.valueArray;
        swapHyperValueReference(tempValueArray + destinationVariable->scopeIndex, &tempValue);
    }
}

void evaluateStatement(heapValue_t *frame, baseStatement_t *statement) {
    switch(statement->type) {
        case STATEMENT_TYPE_INVOCATION:
        {
            invocationStatement_t *invocationStatement = (invocationStatement_t *)statement;
            invokeFunctionWithExpressions(
                frame,
                invocationStatement->function,
                &(invocationStatement->argumentList),
                false
            );
            break;
        }
        case STATEMENT_TYPE_VARIABLE_IMPORT:
        {
            variableImportStatement_t *importStatement = (variableImportStatement_t *)statement;
            script_t *tempScript = importScriptWithExpression(
                frame,
                importStatement->base.path
            );
            if (hasThrownError) {
                return;
            }
            populateImportVariables(
                statement->bodyPos.script,
                &(importStatement->variableList),
                tempScript
            );
            break;
        }
        case STATEMENT_TYPE_NAMESPACE_IMPORT:
        {
            namespaceImportStatement_t *importStatement = (namespaceImportStatement_t *)statement;
            script_t *tempScript = importScriptWithExpression(
                frame,
                importStatement->base.path
            );
            if (hasThrownError) {
                return;
            }
            populateImportVariables(
                statement->bodyPos.script,
                &(importStatement->namespace->variableList),
                tempScript
            );
            break;
        }
        default:
        {
            break;
        }
    }
    markAndSweepDelay += 1;
    if (markAndSweepDelay > maximumMarkAndSweepDelay) {
        markAndSweepHeapValues();
        markAndSweepDelay = 0;
    }
}

void evaluateScript(script_t *script) {
    heapValue_t *tempHandle = createFunctionHandle(
        NULL,
        script->topLevelFunction
    );
    invokeFunctionHandle(&(script->globalFrame), tempHandle, NULL);
}


