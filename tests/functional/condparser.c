#include <stdio.h>

#include "utest.h"

#define CONDPARSER_IMPLEMENTATION
#include "condparser.h"

typedef struct
{
    const char* expr;
    bool expected;
} CondParserTest;

bool condParserTestGetValue(const char* id)
{
    return strcmp(id, "true") == 0;
}

void condParserTestError(const char* msg)
{
    fprintf(stderr, "%s", msg);
}

UTEST(condparser, simple) {
    const CondParserTest tests[] = {
        { "true", true },
        { "false", false },
        { "true && true", true },
        { "true && false", false },
        { "false || true", true },
        { "false || false", false },
        { "!true", false },
        { "!false", true },
        { "true || false && false", true },           // Checks precedence of && over ||
        { "true && true || false", true },            // Checks precedence of && over ||
        { "false || true && false", false },          // Checks mixed precedence
        { "!(true && false)", true },                 // Checks negation with parentheses
        { "!true || false", false },                  // Checks precedence of ! and ||
        { "!(false || true) && true", false },        // Checks negation with mixed operators
        { "true && (false || true)", true },          // Tests parentheses around ||
        { "(true || false) && false", false },        // Tests parentheses around ||
        { "!(true && true) || (false && true)", false }, // Combination with negation and parentheses
        { "!(false || false) && (true || false)", true }, // Negation and mixed operators
        { "(!true || true) && (true || !false)", true },  // Multiple negations and operators
        { "true || !(false && true)", true },         // Negation within && condParserition
        { "(true || false) && !(true && false)", true }, // Tests with &&, ||, and !
        { "!(true && true) || false", false },        // Outer negation and || with false
        { "!((true || false) && (true && true))", false }, // Complex nested structure
        { "!!true", true },                           // Double negation
        { "!((true || false) && !(false || true))", true },  // Nested negations and operators
        { "(!((true && false) || (true || false) && !(false || !true)) && (true || false && true) || (!(true && (false || !false)) || !!false))", false } // Complex nested structure
    };

    const int numTests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < numTests; i++)
    {
        bool res = condParserEvaluate(tests[i].expr, condParserTestGetValue, condParserTestError);
        ASSERT_TRUE_MSG(res == tests[i].expected, tests[i].expr);
    }
}