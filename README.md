
# LogiScript

A scripting language by Jack Eisenmann

## Design Goals

* Very cute
* Adorable
* Maybe actually useful

## Data Types

Every value in LogiScript has one of the following data types:

* **Number** &ndash; Double-precision floating point number
* **String** &ndash; Mutable sequence of ASCII characters
* **List** &ndash; Mutable sequence of values with any data types
* **Function** &ndash; Function handle which may be invoked
* **Void** &ndash; The absence of any value

A number literal may contain numeric digits and a decimal point. Ex: `18`, `20.5`

A character literal consists of an ASCII character enclosed in apostrophes. Character literals have the number data type. Ex: `'A'`, `'$'`

A string literal consists of ASCII characters enclosed in quotation marks. Characters in a string literal may be escaped with a backslash. Ex: `"Hello world!"`, `"Did you just say \"hello\" to me?"`

A list literal consists of comma-separated expressions enclosed in brackets. Ex: `[10, 20, 30, "Very cool"]`

Every function literal must have the following format:

```
{<arg name>, <arg name>, <arg name>...
    <statement>
    <statement>
    <statement>
    ...
}
```

Note that newlines are syntactically significant in function literals, but other whitespace is not. The example function literal below accepts two numbers and prints their sum:

```
{inputNum1, inputNum2
    SET @result, inputNum1 + inputNum2
    PRINT result
}
```

## Statements

Only two kinds of statements are possible in LogiScript:

* **Function invocation statement** &ndash; Invokes a function with a list of arguments
* **Import statement** &ndash; Imports a namespace or list of variables

Statements are separated from each other by a newline. Comments are initiated with a number sign and are terminated by a newline.

A function invocation statement may have one of the following formats:

```
# Calls a function with no arguments.
<function>
```

```
# Calls a function with one or more arguments.
<function> <arg>, <arg>, <arg>...
```

For example, the statement below retrieves the size of the given string:

```
SIZE @dest, "Please measure me"
```

If fewer arguments are provided to the function than expected, the missing arguments will contain the void value.

An import statement may have one of the following formats:

```
# Uses the specified namespace to reference
# all global variables in the given script.
$<namespace> <path>
```

```
# Imports the specified global variables from
# the given script.
$(<name>, <name>, <name>...) <path>
```

For example, the statement below imports two global variables from the given script:

```
$(SUM, AVERAGE) "./mathLib.logi"
```

## Operators and Variables

The unary at operator (`@`) declares a variable in the current scope. For example:

```
# Declares variable myNum, and stores 5 in myNum.
SET @myNum, 5
# Stores a new value in myNum.
SET myNum, 12
```

Variables in function invocations are passed by reference. For example:

```
SET @INCREMENT, {num
    SET num, num + 1
}
SET @count, 1
# The variable count is passed by reference.
INCREMENT count
# The value of count is now 2.
```

Imported namespaces and variables are global, regardless of where import statements are placed.

The binary period operator (`.`) accesses a member variable of a namespace. For example:

```
$MATH "./mathLib.logi"
SET @radius, 3.5
# PI is a global variable defined in mathLib.logi.
SET @circumference, 2 * MATH.PI * radius
```

LogiScript has the following arithmetic operators:

* `+` &ndash; Addition
* `-` &ndash; Subtraction or negation
* `*` &ndash; Multiplication
* `/` &ndash; Division
* `%` &ndash; Remainder after division

The following operators manipulate numbers converted to 32-bit unsigned integers:

* `~` &ndash; Bitwise NOT
* `|` &ndash; Bitwise OR
* `&` &ndash; Bitwise AND
* `^` &ndash; Bitwise XOR
* `<<` &ndash; Bitshift left
* `>>` &ndash; Bitshift right

Boolean values are represented by numbers in LogiScript. Zero is false, and all non-zero numbers are true. The following operators manipulate boolean values:

* `!` &ndash; Logical NOT
* `||` &ndash; Logical OR
* `&&` &ndash; Logical AND
* `^^` &ndash; Logical XOR

LogiScript has the following comparison operators:

* `==` &ndash; Value equality
* `!=` &ndash; Value inequality
* `===` &ndash; Identity equality
* `!==` &ndash; Identity inequality
* `>` &ndash; Greater than
* `>=` &ndash; Greater than or equal to
* `<` &ndash; Less than
* `<=` &ndash; Less than or equal to

A list element or string character may be accessed with the syntax below:

```
<sequence>[<index>]
```

List indices begin at zero. For example:

```
SET @myList, [90, 91, 92]
# Prints the second element, which is 91.
PRINT myList[1]
```

A function may be invoked in an expression by using this syntax:

