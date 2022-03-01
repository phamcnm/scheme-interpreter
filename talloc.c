#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "value.h"
#include "talloc.h"

Value *g_active_list = NULL;

bool isNullTalloc(Value *value) {
	if (!value) {
		return true;
	}
	if (value->type != NULL_TYPE) {
		return false;
	} else {
		return true;
	}
}

Value *consTalloc(Value *newCar, Value *newCdr) {
	Value *v = malloc(sizeof(Value));
	v->type = CONS_TYPE;
	v->c.car = newCar;
	v->c.cdr = newCdr; 
	return v;
}

Value *carTalloc(Value *list) {
	return list->c.car;
}

Value *cdrTalloc(Value *list) {
	return list->c.cdr;
}

void *talloc(size_t size) {
	Value *v = malloc(sizeof(Value));
  	v->type = PTR_TYPE;
  	void *newPointer = malloc(size);
  	v->p = newPointer;
  	g_active_list = consTalloc(v, g_active_list);
  	return newPointer;
}

void tfree() {
	Value *temp = g_active_list;
	while (isNullTalloc(temp) == 0) {
		Value *tofree = temp;
		temp = cdrTalloc(temp);
		free(carTalloc(tofree)->p);
		free(carTalloc(tofree));
		free(tofree);
	}
	free(temp);
	g_active_list = NULL;

}

void texit(int status) {
	tfree();
	exit(0);
}
