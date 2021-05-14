#include <stdio.h>
#include <stdlib.h>
#include "value.h"
#include "interpreter.h"
#include "linkedlist.h"
#include "talloc.h"
#include <string.h> 
#include <stdbool.h>
#include "tokenizer.h"
#include <math.h>


void printCons(Value *tree);

Value *lookUpSymbol(Value *tree, Frame *frame) {
  Value *current = frame->bindings;
  while (current->type != NULL_TYPE) {
    if (!strcmp(current->c.car->c.car->s, tree->s)) {

      return current->c.car->c.cdr;
      }
    else {
      current = current->c.cdr;
    }
  }

  if (frame->parent != NULL) {
    return lookUpSymbol(tree, frame->parent);
  }
  
  else {
 //   printf("First Binding: %s\n", frame->bindings->c.car->c.car->s);
  //  printf("Symbol: %s\n", tree->s);
    printf("Symbol/Function not recognized\n");;
    printf("Tree type:%i\n", tree->type);
    printf("Tree data:%s\n", tree->s);
    tfree();
    exit(EXIT_FAILURE);
    return tree;

  }
}

Value *evalIf(Value *args, Frame *frame) {
  Value *condition = eval(args->c.car, frame);
  if (condition->i == 1) {  
    return eval(car(cdr(args)), frame);

  }
  else if (condition->i == 0) {
    return eval((car(cdr(cdr(args)))), frame);
  }

  else {
    printf("Expresion Doesn't Evaluate to T/F");
    tfree();
    exit(EXIT_FAILURE);
    return args;

  }
}

Value *evalLet(Value *args, Frame *frame) {

  Frame *childFrame = talloc(sizeof(Frame));
  childFrame->parent = frame;
  childFrame->bindings = makeNull();
  Value *ptr = car(args);
  
  while (ptr->type != NULL_TYPE) {
    Value *new = talloc(sizeof(Value));
    new->type = CONS_TYPE;
    new->c.cdr = childFrame->bindings;
    Value *data = talloc(sizeof(Value));
    data->type = CONS_TYPE;
    new->c.car = data;
    data->c.car = ptr->c.car->c.car;
    data->c.cdr = eval(ptr->c.car->c.cdr->c.car, frame);
    childFrame->bindings = new;
    ptr = cdr(ptr);
  }
  

    while (args->c.cdr->type != NULL_TYPE){
      args = args->c.cdr;
      eval(args->c.car, childFrame);
  }

  return eval(args->c.car, childFrame);
}

//
Value *evalLetrec(Value *args, Frame *frame) {

if (args->c.cdr->type == NULL_TYPE){
  printf("Invalid usage of letrec, no second argument");
  tfree();
  exit(EXIT_FAILURE);
}

  Frame *childFrame = talloc(sizeof(Frame));
  childFrame->parent = frame;
  childFrame->bindings = makeNull();
  Value *ptr = car(args);

  Value *UNDEFINED = talloc(sizeof(Frame));
  UNDEFINED->type = BOOL_TYPE;
  UNDEFINED->i = 0;
  
  //Bind LHS
  while (ptr->type != NULL_TYPE) {
    Value *new = talloc(sizeof(Value));
    new->type = CONS_TYPE;
    new->c.cdr = childFrame->bindings;
    Value *data = talloc(sizeof(Value));
    data->type = CONS_TYPE;
    new->c.car = data;
    data->c.car = ptr->c.car->c.car;
    data->c.cdr = UNDEFINED;
    //data->c.cdr = eval(ptr->c.car->c.cdr->c.car, frame);
    childFrame->bindings = new;
    ptr = cdr(ptr);
  }

  //Evaluate RHS, stick in new list to be bound after all are evaled.
  ptr = car(args);
  Value *evaledValues = makeNull();

  while (ptr->type != NULL_TYPE){
    Value *newEval = talloc(sizeof(Value));
    newEval->type = CONS_TYPE;
    newEval->c.cdr = evaledValues;

    Value *evalResult = talloc(sizeof(Value));
    evalResult = eval(ptr->c.car->c.cdr->c.car, childFrame);

    newEval->c.car = evalResult;
    evaledValues = newEval;
    ptr = ptr->c.cdr;
  }
  
  //Assigning evaluated RHS to bound LHS
  Value *bindingsPtr = childFrame->bindings;
  Value *evaledPtr = evaledValues;
    while (evaledPtr->type != NULL_TYPE){
      bindingsPtr->c.car->c.cdr = evaledPtr->c.car;
      bindingsPtr = bindingsPtr->c.cdr;
      evaledPtr = evaledPtr->c.cdr;
  }


  //Iterate to of args while eval'ing along the way
  args = args->c.cdr;
  while (args->c.cdr->type != NULL_TYPE){
    eval(args->c.car->c.car, childFrame);
    args = args->c.cdr;
   }
  //Eval the last thing in the context of the child-frame.
  return eval(args->c.car, childFrame);
}


