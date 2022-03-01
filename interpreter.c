#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "tokenizer.h"
#include "interpreter.h"

// checks if a list contains an entry
int contains(Value* list, Value *entry) {
	while(isNull(list) == 0) {
		if (car(list)->type == entry->type) {
			if (entry->type == BOOL_TYPE || entry->type == INT_TYPE) {
				if (car(list)->i == entry->i) {
					return 1;
				} else {
					return 0;
				}
			} else if (entry->type == STR_TYPE || entry->type == SYMBOL_TYPE) {
				if (strcmp(car(list)->s, entry->s) == 0) {
					return 1;   
				} else {
					return 0;
				}
			}   
		}
		list = cdr(list);
	}
	return 0;
}

// looks up if a symbol is in a frame
Value *lookUpSymbol(Value *args, Frame *frame, Frame **globalFrameAddress) {
	while (frame) {
		Value *curr = frame->bindings;
		while (isNull(curr) == 0) {
			Value *curr2 = car(curr);
			if (strcmp(car(curr2)->s, args->s) == 0) {
				if (cdr(curr2)->type == SYMBOL_TYPE) {
					return eval(cdr(curr2), frame->parent, globalFrameAddress);
				} else {
					return eval(cdr(curr2), frame, globalFrameAddress);
				}
			}
			curr = cdr(curr);
		}
		frame = frame->parent;
	}
	Value *curr = (*globalFrameAddress)->bindings;
	while (isNull(curr) == 0) {
		Value *curr2 = car(curr);
		if (strcmp(car(curr2)->s, args->s) == 0) {
			return eval(cdr(curr2), frame, globalFrameAddress);
		}
		curr = cdr(curr);
	}
	printf("Evaluation error: cannot find symbol\n");
	texit(0);
	return args;
}

// evaluates let statement
Value *evalLet(Value *list, Frame *par, Frame **globalFrameAddress) {
	Frame *frame = talloc(sizeof(Frame));
	frame->parent = par;
	Value *b = makeNull();
	Value *args;
	if (isNull(list) == 0) {
		args = car(list);
	} else {
		printf("Evaluation error: nothing after let\n");
		texit(0);
	}
	if (args->type != CONS_TYPE && args->type != NULL_TYPE) {
		printf("Evaluation error: bad form in let\n");
		texit(0);
	}
	Value *curr;
	Value *toadd;
	Value *seen = makeNull();

	while (isNull(args) == 0) {
		
		if (car(args)->type != CONS_TYPE) {
			printf("Evaluation error: bad form in let\n");
			texit(0);
		}
		curr = car(args);
		toadd = talloc(sizeof(Value));
		toadd->type = CONS_TYPE;
		if (isNull(curr) == 0) {
			if (car(curr)->type != SYMBOL_TYPE) {
				printf("Evaluation error: left side is not a symbol\n");
				texit(0);
			}
			if (contains(seen, car(curr)) == 1) {
				printf("Evaluation error: duplicate variable in let\n");
				texit(0);
			}
			seen = cons(car(curr), seen);
			toadd->c.car = car(curr);
			curr = cdr(curr);
			if (isNull(curr) == 0) {
				toadd->c.cdr = car(curr);
			} else {
				printf("Evaluation error: only 1 argument for let\n");
				texit(0);
			}
		} else {
			printf("Evaluation error: no arguments for let\n");
			texit(0);
		}
		b = cons(toadd, b);
		args = cdr(args);
	}
	frame->bindings = reverse(b);
	Value *e;
	if (isNull(cdr(list)) == 0) {
		e = cdr(list);  
	} else {
		printf("Evaluation error: nothing to evaluate for let\n");
		texit(0);
	}
	while (isNull(cdr(e)) == 0) {
		e = cdr(e);
	}
	return eval(car(e), frame, globalFrameAddress); 
}

