#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct Val {
  enum { VAL_NUMBER, VAL_SYMBOL, VAL_CONS, VAL_NIL, VAL_CLOSURE, VAL_PRIMITIVE } type;
  union {
    double number;
    char *symbol;
    struct { struct Val *car, *cdr; };
    struct { struct Val *params, *body; struct Val *env; };
    struct { char *name; };
  };
} Val;

/* Constructors */
Val *make_number(double n);
Val *make_symbol(const char *s);
Val *make_cons(Val *car, Val *cdr);
Val *make_nil(void);
Val *make_closure(Val *params, Val *body, Val *env);
Val *make_primitive(const char *name);

/* Predicates */
int is_nil(Val *v);
int is_number(Val *v);
int is_symbol(Val *v);
int is_cons(Val *v);
int is_closure(Val *v);
int is_primitive(Val *v);
int is_true(Val *v);

/* Printer */
void print_val(Val *v);

/* Environment */
Val *make_env(Val *parent);
Val *lookup(Val *env, char *name);
void define(Val *env, char *name, Val *value);

/* Parser */
Val *read_val(const char *s);

/* Evaluator */
Val *eval(Val *expr, Val *env);

/* REPL */
void repl(void);