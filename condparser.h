#ifndef CONDPARSER_H_INCLUDED
#define CONDPARSER_H_INCLUDED

/*
    condparser.h - A simple logical expression parser

    The parser supports the following operators:
        - && (AND)
        - || (OR)
        - ! (NOT)

    The parser supports parentheses and identifiers, which are looked up using a callback function.
    It's reentrant, does not use any global variables and does not allocate memory.
    Constants are not supported, but you can use identifiers to represent them.

    I use this parser in my game engine to toggle runtime configuration options based on platform, build configuration, etc.

    USAGE
    ==================================================

    Do this:
        #define CONDPARSER_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation.

    You need to define two callback functions to use the parser:
        - bool getValue(const char* id): This function should return the value of an identifier.
        - void error(const char* message): This function is called to output an error message. Do not add newlines.

    The only API function is:
        bool condParserEvaluate(const char* expr, PFN_condParserGetValue getValue, PFN_condParserError errorFn);

    It takes a logical expression, a callback function to get the value of an identifier, and a callback function to output errors,
    and returns the result of the expression.

    Optionally, you can define CONDPARSER_BUILD_TEST to build a test program.

    You can define the following macros to customise the parser:
        - CONDPARSER_STRNCMP: The string comparison function to use. Default: strncmp
        - CONDPARSER_ID_LENGTH: The maximum length of an identifier. Default: 32

    string.h is only included if CONDPARSER_STRNCMP is not defined.

    LICENSE
    ==================================================

    zlib/libpng license

    Copyright (c) 2024 Andrei Tsurkan

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.

        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.

        3. This notice may not be removed or altered from any source distribution.
*/

#include <stdbool.h>

typedef bool(*PFN_condParserGetValue)(const char*);
typedef void (*PFN_condParserError)(const char*);

#ifdef __cplusplus
extern "C" {
#endif

    bool condParserEvaluate(const char* expr, PFN_condParserGetValue getValue, PFN_condParserError errorFn);

#ifdef __cplusplus
}
#endif

#ifdef CONDPARSER_IMPLEMENTATION

#ifndef CONDPARSER_STRNCMP
#include <string.h>
#define CONDPARSER_STRNCMP strncmp
#endif

#ifndef CONDPARSER_ID_LENGTH
#define CONDPARSER_ID_LENGTH 32
#endif

typedef enum
{
    CondParserToken_ID,
    CondParserToken_LParen,
    CondParserToken_RParen,
    CondParserToken_Not,
    CondParserToken_And,
    CondParserToken_Or,
    CondParserToken_End
} CondParserTokenType;

typedef struct
{
    CondParserTokenType type;
    char id[CONDPARSER_ID_LENGTH];
} CondParserToken;

typedef struct
{
    const char* cur;
    CondParserToken curToken;
    bool error;
    PFN_condParserGetValue getValue;
    PFN_condParserError errorFn;
} CondParserContext;

