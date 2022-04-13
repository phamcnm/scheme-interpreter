#ifndef _VALUE
#define _VALUE

typedef enum {
	INT_TYPE, DOUBLE_TYPE, STR_TYPE, CONS_TYPE, NULL_TYPE, PTR_TYPE,
	OPEN_TYPE, CLOSE_TYPE, BOOL_TYPE, SYMBOL_TYPE, VOID_TYPE, 

	UNSPECIFIED_TYPE,

	// to store users-defined functions
	CLOSURE_TYPE,

	// to store built-in functions
	PRIMITIVE_TYPE
	
	// Types below are only for bonus work (feel free to comment them out)
	// OPENBRACKET_TYPE, CLOSEBRACKET_TYPE, DOT_TYPE, SINGLEQUOTE_TYPE
} valueType;

struct Value {
	valueType type;
	union {
		int i;
		double d;
		char *s;
		void *p;

		//pf is a variable that points to a function that accepts a pointer to a Value and returns a pointer to a Value
		struct Value *(*pf)(struct Value *);

		struct ConsCell {
			struct Value *car;
			struct Value *cdr;
		} c;

		struct Closure {
			// List of parameter names
			struct Value *paramNames;
			// Tree for the body of the function
			struct Value *functionCode;
			// Active frame when function was defined
			struct Frame *frame;
		} cl;
	};
};

typedef struct Value Value;

// A frame contains a pointer to a parent frame and a linked list of bindings. A
// binding is a variable name (represented as a string) and a pointer to the Value 
// struct it is bound to.

struct Frame {
	struct Frame *parent;
	Value *bindings;
};

typedef struct Frame Frame;


#endif