Value *evalDefine(Value *args, Frame *frame){

  Value *voidValue = talloc(sizeof(Value));
  voidValue->type = VOID_TYPE;

  // Error checking
  if (args->type != CONS_TYPE){
    if (args->c.cdr->type != CONS_TYPE || args->c.car->type != SYMBOL_TYPE){
      printf("Invalid Define Format");
      tfree();
      exit(EXIT_FAILURE);
    }
    printf("Invalid Define Format");
    tfree();
    exit(EXIT_FAILURE);
  }


  Value *newBinding = talloc(sizeof(Value));
  Value *bindingData = talloc(sizeof(Value));
  newBinding->type = CONS_TYPE;
  bindingData->type = CONS_TYPE;
  newBinding->c.cdr = frame->bindings;
  newBinding->c.car = bindingData;
  
  bindingData->c.car = args->c.car;
  bindingData->c.cdr = eval(args->c.cdr->c.car, frame);

  frame->bindings = newBinding;

  return voidValue;
}

Value *evalLambda(Value *tree, Frame *frame){
  Value *newClosure = talloc(sizeof(Value));

  if (tree->type == NULL_TYPE || tree->c.cdr->type == NULL_TYPE || tree->c.cdr->c.cdr->type != NULL_TYPE){
    printf("Invalid Lambda Declaration You Twat");
    tfree();
    exit(EXIT_FAILURE);
  }

  newClosure->type = CLOSURE_TYPE;
  newClosure->cl.paramNames = tree->c.car;
  newClosure->cl.functionCode = tree->c.cdr->c.car;
  newClosure->cl.frame = frame;
  return newClosure;
}

Value *evalLetStar(Value *args, Frame *frame){
  
  Value *ptr = car(args);
    Frame *ogFrame = talloc(sizeof(Frame));
    ogFrame->parent = frame;
    ogFrame->bindings = makeNull();
 

  while (ptr->type != NULL_TYPE) {
    if (ogFrame->bindings->type != NULL_TYPE) {
      Frame *childFrame = talloc(sizeof(Frame));
      childFrame->parent = ogFrame;
      childFrame->bindings = makeNull();
      ogFrame = childFrame;
    }

    Value *new = talloc(sizeof(Value));
    new->type = CONS_TYPE;
    new->c.cdr = ogFrame->bindings;
    Value *data = talloc(sizeof(Value));
    data->type = CONS_TYPE;
    new->c.car = data;
    data->c.car = ptr->c.car->c.car;
    data->c.cdr = eval(ptr->c.car->c.cdr->c.car, ogFrame);
    ogFrame->bindings = new;
    ptr = cdr(ptr);
  }

  while (args->c.cdr->type != NULL_TYPE){
    args = args->c.cdr;
    eval(args->c.car, ogFrame);
  }

  return eval(args->c.car, ogFrame);
}


void evalSetBang(Value *args, Frame *frame){
  Value *pointer = frame->bindings;
  while (pointer->type != NULL_TYPE){
    if (!strcmp(args->c.car->s, pointer->c.car->c.car->s)){
      pointer->c.car->c.cdr = eval(args->c.cdr->c.car, frame);
      return;
    }
    else if (frame->parent == NULL){
      printf("Symbol Not Found, Bad Set!\n");
      tfree();
      exit(EXIT_FAILURE);
    }
    else {
      frame = frame->parent;
      pointer = frame->bindings;
    }
  }
}

