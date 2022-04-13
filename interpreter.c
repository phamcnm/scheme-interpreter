#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "value.h"
#include "linkedlist.h"
#include "interpreter.h"
#include "talloc.h"

// prints out the answer for each evaluated s-expression
void print(Value *ans){
	switch (ans->type)
		{
		case INT_TYPE:{
			printf("%i\n", ans->i);
			break;
		}
		case DOUBLE_TYPE:{
			printf("%f\n", ans->d);
			break;
		}
		case STR_TYPE:{
			printf("%s\n", ans->s);
			break;
		}
		case SYMBOL_TYPE:{
			printf("%s\n", ans->s);
			break;
		}
		case BOOL_TYPE:{
			if (ans->i == 1) {
				printf("#t\n");
			} else {
				printf("#f\n");
			}
			break;
		}
		case NULL_TYPE:{
			printf("()\n");
			break;
		}
		case CLOSURE_TYPE:{
			printf("#<procedure>\n");
			break;
		}
		case CONS_TYPE:{
			Value *cur = ans;
			printf("(");
			while (cur->type != NULL_TYPE) {
				if (cur->type == CONS_TYPE){
					print(car(cur));
					cur = cdr(cur);
					if (cur->type != NULL_TYPE){
						printf(" ");
					}
				} else {
					printf(".");
					if (cur->type != NULL_TYPE){
						printf(" ");
					}
					print(cur);
					break;
				}	
			}
			printf(") \n");
			break;
		}
		default:{
			break;
		}
	}
}

// finds a symbol in a frame
Value *lookUpSymbol(Value *tree, Frame *frame){
	Value *currentBinding = frame->bindings;
	while (currentBinding->type != NULL_TYPE){
		if (currentBinding->type != CONS_TYPE){
			printf("Evaluation error: not a linkedlist in lookUpSymbol\n");
			texit(0);
		}
		Value *assignment = currentBinding->c.car;
		if (!strcmp(assignment->c.car->s, tree->s)){
			if (assignment->c.car->type == UNSPECIFIED_TYPE || assignment->c.cdr->type == UNSPECIFIED_TYPE){
				printf("Evaluation error: letrec used a variable before all were computed.\n");
				texit(0);
			}
			return assignment->c.cdr;
		} else {
			currentBinding = cdr(currentBinding);
		}
	}
	if (frame->parent == NULL){
		printf("Evaluation error: found unbound variable: %s.\n", tree->s);
		texit(0);
	}
	return lookUpSymbol(tree, frame->parent);
}

// checks if there are duplicate atoms in a frame
bool checkDuplicatesFrame(Value *atom, Frame *frame){
	Value *curr = frame->bindings;
	bool found = false;
	while (curr->type != NULL_TYPE){
		if (!strcmp(car(car(curr))->s, atom->s)){
			return true;
		}
		curr = cdr(curr);
	}
	return false;
}

// evaluates if
Value *evalIf(Value *args, Frame *frame) {
	// Evaluate test experssion
	bool result;
	Value *test = car(args);
	if (test->type == BOOL_TYPE){
		if (test->i == 0){
			result = false;
		} else {
			result = true;
		} 
	} else if (test->type == SYMBOL_TYPE) {\
		Value *symbol = lookUpSymbol(test, frame);
		if (symbol->type == BOOL_TYPE){
			if (test->i == 0) {
				result = false;
			} else {
				result = true;
			}
		} else {
			result = true;
		}
	} else if (test->type == CONS_TYPE) {
		Value *sExp = eval(test, frame);
		if (sExp->type == BOOL_TYPE){
			if (sExp->i == 0) {
				result = false;
			} else {
				result = true;
			}
		} else if (sExp->type == CLOSURE_TYPE || sExp->type == PRIMITIVE_TYPE) {
			Value *funcResult = eval(test, frame);
			if (funcResult->type == BOOL_TYPE){
				if (funcResult->i == 0) {
					result = false;
				} else {
					result = true;
				}
			} else {
				result = true;
			}
		} else {
			result = true;
		}
	} else {
		result = true;
	}

	args = cdr(args);
	if (args->type == NULL_TYPE){
		printf("Evaluation error: Found if statement but found only two expressions. Expected four.\n");
		texit(0);
	}
	Value *thirdArg = car(args);
	args = cdr(args);
	if (args->type == NULL_TYPE){
		printf("Evaluation error: Found if statement but found only three expressions. Expected four.\n");
		texit(0);
	}
	Value *fourthArg = car(args);
	args = cdr(args);
	if (args->type != NULL_TYPE){
			printf("Evaluation error: Found if statement but found more than four expressions. Expected four.\n");
			texit(0);
	}
	if (result == true){
		return eval(thirdArg, frame);
	} else {
		return eval(fourthArg, frame);
	}
}

