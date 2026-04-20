#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Expr Expr;

void print_expr(Expr *expr);


typedef enum  {
  NIL, 
  SYMBOL,
  NUM,
  CONS,
  STRING,
  LPAREN,
  RPAREN,
  DONE
} Token_Type;

typedef struct {
  Token_Type type;
  union {
    double num;
    char *symbol;
  };
} Atom;

struct Expr {
  Token_Type type;
  union {
    double num;
    const char *symbol;
    struct {
      Expr *car;
      Expr *cdr;
    } pair;
  };
};


#define CAR(x) ((x)->pair.car)
#define CDR(x) ((x)->pair.cdr)
#define is_nil(x) ((x)->type == NIL)
#define is_num(x) ((x)->type == NUM)
#define is_symbol(x) ((x)->type == SYMBOL)
#define is_pair(x) ((x)->type == CONS)
#define is_paren(x) ((x) == '(' || (x) == ')')

void print_expr(Expr *expr);