Value *evalAnd(Value *args, Frame *frame){
  Value *andBool = talloc(sizeof(Value));
  andBool->type = BOOL_TYPE;
  andBool->i = 1;

  if (args->type == NULL_TYPE){
    return andBool;
  }

  if (args->c.cdr->type == NULL_TYPE){
    andBool = eval(args->c.car, frame);
    return andBool;
  }
  else {
    args = args->c.cdr;
    while (args->type != NULL_TYPE){
      if (eval(args->c.car, frame)->i == 0){
        andBool->i = 0;
        return andBool;
      }
    args = args->c.cdr;
    }
  }

  return andBool;
}

Value *evalOr(Value *args, Frame *frame){
  Value *orBool = talloc(sizeof(Value));
  orBool->type = BOOL_TYPE;
  orBool->i = 0;

  if (args->type == NULL_TYPE){
    return orBool;
  }

  if (args->c.cdr->type == NULL_TYPE){
    orBool = eval(args->c.car, frame);
    return orBool;
  }
  else {
    args = args->c.cdr;
    while (args->type != NULL_TYPE){
      if (eval(args->c.car, frame)->i == 1){
        orBool->i = 1;
        return orBool;
      }
    args = args->c.cdr;
    }
  }
  return orBool;
}

Value *evalBegin(Value *args, Frame *frame){

  if (args->type == NULL_TYPE){
    Value *voidReturn = talloc(sizeof(Value));
    voidReturn->type = VOID_TYPE;
    return voidReturn;
  }

  if (args->c.cdr->type == NULL_TYPE){
    return eval(args->c.car, frame);
  }
  else {
    args = args->c.cdr;
    while (args->c.cdr->type != NULL_TYPE){
      eval(args->c.car, frame);
      args = args->c.cdr;
    }
    return eval(args->c.car, frame);
  }
}

Value *evalCond(Value *args, Frame *frame){
  Value *voidReturn = talloc(sizeof(Value));
  voidReturn->type = VOID_TYPE;
  if (args->type == NULL_TYPE){
    return voidReturn;
  }
  while (args->type != NULL_TYPE){
    if (eval(args->c.car->c.car, frame)->i == 1) {
      Value *pointer = args->c.car->c.cdr;
      while (pointer->c.cdr->type != NULL_TYPE) {
        eval(pointer->c.cdr, frame);
        pointer = pointer->c.cdr;
      }
      return eval(pointer->c.car, frame);
    }
    args = args->c.cdr;
  }
  return voidReturn;
}


Value *evalEach(Value *tree, Frame *frame){
  Value *curr = tree;
  Value *evaledArgs = makeNull();
  while (curr->type != NULL_TYPE){
    Value *arg = talloc(sizeof(Value));
    arg->type = CONS_TYPE;
    arg->c.car = eval(curr->c.car, frame);
    arg->c.cdr = evaledArgs;
    curr = curr->c.cdr;
    evaledArgs = arg;
  }
    return reverse(evaledArgs);
}


Value *apply(Value *func, Value *args){

  if (func->type == PRIMITIVE_TYPE){
    
    return func->pf(args);
  }

  else{
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->bindings = makeNull();
    newFrame->parent = func->cl.frame;
    Value *curr = func->cl.paramNames;

    while (curr->type != NULL_TYPE &&  args->type != NULL_TYPE){
      
      if (curr->type == NULL_TYPE|| args->type == NULL_TYPE){
        printf("Invalid number of arguments\n");
        tfree();
        exit(EXIT_FAILURE);
      }

      Value *newBinding = talloc(sizeof(Value));
      Value *bindingData = talloc(sizeof(Value));

      newBinding->type = CONS_TYPE;
      bindingData->type = CONS_TYPE;
      newBinding->c.cdr = newFrame->bindings;
      newBinding->c.car = bindingData;
      bindingData->c.car = curr->c.car;
      bindingData->c.cdr = args->c.car;

      curr = curr->c.cdr;
      args = args->c.cdr;
      newFrame->bindings = newBinding;
    }
    return eval(func->cl.functionCode, newFrame);
  }

}