// evaluates let
Value *evalLet(Value *args, Frame *frame){
	Frame *newFrame = talloc(sizeof(Frame));
	newFrame->parent = frame;
	newFrame->bindings = makeNull();

	if (args->type != CONS_TYPE){
		printf("Evaluation error: Found let statement did not find variable bindings => Should be in form (let ((x 1) ...) body).\n");
		texit(0);
	}
	// Add variable bindings to the frame
	Value *vars = car(args);
	while (vars->type != NULL_TYPE){
		if (vars->type != CONS_TYPE){
			printf("Evaluation error: bad form in let.\n");
			texit(0);
		}
		Value *curr2 = car(vars);
		if (curr2->type ==  NULL_TYPE){
			printf("Evaluation error: null binding in let.\n");
			texit(0);
		} else if (curr2->type !=  CONS_TYPE) {
			printf("Evaluation error: bad form in let\n");
			texit(0);
		}
		Value *variable = car(curr2);
		if (variable->type != SYMBOL_TYPE){
			printf("Evaluation error: left side of a let pair doesn't have a variable.\n");
			texit(0);
		}
		if (checkDuplicatesFrame(variable, newFrame)){
			printf("Evaluation error: duplicate variable in let\n");
			texit(0);
		}
		if (curr2->c.cdr->type == NULL_TYPE){
			printf("Evaluation error: bad form in let\n");
			texit(0);
		}
		Value *theValue = eval(car(cdr(curr2)), frame);
		Value *assignment = cons(variable, theValue);
		newFrame->bindings = cons(assignment, newFrame->bindings);

		vars = cdr(vars);
	}

	// Evaluate body with newFrame
	args = cdr(args);
	if (args->type != CONS_TYPE){
		printf("Evaluation error: no args following the bindings in let.\n");
		texit(0);
	}
	Value *returnVal;
	while (args->c.cdr->type != NULL_TYPE) {
		returnVal = eval(car(args), newFrame);
		args = cdr(args);
	}
	returnVal = eval(car(args), newFrame);
	return returnVal;
}

// evaluates quote
Value *evalQuote(Value *args, Frame *frame){
	Value *expr = args;
	if (args->type != CONS_TYPE){
		printf("Evaluation error: no expression following the quote statement\n");
		texit(0);
	}
	args = cdr(args);
	if (args->type != NULL_TYPE){
		printf("Evaluation error: multiple arguments to quote\n");
		texit(0);
	}
	return car(expr);
}

// evaluates define
Value *evalDefine(Value *args, Frame *frame){
	if (args->type == NULL_TYPE)
	{
		printf("Evaluation error: Found define statement did not find bindings => Should be in form (define x 3).\n");
		texit(0);
	}
	
	Value *var = car(args);
	if (var->type != SYMBOL_TYPE)
	{
		printf("Evaluation error: Found define statement did not find variable symbol => Should be in form (define x 3).\n");
		texit(0);
	}
	
	Value *expr = cdr(args);
	if (expr->type == NULL_TYPE)
	{
		printf("Evaluation error: Found define statement did not find variable binding => Should be in form (define x 3).\n");
		texit(0);
	}
	Value *theValue= eval(car(expr), frame);
	
	Value *assignment = cons(var, theValue);
	assignment->type = VOID_TYPE;
	frame->bindings = cons(assignment, frame->bindings);
	return assignment;
}