// evaluates if statement
Value *evalIf(Value *args, Frame *frame, Frame **globalFrameAddress) {
	int foundTrue = 0;
	if (isNull(args) == 0) {
		if (car(args)->type == BOOL_TYPE) {
			if (car(args)->i == 1) {
				foundTrue = 1;
			}
		} else if (car(args)->type == SYMBOL_TYPE) {
			Value *result = lookUpSymbol(car(args), frame, globalFrameAddress);
			if (result->type == BOOL_TYPE) {
				if (result->i == 1) {
					foundTrue = 1;
				}
			}
		}
		args = cdr(args);
		if (isNull(args) == 0) {
			if (foundTrue) {
				return eval(car(args), frame, globalFrameAddress);
			} else {
				if (isNull(cdr(args)) == 0) {
					return eval(car(cdr(args)), frame, globalFrameAddress);
				} else {
					return makeNull();
				}
			}
		} else {
			printf("Evaluation error: if statement only 1 argument\n");
			texit(0);
		}
	} else {
		printf("Evaluation error: if statement has 0 arguments\n");
		texit(0);
	}
	return makeNull();
}

// evaluates quote statement
Value *evalQuote(Value *args) {
	if (isNull(args)) {
		printf("Evaluation error\n");
		texit(0);
	}
	return args;
}

// evaluates define statement
Value *evalDefine(Value *args, Frame **globalFrameAddress) {
	if (isNull(args)) {
		printf("Evaluation error: nothing follows define\n");
		texit(0);
	}
	if (isNull(cdr(args))) {
		printf("Evaluation error: only 1 arguments in define\n");
		texit(0);
	}
	if (car(args)->type != SYMBOL_TYPE) {
		printf("Evaluation error: define is not followed by a symbol\n");
		texit(0);
	}
	if (car(cdr(args))->type == SYMBOL_TYPE && !contains((*globalFrameAddress)->bindings, car(cdr(args)))) {
		printf("Evaluation error: symbol is not defined\n");
		texit(0);
	}
	Value *voidValue = talloc(sizeof(Value));
	voidValue->type = VOID_TYPE;

	Value *curr = (*globalFrameAddress)->bindings;
	while (isNull(curr) == 0) {
		Value *curr2 = car(curr);
		if (strcmp(car(curr2)->s, car(args)->s) == 0) {
			cdr(curr2)->s = car(cdr(args))->s;
			return voidValue;
		}
		curr = cdr(curr);
	}
	Value *toadd = talloc(sizeof(Value));
	toadd->type = CONS_TYPE;
	toadd->c.car = car(args);
	toadd->c.cdr = car(cdr(args));
	(*globalFrameAddress)->bindings = cons(toadd, (*globalFrameAddress)->bindings);
	return voidValue;
}

// evaluates lambda statement
Value *evalLambda(Value *args, Frame *frame) {
	Value *lambda = talloc(sizeof(Value));
	lambda->type = CLOSURE_TYPE;
	if (isNull(args)) {
		printf("Evaluation error: nothing follows lambda\n");
		texit(0);
	}
	Value *curr = car(args);
	Value *seen = makeNull();
	while (isNull(curr) == 0) {
		if (car(curr)->type != SYMBOL_TYPE) {
			printf("Evaluation error: params include non-symbols\n");
			texit(0);
		}
		if (contains(seen, car(curr))) {
			printf("Evaluation error: duplicates in params\n");
			texit(0);
		}
		seen = cons(car(curr), seen);
		curr = cdr(curr);
	}
	lambda->cl.paramNames = car(args);
	if (isNull(cdr(args))) {
		printf("Evaluation error: no code in lambda\n");
		texit(0);
	}
	lambda->cl.functionCode = cdr(args);
	lambda->cl.frame = frame;
	return lambda;
}

// executes lambda statement
Value *apply(Value *function, Value *args, Frame **globalFrameAddress) {
	Frame *newFrame = talloc(sizeof(Frame));
	newFrame->parent = function->cl.frame;
	newFrame->bindings = makeNull();
	Value *currParam = function->cl.paramNames;
	while (isNull(args) == 0) {
		Value *toadd = talloc(sizeof(Value));
		toadd->type = CONS_TYPE;
		toadd->c.car = car(currParam);
		toadd->c.cdr = car(args);
		newFrame->bindings = cons(toadd, newFrame->bindings);
		currParam = cdr(currParam);
		args = cdr(args);
	}
	return eval(car(function->cl.functionCode), newFrame, globalFrameAddress);
}