Value *eval(Value *tree, Frame *frame) {
   switch (tree->type)  {
     case INT_TYPE: {
        return tree;
        break;
     }
     case DOUBLE_TYPE: {
        return tree;
        break;
     }
     case STR_TYPE: {
        return tree;
        break;
     }
     case SYMBOL_TYPE: {
        return lookUpSymbol(tree, frame);
        break;
     }
     case BOOL_TYPE: {
        return tree;
        break;
     }

     case NULL_TYPE: {
       return tree;
       break;
     }


     case CONS_TYPE: {
       
       if (tree->c.car->type == CONS_TYPE){
         return eval(tree->c.car, frame);
         break;
       }


        Value *first = car(tree);
        Value *args = cdr(tree);


        if (!strcmp(first->s,"if")) {
            return evalIf(args,frame);
        }

        else if (!strcmp(first->s,"let")) {
            return evalLet(args,frame);
        }

        else if (!strcmp(first->s, "quote")) {
          if (args->c.cdr->type != NULL_TYPE){
            printf("Invalid quote usage\n");
            tfree();
            exit(EXIT_FAILURE);
          }

            return args->c.car;
        }

        else if (!strcmp(first->s, "define")){
          return evalDefine(args, frame);

        }

        else if (!strcmp(first->s, "lambda")){
          return evalLambda(args, frame);
        }

        else if (!strcmp(first->s, "let*")){
          return evalLetStar(args, frame);
        }

        else if (!strcmp(first->s, "set!")){
          evalSetBang(args, frame);
          return tree;
        }
        else if (!strcmp(first->s, "and")){
          return evalAnd(args, frame);
        }
        else if (!strcmp(first->s, "or")){
          return evalOr(args, frame);
        }
        else if (!strcmp(first->s, "begin")){
          return evalBegin(args, frame);
        }
        else if (!strcmp(first->s, "cond")){
          return evalCond(args, frame);
        }
        else if (!strcmp(first->s,"letrec")){
          return evalLetrec(args, frame);
        }
        
        else {
          Value *eavledOperator = eval(first, frame);
          Value *evaledArgs = evalEach(args, frame);
          return apply(eavledOperator,evaledArgs);





          printf("Nonexistant Function Called\n");
          tfree();
          exit(EXIT_FAILURE);

        }
        break;
     }

     default:
      return tree;
        

    }
}


void printCons(Value *tree){
  while (tree->type != NULL_TYPE){

      bool space = (tree->c.cdr->type != NULL_TYPE);

      switch(tree->c.car->type){

        case BOOL_TYPE: {
          if (space == true){
            if (tree->c.car->i == 1) {
              printf("#t ");
            }
            else {
              printf("#f ");
            }  
          }

          else if (space == false){
            if (tree->c.car->i == 1) {
              printf("#t");
            }
            else {
              printf("#f");
            }
          }
          break;
        }


        case INT_TYPE: {
          if (space == true){
            printf("%i ",tree->c.car->i);
          }
          else if (space == false){
            printf("%i",tree->c.car->i); 
          }
          else{
            printf("This code mega sucks, fix it int\n");
            tfree();
            exit(EXIT_FAILURE);
          }
          break;
        }

        case DOUBLE_TYPE: {
          if (space == true){
            printf("%f ",tree->c.car->d);
          }
          else if (space == false){
            printf("%f",tree->c.car->d); 
          }
          else{
            printf("This code mega sucks, fix it double\n");
            tfree();
            exit(EXIT_FAILURE);
          }
          break;
        }

        case STR_TYPE: {
          if (space == true){
            printf("%s ",tree->c.car->s);
          }
          else if (space == false){
            printf("%s",tree->c.car->s); 
          }
          else{
            printf("This code mega sucks, fix it string\n");
            tfree();
            exit(EXIT_FAILURE);
          }
          break;
        }

        case SYMBOL_TYPE: {
          if (space == true){
            printf("%s ",tree->c.car->s);
          }
          else if (space == false){
            printf("%s",tree->c.car->s); 
          }
          else{
            printf("This code mega sucks, fix it string\n");
            tfree();
            exit(EXIT_FAILURE);
          }
          break;
        }

        case CONS_TYPE: {
          if (space == true){
            printf("(");
            printCons(tree->c.car);
            printf(") ");
          }
          else if (space == false){
            printf("(");
            printCons(tree->c.car);
            printf(")"); 
          }
          break;
        }

        case NULL_TYPE:{
          printf("()");
          break;
        }

        default:
          printf("%i\n",tree->c.car->type);
          printf("Interpreter Error\n");
          tfree();
          exit(EXIT_FAILURE);
          } // End of Switch



    if (tree->c.cdr->type != CONS_TYPE && tree->c.cdr->type != NULL_TYPE){
      printf(". ");

        switch(tree->c.cdr->type){

        case BOOL_TYPE: {
            if (tree->c.cdr->i == 1) {
              printf("#t");
            }
            else {
              printf("#f");
            }
            break;
          }

        case INT_TYPE: {
            printf("%i",tree->c.cdr->i); 
          }
          break;

        case DOUBLE_TYPE: {
            printf("%f",tree->c.cdr->d); 
          break;
        }

        case STR_TYPE: {
            printf("%s",tree->c.cdr->s); 
          break;
        }

        case SYMBOL_TYPE: {
            printf("%s",tree->c.cdr->s); 
          break;
        }

        case NULL_TYPE:{
          printf("()");
          break;
        }

        case CLOSURE_TYPE:{
          printf("Closure");
          break;
        }

        default:
          printf("%i\n",tree->c.cdr->type);
          printf("Wuh oh... this is not a good error!");
          tfree();
          exit(EXIT_FAILURE);
          } // End of Switch 2
      break;
    }

        tree = tree->c.cdr;
  }// End of While
} // End of printCons


