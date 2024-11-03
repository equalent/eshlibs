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

#define COND_TEST(expr) { #expr, expr }

UTEST(condparser, simple) {
    const CondParserTest tests[] = {
        COND_TEST(true),
        COND_TEST(false),
        COND_TEST(true && true),
        COND_TEST(true && false),
        COND_TEST(false || true),
        COND_TEST(false || false),
        COND_TEST(!true),
        COND_TEST(!false),
        COND_TEST(true || false && false),           // Checks precedence of && over ||
        COND_TEST(true && true || false),            // Checks precedence of && over ||
        COND_TEST(false || true && false),          // Checks mixed precedence
        COND_TEST(!(true && false)),                 // Checks negation with parentheses
        COND_TEST(!true || false),                  // Checks precedence of ! and ||
        COND_TEST(!(false || true) && true),        // Checks negation with mixed operators
        COND_TEST(true && (false || true)),          // Tests parentheses around ||
        COND_TEST((true || false) && false),        // Tests parentheses around ||
        COND_TEST(!(true && true) || (false && true)), // Combination with negation and parentheses
        COND_TEST(!(false || false) && (true || false)), // Negation and mixed operators
        COND_TEST((!true || true) && (true || !false)),  // Multiple negations and operators
        COND_TEST(true || !(false && true)),         // Negation within && condParserition
        COND_TEST((true || false) && !(true && false)), // Tests with &&, ||, and !
        COND_TEST(!(true && true) || false),        // Outer negation and || with false
        COND_TEST(!((true || false) && (true && true))), // Complex nested structure
        COND_TEST(!!true),                           // Double negation
        COND_TEST(!((true || false) && !(false || true))),  // Nested negations and operators
        COND_TEST((!((true && false) || (true || false) && !(false || !true)) && (true || false && true) || (!(true && (false || !false)) || !!false))) // Complex nested structure
    };

    const int numTests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < numTests; i++)
    {
        bool res = condParserEvaluate(tests[i].expr, condParserTestGetValue, condParserTestError);
        ASSERT_TRUE_MSG(res == tests[i].expected, tests[i].expr);
    }
}