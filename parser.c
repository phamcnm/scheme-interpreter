#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "tokenizer.h"


// traverse vertically to the depth you want to add the token to
// add the token to the front of whatever that is
// if it was not the head of the tree now, attach the parent to it
// return the head of the tree
Value* addToParseTree(Value *tree, Value *token, int *depth) {
    int justSawOpen = 0;
    Value *curr = tree;
    Value *parent = tree;
    Value *toadd = talloc(sizeof(Value));
    for (int i = 1; i <= *depth; i++) {
        curr = car(curr);      
    }
    for (int i = 2; i <= *depth; i++) {
        parent = car(parent);
    }
    if (token->type == OPEN_TYPE) {
        justSawOpen = 1;
        toadd = makeNull();
        (*depth)++;
    } else if (token->type == CLOSE_TYPE) {
        justSawOpen = 0;
        curr = reverse(curr);
        if (*depth > 0) {
            parent->c.car = curr;
        }
        (*depth)--;
        return tree;
    } else {
        justSawOpen = 0;
        toadd->type = token->type;
        if (token->type == INT_TYPE || token->type == BOOL_TYPE) {
            toadd->i = token->i;
        } else if (token->type == DOUBLE_TYPE) {
            toadd->d = token->d;
        } else if (token->type == STR_TYPE || token->type == SYMBOL_TYPE) {
            toadd->s = (char *) talloc(strlen(token->s) + 1);
            strcpy(toadd->s, token->s);
        }
    }
    curr = cons(toadd, curr);
    if (*depth == 0) {
        return curr;
    } else if (*depth == 1) {
        if (justSawOpen == 1) {
            return curr;
        } else {
            parent->c.car = curr;
            return tree;
        }
    } else {
        parent->c.car = curr;
        return tree;
    }
}

// Return a pointer to a parse tree representing the structure of a Scheme 
// program, given a list of tokens in the program.
Value *parse(Value *tokens) {

    assert(tokens != NULL && "Error (parse): null pointer");

    Value *tree = makeNull();
    int depth = 0;

    Value *current = tokens;
    while (current->type != NULL_TYPE) {
        Value *token = car(current);
        tree = addToParseTree(tree, token, &depth);
        current = cdr(current);
    }

    if (depth != 0) {
        printf("syntax error");
    }

    return reverse(tree);
}


// Print a parse tree to the screen in a readable fashion. It should look 
// just like Scheme code (use parentheses to mark subtrees).
void printTree(Value *tree) {
    display(tree);
}