Value *primitiveAdd(Value *args){
  Value *sum = talloc(sizeof(Value));
  sum->type = DOUBLE_TYPE;
  sum->d = 0;

  
  
  
  while (args->type != NULL_TYPE){
    Value *evalValue = args->c.car;
    if (evalValue->type == INT_TYPE){
      sum->d = sum->d + evalValue->i;
    }
    else if (evalValue->type == DOUBLE_TYPE){
      sum->d = sum->d + evalValue->d;
    }
    else {
      printf("Invalid Function Call : Bad Params\n");
      tfree();
      exit(EXIT_FAILURE);
    }

    args = args->c.cdr;
  }



  if ((int)sum->d == sum->d){
    sum->type = INT_TYPE;
    sum->i = sum->d;
  }
  return sum;
}

Value *primitiveNull(Value *args){
  if (args->c.cdr->type != NULL_TYPE || args->type == NULL_TYPE){
    printf("Incorrect usage of null?\n");
    tfree();
    exit(EXIT_FAILURE);
  }

  Value *returnBool = talloc(sizeof(Value));
  returnBool->type = BOOL_TYPE;
 
  if (args->type != CONS_TYPE){
    printf("Expected argument to null?, recieved none");
    tfree();
    exit(EXIT_FAILURE);      
    return returnBool;
  }


  if (args->c.car->type == NULL_TYPE && args->c.cdr->type == NULL_TYPE){
    returnBool->i = 1;
    return returnBool;
  }
  else if (args->c.car->type != NULL_TYPE || args->c.cdr->type == NULL_TYPE){
    returnBool->i = 0;
    return returnBool;
  }

  else{
    printf("uh oh")
    tfree();
    exit(EXIT_FAILURE);
  }
}

Value *primitiveCar(Value *args){
    if (args->type != CONS_TYPE || args->c.cdr->type != NULL_TYPE){
      printf("Incorrect usage of Car?\n");
      tfree();
      exit(EXIT_FAILURE);
  }

  if (args->type == NULL_TYPE){
    printf("cannot car null\n");
    tfree();
    exit(EXIT_FAILURE);
  }
  return args->c.car->c.car;

}

Value *primitiveCdr(Value *args){
 
    if (args->type != CONS_TYPE || args->c.cdr->type != NULL_TYPE){
      printf("Incorrect usage of cdr??\n");
      tfree();
      exit(EXIT_FAILURE);
  }

  if (args->type == NULL_TYPE){
    printf("cannot cdr null\n");
    tfree();
    exit(EXIT_FAILURE);
  }
  return args->c.car->c.cdr;

}