Value *builtInCar(Value *args, Frame *frame, Frame **globalFrameAddress) {
	if (isNull(args)) {
		printf("Evaluation error: no arguments for car\n");
		texit(0);
	}
	if (isNull(cdr(args)) == 0) {
		printf("Evaluation error: car takes in more than 1 argument\n");
		texit(0);
	}
	if (car(args)->type != CONS_TYPE) {
		printf("Evaluation error: argument is not a list\n");
		texit(0);
	}
	if (isNull(car(args))) {
		printf("Evaluation error: trying to car an empty list\n");
		texit(0);
	}
	return car(car(eval(car(args), frame, globalFrameAddress)));
}

Value *builtInCdr(Value *args, Frame *frame, Frame **globalFrameAddress) {
	if (isNull(args)) {
		printf("Evaluation error: no arguments for cdr\n");
		texit(0);
	}
	if (isNull(cdr(args)) == 0) {
		printf("Evaluation error: cdr takes in more than 1 argument\n");
		texit(0);
	}
	if (args->type != CONS_TYPE) {
		printf("Evaluation error: argument is not a list\n");
		texit(0);
	}
	if (isNull(car(args))) {
		printf("Evaluation error: trying to cdr an empty list\n");
		texit(0);
	}
	Value *result = talloc(sizeof(Value));
	result->type = CONS_TYPE;
	result->c.car = cdr(car(eval(car(args), frame, globalFrameAddress)));
	result->c.cdr = makeNull();
	if (car(result->c.car)->type == SYMBOL_TYPE && strcmp(car(result->c.car)->s, ".") == 0) {
		return cdr(result->c.car);
	}
	return result;
}

Value *builtInAdd(Value *args, Frame *frame, Frame **globalFrameAddress) {
	Value *result = talloc(sizeof(Value));
	int sawDouble = 0;
	double sum = 0;
	while (isNull(args) == 0) {
		if (car(args)->type == INT_TYPE) {
			sum = sum + car(args)->i;
		} else if (car(args)->type == DOUBLE_TYPE) {
			sum = sum + car(args)->d;
			sawDouble = 1;
		} else if (car(args)->type == SYMBOL_TYPE) {
			Value *c = lookUpSymbol(car(args), frame, globalFrameAddress);
			if (c->type == INT_TYPE) {
				sum = sum + c->i;
			} else if (c->type == DOUBLE_TYPE) {
				sum = sum + c->d;
				sawDouble = 1;
			} else {
				printf("Evaluation error: adding a non-number\n");
				texit(0);
			}
		} else {
			printf("Evaluation error: adding a non-number\n");
			texit(0);
		}
		args = cdr(args);
	}
	if (sawDouble == 1) {
		result->type = DOUBLE_TYPE;
		result->d = sum;
	} else {
		result->type = INT_TYPE;
		result->i = sum;
	}
	return result;
}

Value *builtInNull(Value *args, Frame *frame, Frame **globalFrameAddress) {
	Value *result = talloc(sizeof(Value));
	result->type = BOOL_TYPE;
	if (isNull(args)) {
		printf("Evaluation error: no arguments for null?\n");
		texit(0);
	}
	Value *v = eval(car(args), frame, globalFrameAddress); 
	if (isNull(v)) {
		result->i = 1;
	} else {
		if (v->type == CONS_TYPE && (isNull(car(v)))) {
			result->i = 1;
		} else {
			result->i = 0;
		}
	}
	if (isNull(cdr(args)) == 0) {
		printf("Evaluation error: car takes in more than 1 argument\n");
		texit(0);
	}
	return result;
}