// evaluates lambda
Value *evalLambda(Value *args, Frame *frame){
	Value *newClosure = talloc(sizeof(Value));
	newClosure->type = CLOSURE_TYPE;
	if (args->type == NULL_TYPE){
		printf("Evaluation error: no args following lambda.\n");
		texit(0);
	}
	Value *params = car(args);
	if (params->type != CONS_TYPE && params->type != NULL_TYPE){
		printf("Evaluation error: Incorrect Format of Parameters\n");
		texit(0);
	}
	newClosure->cl.paramNames = makeNull();
	while (params->type != NULL_TYPE) {
		Value *newParamName = car(params);
		if (newParamName->type != SYMBOL_TYPE){
			printf("Evaluation error: formal parameters for lambda must be symbols.\n");
			texit(0);
		}
		Value *cur = newClosure->cl.paramNames;
		while (cur->type != NULL_TYPE){
			if (!strcmp(newParamName->s, cur->c.car->s)){
				printf("Evaluation error: duplicate identifier in lambda.\n");
				texit(0);
			}
			cur = cdr(cur);
		}
		newClosure->cl.paramNames = cons(newParamName, newClosure->cl.paramNames);
		params = cdr(params);
	}
	args = cdr(args);
	if (args->type == NULL_TYPE){
		printf("Evaluation error: no code in lambda following parameters.\n");
		texit(0);
	}
	// Return a pointer to the body of the function
	newClosure->cl.functionCode = car(args);
	// Return A pointer to the frame that was active when the function was created
	newClosure->cl.frame = frame;

	args = cdr(args);
	if (args->type != NULL_TYPE){
		printf("Evaluation error: Expected form (lambda (x) body) but had something after.\n");
		texit(0);
	}
	return newClosure;
}

// evaluates each argument in a linked list
Value *evalEach(Value *args, Frame *frame){
	Value *listOfArgs = makeNull();
	while (args->type != NULL_TYPE){
		listOfArgs = cons(eval(car(args), frame), listOfArgs);
		args = cdr(args);
	}
	return listOfArgs;
}

// evaluates let*
Value *evalLetStar(Value *args, Frame *frame){
	if (args->type != CONS_TYPE){
		printf("Evaluation error: Found let statement did not find variable bindings => Should be in form (let ((x 1) ...) body).\n");
		texit(0);
	}

	Frame *prevFrame = frame;

	Value *curr = car(args);
	while (curr->type != NULL_TYPE) {
		if (curr->type != CONS_TYPE) {
			printf("Evaluation error: bad form in letstar.\n");
			texit(0);
		}
		Value *curr2 = car(curr);
		if (curr2->type ==  NULL_TYPE) {
			printf("Evaluation error: nothing in a letstar pair.\n");
			texit(0);
		} else if (curr2->type != CONS_TYPE) {
			printf("Evaluation error: bad form in letstar\n");
			texit(0);
		}
		Value *atom = car(curr2);
		if (atom->type != SYMBOL_TYPE) {
			printf("Evaluation error: letstar pair doesn't have a symbol lhs\n");
			texit(0);
		}
		if (cdr(curr2)->type == NULL_TYPE) {
			printf("Evaluation error: bad form in letstar\n");
			texit(0);
		}
		Frame *newFrame = talloc(sizeof(Frame));
		newFrame->parent = prevFrame;
		newFrame->bindings = makeNull();
		Value *evalAtom = eval(car(cdr(curr2)), prevFrame);
		Value *assignment = cons(atom, evalAtom);
		newFrame->bindings = cons(assignment, newFrame->bindings);
		prevFrame = newFrame;
		curr = cdr(curr);
	}
	// Evaluate body with newFrame
	args = cdr(args);
	if (args->type != CONS_TYPE) {
		printf("Evaluation error: nothing to evaluate in letstar\n");
		texit(0);
	}
	Value *result;
	while (cdr(args)->type != NULL_TYPE) {
		result = eval(car(args), prevFrame);
		args = cdr(args);
	}
	result = eval(car(args), prevFrame);
	return result;
}

