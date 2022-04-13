#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"

// Read source code that is input via stdin, and return a linked list consisting of the
// tokens in the source code. Each token is represented as a Value struct instance, where
// the Value's type is set to represent the token type, while the Value's actual value
// matches the type of value, if applicable. For instance, an integer token should have
// a Value struct of type INT_TYPE, with an integer value stored in struct variable i.
// See the assignment instructions for more details. 
Value *tokenize() {

    // Prepare list of tokens
    Value *tokensList = makeNull();

    // Prepare the character stream
    char nextChar;
    nextChar = (char)fgetc(stdin);

    char curr[300] = "";
    valueType currType = NULL_TYPE;

    int inComment = 0;
    int justSawMinus = 0;
    int justSawPlus = 0;

    // Start tokenizing!
    while (nextChar != EOF) {
        if (nextChar == ';' && currType == NULL_TYPE) { // CHECK TO GO IN COMMENT MODE
            inComment = 1;
        }
        if (inComment == 1 && nextChar == '\n') { // NEW LINE TO GET OUT OF COMMENT MODE
            inComment = 0;
            currType = NULL_TYPE;
        }
        if (inComment == 0) {
            if (justSawMinus == 1 && currType == NULL_TYPE) { // JUST SAW MINUS
                justSawMinus = 0;
                if (isdigit(nextChar)) { // NEGATIVE NUMBER
                    currType = INT_TYPE;
                    strcpy(curr, "-");
                    continue;
                } else { // SYMBOL -
                    Value *temp = talloc(sizeof(Value));
                    temp->type = SYMBOL_TYPE;
                    temp->s = (char *) talloc(strlen("-") + 1);
                    strcpy(temp->s, "-");
                    tokensList = cons(temp, tokensList);
                    currType = NULL_TYPE;
                    strcpy(curr, "");
                    continue;
                }
            } else if (justSawPlus == 1 && currType == NULL_TYPE) { // SYMBOL +
                justSawPlus = 0;
                Value *temp = talloc(sizeof(Value));
                temp->type = SYMBOL_TYPE;
                temp->s = (char *) talloc(strlen("+") + 1);
                strcpy(temp->s, "+");
                tokensList = cons(temp, tokensList);
                currType = NULL_TYPE;
                strcpy(curr, "");
                continue;
            }
            if (currType == NULL_TYPE && nextChar == '(') { // OPEN
                Value *temp = talloc(sizeof(Value));
                temp->type = OPEN_TYPE;
                temp->s = (char *) talloc(sizeof(char) + 1);
                strcpy(temp->s, "(");
                tokensList = cons(temp, tokensList);
            } else if (currType == NULL_TYPE && nextChar == ')') { // CLOSE
                Value *temp = talloc(sizeof(Value));
                temp->type = CLOSE_TYPE;
                temp->s = (char *) talloc(sizeof(char) + 1);
                strcpy(temp->s, ")");
                tokensList = cons(temp, tokensList);
            } else if (currType == BOOL_TYPE) { // END OF BOOLEAN
                Value *temp = talloc(sizeof(Value));
                temp->type = BOOL_TYPE;
                if (nextChar == 't') {
                    temp->i = 1;
                } else if (nextChar == 'f') {
                    temp->i = 0;
                } else {
                    printf("Syntax error (readBoolean): boolean was not #t or #f\n");
                    texit(0);
                }
                tokensList = cons(temp, tokensList);
                currType = NULL_TYPE;
            } else if (currType == NULL_TYPE && nextChar == '#') { // START OF BOOLEAN
                currType = BOOL_TYPE;
            } else if (currType == STR_TYPE && nextChar != '\"') { // MIDDLE OF STRING
                strncat(curr, &nextChar, 1);
            } else if (nextChar == '\"') {
                if (currType == STR_TYPE) { // END OF STRING
                    Value *temp = talloc(sizeof(Value));
                    temp->type = STR_TYPE;
                    temp->s = (char *) talloc(strlen(curr) + 1);
                    strcpy(temp->s, curr);
                    tokensList = cons(temp, tokensList);
                    currType = NULL_TYPE;
                    strcpy(curr, "");
                } else if (currType == NULL_TYPE) { // START OF STRING
                    currType = STR_TYPE;
                } else {
                    printf("Syntax error: not a string but is reading \"\n");
                    texit(0);
                }
            } else if (currType == NULL_TYPE && isdigit(nextChar)) { // START OF NUMBER
                currType = INT_TYPE;
                strncat(curr, &nextChar, 1);
            } else if (currType == NULL_TYPE && nextChar == '-') { // SAW A MINUS
                justSawMinus = 1;
            } else if (currType == NULL_TYPE && nextChar == '+') { // SAW A PLUS
                justSawPlus = 1;
            } else if (currType == INT_TYPE && nextChar == '.') { // DECIMAL NUMBER
                currType = DOUBLE_TYPE;
                strncat(curr, &nextChar, 1);
            } else if (currType == INT_TYPE && isdigit(nextChar)) { // MIDDLE OF NUMBER
                strncat(curr, &nextChar, 1);
            } else if (currType == DOUBLE_TYPE && isdigit(nextChar)) { // MIDDLE OF DECIMAL
                strncat(curr, &nextChar, 1);
            } else if (currType == INT_TYPE && isdigit(nextChar) == 0) { // END OF INT
                Value *temp = talloc(sizeof(Value));
                char *ptr;
                temp->type = currType;
                temp->i = strtod(curr, &ptr);
                tokensList = cons(temp, tokensList);
                currType = NULL_TYPE;
                strcpy(curr, "");
                continue;
            } else if (currType == DOUBLE_TYPE && isdigit(nextChar) == 0) { // END OF DOUBLE
                Value *temp = talloc(sizeof(Value));
                char *ptr;
                temp->type = currType;
                temp->d = strtod(curr, &ptr);
                tokensList = cons(temp, tokensList);
                currType = NULL_TYPE;
                strcpy(curr, "");
                continue;
            } else if (currType == SYMBOL_TYPE) { 
                if (isdigit(nextChar) || isalpha(nextChar) || nextChar == '!' 
                    || nextChar == '$' || nextChar == '%' || nextChar == '&' || nextChar == '*' 
                    || nextChar == '/' || nextChar == ':' || nextChar == '<' || nextChar == '=' 
                    || nextChar == '>' || nextChar == '?' || nextChar == '~' || nextChar == '_' 
                    || nextChar == '^' || nextChar == '+' || nextChar == '-' || nextChar == '.') {
                    // MIDDLE OF SYMBOL
                    strncat(curr, &nextChar, 1);
                } else { // END OF SYMBOL                
                    Value *temp = talloc(sizeof(Value));
                    temp->type = SYMBOL_TYPE;
                    temp->s = (char *) talloc(strlen(curr) + 1);
                    strcpy(temp->s, curr);
                    tokensList = cons(temp, tokensList);
                    currType = NULL_TYPE;
                    strcpy(curr, "");
                    continue;
                }
            } else if (nextChar == ' ') { // SPACE
                currType = NULL_TYPE;
            } else if (isalpha(nextChar) || nextChar == '!' || nextChar == '$' 
                || nextChar == '%' || nextChar == '&' || nextChar == '*' || nextChar == '/' 
                || nextChar == ':' || nextChar == '<' || nextChar == '=' || nextChar == '>' 
                || nextChar == '?' || nextChar == '~' || nextChar == '_' || nextChar == '^') {
                // START OF SYMBOL
                currType = SYMBOL_TYPE;
                strncat(curr, &nextChar, 1);
            } else if (nextChar == '\n') { // NEW LINE
            } else {
                // printf("Syntax Error: don't know what's being read.\n");
                // texit(0);
            }
        }

        // Read next character
        nextChar = (char)fgetc(stdin);
    }

    // Reverse the tokens list, to put it back in order
    Value *reversedList = reverse(tokensList);
    return reversedList;
}

// Display the contents of the list of tokens, along with associated type information.
// The tokens are displayed one on each line, in the format specified in the instructions.
void displayTokens(Value *list) {
    Value *temp = list;

    while (isNull(temp) == 0) {
        if (car(temp)->type == INT_TYPE) {
            printf("%i:integer\n",car(temp)->i);
        } else if (car(temp)->type == DOUBLE_TYPE) {
            printf("%f:double\n",car(temp)->d);
        } else if (car(temp)->type == STR_TYPE) {
            printf("\"%s\":string\n",car(temp)->s);
        } else if (car(temp)->type == OPEN_TYPE) {
            printf("(:open\n");
        } else if (car(temp)->type == CLOSE_TYPE) {
            printf("):close\n");
        } else if (car(temp)->type == BOOL_TYPE) {
            if (car(temp)->i == 0) {
                printf("#f:boolean\n");
            } else {
                printf("#t:boolean\n");
            }
        } else if (car(temp)->type == SYMBOL_TYPE) {
            printf("%s:symbol\n",car(temp)->s);
        }
        temp = cdr(temp);
    }
}