Value *primitiveCons(Value *args){

  if (args->type != CONS_TYPE || args->c.cdr->type != CONS_TYPE || args->c.cdr->c.cdr->type != NULL_TYPE){
      printf("Incorrect usage of cons\n");
      tfree();
      exit(EXIT_FAILURE); 
  }

  Value *newCons = talloc(sizeof(Value));
  newCons->type = CONS_TYPE;

  newCons->c.car = args->c.car;
  newCons->c.cdr = args->c.cdr->c.car;
  return newCons;

}

Value *primitiveMult(Value *args){
  printCons(args);
  printf("\n");

  Value *product = talloc(sizeof(Value));
  product->type = DOUBLE_TYPE;
  product->d = 1;
  
  while (args->type != NULL_TYPE){
    Value *evalValue = args->c.car;
    if (evalValue->type == INT_TYPE){
      product->d = product->d * evalValue->i;
    }
    else if (evalValue->type == DOUBLE_TYPE){
      product->d = product->d * evalValue->d;
    }
    else {
      printf("Invalid Mult - Bad Params\n");
      tfree();
      exit(EXIT_FAILURE);
    }

    args = args->c.cdr;
  }

    if ((int)product->d == product->d){
    product->type = INT_TYPE;
    product->i = product->d;
  }


  return product;

}

Value *primitiveSubtract(Value *args){

 if (args->type == NULL_TYPE){
    printf("Must declare args for /\n");
    tfree();
    exit(EXIT_FAILURE);
  }

    Value *sum = talloc(sizeof(Value));
    sum->type = DOUBLE_TYPE;

    switch(args->c.car->type){
      case INT_TYPE:{
        sum->d = (double)args->c.car->i;
        break;
      }
      case DOUBLE_TYPE:{
        sum->d = args->c.car->d;
        break;
      }
      default:
        printf("oh noes.  Prim Divide");
        break;
    }

    args = args->c.cdr;
  
  
  while (args->type != NULL_TYPE){
    Value *evalValue = args->c.car;
    if (evalValue->type == INT_TYPE){
      sum->d = sum->d - evalValue->i;
    }
    else if (evalValue->type == DOUBLE_TYPE){
      sum->d = sum->d - evalValue->d;
    }
    else {
      printf("Invalid Function Call : Bad Params\n");
      tfree();
      exit(EXIT_FAILURE);
    }

    args = args->c.cdr;
  }

    if ((int)sum->d == sum->d){
    sum->type = INT_TYPE;
    sum->i = sum->d;
  }


  return sum;

}

Value *primitiveLessThan(Value *args){
  Value *lessThanBool = talloc(sizeof(Value));
  lessThanBool->type = BOOL_TYPE;
  lessThanBool->i = 1;
  Value *firstValue = args;

  if (args->type == NULL_TYPE) {
    printf("Must have at least one argument for <\n");
    tfree();
    exit(EXIT_FAILURE);
  }

  if (args->c.cdr->type == NULL_TYPE) {
    return lessThanBool;
  }

  switch(args->c.car->type){
      case INT_TYPE:{
        firstValue->d = (double)args->c.car->i;
        break;
      }
      case DOUBLE_TYPE:{
        firstValue->d = args->c.car->d;
        break;
      }
      default:
        printf("oh noes.  Prim Less Than");
        break;
    }
  
  args = args->c.cdr;

  while (lessThanBool->i == 1) {

    while (args->type != NULL_TYPE) {

      printf("%f\n", firstValue->d);

      switch(args->c.car->type){

        case INT_TYPE:{
          if (!(firstValue->d < (double)args->c.car->i)) {
            lessThanBool->i = 0;        }
          firstValue->d = (double)args->c.car->i;
          break;
        }

        case DOUBLE_TYPE:{
          if (!(firstValue->d < args->c.car->d)) {
            lessThanBool->i = 0;
          }
          firstValue->d = args->c.car->d;
          break;
        }

        default:
          printf("oh noes.  Prim Less Than\n");
          break;
      }
    args = args->c.cdr;

    }
    return lessThanBool;
  }
  return lessThanBool;

}