// evaluates letrec
Value *evalLetrec(Value *args, Frame *frame) {
	Frame *newFrame = talloc(sizeof(Frame));
	newFrame->bindings = makeNull();
	newFrame->parent = frame;

	if (args->type != CONS_TYPE) {
		printf("Evaluation error: no args following letrec.\n");
		texit(0);
	}
	Value *curr = car(args);
	while (curr->type != NULL_TYPE) {
		if (curr->type != CONS_TYPE) {
			printf("Evaluation error: bad form in letrec.\n");
			texit(0);
		}
		Value *curr2 = car(curr);
		if (curr2->type ==  NULL_TYPE) {
			printf("Evaluation error: null binding in letrec.\n");
			texit(0);
		} else if (curr2->type !=  CONS_TYPE) {
			printf("Evaluation error: bad form in letrec\n");
			texit(0);
		}
		Value *atom = car(curr2);
		if (atom->type != SYMBOL_TYPE) {
			printf("Evaluation error: letrec pair doesn't have a symbol lhs\n");
			texit(0);
		}
		if (checkDuplicatesFrame(atom, newFrame)) {
			printf("Evaluation error: duplicate atom in let\n");
			texit(0);
		}
		if (curr2->c.cdr->type == NULL_TYPE){
			printf("Evaluation error: bad form in let\n");
			texit(0);
		}
		Value *rhs = car(cdr(curr2));
		atom->type = UNSPECIFIED_TYPE;
		Value *assignment = cons(atom, rhs);
		newFrame->bindings = cons(assignment, newFrame->bindings);

		curr = cdr(curr);
	}
	
	Value *c = newFrame->bindings;
	while (c->type != NULL_TYPE) {
		Value *temp = eval(cdr(car(c)), newFrame);
		c = cdr(c);
	}
	c = newFrame->bindings;
	while (c->type != NULL_TYPE) {
		car(c)->c.cdr = eval(cdr(car(c)), newFrame);
		car(car(c))->type = SYMBOL_TYPE;
		c = cdr(c);
	}
	// Evaluate body with newFrame
	args = cdr(args);
	if (args->type != CONS_TYPE){
		printf("Evaluation error: no args following the bindings in let.\n");
		texit(0);
	}
	Value *result;
	while (cdr(args)->type != NULL_TYPE) {
		result = eval(car(args), newFrame);
		args = cdr(args);
	}
	result = eval(car(args), newFrame);
	return result;
}

// evaluates set!
Value *evalSetExclamation(Value *args, Frame *frame){
	Frame *curFrame = frame;
	char *variable = car(args)->s;
	while (curFrame) {
		Value *curBinding = curFrame->bindings;
		while (curBinding->type != NULL_TYPE) {
			if (strcmp(curBinding->c.car->c.car->s, variable) == 0){
				curBinding->c.car->c.cdr = eval(car(cdr(args)), frame);
			}
			curBinding = cdr(curBinding);
		}
		curFrame = curFrame->parent;
	}
	Value *retvoid = talloc(sizeof(Value));
	retvoid->type = VOID_TYPE;
	return retvoid;
}

// evaluates begin
Value *evalBegin(Value *args, Frame *frame){
	Value *returnVal = talloc(sizeof(Value));
	if (args->type == NULL_TYPE){
		returnVal->type = VOID_TYPE;
		return returnVal;
	}
	while (args->type != NULL_TYPE) {
		returnVal = eval(car(args), frame);
		args = cdr(args);
	}
	return returnVal;
}

// evaluates and
Value *evalAnd(Value *args, Frame *frame){
	bool isFalse = false;
	while (args->type != NULL_TYPE) {
		Value *evalResult = eval(car(args), frame);
		if (evalResult->type == BOOL_TYPE){
			if (evalResult->i == 0){
				isFalse = true;
				break;
			}
		} else {
			isFalse = true;
			break;
		}
		args = cdr(args);
	}
	Value *result = talloc(sizeof(Value));
	result->type = BOOL_TYPE;
	if (isFalse == false){
		result->i = 1;
	} else {
		result->i = 0;
	}
	return result;
}

