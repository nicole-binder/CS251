#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include <assert.h> 
#include <stdio.h>


#ifndef _PARSER
#define _PARSER

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.

//NOTE:  I use the included binaries!


Value *addToParseTree(Value *tree, int *depth, Value *token){
    if (token->type == OPEN_TYPE){
        *depth = *depth + 1;
    }

    if (token->type != CLOSE_TYPE){
        Value *newEntry = talloc(sizeof(Value));
        newEntry->type = CONS_TYPE;
        newEntry->c.car = token;
        newEntry->c.cdr = tree;
        return newEntry;        
    }

    else{
        Value *subTree = makeNull();


        while (tree->c.car->type != OPEN_TYPE){
            Value *poppedValue = talloc(sizeof(Value));
            poppedValue->type = CONS_TYPE;
            poppedValue->c.car = tree->c.car;
            poppedValue->c.cdr = subTree;
            subTree = poppedValue;
            tree = cdr(tree);
            if (tree->type == NULL_TYPE){
                printf("Syntax error: not enough open parentheses\n");
                tfree();
                exit(EXIT_FAILURE);
            }
        }

        tree->c.car = subTree;
        *depth = *depth - 1;
        return tree;
    }

}

Value *parse(Value *tokens){

    Value *tree = makeNull();
    int depth = 0;

    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");
    while (current->type != NULL_TYPE) {
        Value *token = car(current);
        tree = addToParseTree(tree, &depth, token);
        current = cdr(current);
    }
    if (depth != 0) {
        printf("Syntax error: not enough close parentheses\n");
        tfree();       
        exit(EXIT_FAILURE);
    }
    return tree;
}




// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.


void printSubTrees(Value *tree){
    while (tree->type != NULL_TYPE){
        if (tree->c.car->type == CONS_TYPE){
            printf("( n ");
            printSubTrees(tree->c.car);
            printf(")");
        }
        switch(tree->c.car->type){
            case BOOL_TYPE:
                if (tree->c.car->i == 0){
                    printf("#f ");
                }
                else if (tree->c.car->i == 1){
                    printf("#t ");
                }
                break;
            case INT_TYPE: 
                printf("%i ",tree->c.car->i);
                tree = car(tree);
                break;
            case DOUBLE_TYPE:
                printf("%f ",tree->c.car->d);
                break;
            case STR_TYPE:
                printf("%s ",tree->c.car->s);
                break;
            case SYMBOL_TYPE:
                printf("%s ",tree->c.car->s);
            default:
                break;
        }
        tree = cdr(tree);
    }
}

//We need to do this really annoying thing and reverse the big boy overhead list
//but we can't do that in the other function since we're recursively calling it
//and we don't want to have to use reverse in main since its a headache to keep track of.
void printTree(Value *tree){
    tree = reverse(tree);
    printSubTrees(tree);
}


#endif