Value *primitiveGreaterThan(Value *args){
  Value *greaterThanBool = talloc(sizeof(Value));
    greaterThanBool->type = BOOL_TYPE;
    greaterThanBool->i = 1;
    Value *firstValue = args;

    if (args->type == NULL_TYPE) {
      printf("Must have at least one argument for <\n");
      tfree();
      exit(EXIT_FAILURE);
    }

    if (args->c.cdr->type == NULL_TYPE) {
      return greaterThanBool;
    }

    switch(args->c.car->type){
        case INT_TYPE:{
          firstValue->d = (double)args->c.car->i;
          break;
        }
        case DOUBLE_TYPE:{
          firstValue->d = args->c.car->d;
          break;
        }
        default:
          printf("oh noes.  Prim Greater Than");
          break;
      }
    
    args = args->c.cdr;

    while (greaterThanBool->i == 1) {

      while (args->type != NULL_TYPE) {

        printf("%f\n", firstValue->d);

        switch(args->c.car->type){

          case INT_TYPE:{
            if (!(firstValue->d > (double)args->c.car->i)) {
              greaterThanBool->i = 0;        }
            firstValue->d = (double)args->c.car->i;
            break;
          }

          case DOUBLE_TYPE:{
            if (!(firstValue->d > args->c.car->d)) {
              greaterThanBool->i = 0;
            }
            firstValue->d = args->c.car->d;
            break;
          }

          default:
            printf("oh noes.  Prim Great Than\n");
            break;
        }
      args = args->c.cdr;

      }
      return greaterThanBool;
    }
    return greaterThanBool;

  }


Value *primitiveDivide(Value *args){
  
  if (args->type == NULL_TYPE){
    printf("Must declare args for /\n");
    tfree();
    exit(EXIT_FAILURE);
  }

    Value *product = talloc(sizeof(Value));
    product->type = DOUBLE_TYPE;

    switch(args->c.car->type){
      case INT_TYPE:{
        product->d = (double)args->c.car->i;
        break;
      }
      case DOUBLE_TYPE:{
        product->d = args->c.car->d;
        break;
      }
      default:
        printf("oh noes.  Prim Divide");
        break;
    }

    args = args->c.cdr;
    
    while (args->type != NULL_TYPE){
      Value *evalValue = args->c.car;
      if (evalValue->type == INT_TYPE){


        if (evalValue->i == 0){
          printf("Div by 0");
          tfree();
          exit(EXIT_FAILURE);
        }


        product->d = product->d / (double)evalValue->i;
      }


      else if (evalValue->type == DOUBLE_TYPE){
        if (evalValue->d == 0){
          printf("Div by 0");
          tfree();
          exit(EXIT_FAILURE);
        }
        product->d = product->d / evalValue->d;
      }
      else {
        printf("Invalid Function Call : Bad Params\n");
        tfree();
        exit(EXIT_FAILURE);
      }

      args = args->c.cdr;
    }

    if ((int)product->d == product->d){
    product->type = INT_TYPE;
    product->i = product->d;
  }


    return product;
  }

Value *primitiveMod(Value *args){
  Value *mod = talloc(sizeof(Value));
  mod->type = INT_TYPE;
  mod->i = 0;

  if (args->type == NULL_TYPE || args->c.cdr->type == NULL_TYPE || args->c.cdr->c.cdr->type != NULL_TYPE ) {
    printf("Invalid number of arguments for modulo\n");
    tfree();
    exit(EXIT_FAILURE);
  }

  if (args->c.car->type == INT_TYPE && args->c.cdr->c.car->type == INT_TYPE){
    if (args->c.cdr->c.car->i == 0) {
      printf("modulo: undefined for 0\n");
      tfree();
      exit(EXIT_FAILURE);
    }
    else {
      mod->i = args->c.car->i % args->c.cdr->c.car->i;
    }
  }
  else {
    printf("Invalid use of modulo\n");
    tfree();
    exit(EXIT_FAILURE);
  }  
  return mod;
}