```
<function>(<arg>, <arg>, <arg>...)
```

In this case, the first function argument provides the return value to be consumed by the parent expression. The first function argument is excluded from the parentheses in the function invocation. For example, the pieces of code below are equivalent:

```
SET @CALCULATE, {dest, x, y
    SET dest, x + y * 2
}
CALCULATE @value, 3, 4
# Prints the number 11.
PRINT value
```

```
SET @CALCULATE, {dest, x, y
    SET dest, x + y * 2
}
# Also prints the number 11.
PRINT CALCULATE(3, 4)
```

## Built-In Functions

`SET dest, value`  
Stores `value` in `dest`.

`TYPE dest, value`  
Stores the type of `value` in `dest` as a number.

`CONVERT dest, value, type`  
Changes the type of `value` to `type`, and stores the result in `dest`.

`SIZE dest, sequence`  
Stores the size of string or list `sequence` in `dest`.

`RESIZE sequence, size`  
Changes the size of string or list `sequence` to `size`. Either truncates or appends extra elements at the end of `sequence`.

`IF condition, function`  
Invokes `function` with no arguments if `condition` is true.

`LOOP function`  
Invokes `function` repeatedly with no arguments until `function` throws an uncaught value.

`THROW channel, value`  
Throws `value` on channel number `channel`. Interrupts invocation of the current function.

`CATCH dest, channel, function`  
Invokes `function` with no arguments, and stores the value thrown on channel number `channel` in `dest`. If no value is thrown on the specified channel, `dest` will be void.

`PRINT value`  
Prints `value` as text to standard output.

`PROMPT dest`  
Pauses execution until the user inputs text through standard input. Stores the input text in `dest`.

## Built-In Constants

The void constant is `VOID`.

The following boolean value constants are available:

* `TRUE`
* `FALSE`

The constants below are used by the built-in `TYPE` and `CONVERT` functions:

* `NUMBER_TYPE`
* `STRING_TYPE`
* `LIST_TYPE`
* `FUNCTION_TYPE`
* `VOID_TYPE`

Built-in errors have the format `[<error number>, <error message>]`. The channel number for built-in errors zero, and is available as the constant `ERROR_CHANNEL`.

The following error number constants are used in built-in errors:

* `TYPE_ERROR` &ndash; Thrown when a value has the wrong type
* `NUMBER_ERROR` &ndash; Thrown when a number is not in an acceptable range
* `DATA_ERROR` &ndash; Thrown when a data structure contains malformed data
* `STATE_ERROR` &ndash; Thrown when a resource is in the wrong state or missing

## Example Scripts

The script below prints the prime numbers below 100:

```
SET @MY_CHANNEL, 1

# Defines a function which determines
# whether the given number is prime.
SET @IS_PRIME, {dest, number
    SET @factor, 2
    CATCH dest, MY_CHANNEL, {
        LOOP {
            IF factor >= number, {
                THROW MY_CHANNEL, TRUE
            }
            IF number % factor == 0, {
                THROW MY_CHANNEL, FALSE
            }
            SET factor, factor + 1
        }
    }
}

# Iterates from 2 to 99.
SET @number, 2
CATCH @bupkis, MY_CHANNEL, {
    LOOP {
        IF number >= 100, {
            THROW MY_CHANNEL
        }
        IF IS_PRIME(number), {
            PRINT number
        }
        SET number, number + 1
    }
}
```

This script sorts the variable `numberList`:

```
SET @numberList, [20, 15, 300, 4, 50, 0]

PRINT "Before sorting:"
PRINT numberList

# Defines a function which calls HANDLE
# from startIndex to endIndex.
SET @ITERATE, {startIndex, endIndex, HANDLE
    SET @BREAK_CHANNEL, 1
    SET @index, startIndex
    CATCH @dummy, BREAK_CHANNEL, {
        LOOP {
            IF index >= endIndex, {
                THROW BREAK_CHANNEL
            }
            HANDLE index
            SET index, index + 1
        }
    }
}

# Performs selection sort.
ITERATE 0, SIZE(numberList) - 1, {index1
    # Finds the minimum number in the rest of numberList.
    SET @minNumberIndex, index1
    SET @minNumber, numberList[minNumberIndex]
    ITERATE index1 + 1, SIZE(numberList), {index2
        SET @tempNumber, numberList[index2]
        IF tempNumber < minNumber, {
            SET minNumberIndex, index2
            SET minNumber, tempNumber
        }
    }
    # Swaps the minimum number into the correct place.
    SET numberList[minNumberIndex], numberList[index1]
    SET numberList[index1], minNumber
}

PRINT "After sorting:"
PRINT numberList
```


