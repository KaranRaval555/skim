#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <string.h>

typedef struct Expr Expr;

void print_expr(Expr *expr);

typedef enum {
  TOK_EOF,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_SYMBOL,
  TOK_NUMBER,
} TokenType;

typedef enum {
  EXPR_NIL,
  EXPR_SYMBOL,
  EXPR_NUMBER,
  EXPR_CONS
} ExprType;

typedef struct {
  TokenType type;
  union {
    double number;
    char *symbol;
  };
} Token;

typedef struct Expr {
  ExprType type;
  union {
    double number;
    char *symbol;
    struct {
      struct Expr *car;
      struct Expr *cdr;
    };
  };
} Expr;

#define is_nil(x) ((x)->type == EXPR_NIL)
#define is_num(x) ((x)->type == EXPR_NUMBER)
#define is_symbol(x) ((x)->type == EXPR_SYMBOL)
#define is_pair(x) ((x)->type == EXPR_CONS)
#define is_paren(x) ((x) == '(' || (x) == ')')

Expr *make_number(double x);
Expr *make_symbol(const char *s);
Expr *make_cons(Expr *car, Expr *cdr);
Expr *make_nil(void);

void print_expr(Expr *expr);


