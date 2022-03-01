#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "linkedlist.h"
#include "talloc.h"

void printValue(Value *v) {
	switch (v->type) {
		case NULL_TYPE:
			printf("");
			break;
		case BOOL_TYPE:
			if (v->i == 1) {
				printf("#t");
			} else {
				printf("#f");
			}
			break;
		case INT_TYPE:
			printf("%i", v->i);
			break;
		case DOUBLE_TYPE:
			printf("%f", v->d);
			break;
		case STR_TYPE:
			printf("\"%s\"", v->s);
			break;
		case SYMBOL_TYPE:
			printf("%s", v->s);
			break;
		case PTR_TYPE:
			printf("");
			break;
		case CONS_TYPE:
			printf("");
			break;
		case OPEN_TYPE:
			printf("(");
			break;
		case CLOSE_TYPE:
			printf(")");
			break;
		case CLOSURE_TYPE:
			printf("#<procedure>");
			break;
		default:
			break;      
	}
}

void printType(Value *v) {
	switch (v->type) {
		case NULL_TYPE:
			printf(" -null- ");
			break;
		case BOOL_TYPE:
			printf(" -bool- ");
			break;
		case INT_TYPE:
			printf(" -int- ");
			break;
		case DOUBLE_TYPE:
			printf(" -double- ");
			break;
		case STR_TYPE:
			printf(" -string- ");
			break;
		case SYMBOL_TYPE:
			printf(" -symbol- ");
			break;
		case PTR_TYPE:
			printf(" -pointer- ");
			break;
		case CONS_TYPE:
			printf(" -cons- ");
			break;
		case OPEN_TYPE:
			printf(" -open- ");
			break;
		case CLOSE_TYPE:
			printf(" -close- ");
			break;
		default:
			break;
	}
}

Value *makeNull() {
	Value *v = talloc(sizeof(Value));
	v->type = NULL_TYPE;
	return v;
}

bool isNull(Value *value) {
	if (value->type != NULL_TYPE) {
		return false;
	} else {
		return true;
	}
}

Value *cons(Value *newCar, Value *newCdr) {
	Value *v = talloc(sizeof(Value));
	v->type = CONS_TYPE;
	v->c.car = newCar;
	v->c.cdr = newCdr;
	return v;
}

Value *car(Value *list) {
	assert(list->type != NULL_TYPE && "returning null pointer in car");
	return list->c.car;
}

Value *cdr(Value *list) {
	assert(list->type != NULL_TYPE && "returning null pointer in cdr");
	return list->c.cdr;
}

void display(Value *list) {
	Value *temp = list;
	while (isNull(temp) == 0) {
		if (car(temp)->type == BOOL_TYPE) {
			if (car(temp)->i == 0) {
				printf(" #f");
			} else {
				printf(" #t");
			}
		} else if (car(temp)->type == INT_TYPE) {
			printf(" %i", car(temp)->i);
		} else if (car(temp)->type == DOUBLE_TYPE) {
			printf(" %f", car(temp)->d);
		} else if (car(temp)->type == STR_TYPE) {
			printf(" \"%s\"", car(temp)->s);
		} else if (car(temp)->type == SYMBOL_TYPE) {
			printf(" %s", car(temp)->s);
		} else if (car(temp)->type == SYMBOL_TYPE) {
			printf(" #<procedure>");
		} 
		else if (car(temp)->type == CONS_TYPE) {
			printf(" (");
			display(car(temp));
			printf(")");
		} else {
			printf("()");
		}
		temp = cdr(temp);
	}
}

Value *reverse(Value *list) {
	Value *temp = list;
	Value *tail = makeNull();
	while (isNull(temp) == 0) {
		Value *v = talloc(sizeof(Value));
		v->type = car(temp)->type;
		switch (car(temp)->type) {
			case BOOL_TYPE:
				v->i = car(temp)->i;
				break;
			case INT_TYPE:
				v->i = car(temp)->i;
				break;
			case DOUBLE_TYPE:
				v->d = car(temp)->d;
				break;
			case STR_TYPE:
				v->s = car(temp)->s;
				break;
			case SYMBOL_TYPE:
				v->s = car(temp)->s;
				break;
			case PTR_TYPE:
				v->p = car(temp)->p;
				break;
			case CONS_TYPE:
				v->c = car(temp)->c;
				break;
			default:
				break;      
		}
		tail = cons(v, tail);
		temp = cdr(temp);
	}
	return tail;
}

int length(Value *value) {
	int length = 0;
	Value *temp = value;
	while (isNull(temp) == 0) {
		length++;
		temp = cdr(temp);
	}
	return length;
}