#ifdef __cplusplus
extern "C" {
#endif

    static bool condParserIsSpace(char c)
    {
        return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\v') || (c == '\f') || (c == '\r');
    }

    static bool condParserIsAlpha(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static bool condParserIsAlnum(char c)
    {
        return condParserIsAlpha(c) || (c >= '0' && c <= '9');
    }

    static void condParserPrintError(const CondParserContext* ctx, const char* msg)
    {
        if (ctx->errorFn)
        {
            ctx->errorFn(msg);
        }
    }

    static void condParserPrintToken(const CondParserContext* ctx)
    {
        switch (ctx->curToken.type)
        {
        case CondParserToken_ID:
            condParserPrintError(ctx, "ID [");
            condParserPrintError(ctx, ctx->curToken.id);
            condParserPrintError(ctx, "]");
            break;
        case CondParserToken_LParen:
            condParserPrintError(ctx, "LPAREN");
            break;
        case CondParserToken_RParen:
            condParserPrintError(ctx, "RPAREN");
            break;
        case CondParserToken_Not:
            condParserPrintError(ctx, "NOT");
            break;
        case CondParserToken_And:
            condParserPrintError(ctx, "AND");
            break;
        case CondParserToken_Or:
            condParserPrintError(ctx, "OR");
            break;
        case CondParserToken_End:
            condParserPrintError(ctx, "END");
            break;
        default:
            condParserPrintError(ctx, "<<UNKNOWN TOKEN>>");
            break;
        }
    }

    static void condParserNextToken(CondParserContext* ctx)
    {
        // skip ws
        while (condParserIsSpace(*ctx->cur)) ctx->cur++;

        if (*ctx->cur == '\0') {
            ctx->curToken.type = CondParserToken_End;
        }
        else if (CONDPARSER_STRNCMP(ctx->cur, "&&", 2) == 0) {
            ctx->curToken.type = CondParserToken_And;
            ctx->cur += 2;
        }
        else if (CONDPARSER_STRNCMP(ctx->cur, "||", 2) == 0) {
            ctx->curToken.type = CondParserToken_Or;
            ctx->cur += 2;
        }
        else if (*ctx->cur == '!') {
            ctx->curToken.type = CondParserToken_Not;
            ctx->cur++;
        }
        else if (*ctx->cur == '(') {
            ctx->curToken.type = CondParserToken_LParen;
            ctx->cur++;
        }
        else if (*ctx->cur == ')') {
            ctx->curToken.type = CondParserToken_RParen;
            ctx->cur++;
        }
        else if (condParserIsAlpha(*ctx->cur)) {
            ctx->curToken.type = CondParserToken_ID;
            int i = 0;
            while (condParserIsAlnum(*ctx->cur) && i < (CONDPARSER_ID_LENGTH - 1)) {
                ctx->curToken.id[i++] = *ctx->cur++;
            }
            ctx->curToken.id[i] = '\0';
        }
        else {
            const char ctxCur[2] = { *ctx->cur, '\0' };
            condParserPrintError(ctx, "Unknown character: ");
            condParserPrintError(ctx, ctxCur);
            condParserPrintError(ctx, "\n");
            ctx->cur++;

            ctx->error = true;
        }
    }

    static bool condParserParseExpr(CondParserContext* ctx);

    static bool condParserParsePrimary(CondParserContext* ctx)
    {
        if (ctx->curToken.type == CondParserToken_ID) {
            bool value = ctx->getValue(ctx->curToken.id);
            condParserNextToken(ctx);
            return value;
        }
        else if (ctx->curToken.type == CondParserToken_LParen) {
            condParserNextToken(ctx); // consume '('
            bool value = condParserParseExpr(ctx);

            if (ctx->curToken.type != CondParserToken_RParen) {
                condParserPrintError(ctx, "Error: expected ')', found: ");
                condParserPrintToken(ctx);
                condParserPrintError(ctx, "\n");
                return 0;
            }
            condParserNextToken(ctx); // consume ')'
            return value;
        }
        else {
            condParserPrintError(ctx, "Error: expected identifier or '('\n");
            return 0;
        }
    }

    static bool condParserParseNot(CondParserContext* ctx)
    {
        int notCount = 0;

        // count nots
        while (ctx->curToken.type == CondParserToken_Not)
        {
            notCount++;
            condParserNextToken(ctx);
        }

        bool res = condParserParsePrimary(ctx);

        // negate if odd
        if (notCount % 2 != 0)
        {
            res = !res;
        }

        return res;
    }

    static bool condParserParseAnd(CondParserContext* ctx)
    {
        bool value = condParserParseNot(ctx);
        while (ctx->curToken.type == CondParserToken_And) {
            condParserNextToken(ctx);
            bool res = condParserParseNot(ctx);

            value = value && res;
        }
        return value;
    }

    static bool condParserParseOr(CondParserContext* ctx)
    {
        bool value = condParserParseAnd(ctx);
        while (ctx->curToken.type == CondParserToken_Or) {
            condParserNextToken(ctx);

            bool res = condParserParseAnd(ctx);
            value = value || res;
        }

        return value;
    }

    static bool condParserParseExpr(CondParserContext* ctx)
    {
        bool res = condParserParseOr(ctx);
        return res;
    }

    bool condParserEvaluate(const char* expr, PFN_condParserGetValue getValue, PFN_condParserError errorFn)
    {
        CondParserContext ctx;
        ctx.cur = expr;
        ctx.error = false;
        ctx.getValue = getValue;
        ctx.errorFn = errorFn;

        condParserNextToken(&ctx);

        return condParserParseExpr(&ctx);
    }

#ifdef __cplusplus
}
#endif

#endif // CONDPARSER_IMPLEMENTATION

#ifdef CONDPARSER_BUILD_TEST

#include <stdio.h>

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

int main(int argc, char** argv)
{
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
        { "!((true || false) && !(false || true))", true }  // Nested negations and operators
    };

    const int numTests = sizeof(tests) / sizeof(tests[0]);

    bool success = true;

    for (int i = 0; i < numTests; i++)
    {
        fprintf(stdout, "Testing: %s\n", tests[i].expr);

        bool res = condParserEvaluate(tests[i].expr, condParserTestGetValue, condParserTestError);
        if (res != tests[i].expected)
        {
            fprintf(stderr, "Test %d failed: %s\n", i, tests[i].expr);
            success = false;
        }
    }

    if (success)
    {
        puts("All tests passed!");
    }
    else
    {
        puts("Some tests failed!");
    }

    return success ? 0 : 1;
}

#endif // CONDPARSER_BUILD_TEST

#endif // CONDPARSER_H_INCLUDED