// evaluates or
Value *evalOr(Value *args, Frame *frame){
	bool isTrue = false;
	 while (args->type != NULL_TYPE) {
		Value *evalResult = eval(car(args), frame);
		if (evalResult->type == BOOL_TYPE){
			if (evalResult->i == 1){
				isTrue = true;
				break;
			}
		}
		args = cdr(args);
	}
	Value *result = talloc(sizeof(Value));
	result->type = BOOL_TYPE;
	if (isTrue == true){
		result->i = 1;
	} else {
		result->i = 0;
	}
	return result;
}

// evaluates cond
Value *evalCond(Value *args, Frame *frame){
	Value *toRet = talloc(sizeof(Value));
	toRet->type = VOID_TYPE;
	bool foundTrue = false;
	while (args->type != NULL_TYPE && foundTrue != true) {

		Value *each = car(car(args));
		if (each->type == SYMBOL_TYPE && strcmp(each->s, "else") == 0) {
			toRet = eval(car(cdr(car(args))), frame);
			foundTrue = true;
		} else {
			Value *result = eval(each, frame);
			if (result->i == 1 && foundTrue != true){
				toRet = eval(car(cdr(car(args))), frame);
				foundTrue = true;
			}
		}
		args = cdr(args);
	}
	return toRet;
}

// built in car
Value *builtInCar(Value *args)
{
	if (isNull(args))
	{
		printf("Evaluation error: no arguments supplied to car\n");
		texit(0);
	}

	if (!isNull(cdr(args)))
	{
		printf("Evaluation error: car takes one argument\n");
		texit(0);
	}
	Value *evaluated = car(args);
	if (evaluated->type != CONS_TYPE){
		printf("Evaluation error: car takes a pair\n");
		texit(0);
	}
	return car(evaluated);
}

// built in cdr
Value *builtInCdr(Value *args){
	if (isNull(args))
	{
		printf("Evaluation error: no arguments supplied to cdr\n");
		texit(0);
	}

	if (!isNull(cdr(args)))
	{
		printf("Evaluation error: cdr takes one argument\n");
		texit(0);
	}
	Value *evaluated = car(args);
	if (evaluated->type != CONS_TYPE){
		printf("Evaluation error: cdr takes a pair\n");
		texit(0);
	}
	return cdr(evaluated);
}

// built in +
Value *builtInAdd(Value *args) {
	double sum = 0;
	bool isDouble = false;
	while (args->type != NULL_TYPE){
		if (args->c.car->type == INT_TYPE){
			sum += args->c.car->i;
		} else if (args->c.car->type == DOUBLE_TYPE){
			sum += args->c.car->d;
			isDouble = true;
		} else {
			printf("Evaluation error: + must take numbers.\n");
			texit(0);
		}
		args = cdr(args);
	}
	Value *newNum = talloc(sizeof(Value));
	if (isDouble == false){
		newNum->type = INT_TYPE;
		newNum->i = (int) sum;
	} else {
		newNum->type = DOUBLE_TYPE;
		newNum->d = sum;
	}
	return newNum;
}

// built in null?
Value *builtInNullQuestionMark(Value *args){
	if (isNull(args)) {
		printf("Evaluation error: no arguments supplied to null?\n");
		texit(0);
	}
	if (!isNull(cdr(args))) {
		printf("Evaluation error: null? takes one argument\n");
		texit(0);
	}
	// check for only one args
	Value *evaluated = car(args);
	Value *result = talloc(sizeof(Value));
	result->type = BOOL_TYPE;
	if (isNull(evaluated)){
		result->i = 1;
	} else {
		result->i = 0;
	}
	return result;
}

// built in cons function
Value *builtInCons(Value *args) {
	if (isNull(args)){
		printf("Evaluation error: no arguments supplied to cons.\n");
		texit(0);
	}
	if (isNull(cdr(args))){
		printf("Evaluation error: Evaluation error: cons takes two arguments, only one supplied.\n");
		texit(0);
	}
	if (!isNull(cdr(cdr(args)))){
		printf("Evaluation error: cons takes two arguments, three or more supplied.\n");
		texit(0);
	}
	Value *firstArg = args->c.car;
	Value *secondArg = args->c.cdr->c.car;
	if (firstArg->type == CONS_TYPE){
		firstArg = cons(secondArg, firstArg);
		return firstArg;
	} else {
		return cons(secondArg, firstArg);
	}
}

