#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "tokenizer.h"

// Prints all strings in the given svec.
void
print_tokens(svec* tokens)
{
    for (int ii = 0; ii < tokens->size; ++ii) {
        printf("%s\n", svec_get(tokens, ii));
    }
}

// Returns true if the input is a special operator
// that needs no preceding or following spaces.
int
isoperator(char input)
{
    return (input == '&') || (input == '|') || (input == '<')
        || (input == '>') || (input == ';');
}

// Return a substring of the input from ii to nn.
char*
copy_substring(char* input, int ii, int nn)
{
    // Copy over memory into token, with one extra byte for the null terminator.
    char* substring = malloc(nn + 1);
    memcpy(substring, input + ii, nn);
    substring[nn] = 0;
    return substring;
}

// Reads the next token from the input starting at ii.
char*
read_token(char* input, int ii)
{
    int nn = 0;
    while ((!isspace(input[ii + nn])) && (!isoperator(input[ii + nn]))) {
        ++nn;
    }

    return copy_substring(input, ii, nn);
}

// Reads the operator from the input starting at ii.
char*
read_operator(char* input, int ii)
{
    int nn = 0;
    while (input[ii + nn] != 0 && (!isspace(input[ii + nn])) && (!isalnum(input[ii + nn]))) {
        ++nn;
    }

    return copy_substring(input, ii, nn);
}

// Returns an svec containing all tokens from the given input,
// in the order they appear.
// Inspired By: Nat Tuck
svec*
tokenize(char* input)
{
    int nn = strlen(input);
    svec* tokens = make_svec();

    int ii = 0;
    while(ii < nn) {
        if (isspace(input[ii])) {
            ++ii;
            continue;
        }

        char* token;
        if (isoperator(input[ii])) {
            token = read_operator(input, ii);
        }
        else {
            token = read_token(input, ii);
        }

        svec_push(tokens, token);
        ii += strlen(token);

        free(token);
    }

    return tokens;
}