Value *builtInCons(Value *args, Frame *frame, Frame **globalFrameAddress) {
	if (isNull(args)) {
		printf("Evaluation error: no arguments for cons\n");
		texit(0);
	}
	if (isNull(cdr(args))) {
		printf("Evaluation error: cons only has 1 argument\n");
		texit(0);
	}
	if (isNull(cdr(cdr(args))) == 0) {
		printf("Evaluation error: cons has more than 2 arguments\n");
		texit(0);
	}
	Value *result = talloc(sizeof(Value));
	result->type = CONS_TYPE;
	result->c.cdr = makeNull();
	if (car(cdr(args))->type != CONS_TYPE) {
		Value *next = eval(car(args), frame, globalFrameAddress);
		Value *dot = talloc(sizeof(Value));
		dot->type = SYMBOL_TYPE;
		dot->s = talloc(strlen(".") + 1);
		strcpy(dot->s, ".");
		Value *next2 = eval(car(cdr(args)), frame, globalFrameAddress);
		Value *end = makeNull();
		result->c.car = cons(next2, end);
		result->c.car = cons(dot, result->c.car);
		result->c.car = cons(next, result->c.car);
		return result;

	} else {
		Value *next = eval(car(cdr(args)), frame, globalFrameAddress);
		result->c.car = cons(car(args), car(next));
		return result;
	}
	return args;
}

// void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
// 	// Bind 'name' to 'function' in 'frame'
// 	Value *value = talloc(sizeof(Value));
// 	value->type = PRIMITIVE_TYPE;
// 	value->pf = function;
// 	frame->bindings = ...
// 	...
// }

// interprets the data
void interpret(Value *tree) {
	Frame *globalFrame = talloc(sizeof(Frame));
	globalFrame->bindings = makeNull();
	globalFrame->parent = NULL;
	// bind("car", &builtInCar, globalFrame);
	// bind("cons", &builtInCdr, globalFrame);
	while (isNull(tree) == 0) {
		Value *result = eval(car(tree), NULL, &globalFrame);
		if (result->type == CONS_TYPE) {
			if (car(result)->type != CONS_TYPE && isNull(cdr(result)) == 0) {
				printf("Evaluation error\n");
				texit(0);
			}
			display(result);
		} else {
			printValue(result);
		}       
		printf(" ");
		tree = cdr(tree);
	}
	printf("\n");
}

// evaluate an s-expression
Value *eval(Value *tree, Frame *frame, Frame **globalFrameAddress) {
	if (tree->type == INT_TYPE || tree->type == DOUBLE_TYPE || tree->type == BOOL_TYPE || tree->type == STR_TYPE) {
		return tree;
	} else if (tree->type == SYMBOL_TYPE) {
		return lookUpSymbol(tree, frame, globalFrameAddress);
	} else if (tree->type == CONS_TYPE) {
		Value *first = car(tree);
		Value *args = cdr(tree);
		if (strcmp(first->s, "if") == 0) {
			return evalIf(args, frame, globalFrameAddress);
		} else if (strcmp(first->s, "let") == 0) {
			return evalLet(args, frame, globalFrameAddress);
		} else if (strcmp(first->s, "quote") == 0) {
			return evalQuote(args);
		} else if (strcmp(first->s, "define") == 0) {
			return evalDefine(args, globalFrameAddress);
		} else if (strcmp(first->s, "lambda") == 0) {
			return evalLambda(args, frame);
		} else if (strcmp(first->s, "+") == 0) {
			return builtInAdd(args, frame, globalFrameAddress);
		} else if (strcmp(first->s, "car") == 0) {
			return builtInCar(args, frame, globalFrameAddress);
		} else if (strcmp(first->s, "cdr") == 0) {
			return builtInCdr(args, frame, globalFrameAddress);
		} else if (strcmp(first->s, "null?") == 0) {
			return builtInNull(args, frame, globalFrameAddress);
		} else if (strcmp(first->s, "cons") == 0) {
			return builtInCons(args, frame, globalFrameAddress);
		} else {
			Value *evaluatedOperator = eval(first, frame, globalFrameAddress);
			Value *evaluatedArgs = args;
			return apply(evaluatedOperator, evaluatedArgs, globalFrameAddress);
		}
	}
	return tree;
}