// built in -
Value *builtInSubtract(Value *args){
	bool isDouble = false;
	if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE || cdr(cdr(args))->type != NULL_TYPE) {
		printf("Evaluation error: - needs exactly two arguments.\n");
		texit(0);
	}
	Value *firstArg = car(cdr(args));
	Value *secondArg = car(args);
	Value *result = talloc(sizeof(Value));
	if (firstArg->type == INT_TYPE && secondArg->type == INT_TYPE){
		result->type = INT_TYPE;
		result->i = firstArg->i - secondArg->i;
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == INT_TYPE){
		result->type = DOUBLE_TYPE;
		result->d = firstArg->d - secondArg->i;
	} else if (firstArg->type == INT_TYPE && secondArg->type == DOUBLE_TYPE){
		result->type = DOUBLE_TYPE;
		result->d = firstArg->i - secondArg->d;
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == DOUBLE_TYPE){
		result->type = DOUBLE_TYPE;
		result->d = firstArg->d - secondArg->d;
	} else {
		printf("Evaluation error: arguments for subtract are not numbers\n");
		texit(0);
	}
	return result;
}

// built in *
Value *builtInMultiply(Value *args){
	double product = 1;
	bool isDouble = false;
	if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE){
		printf("Evaluation error: * needs at least two arguments.\n");
		texit(0);
	}
	while (args->type != NULL_TYPE){
		if (car(args)->type == INT_TYPE){
			product *= car(args)->i;
		} else if (car(args)->type == DOUBLE_TYPE){
			product *= cdr(args)->d;
			isDouble = true;
		} else {
			printf("Evaluation error: arguments for mult are not numbers\n");
			texit(0);
		}
		args = cdr(args);
	}
	Value *result = talloc(sizeof(Value));
	if (isDouble == false){
		result->type = INT_TYPE;
		result->i = product;
	} else {
		result->type = DOUBLE_TYPE;
		result->d = product;
	}
	return result;
}

// built in /
Value *builtInDivide(Value *args){
	bool isDouble = false;
	if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE || cdr(cdr(args))->type != NULL_TYPE) {
		printf("Evaluation error: / needs exactly two arguments.\n");
		texit(0);
	}
	Value *firstArg = car(cdr(args));
	Value *secondArg = car(args);
	Value *result = talloc(sizeof(Value));
	if (firstArg->type == INT_TYPE && secondArg->type == INT_TYPE){
		if (firstArg->i % secondArg->i == 0){
			result->type = INT_TYPE;
			result->i = firstArg->i / secondArg->i;
		} else {
			result->type = DOUBLE_TYPE;
			result->d = (double) firstArg->i / (double) secondArg->i;
		}
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == INT_TYPE){
		result->type = DOUBLE_TYPE;
		result->d = firstArg->d / (double) secondArg->i;
	} else if (firstArg->type == INT_TYPE && secondArg->type == DOUBLE_TYPE){
		result->type = DOUBLE_TYPE;
		result->d = (double) firstArg->i / secondArg->d;
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == DOUBLE_TYPE){
		result->type = DOUBLE_TYPE;
		result->d = firstArg->d / secondArg->d;
	} else {
		printf("Evaluation error: arguments for divide are not numbers\n");
		texit(0);
	}
	return result;
}

// built in modulo, only works for integers
Value *builtInModulo(Value *args){
	if (args->type != CONS_TYPE || car(args)->type != INT_TYPE || cdr(args)->type != CONS_TYPE || car(cdr(args))->type != INT_TYPE || cdr(cdr(args))->type != NULL_TYPE){
		printf("Evaluation error: modulo needs exactly two integer arguments\n");
		texit(0);
	}
	Value *newNum = talloc(sizeof(Value));
	newNum->type = INT_TYPE;
	newNum->i = car(cdr(args))->i % car(args)->i;
	return newNum;
}