Value *primitiveEqual(Value *args){
  Value *equalBool = talloc(sizeof(Value));
  equalBool->type = BOOL_TYPE;
  equalBool->i = 0;

  if (args->type == NULL_TYPE || args->c.cdr->type == NULL_TYPE || args->c.cdr->c.cdr->type != NULL_TYPE ) {
    printf("Invalid number of arguments for equal\n");
    tfree();
    exit(EXIT_FAILURE);
  }


  switch (args->c.car->type)  {
      case INT_TYPE: {
        if (args->c.cdr->c.car->type == DOUBLE_TYPE) {
          if ((double)args->c.car->i == args->c.cdr->c.car->d){
            equalBool->i = 1;
          }
        }
        else if (args->c.cdr->c.car->type == INT_TYPE) {
          if (args->c.car->i == args->c.cdr->c.car->i){
            equalBool->i = 1;
          }
        }
        break;
      }
      case DOUBLE_TYPE: {
        if (args->c.cdr->c.car->type == INT_TYPE) {
          if (args->c.car->d == (double)args->c.cdr->c.car->i){
            equalBool->i = 1;
          }
        }
        else if (args->c.cdr->c.car->type == DOUBLE_TYPE) {
          if (args->c.car->d == args->c.cdr->c.car->d){
            equalBool->i = 1;
          }
        }
        break;
      }
      case STR_TYPE: {
        if (args->c.cdr->c.car->type != STR_TYPE) {
          break;
        }
        if (!strcmp(args->c.car->s,args->c.cdr->c.car->s)){
          equalBool->i = 1;
        }
        break;
      }
      case BOOL_TYPE: {
        if (args->c.cdr->c.car->type != BOOL_TYPE) {
          break;
        }
        if (args->c.car->i == args->c.cdr->c.car->i){
          equalBool->i = 1;
        }
        break;
      }
      default: {
        printf("Invalid use of equal?\n");
        tfree();
        exit(EXIT_FAILURE);
      }
  }


 // tfree();
 // exit(EXIT_FAILURE);
  return equalBool;
}



void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
  
  Value *bindingsCons = talloc(sizeof(Value));
  bindingsCons->type = CONS_TYPE;
  bindingsCons->c.cdr = frame->bindings;

  
  Value *dataCons = talloc(sizeof(Value));
  dataCons->type = CONS_TYPE;

  bindingsCons->c.car = dataCons;
  
  Value *functionSymbol = talloc(sizeof(Value));
  functionSymbol->type = SYMBOL_TYPE;
  functionSymbol->s = name;

  dataCons->c.car = functionSymbol;
  
  Value *primValue = talloc(sizeof(Value));
  primValue->type = PRIMITIVE_TYPE;
  primValue->pf = function;

  dataCons->c.cdr = primValue;

  frame->bindings = bindingsCons;
}

void interpret(Value *tree){
  Frame *theFrame = talloc(sizeof(Frame));
  theFrame->bindings = makeNull();
  theFrame->parent = NULL;

  bind("+",primitiveAdd,theFrame);
  bind("null?",primitiveNull,theFrame);
  bind("car",primitiveCar,theFrame);
  bind("cdr",primitiveCdr,theFrame);
  bind("cons",primitiveCons,theFrame);
  bind("-",primitiveSubtract,theFrame);
  bind("*",primitiveMult,theFrame);
  bind("/",primitiveDivide,theFrame);
  bind("<",primitiveLessThan,theFrame);
  bind(">",primitiveGreaterThan,theFrame);
  bind("=",primitiveEqual,theFrame);
  bind("modulo", primitiveMod,theFrame);


  while (tree->type != NULL_TYPE){
    Value *answer = eval(tree->c.car, theFrame);
    switch (answer->type)  {
      case INT_TYPE: {
          printf("%i\n",answer->i);
          break;
      }
      case DOUBLE_TYPE: {
          printf("%f\n",answer->d);
          break;
      }
      case STR_TYPE: {
          printf("%s\n",answer->s);
          break;
      }
      case BOOL_TYPE: {
          if (answer->i == 1) {
            printf("#t\n");
          }
          else {
            printf("#f\n");
          }
          break;
      }

      case CONS_TYPE: {
        printf("(");
        printCons(answer);
        printf(")");
        printf("\n");
        break;

        }
      
      case NULL_TYPE: {
        printf("()\n");
      }
      
      case VOID_TYPE: {
        break;
      }
    
      default:
          printf("Not a valid value INTERPRET\n");
          printf("%i\n", answer->type);
          tfree();
          exit(EXIT_FAILURE);
          break;
      }
      tree = tree->c.cdr;
  }
}