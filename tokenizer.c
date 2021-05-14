#include "value.h"
#include <stdio.h>
#include "talloc.h"
#include "linkedlist.h"
#include <stdlib.h>
#include <string.h>


#ifndef _TOKENIZER
#define _TOKENIZER

// This code is very much so not working.  We've been trying to debug it for two late-days worth of time and have made progress but
// It's still buggy af.
// We really don't know where all the issues are coming from, there's some logical stuff here that we're not understanding 
// why it's incorrect.
// We'll be talking with prof in office hours to fix the code but
// RN it's kind of a meme.
// But hey at least it's not printing '):open' anymore which it did for 3 hours.
// But the reinforce the point that we don't know what's going on, it basically just stopped doing that for no reason so.
// :shrug: rip grade.


Value *addList (Value *data, Value *list){
    Value *newNode = talloc(sizeof(Value));
    newNode->type = CONS_TYPE;
    newNode->c.car = data;
    newNode->c.cdr = list;

    return newNode;
}

char* concat(const char *s1, const char *s2)
{
    char *result = talloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize() {
    char charRead;
    Value *list = makeNull();
    Value *tempList = makeNull();

    charRead = (char)fgetc(stdin);

    while (charRead != EOF) {

        //Open
        if (charRead == '(') {
            printf("ENTERED OPEN -- ");
            Value *openParen = talloc(sizeof(Value));
            openParen->type = OPEN_TYPE;
            char *paren = "(";
            openParen->s = talloc((strlen(paren)+1)*sizeof(char));
            strcpy(openParen->s, paren);
            tempList = addList(openParen, list);
            list = tempList;
        }

        //Close
        else if (charRead == ')') {
            printf("ENTERED CLOSED????");
            Value *closedParen = talloc(sizeof(Value));
            closedParen->type = CLOSE_TYPE;
            char *paren = ")";
            closedParen->s = talloc((strlen(paren)+1)*sizeof(char));
            strcpy(closedParen->s, paren);
            list = addList(closedParen, list);
        }
        //Str
         else if (charRead == '\"') {
             int count = 0;
             char *buildStr = "\"";
             charRead = (char)fgetc(stdin);
             while (charRead !='\"' || count <= 300 || charRead != EOF) {
                 buildStr = concat(buildStr, &charRead);
                 count = count + 1;

             }
             Value *string = talloc(sizeof(Value));
             string->type = STR_TYPE;
             string->s = talloc((strlen(buildStr)+1)*sizeof(char));
             strcpy(string->s, buildStr);
             list = addList(string, list);
         }

         //Bool
         else if (charRead == '#'){
             charRead = (char)fgetc(stdin);
             if (charRead == 't'){
                 Value *tru = talloc(sizeof(Value));
                 tru->type = BOOL_TYPE;
                 tru->i = 1;
                 list = addList(tru, list);
             }        
             else if (charRead == 'f'){
                 Value *fal = talloc(sizeof(Value));
                 fal->type = BOOL_TYPE;
                 fal->i = 0;
                 list = addList(fal, list);
             }
             else {
                 perror("# may only begin booleans");
                exit(EXIT_FAILURE);
             }

             charRead = (char)fgetc(stdin);
             if (charRead != ' ' || charRead != '\r'){
                perror("# may only begin booleans");
                exit(EXIT_FAILURE);
             }
         }
         //Numbers
         else if (charRead == '+'||  charRead == '-'|| charRead == '1'|| charRead == '2' 
         || charRead == '3'|| charRead == '4'|| charRead == '5'|| charRead == '6'
         || charRead == '7'|| charRead == '8'|| charRead == '9'|| charRead == '0'
         || charRead == '.'){

            char *buildNum;    
            int counter = 0;
            int TYPE = 0;
            // 0 = Int
            // 1 = Double


            while (charRead != ' ' || charRead != '\r' || charRead != EOF || counter <= 300){
            
            //Double
                if (charRead == '.'){
                    TYPE = 1;
                }
                buildNum = concat(buildNum, &charRead);
                counter = counter + 1;
                charRead = (char)fgetc(stdin);

            }

            Value *num = talloc(sizeof(Value));
            char* end;
            if ((int)buildNum == '+' || (int)buildNum == '-'){
                num->type = SYMBOL_TYPE;
                num->s = talloc((strlen(buildNum)+1)*sizeof(char));
                strcpy(num->s, buildNum);
                list = addList(num, list);
            }

            else if (TYPE == 1){
                num->type = DOUBLE_TYPE;
                num->d = strtod(buildNum, &end);
            }

            else{
                num->type = INT_TYPE;
                num->i = strtol(buildNum, &end, 10);
            }
            list = addList(num, list);  
         }


         //Symbols

         //<initial>
         else if (charRead == 'a' || charRead == 'b' || charRead == 'c' ||
         charRead == 'd' || charRead == 'e' || charRead == 'f' || charRead == 'g' ||
         charRead == 'h' || charRead == 'i' || charRead == 'j' || charRead == 'k' || 
         charRead == 'l' || charRead == 'm' || charRead == 'n' || charRead == 'o' ||
         charRead == 'p' || charRead == 'q' || charRead == 'r' || charRead == 's' ||
         charRead == 't' || charRead == 'u' || charRead == 'v' || charRead == 'w' ||
         charRead == 'x' || charRead == 'y' || charRead == 'z' || charRead == 'A' || 
         charRead == 'B' || charRead == 'C' ||
         charRead == 'D' || charRead == 'E' || charRead == 'F' || charRead == 'G' ||
         charRead == 'H' || charRead == 'I' || charRead == 'J' || charRead == 'K' || 
         charRead == 'L' || charRead == 'M' || charRead == 'N' || charRead == 'O' ||
         charRead == 'P' || charRead == 'Q' || charRead == 'R' || charRead == 'S' ||
         charRead == 'T' || charRead == 'U' || charRead == 'V' || charRead == 'W' ||
         charRead == 'X' || charRead == 'Y' || charRead == 'Z' ||charRead == '!'  || charRead == '$'
         || charRead == '%' || charRead == '&' || charRead == '*' || charRead == '/' || charRead == ':' || charRead == '<' || charRead == '='
         || charRead == '>' || charRead == '?' || charRead == '~' || charRead == '_' || charRead == '^') {

            int count = 0;
            char *buildSym;

             while (charRead != ' ' || charRead != '\r'|| charRead != EOF || count <= 300){
                if (  
                    charRead == 'a' || charRead == 'b' || charRead == 'c' ||
         charRead == 'd' || charRead == 'e' || charRead == 'f' || charRead == 'g' ||
         charRead == 'h' || charRead == 'i' || charRead == 'j' || charRead == 'k' || 
         charRead == 'l' || charRead == 'm' || charRead == 'n' || charRead == 'o' ||
         charRead == 'p' || charRead == 'q' || charRead == 'r' || charRead == 's' ||
         charRead == 't' || charRead == 'u' || charRead == 'v' || charRead == 'w' ||
         charRead == 'x' || charRead == 'y' || charRead == 'z' || 
         charRead == 'A' || 
         charRead == 'B' || charRead == 'C' ||
         charRead == 'D' || charRead == 'E' || charRead == 'F' || charRead == 'G' ||
         charRead == 'H' || charRead == 'I' || charRead == 'J' || charRead == 'K' || 
         charRead == 'L' || charRead == 'M' || charRead == 'N' || charRead == 'O' ||
         charRead == 'P' || charRead == 'Q' || charRead == 'R' || charRead == 'S' ||
         charRead == 'T' || charRead == 'U' || charRead == 'V' || charRead == 'W' ||
         charRead == 'X' || charRead == 'Y' || charRead == 'Z' || charRead == '!'  || 
         charRead == '$' || charRead == '%' || charRead == '&' || charRead == '*' || charRead == '/' || charRead == ':' || 
         charRead == '<' || charRead == '=' || charRead == '>' || charRead == '?' || charRead == '~' || charRead == '_' || 
         charRead == '^' || charRead == '+' ||  charRead == '-' || charRead == '1' || charRead == '2' || charRead == '3' || 
         charRead == '4' || charRead == '5' || charRead == '6' || charRead == '7' || charRead == '8' || charRead == '9' || 
         charRead == '0' || charRead == '.') {

                    buildSym = concat(buildSym, &charRead);
                    count = count + 1;
                    }

                //Comments -> don't tokenize
                else if (charRead == ';'){
                    while (charRead != '\r') {
                        charRead = (char)fgetc(stdin);
                    }
                }
                    
                else{
                    perror("symbols present not recognized by Scheme");
                    exit(EXIT_FAILURE);
                } 

                charRead = (char)fgetc(stdin);
            }
            

             Value *symbol = talloc(sizeof(Value));
             symbol->type = SYMBOL_TYPE;
             symbol->s = talloc((strlen(buildSym)+1)*sizeof(char));
             strcpy(symbol->s, buildSym);
             list = addList(symbol, list);

         } // Bracket for this else-if

        charRead = (char)fgetc(stdin);
    } //While loop bracket


    Value *revList = reverse(list);
    return revList;
} // Method bracket

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list){
    switch (list->type) {
    case INT_TYPE:
        printf("%i:integer\n", list->i);
        break;
    case DOUBLE_TYPE:
        printf("%f:double\n", list->d);
        break;
    case STR_TYPE:
        printf("%s:string\n", list->s);
        break;
    case CONS_TYPE:
        displayTokens(list->c.car);
        displayTokens(list->c.cdr);
        break;
    case NULL_TYPE:
        printf("WE HIT NULL \n");
        break;
    case PTR_TYPE:
        break;
    case OPEN_TYPE:
        printf("(:open\n");
    case CLOSE_TYPE:
        printf("):close\n");
    case BOOL_TYPE:
        printf("%s:bool\n", list->s);
    case SYMBOL_TYPE:
        printf("%s:symbol\n", list->s);
    default:
        break;

}
}

#endif