// built in <
Value *builtInLessThan(Value *args){
	if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE || cdr(cdr(args))->type != NULL_TYPE) {
		printf("Evaluation error: < needs exactly two arguments.\n");
		texit(0);
	}
	Value *firstArg = car(cdr(args));
	Value *secondArg = car(args);
	Value *result = talloc(sizeof(Value));
	result->type = BOOL_TYPE;
	if (firstArg->type == INT_TYPE && secondArg->type == INT_TYPE){
		if (firstArg->i < secondArg->i){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == INT_TYPE){
		if (firstArg->d < secondArg->i){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else if (firstArg->type == INT_TYPE && secondArg->type == DOUBLE_TYPE){
		if (firstArg->i < secondArg->d){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == DOUBLE_TYPE){
		if (firstArg->d < secondArg->d){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else {
		printf("Evaluation error: arguments for < are not numbers\n");
		texit(0);
	}
	return result;
}

// built in >
Value *builtInGreaterThan(Value *args){
	if (args->type != CONS_TYPE || cdr(args)->type != CONS_TYPE || cdr(cdr(args))->type != NULL_TYPE) {
		printf("Evaluation error: > needs exactly two arguments.\n");
		texit(0);
	}
	Value *firstArg = car(cdr(args));
	Value *secondArg = car(args);
	Value *result = talloc(sizeof(Value));
	result->type = BOOL_TYPE;
	if (firstArg->type == INT_TYPE && secondArg->type == INT_TYPE){
		if (firstArg->i > secondArg->i){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == INT_TYPE){
		if (firstArg->d > secondArg->i){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else if (firstArg->type == INT_TYPE && secondArg->type == DOUBLE_TYPE){
		if (firstArg->i > secondArg->d){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == DOUBLE_TYPE){
		if (firstArg->d > secondArg->d){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else {
		printf("Evaluation error: arguments for > are not numbers\n");
		texit(0);
	}
	return result;
}

// built in =
Value *builtInEquals(Value *args){
	if (args->type != CONS_TYPE || args->c.cdr->type != CONS_TYPE || args->c.cdr->c.cdr->type != NULL_TYPE){
		printf("Evaluation error: '=' must take two arguments.\n");
		texit(0);
	}
	Value *firstArg = car(cdr(args));
	Value *secondArg = car(args);
	Value *result = talloc(sizeof(Value));
	result->type = BOOL_TYPE;
	if (firstArg->type == INT_TYPE && secondArg->type == INT_TYPE){
		if (firstArg->i == secondArg->i){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == INT_TYPE){
		if (firstArg->d == secondArg->i){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else if (firstArg->type == INT_TYPE && secondArg->type == DOUBLE_TYPE){
		if (firstArg->i == secondArg->d){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else if (firstArg->type == DOUBLE_TYPE && secondArg->type == DOUBLE_TYPE){
		if (firstArg->d == secondArg->d){
			result->i = 1;
		} else {
			result->i = 0;
		}
	} else {
		printf("Evaluation error: arguments for = are not numbers\n");
		texit(0);
	}
	return result;
}

// Bind 'name' to 'function' in 'frame'
void bindPrimitiveFunction(char *name, Value *(*function)(struct Value *), Frame *frame) {
	 Value *value = talloc(sizeof(Value));
	 value->type = PRIMITIVE_TYPE;
	 value->pf = function;
	 Value *new_symbol = talloc(sizeof(Value));
	 new_symbol->type = SYMBOL_TYPE;
	 new_symbol->s = name;
	 frame->bindings = cons(cons(new_symbol, value), frame->bindings);
}

// executes lambda statement
Value *apply(Value *function, Value *args){
	Frame *newFrame = talloc(sizeof(Frame));
	newFrame->parent = function->cl.frame;
	newFrame->bindings = makeNull();
	if (function->type == CLOSURE_TYPE){
		Value *emptyParams = function->cl.paramNames;
		while (emptyParams->type != NULL_TYPE){
			Value *curr2 = cons(emptyParams->c.car, args->c.car);
			newFrame->bindings = cons(curr2, newFrame->bindings);
			emptyParams = cdr(emptyParams);
			args = cdr(args);
		}
		Value *returnVal = eval(function->cl.functionCode, newFrame);
		return returnVal;
	} else {
		return (function->pf)(args);
	}
}

// iterates through each s-expression to call eval on each of them
void interpret(Value *tree){
	Value *bTree = tree;

	// Create the global frame
	Frame *globalFrame = talloc(sizeof(Frame));
	globalFrame->bindings = makeNull();
	globalFrame->parent = NULL;

	// Create bindings in the global frame for all of
	// the built-in functions.
	bindPrimitiveFunction("car", &builtInCar, globalFrame);
	bindPrimitiveFunction("cdr", &builtInCdr, globalFrame);
	bindPrimitiveFunction("cons", &builtInCons, globalFrame);
	bindPrimitiveFunction("+", &builtInAdd, globalFrame);
	bindPrimitiveFunction("null?", &builtInNullQuestionMark, globalFrame);
	bindPrimitiveFunction("-", &builtInSubtract, globalFrame);
	bindPrimitiveFunction("*", &builtInMultiply, globalFrame);
	bindPrimitiveFunction("/", &builtInDivide, globalFrame);
	bindPrimitiveFunction("modulo", &builtInModulo, globalFrame);
	bindPrimitiveFunction("<", &builtInLessThan, globalFrame);
	bindPrimitiveFunction(">", &builtInGreaterThan, globalFrame);
	bindPrimitiveFunction("=", &builtInEquals, globalFrame);
	
	while (!isNull(bTree)){
		if (bTree->type != CONS_TYPE) {
			printf("Evaluation error: Expected CONS_TYPE for top-level S-expressions but recieved other type.\n");
			texit(0);
		}
		Value *xTree = car(bTree);
		Value *ans = eval(xTree, globalFrame);
		if (ans->type != VOID_TYPE)
		{
			print(ans);
		}

		bTree = cdr(bTree);
	}
}

// evaluates one s-expression
Value *eval(Value *tree, Frame *frame) {
	Value *returnVal;
	switch (tree->type)  {
		case SYMBOL_TYPE: {
			returnVal = lookUpSymbol(tree, frame);
			break;
		}
		case CONS_TYPE: {
			Value *first = car(tree);
			Value *args = cdr(tree);
			if (first->type == SYMBOL_TYPE || first->type == CONS_TYPE){
				if (!strcmp(first->s, "if")) {
					returnVal = evalIf(args, frame);
				} else if (!strcmp(first->s, "let")) {
					returnVal = evalLet(args, frame);
				} else if (!strcmp(first->s, "quote")){
					returnVal = evalQuote(args, frame);
				} else if (!strcmp(first->s, "define")){
					returnVal = evalDefine(args, frame);
				} else if (!strcmp(first->s, "lambda")){
					returnVal = evalLambda(args, frame);
				} else if (!strcmp(first->s, "let*")){
					returnVal = evalLetStar(args, frame);
				} else if (!strcmp(first->s, "letrec")){
					returnVal = evalLetrec(args, frame);
				} else if (!strcmp(first->s, "set!")){
					returnVal = evalSetExclamation(args, frame);
				} else if (!strcmp(first->s, "begin")){
					returnVal = evalBegin(args, frame);
				} else if (!strcmp(first->s, "and")){
					returnVal = evalAnd(args, frame);
				} else if (!strcmp(first->s, "or")){
					returnVal = evalOr(args, frame);
				} else if (!strcmp(first->s, "cond")){
					returnVal = evalCond(args, frame);
				} else {
					Value *evaluatedOperator = eval(first, frame);
					Value *evaluatedArgs = evalEach(args, frame);
					return apply(evaluatedOperator, evaluatedArgs);
				}
			} else if (first->type == INT_TYPE){
				returnVal = tree;
			}
			break;
		}
		default: {
			returnVal = tree;
			break;
		}
	}
	return returnVal;
}